# 이미지 리스트 셔플 → 80%는 train, 20%는 val
# images → datasets/kitti_split/images/{train,val}
# labels → datasets/kitti_split/labels/{train,val}

import random
from pathlib import Path
import shutil

# 원본 KITTI 데이터셋 경로
BASE_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti")
IMG_DIR = BASE_DIR / "images"
LAB_DIR = BASE_DIR / "labels"

# 분할된 데이터 저장 경로
OUT_BASE = BASE_DIR.parent / "kitti_split"
OUT_IMG = OUT_BASE / "images"
OUT_LAB = OUT_BASE / "labels"

# 분할 비율 설정
TRAIN_RATIO = 0.8  # 80% for train, 20% for val

IMG_EXT = ".png"  # 또는 ".jpg"

# 폴더 생성
for sub in ["train", "val"]:
    (OUT_IMG / sub).mkdir(parents=True, exist_ok=True)
    (OUT_LAB / sub).mkdir(parents=True, exist_ok=True)

# 이미지 파일 리스트 로드 및 셔플
all_imgs = sorted(IMG_DIR.glob(f"*{IMG_EXT}"))
random.seed(42)
random.shuffle(all_imgs)

# 분할 인덱스 계산
split_idx = int(len(all_imgs) * TRAIN_RATIO)
train_imgs = all_imgs[:split_idx]
val_imgs = all_imgs[split_idx:]

# 파일 복사 함수
def copy_subset(img_list, subset_name):
    for img_path in img_list:
        # 이미지 복사
        dest_img = OUT_IMG / subset_name / img_path.name
        shutil.copy(img_path, dest_img)
        # 라벨 파일 이름
        lab_name = img_path.stem + ".txt"
        src_lab = LAB_DIR / lab_name
        dest_lab = OUT_LAB / subset_name / lab_name
        # 라벨 복사
        if src_lab.exists():
            shutil.copy(src_lab, dest_lab)
        else:
            print(f"Warning: Label not found for {img_path.name}")

# train/val 복사 실행
print(f"Copying {len(train_imgs)} images to train, {len(val_imgs)} images to val...")
copy_subset(train_imgs, "train")
copy_subset(val_imgs, "val")

print("Dataset split complete.")
