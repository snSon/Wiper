import torch  # PyTorch 라이브러리 임포트
import torch.nn as nn  # 신경망 관련 모듈 임포트
import torchvision  # 이미지 처리 관련 라이브러리 임포트
import torch.backends.cudnn as cudnn  # CUDA 최적화 모듈 임포트
import torch.optim  # 최적화 알고리즘 모듈 임포트
import os  # OS 관련 기능 제공 모듈
import sys  # 시스템 관련 기능 제공 모듈
import argparse  # 명령줄 인자 파싱을 위한 모듈
import time  # 시간 측정을 위한 모듈
import dataloader  # 사용자 정의 데이터 로더 모듈
import net  # 사용자 정의 네트워크 모듈
import numpy as np  # 수치 계산을 위한 라이브러리
from torchvision import transforms  # 이미지 변환 관련 기능 제공


def weights_init(m):
    """신경망 가중치 초기화 함수"""
    classname = m.__class__.__name__  # 클래스 이름 가져오기
    if classname.find('Conv') != -1:  # Convolution 레이어이면
        m.weight.data.normal_(0.0, 0.02)  # 가중치를 평균 0, 표준편차 0.02로 초기화
    elif classname.find('BatchNorm') != -1:  # BatchNorm 레이어이면
        m.weight.data.normal_(1.0, 0.02)  # 가중치를 평균 1, 표준편차 0.02로 초기화
        m.bias.data.fill_(0)  # 편향을 0으로 초기화


def train(config):
    """모델 학습을 위한 함수"""
    dehaze_net = net.dehaze_net().cuda()  # 네트워크 모델 생성 및 GPU로 이동
    dehaze_net.apply(weights_init)  # 가중치 초기화 적용

    # 학습 데이터 및 검증 데이터 로딩
    train_dataset = dataloader.dehazing_loader(config.orig_images_path, config.hazy_images_path)  
    val_dataset = dataloader.dehazing_loader(config.orig_images_path, config.hazy_images_path, mode="val")  
    
    train_loader = torch.utils.data.DataLoader(train_dataset, batch_size=config.train_batch_size, shuffle=True, num_workers=config.num_workers, pin_memory=True)
    val_loader = torch.utils.data.DataLoader(val_dataset, batch_size=config.val_batch_size, shuffle=True, num_workers=config.num_workers, pin_memory=True)
    
    criterion = nn.MSELoss().cuda()  # 손실 함수 정의 (Mean Squared Error Loss)
    optimizer = torch.optim.Adam(dehaze_net.parameters(), lr=config.lr, weight_decay=config.weight_decay)  # Adam 옵티마이저 정의
    
    dehaze_net.train()  # 모델을 학습 모드로 설정
    
    for epoch in range(config.num_epochs):  # 에포크 반복
        for iteration, (img_orig, img_haze) in enumerate(train_loader):  # 배치 단위 학습
            img_orig = img_orig.cuda()  # 원본 이미지 GPU로 이동
            img_haze = img_haze.cuda()  # 안개 낀 이미지 GPU로 이동

            clean_image = dehaze_net(img_haze)  # 네트워크를 통해 복원된 이미지 생성

            loss = criterion(clean_image, img_orig)  # 원본 이미지와 복원된 이미지 간의 손실 계산

            optimizer.zero_grad()  # 그래디언트 초기화
            loss.backward()  # 역전파 수행
            torch.nn.utils.clip_grad_norm(dehaze_net.parameters(), config.grad_clip_norm)  # 그래디언트 클리핑 적용
            optimizer.step()  # 가중치 업데이트

            if ((iteration+1) % config.display_iter) == 0:  # 일정 간격마다 손실 출력
                print("Loss at iteration", iteration+1, ":", loss.item())
            if ((iteration+1) % config.snapshot_iter) == 0:  # 일정 간격마다 모델 저장
                torch.save(dehaze_net.state_dict(), config.snapshots_folder + "Epoch" + str(epoch) + '.pth') 

        # 검증 과정 수행
        for iter_val, (img_orig, img_haze) in enumerate(val_loader):
            img_orig = img_orig.cuda()
            img_haze = img_haze.cuda()
            clean_image = dehaze_net(img_haze)  # 네트워크를 통해 복원된 이미지 생성

            # 원본, 안개 낀 이미지, 복원된 이미지를 하나로 합쳐 저장
            torchvision.utils.save_image(torch.cat((img_haze, clean_image, img_orig), 0), config.sample_output_folder + str(iter_val+1) + ".jpg")

        # 매 에포크마다 모델 저장
        torch.save(dehaze_net.state_dict(), config.snapshots_folder + "dehazer.pth") 


if __name__ == "__main__":
    parser = argparse.ArgumentParser()  # 명령줄 인자 파서를 생성
    
    # 입력 파라미터 설정
    parser.add_argument('--orig_images_path', type=str, default="data/images/")  # 원본 이미지 경로
    parser.add_argument('--hazy_images_path', type=str, default="data/data/")  # 안개 낀 이미지 경로
    parser.add_argument('--lr', type=float, default=0.0001)  # 학습률
    parser.add_argument('--weight_decay', type=float, default=0.0001)  # 가중치 감쇠
    parser.add_argument('--grad_clip_norm', type=float, default=0.1)  # 그래디언트 클리핑 한계값
    parser.add_argument('--num_epochs', type=int, default=10)  # 총 학습 에포크 수
    parser.add_argument('--train_batch_size', type=int, default=8)  # 학습 배치 크기
    parser.add_argument('--val_batch_size', type=int, default=8)  # 검증 배치 크기
    parser.add_argument('--num_workers', type=int, default=4)  # 데이터 로딩을 위한 워커 수
    parser.add_argument('--display_iter', type=int, default=10)  # 손실 출력 간격
    parser.add_argument('--snapshot_iter', type=int, default=200)  # 모델 저장 간격
    parser.add_argument('--snapshots_folder', type=str, default="snapshots/")  # 모델 저장 폴더
    parser.add_argument('--sample_output_folder', type=str, default="samples/")  # 샘플 출력 폴더

    config = parser.parse_args()  # 명령줄 인자 파싱
    
    # 스냅샷 폴더와 샘플 출력 폴더 생성
    if not os.path.exists(config.snapshots_folder):
        os.mkdir(config.snapshots_folder)
    if not os.path.exists(config.sample_output_folder):
        os.mkdir(config.sample_output_folder)
    
    train(config)  # 학습 함수 실행
