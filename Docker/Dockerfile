# Dockerfile
FROM nvcr.io/nvidia/l4t-pytorch:r35.2.1-pth2.0-py3

# 1) 시스템 의존성 설치
RUN apt-get update && apt-get install -y \
      libjpeg-dev libpng-dev git python3-pip \
      vim\    
     && rm -rf /var/lib/apt/lists/* \
&& apt-get remove -y python3-opencv libopencv* \
    && pip3 uninstall -y opencv-python opencv-python-headless opencv-contrib-python \
    && rm -rf /usr/lib/python3/dist-packages/cv2* /usr/local/lib/python3.8/dist-packages/cv2*

# 2) pip 최신화 및 추가 파이썬 패키지 설치
RUN pip3 install --upgrade pip setuptools wheel \
    && pip3 install numpy==1.23.5
RUN pip3 install opencv-python-headless


# 3) 작업 디렉토리 설정
WORKDIR /workspace/wiper

# 4) 로컬 코드(바인드 마운트) 설치를 위해 requirements만 복사
#    (추가 의존성이 requirements.txt 에 있다면 여기에)
COPY requirements.txt ./requirements.txt

# 5) 프로젝트 의존성 설치
RUN pip3 install --no-cache-dir -r requirements.txt

# 기본 쉘로 진입
ENTRYPOINT ["/bin/bash"]


