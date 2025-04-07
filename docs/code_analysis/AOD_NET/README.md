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



 test_images/test11.jpg
CPU usage:99.8% | RAM usage:3715.10MB
GPU allocated 2.76MB| Reserved 22.00MB
completed time: 6.27 sec
CPU usage:98.9% | RAM usage:5253.31MB
GPU allocated 42.76MB| Reserved 82.00MB
test_images/test11.jpg done!

 test_images/guogong.png
CPU usage:100.0% | RAM usage:5256.12MB
GPU allocated 2.76MB| Reserved 82.00MB
completed time: 0.39 sec
CPU usage:99.1% | RAM usage:5258.85MB
GPU allocated 42.76MB| Reserved 82.00MB
test_images/guogong.png done!

 test_images/test6.jpg
CPU usage:100.0% | RAM usage:5296.17MB
GPU allocated 20.13MB| Reserved 104.00MB
completed time: 0.54 sec
CPU usage:96.6% | RAM usage:5693.19MB
GPU allocated 301.83MB| Reserved 572.00MB
test_images/test6.jpg done!

 test_images/test13.jpg
CPU usage:100.0% | RAM usage:5693.19MB
GPU allocated 2.26MB| Reserved 572.00MB
completed time: 0.09 sec
CPU usage:97.2% | RAM usage:5693.19MB
GPU allocated 33.76MB| Reserved 572.00MB
test_images/test13.jpg done!

 test_images/test15.jpg
CPU usage:100.0% | RAM usage:5693.19MB
GPU allocated 2.05MB| Reserved 572.00MB
completed time: 0.08 sec
CPU usage:97.0% | RAM usage:5693.19MB
GPU allocated 30.66MB| Reserved 572.00MB
test_images/test15.jpg done!

 test_images/test14.jpg
CPU usage:100.0% | RAM usage:5693.19MB
GPU allocated 2.76MB| Reserved 572.00MB
completed time: 0.09 sec
CPU usage:94.3% | RAM usage:5693.19MB
GPU allocated 42.76MB| Reserved 572.00MB
test_images/test14.jpg done!

 test_images/indoor.jpg
CPU usage:100.0% | RAM usage:5693.19MB
GPU allocated 3.53MB| Reserved 572.00MB
completed time: 0.12 sec
CPU usage:94.5% | RAM usage:5693.20MB
GPU allocated 52.74MB| Reserved 572.00MB
test_images/indoor.jpg done!

 test_images/test8.jpg
CPU usage:96.3% | RAM usage:5693.11MB
GPU allocated 6.81MB| Reserved 572.00MB
completed time: 0.19 sec
CPU usage:93.3% | RAM usage:5692.80MB
GPU allocated 102.01MB| Reserved 572.00MB
test_images/test8.jpg done!

 test_images/test4.jpg
CPU usage:95.0% | RAM usage:5692.80MB
GPU allocated 2.07MB| Reserved 572.00MB
completed time: 0.09 sec
CPU usage:97.1% | RAM usage:5692.80MB
GPU allocated 30.91MB| Reserved 572.00MB
test_images/test4.jpg done!

 test_images/test.png
CPU usage:100.0% | RAM usage:5693.05MB
GPU allocated 3.10MB| Reserved 572.00MB
completed time: 0.50 sec
CPU usage:99.3% | RAM usage:5693.05MB
GPU allocated 46.36MB| Reserved 572.00MB
test_images/test.png done!

 test_images/test17.bmp
CPU usage:100.0% | RAM usage:5693.05MB
GPU allocated 3.16MB| Reserved 572.00MB
completed time: 0.11 sec
CPU usage:100.0% | RAM usage:5692.80MB
GPU allocated 47.21MB| Reserved 572.00MB
test_images/test17.bmp done!

 test_images/canyon.png
CPU usage:73.9% | RAM usage:5692.80MB
GPU allocated 5.07MB| Reserved 572.00MB
completed time: 0.58 sec
CPU usage:15.8% | RAM usage:5692.94MB
GPU allocated 75.95MB| Reserved 572.00MB
test_images/canyon.png done!

 test_images/test10.jpg
CPU usage:20.0% | RAM usage:5681.97MB
GPU allocated 18.02MB| Reserved 572.00MB
completed time: 0.49 sec
CPU usage:8.8% | RAM usage:5686.12MB
GPU allocated 270.12MB| Reserved 572.00MB
test_images/test10.jpg done!

 test_images/test9.jpg
CPU usage:21.7% | RAM usage:5625.48MB
GPU allocated 30.39MB| Reserved 572.00MB
completed time: 0.88 sec
CPU usage:30.2% | RAM usage:5943.25MB
GPU allocated 456.19MB| Reserved 1014.00MB
test_images/test9.jpg done!

 test_images/NYU2_102_7_2.jpg
CPU usage:23.5% | RAM usage:5943.59MB
GPU allocated 3.53MB| Reserved 1014.00MB
completed time: 0.10 sec
CPU usage:8.7% | RAM usage:5944.33MB
GPU allocated 52.74MB| Reserved 1014.00MB
test_images/NYU2_102_7_2.jpg done!

 test_images/test7.jpg
CPU usage:20.0% | RAM usage:5944.16MB
GPU allocated 6.27MB| Reserved 1014.00MB
completed time: 0.17 sec
CPU usage:13.8% | RAM usage:5943.81MB
GPU allocated 94.63MB| Reserved 1014.00MB
test_images/test7.jpg done!

 test_images/test2.jpg
CPU usage:24.1% | RAM usage:5943.64MB
GPU allocated 9.01MB| Reserved 1014.00MB
completed time: 0.23 sec
CPU usage:8.3% | RAM usage:5943.41MB
GPU allocated 135.01MB| Reserved 1014.00MB
test_images/test2.jpg done!

 test_images/NYU2_741_7_2.jpg
CPU usage:20.0% | RAM usage:5943.41MB
GPU allocated 3.53MB| Reserved 1014.00MB
completed time: 0.10 sec
CPU usage:11.4% | RAM usage:5943.61MB
GPU allocated 52.74MB| Reserved 1014.00MB
test_images/NYU2_741_7_2.jpg done!

 test_images/test3.jpg
CPU usage:28.6% | RAM usage:5943.45MB
GPU allocated 1.91MB| Reserved 1014.00MB
completed time: 0.07 sec
CPU usage:14.3% | RAM usage:5943.43MB
GPU allocated 28.51MB| Reserved 1014.00MB
test_images/test3.jpg done!

 test_images/test5.jpg
CPU usage:25.0% | RAM usage:5943.68MB
GPU allocated 3.53MB| Reserved 1014.00MB
completed time: 0.11 sec
CPU usage:12.2% | RAM usage:5943.55MB
GPU allocated 52.85MB| Reserved 1014.00MB
test_images/test5.jpg done!

 test_images/man.png
CPU usage:15.8% | RAM usage:5943.70MB
GPU allocated 2.90MB| Reserved 1014.00MB
completed time: 0.35 sec
CPU usage:16.5% | RAM usage:5944.04MB
GPU allocated 43.32MB| Reserved 1014.00MB
test_images/man.png done!

 test_images/test12.jpg
CPU usage:33.3% | RAM usage:5943.99MB
GPU allocated 2.76MB| Reserved 1014.00MB
completed time: 0.07 sec
CPU usage:12.9% | RAM usage:5944.15MB
GPU allocated 42.76MB| Reserved 1014.00MB
test_images/test12.jpg done!
