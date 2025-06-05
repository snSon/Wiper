import torch
import os
from JetDehaze import JetDehazeNet  # 네가 준 클래스

print("현재 경로:", os.getcwd())
print("JetDehaze.pth 존재 여부:", os.path.exists("JetDehaze.pth"))

# 1. 모델 로드
model = JetDehazeNet()
model.load_state_dict(torch.load("JetDehaze.pth", map_location="cpu"))
model.eval()

# 2. 입력 샘플 정의
dummy_input = torch.randn(1, 3, 256, 256)  # 입력 크기 맞게 수정 가능

# 3. ONNX export
torch.onnx.export(
    model,
    dummy_input,
    "JetDehaze.onnx",
    input_names=["input"],
    output_names=["output"],
    opset_version=11,  # 보통 11~17 무난
    dynamic_axes={"input": {0: "batch_size"}, "output": {0: "batch_size"}}
)

print("✅ JetDehazeNet → ONNX 변환 완료: JetDehaze.onnx")