import torch
import torch.nn as nn
import torch.nn.functional as F

# 기본 CNN 블록 (DehazeNet)
class ConvBlock(nn.Module):
    def __init__(self, in_channels, out_channels, kernel_size=3, stride=1):
        super(ConvBlock, self).__init__()
        self.conv = nn.Sequential(
            nn.Conv2d(in_channels, out_channels, kernel_size, stride, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True)
        )

    def forward(self, x):
        return self.conv(x)

# Channel Attention Block (FFA-Net)
class AttentionBlock(nn.Module):
    def __init__(self, channels):
        super(AttentionBlock, self).__init__()
        self.avg_pool = nn.AdaptiveAvgPool2d(1)
        self.fc = nn.Sequential(
            nn.Conv2d(channels, channels // 8, 1),
            nn.ReLU(inplace=True),
            nn.Conv2d(channels // 8, channels, 1),
            nn.Sigmoid()
        )

    def forward(self, x):
        w = self.avg_pool(x)
        w = self.fc(w)
        return x * w

# Multi-Scale Feature Extraction (MSBDN)
class MultiScaleBlock(nn.Module):
    def __init__(self, channels):
        super(MultiScaleBlock, self).__init__()
        self.down1 = nn.AvgPool2d(2, ceil_mode=True)
        self.conv1 = ConvBlock(channels, channels)

        self.down2 = nn.AvgPool2d(4, ceil_mode=True)
        self.conv2 = ConvBlock(channels, channels)

    def forward(self, x):
        h, w = x.shape[2], x.shape[3]

        x1 = self.conv1(self.down1(x))
        x1 = F.interpolate(x1, size=(h, w), mode='bilinear', align_corners=True)

        x2 = self.conv2(self.down2(x))
        x2 = F.interpolate(x2, size=(h, w), mode='bilinear', align_corners=True)

        return x + x1 + x2

# 전체 네트워크 구성 (AOD-Net의 end-to-end)
class JetDehazeNet(nn.Module):
    def __init__(self, in_channels=3, base_channels=32):
        super(JetDehazeNet, self).__init__()
        self.enc1 = ConvBlock(in_channels, base_channels)
        self.enc2 = ConvBlock(base_channels, base_channels * 2)

        self.multi_scale = MultiScaleBlock(base_channels * 2)
        self.attn = AttentionBlock(base_channels * 2)

        self.dec1 = ConvBlock(base_channels * 2, base_channels)
        self.out_conv = nn.Conv2d(base_channels, in_channels, kernel_size=1)

    def forward(self, x):
        x1 = self.enc1(x)
        x2 = self.enc2(x1)

        x2 = self.multi_scale(x2)
        x2 = self.attn(x2)

        x3 = self.dec1(x2)
        out = self.out_conv(x3)
        return torch.clamp(out + x, 0, 1)  # Residual connection (Restormer의 출력 방식)

