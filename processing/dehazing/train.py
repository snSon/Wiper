# train.py

import os
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from torchvision import transforms
from tqdm import tqdm
from model import JetDehazeNet
from dataset import RESIDEDataset

# 설정
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
batch_size = 8
num_epochs = 50
learning_rate = 1e-4
train_data_dir = "./RESIDE/ITS/train"
val_data_dir = "./RESIDE/ITS/val"
save_model_path = "./checkpoints"

os.makedirs(save_model_path, exist_ok=True)

# 데이터 전처리
transform = transforms.Compose([
    transforms.ToTensor()
])

# 데이터셋 및 데이터로더
train_dataset = RESIDEDataset(root_dir=train_data_dir, transform=transform)
val_dataset = RESIDEDataset(root_dir=val_data_dir, transform=transform)

train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True, num_workers=4)
val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False, num_workers=4)

# 모델, 손실 함수, 옵티마이저
model = JetDehazeNet().to(device)
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=learning_rate)

# 학습 루프
for epoch in range(num_epochs):
    model.train()
    running_loss = 0.0
    loop = tqdm(train_loader, desc=f"[Train] Epoch [{epoch+1}/{num_epochs}]")
    for hazy_img, clean_img in loop:
        hazy_img, clean_img = hazy_img.to(device), clean_img.to(device)

        optimizer.zero_grad()
        output = model(hazy_img)
        loss = criterion(output, clean_img)
        loss.backward()
        optimizer.step()

        running_loss += loss.item()
        loop.set_postfix(loss=loss.item())

    avg_train_loss = running_loss / len(train_loader)
    print(f"Epoch [{epoch+1}/{num_epochs}] Training Loss: {avg_train_loss:.4f}")

    # 검증 루프
    model.eval()
    val_loss = 0.0
    with torch.no_grad():
        for hazy_img, clean_img in val_loader:
            hazy_img, clean_img = hazy_img.to(device), clean_img.to(device)
            output = model(hazy_img)
            loss = criterion(output, clean_img)
            val_loss += loss.item()

    avg_val_loss = val_loss / len(val_loader)
    print(f"Epoch [{epoch+1}/{num_epochs}] Validation Loss: {avg_val_loss:.4f}")

    # 모델 저장
    torch.save(model.state_dict(), os.path.join(save_model_path, f"jetDehaze_epoch_{epoch+1}.pth"))

print("Training completed.")
