from pathlib import Path
import shutil
import sys

# 안개 이미지/라벨 원본 폴더
FOG_IMG_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog/images")
FOG_LAB_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog/labels")

# 기존 분리된 KITTI 이미지 폴더
SPLIT_IMG_DIR = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_split/images")

# 결과 저장할 폴더
OUT_IMG = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog_split/images")
OUT_LAB = Path("/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_fog_split/labels")

# train, val, test 디렉토리 생성 및 존재 확인
for subset in ["train", "val", "test"]:
    img_subdir = OUT_IMG / subset
    lab_subdir = OUT_LAB / subset
    img_subdir.mkdir(parents=True, exist_ok=True)
    lab_subdir.mkdir(parents=True, exist_ok=True)
    print(f"[DEBUG] Created directories: {img_subdir}, {lab_subdir}")

# train/val/test 디렉토리 순회하면서 복사
for subset in ["train", "val", "test"]:
    src_folder = SPLIT_IMG_DIR / subset
    if not src_folder.exists():
        print(f"ERROR: 원본 split 폴더가 없습니다: {src_folder}")
        sys.exit(1)
    imgs = sorted(src_folder.glob("*.*"))
    if not imgs:
        print(f"WARNING: 이미지 파일이 없습니다 in {src_folder}")
    for img_path in imgs:
        name = img_path.name
        # 이미지 복사
        src_img = FOG_IMG_DIR / name
        dst_img = OUT_IMG / subset / name
        if not src_img.exists():
            print(f"ERROR: fog 이미지 없음 → {src_img}")
            continue
        shutil.copy2(src_img, dst_img)
        # 라벨 복사
        lab_name = img_path.stem + ".txt"
        src_lab = FOG_LAB_DIR / lab_name
        dst_lab = OUT_LAB / subset / lab_name
        if src_lab.exists():
            shutil.copy2(src_lab, dst_lab)
        else:
            print(f"WARNING: fog 라벨 없음 → {src_lab}")

print("안개 버전 train/val/test 분리 완료!")
