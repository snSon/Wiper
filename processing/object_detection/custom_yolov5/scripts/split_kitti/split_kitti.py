#!/usr/bin/env python3
"""
KITTI 데이터셋을 train:val:test = 8:1:1 비율로 분할하고,
train.txt, val.txt, test.txt 및 전체 파일명을 저장한 list.txt 생성
그리고 각 split별 이미지와 라벨 파일을 복사하는 스크립트

사용법:
    python kitti_split.py [--src_dir SRC] [--label_dir LABEL] [--dst_dir DST] [--ext EXT] [--seed SEED]

기본 경로:
  SRC: /workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti/images/
  LABEL: /workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti/labels/
  DST: /workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_split/
"""
import argparse
import os
import random
import shutil

def parse_args():
    parser = argparse.ArgumentParser(description="KITTI dataset split and copy script")
    parser.add_argument('--src_dir', type=str,
                        default='/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti/images/',
                        help='원본 이미지 경로 (images 디렉토리)')
    parser.add_argument('--label_dir', type=str,
                        default='/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti/labels/',
                        help='원본 라벨 경로 (labels 디렉토리)')
    parser.add_argument('--dst_dir', type=str,
                        default='/workspace/wiper/jiwan/Wiper/processing/object_detection/datasets/kitti_split/',
                        help='분할된 데이터셋 저장 경로')
    parser.add_argument('--ext', type=str, default='png', help='image format (default: png)')
    parser.add_argument('--seed', type=int, default=42, help='random seed setting (default: 42)')
    return parser.parse_args()

def main():
    args = parse_args()
    src = args.src_dir.rstrip('/')
    label_dir = args.label_dir.rstrip('/')
    dst = args.dst_dir.rstrip('/')
    ext = args.ext.lower()
    random.seed(args.seed)

    # 이미지 목록 수집
    imgs = [f for f in os.listdir(src) if f.lower().endswith('.' + ext)]
    imgs.sort()
    random.shuffle(imgs)

    total = len(imgs)
    if total == 0:
        print(f"ERROR: {src}에 '*.{ext}' 파일이 없습니다.")
        return

    # split 수 계산
    n_train = int(total * 0.8)
    n_val = int(total * 0.1)
    n_test = total - n_train - n_val

    splits = {
        'train': imgs[:n_train],
        'val': imgs[n_train:n_train + n_val],
        'test': imgs[n_train + n_val:]
    }

    # 디렉토리 생성
    os.makedirs(dst, exist_ok=True)
    for split in ['train', 'val', 'test']:
        os.makedirs(os.path.join(dst, 'images', split), exist_ok=True)
        os.makedirs(os.path.join(dst, 'labels', split), exist_ok=True)

    # split별 처리
    for name, files in splits.items():
        txt_path = os.path.join(dst, f'{name}.txt')
        with open(txt_path, 'w') as list_f:
            for img in files:
                src_img = os.path.join(src, img)
                src_label = os.path.join(label_dir, img.replace('.' + ext, '.txt'))
                dst_img = os.path.join(dst, 'images', name, img)
                dst_label = os.path.join(dst, 'labels', name, img.replace('.' + ext, '.txt'))

                # 복사
                shutil.copy2(src_img, dst_img)
                if os.path.exists(src_label):
                    shutil.copy2(src_label, dst_label)
                else:
                    print(f"WARNING: 라벨 파일이 없습니다: {src_label}")

                # 리스트에 경로 기록
                list_f.write(f"{dst_img} {dst_label}\n")

        print(f"[{name}] {len(files)}개 처리 완료 -> {txt_path}")

if __name__ == '__main__':
    main()
