from pathlib import Path
import shutil

# 안개 이미지/라벨 원본 폴더
FOG_IMG_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/foggy_01_kitti/images")
FOG_LAB_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/foggy_01_kitti/labels")

# 기존 분리된 폴더
SPLIT_IMG_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_split/images")
# 결과 저장할 폴더
OUT_IMG = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog_split/images")
OUT_LAB = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog_split/labels")

# 폴더 생성
for sub in ["train", "val"]:
    (OUT_IMG/sub).mkdir(parents=True, exist_ok=True)
    (OUT_LAB/sub).mkdir(parents=True, exist_ok=True)

# train/val 디렉토리 순회하면서 복사
for subset in ["train", "val"]:
    src_imgs = sorted((SPLIT_IMG_DIR/subset).glob("*.*"))
    for img_path in src_imgs:
        name = img_path.name
        # 이미지 복사
        shutil.copy(FOG_IMG_DIR/name, OUT_IMG/subset/name)
        # 라벨 복사
        lab_name = img_path.stem + ".txt"
        src_lab = FOG_LAB_DIR/lab_name
        if src_lab.exists():
            shutil.copy(src_lab, OUT_LAB/subset/lab_name)
        else:
            print(f"Warning: fog 라벨 없음 → {lab_name}")

print("안개 버전 train/val 분리 완료!")

