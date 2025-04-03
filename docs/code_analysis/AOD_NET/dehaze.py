import torch  # PyTorch의 핵심 라이브러리
import torch.nn as nn  # 신경망 모듈
import torchvision  # 이미지 처리 라이브러리
import torch.backends.cudnn as cudnn  # cuDNN 백엔드 설정
import torch.optim  # 최적화 알고리즘 제공
import os  # OS 관련 기능
import sys  # 시스템 관련 기능
import argparse  # 명령행 인자 파싱
import time  # 시간 측정
import dataloader  # 사용자 정의 데이터 로더 모듈
import net  # 디헤이징 네트워크 정의 모듈
import numpy as np  # 행렬 연산 라이브러리
from torchvision import transforms  # 데이터 변환 기능 제공
from PIL import Image  # 이미지 처리 라이브러리
import glob  # 파일 경로 검색 라이브러리


def dehaze_image(image_path):
    """주어진 이미지에 디헤이징(안개 제거) 네트워크를 적용"""
    data_hazy = Image.open(image_path)  # 이미지 로드
    data_hazy = (np.asarray(data_hazy) / 255.0)  # 0~1 범위로 정규화

    data_hazy = torch.from_numpy(data_hazy).float()
    data_hazy = data_hazy.permute(2, 0, 1)  # 채널 차원 변경 (HWC -> CHW)
    data_hazy = data_hazy.cuda().unsqueeze(0)  # 배치 차원 추가

    dehaze_net = net.dehaze_net().cuda()  # 네트워크 로드
    dehaze_net.load_state_dict(torch.load('snapshots/dehazer.pth'))  # 저장된 가중치 로드

    clean_image = dehaze_net(data_hazy)  # 디헤이징 수행
    torchvision.utils.save_image(torch.cat((data_hazy, clean_image), 0), "results/" + image_path.split("/")[-1])  # 결과 저장


if __name__ == '__main__':
    test_list = glob.glob("test_images/*")  # 테스트 이미지 리스트 불러오기

    for image in test_list:
        dehaze_image(image)  # 디헤이징 수행
        print(image, "done!")
