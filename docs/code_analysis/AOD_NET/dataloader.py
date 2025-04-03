import os  # OS 관련 기능
import sys  # 시스템 관련 기능

import torch  # PyTorch의 핵심 라이브러리
import torch.utils.data as data  # 데이터 로딩 관련 유틸리티

import numpy as np  # 행렬 연산 라이브러리
from PIL import Image  # 이미지 처리 라이브러리
import glob  # 파일 경로 검색 라이브러리
import random  # 무작위 선택 기능
import cv2  # OpenCV 라이브러리 (사용되지 않지만 향후 활용 가능)

random.seed(1143)  # 난수 시드 설정


def populate_train_list(orig_images_path, hazy_images_path):
    """훈련 및 검증 데이터셋 리스트를 생성"""
    train_list = []
    val_list = []
    
    image_list_haze = glob.glob(hazy_images_path + "*.jpg")  # 흐린 이미지 파일 목록 가져오기

    tmp_dict = {}

    for image in image_list_haze:
        image = image.split("/")[-1]  # 파일명 추출
        key = image.split("_")[0] + "_" + image.split("_")[1] + ".jpg"  # 원본 이미지 키 생성
        if key in tmp_dict.keys():
            tmp_dict[key].append(image)
        else:
            tmp_dict[key] = [image]

    train_keys = []
    val_keys = []

    len_keys = len(tmp_dict.keys())
    for i in range(len_keys):
        if i < len_keys * 9 / 10:  # 90%는 훈련 데이터
            train_keys.append(list(tmp_dict.keys())[i])
        else:  # 10%는 검증 데이터
            val_keys.append(list(tmp_dict.keys())[i])

    for key in list(tmp_dict.keys()):
        if key in train_keys:
            for hazy_image in tmp_dict[key]:
                train_list.append([orig_images_path + key, hazy_images_path + hazy_image])
        else:
            for hazy_image in tmp_dict[key]:
                val_list.append([orig_images_path + key, hazy_images_path + hazy_image])

    random.shuffle(train_list)  # 훈련 데이터 섞기
    random.shuffle(val_list)  # 검증 데이터 섞기

    return train_list, val_list


class dehazing_loader(data.Dataset):
    """디헤이징 데이터 로더 클래스"""
    def __init__(self, orig_images_path, hazy_images_path, mode='train'):
        self.train_list, self.val_list = populate_train_list(orig_images_path, hazy_images_path) 

        if mode == 'train':
            self.data_list = self.train_list
            print("Total training examples:", len(self.train_list))
        else:
            self.data_list = self.val_list
            print("Total validation examples:", len(self.val_list))

    def __getitem__(self, index):
        """데이터셋에서 하나의 샘플을 가져옴"""
        data_orig_path, data_hazy_path = self.data_list[index]

        data_orig = Image.open(data_orig_path)  # 원본 이미지 로드
        data_hazy = Image.open(data_hazy_path)  # 흐린 이미지 로드

        data_orig = data_orig.resize((480, 640), Image.ANTIALIAS)  # 이미지 크기 조정
        data_hazy = data_hazy.resize((480, 640), Image.ANTIALIAS)

        data_orig = (np.asarray(data_orig) / 255.0)  # 0~1 정규화
        data_hazy = (np.asarray(data_hazy) / 255.0)

        data_orig = torch.from_numpy(data_orig).float()  # NumPy 배열을 Tensor로 변환
        data_hazy = torch.from_numpy(data_hazy).float()

        return data_orig.permute(2, 0, 1), data_hazy.permute(2, 0, 1)  # (HWC -> CHW 변환)

    def __len__(self):
        """데이터셋 크기 반환"""
        return len(self.data_list)
