# remote/Wiper/processing/dehazing/dehazing_pipeline.py
import torch
import numpy as np
import torchvision.transforms as transforms
import cv2
import os

### TensorRT 관련 라이브러리
import tensorrt as trt
import pycuda.driver as cuda
import pycuda.autoinit  # PyCUDA 초기화
from dehazing.JetDehaze.JetDehaze import JetDehazeNet

# 전역에 캐시용 변수 선언
_trt_engine = None
_trt_context = None

# 모델 및 설정 초기화 (1회만)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

def load_engine(engine_path):
    TRT_LOGGER = trt.Logger(trt.Logger.WARNING)
    with open(engine_path, "rb") as f, trt.Runtime(TRT_LOGGER) as runtime:
        return runtime.deserialize_cuda_engine(f.read())

# TensorRT를 사용한 Dehazing 적용 함수
def apply_dehazing_tensorrt(frame_bgr):
    global _trt_engine, _trt_context

    # 0. TensorRT 엔진 경로 설정
    engine_path = os.path.join(os.path.dirname(__file__), "JetDehaze", "JetDehaze.engine")

    # 1. 엔진 로드 (1회만)
    if _trt_engine is None:
        _trt_engine = load_engine(engine_path)
        _trt_context = _trt_engine.create_execution_context()
        
    engine = _trt_engine
    context = _trt_context        
            
    # 2. 원본 해상도 저장
    original_h, original_w = frame_bgr.shape[:2]

    # 3. 입력 이미지 리사이즈 (TensorRT 엔진 입력 크기와 일치해야 함)
    resized_bgr = cv2.resize(frame_bgr, (240, 320))  # (W, H)
    rgb = cv2.cvtColor(resized_bgr, cv2.COLOR_BGR2RGB).astype(np.float32) / 255.0
    chw = np.transpose(rgb, (2, 0, 1))  # HWC → CHW
    input_np = np.ascontiguousarray(np.expand_dims(chw, axis=0).astype(np.float32))

    # 4. 입력/출력 버퍼 준비
    d_input = cuda.mem_alloc(int(input_np.nbytes))
    output_shape = engine.get_binding_shape(1)  # (1, 3, 240, 320)
    d_output = cuda.mem_alloc(int(np.prod(output_shape)) * 4)

    # 5. 추론 실행
    try:
        cuda.memcpy_htod(d_input, input_np)
        context.execute_v2([int(d_input), int(d_output)])
        output_np = np.empty(output_shape, dtype=np.float32)
        cuda.memcpy_dtoh(output_np, d_output)
    except Exception as e:
        print(f"[Dehazing_Debug] TensorRT inference error: {e}")
        return None

    # 6. 후처리: CHW → HWC, Clip, Gamma 보정, Saturation 보정, RGB → BGR
    output_np = output_np.squeeze(0).transpose(1, 2, 0)
    output_np = np.clip(output_np, 0.0, 1.0)  # 안정화
    
     # (1) Gamma 보정 (밝기 조절)
    gamma = 1.5  # 1.2~2.2 사이 조절 가능
    output_np = np.power(output_np, 1.0 / gamma)

    # (2) float → uint8
    output_img = (output_np * 255).astype(np.uint8)

    # # (3) Saturation 조절 (HSV 기반)
    # hsv = cv2.cvtColor(output_img, cv2.COLOR_RGB2HSV)
    # h, s, v = cv2.split(hsv)
    # s = cv2.equalizeHist(s)  # 채도 히스토그램 평활화
    # hsv_eq = cv2.merge([h, s, v])
    # rgb_eq = cv2.cvtColor(hsv_eq, cv2.COLOR_HSV2RGB)
    
    # (3) Saturation 보정 (CLAHE 적용)
    hsv = cv2.cvtColor(output_img, cv2.COLOR_RGB2HSV)
    h, s, v = cv2.split(hsv)
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(4, 4))
    s_clahe = clahe.apply(s)
    hsv_eq = cv2.merge([h, s_clahe, v])
    rgb_eq = cv2.cvtColor(hsv_eq, cv2.COLOR_HSV2RGB)

    # (4) RGB → BGR, 원본 해상도 복원
    output_bgr = cv2.cvtColor(rgb_eq, cv2.COLOR_RGB2BGR)
    output_bgr = cv2.resize(output_bgr, (original_w, original_h))

    # 7. 최종 결과를 원본 해상도로 리사이즈
    return output_bgr