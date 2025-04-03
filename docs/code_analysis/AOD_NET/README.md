## AOD NET 구현 코드
[원작자](https://github.com/MayankSingal/PyTorch-Image-Dehazing)

**준비:**  
1. "data" 폴더를 생성합니다.  
2. 원본 작성자의 프로젝트 페이지(https://sites.google.com/site/boyilics/website-builder/project-page)에서 데이터셋을 다운로드하여 "data" 폴더에 압축을 풀어줍니다.  

**학습:**  
1. `train.py`를 실행합니다.  
   - 매 epoch마다 "samples" 폴더에 검증 결과 일부가 저장됩니다.  
   - 모델 스냅샷은 "snapshots" 폴더에 저장됩니다.  

**테스트:**  
1. `dehaze.py`를 실행합니다.  
   - "test_images" 폴더의 이미지를 처리하여 "results" 폴더에 복원된 이미지를 저장합니다.  
   - "snapshots" 폴더에 제공된 사전 학습된 모델을 사용합니다.  
