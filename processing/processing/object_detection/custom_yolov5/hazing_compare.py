import os
import numpy as np
import pandas as pd
from collections import defaultdict
from sklearn.metrics import average_precision_score

def compute_iou(box1, box2):
    x1_min = box1[0] - box1[2] / 2
    x1_max = box1[0] + box1[2] / 2
    y1_min = box1[1] - box1[3] / 2
    y1_max = box1[1] + box1[3] / 2

    x2_min = box2[0] - box2[2] / 2
    x2_max = box2[0] + box2[2] / 2
    y2_min = box2[1] - box2[3] / 2
    y2_max = box2[1] + box2[3] / 2

    inter_x_min = max(x1_min, x2_min)
    inter_y_min = max(y1_min, y2_min)
    inter_x_max = min(x1_max, x2_max)
    inter_y_max = min(y1_max, y2_max)

    inter_area = max(0, inter_x_max - inter_x_min) * max(0, inter_y_max - inter_y_min)
    area1 = (x1_max - x1_min) * (y1_max - y1_min)
    area2 = (x2_max - x2_min) * (y2_max - y2_min)
    union_area = area1 + area2 - inter_area
    return inter_area / union_area if union_area > 0 else 0

# 경로
gt_dir = "runs/test_detect/trt_engine_run13/labels"
pred_dir = "runs/test_detect/trt_engine_run16/labels"

gt_by_class = defaultdict(list)
pred_by_class = defaultdict(list)
conf_by_class = defaultdict(list)

for i in range(1, 932):
    fname_1 = f"test_drive_30_{i}.txt"
    fname_2 = f"dehazed_video_with_blended_center_{i}.txt"
    path_gt = os.path.join(gt_dir, fname_1)
    path_pred = os.path.join(pred_dir, fname_2)

    if not os.path.exists(path_gt) or not os.path.exists(path_pred):
        print(f"    - GT exists: {os.path.exists(path_gt)}")
        print(f"    - Pred exists: {os.path.exists(path_pred)}")
        continue

    try:
        gt = np.loadtxt(path_gt)
        pred = np.loadtxt(path_pred)

        if gt.ndim == 1: gt = np.expand_dims(gt, axis=0)
        if pred.ndim == 1: pred = np.expand_dims(pred, axis=0)

        gt_used = np.zeros(len(gt), dtype=bool)

        for p in pred:
            p_cls, px, py, pw, ph, p_conf = p
            best_iou, best_match = 0, -1
            for idx, g in enumerate(gt):
                g_cls, gx, gy, gw, gh, _ = g
                
                if gt_used[idx]: continue
                if p_cls != g_cls: continue
                
                iou = compute_iou([px, py, pw, ph], [gx, gy, gw, gh])
                if iou > best_iou:
                    best_iou, best_match = iou, idx
            if best_iou >= 0.5:
                gt_used[best_match] = True
                gt_by_class[int(p_cls)].append(1)
                pred_by_class[int(p_cls)].append(1)
                conf_by_class[int(p_cls)].append(p_conf)
            else:
                gt_by_class[int(p_cls)].append(0)
                pred_by_class[int(p_cls)].append(1)
                conf_by_class[int(p_cls)].append(p_conf)

        for idx, used in enumerate(gt_used):
            if not used:
                g_cls = int(gt[idx][0])
                gt_by_class[g_cls].append(1)
                pred_by_class[g_cls].append(0)
                conf_by_class[g_cls].append(0.0)

    except Exception as e:
        print(f"[!] Error on : {e}")

# 결과 계산
rows = []
for cls in sorted(gt_by_class.keys()):
    y_true = gt_by_class[cls]
    y_pred = conf_by_class[cls]
    try:
        ap = average_precision_score(y_true, y_pred)
    except:
        ap = 0.0
    tp = sum((np.array(y_true) == 1) & (np.array(pred_by_class[cls]) == 1))
    fp = sum((np.array(y_true) == 0) & (np.array(pred_by_class[cls]) == 1))
    fn = sum((np.array(y_true) == 1) & (np.array(pred_by_class[cls]) == 0))
    rows.append({
        "class_id": cls,
        "AP@0.5": round(ap, 4),
        "TP": tp,
        "FP": fp,
        "FN": fn
    })

df = pd.DataFrame(rows)
df.to_csv("dehazing_output/blended_roi_aod_dehazed_mAP_evaluation.csv", index=False)
print("[✔] mAP@0.5 평가 완료 → mAP_evaluation.csv 저장됨")
