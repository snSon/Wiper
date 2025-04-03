import torch  # PyTorch 라이브러리
import torch.nn as nn  # 신경망 모듈 
import math

class dehaze_net(nn.Module):
    """이미지 디헤이징(안개 제거) 네트워크 정의"""
    def __init__(self):
        super(dehaze_net, self).__init__()

        self.relu = nn.ReLU(inplace=True)  # ReLU 활성화 함수 정의
        
        # 여러 크기의 컨볼루션 필터를 사용하여 특징을 추출
        self.e_conv1 = nn.Conv2d(3, 3, 1, 1, 0, bias=True)  # 1x1 컨볼루션
        self.e_conv2 = nn.Conv2d(3, 3, 3, 1, 1, bias=True)  # 3x3 컨볼루션
        self.e_conv3 = nn.Conv2d(6, 3, 5, 1, 2, bias=True)  # 5x5 컨볼루션
        self.e_conv4 = nn.Conv2d(6, 3, 7, 1, 3, bias=True)  # 7x7 컨볼루션
        self.e_conv5 = nn.Conv2d(12, 3, 3, 1, 1, bias=True)  # 3x3 컨볼루션
        
    def forward(self, x):
        """순전파 함수"""
        source = []
        source.append(x)

        x1 = self.relu(self.e_conv1(x))  # 첫 번째 컨볼루션 후 ReLU 적용
        x2 = self.relu(self.e_conv2(x1))  # 두 번째 컨볼루션 후 ReLU 적용

        concat1 = torch.cat((x1, x2), 1)  # 특징 맵 결합
        x3 = self.relu(self.e_conv3(concat1))  # 세 번째 컨볼루션 후 ReLU 적용

        concat2 = torch.cat((x2, x3), 1)  # 특징 맵 결합
        x4 = self.relu(self.e_conv4(concat2))  # 네 번째 컨볼루션 후 ReLU 적용

        concat3 = torch.cat((x1, x2, x3, x4), 1)  # 모든 특징 맵 결합
        x5 = self.relu(self.e_conv5(concat3))  # 다섯 번째 컨볼루션 후 ReLU 적용

        # 최종 디헤이징된 이미지 계산
        clean_image = self.relu((x5 * x) - x5 + 1) 
        
        return clean_image
