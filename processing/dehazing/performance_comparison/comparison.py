from psnr import psnr
from ssim import ssim
import cv2

test = cv2.imread('test.jpg')
result = cv2.imread('result.jpg')

if test.shape != result.shape:
    print("image size mismatch")

ssim_val, _ = ssim(test, result)
peaksnr, snr = psnr(test, result)

print(f"SSIM: {ssim_val:.4f}")
print(f"PSNR: {peaksnr:.2f} dB, SNR: {snr:.2f} dB")
