# dataset.py

import os
from PIL import Image
from torch.utils.data import Dataset

class RESIDEDataset(Dataset):
    def __init__(self, root_dir, transform=None):
        self.hazy_dir = os.path.join(root_dir, "hazy")
        self.clean_dir = os.path.join(root_dir, "clear")
        self.hazy_images = sorted(os.listdir(self.hazy_dir))
        self.clean_images = sorted(os.listdir(self.clean_dir))
        self.transform = transform

    def __len__(self):
        return len(self.hazy_images)

    def __getitem__(self, idx):
        hazy_path = os.path.join(self.hazy_dir, self.hazy_images[idx])
        clean_path = os.path.join(self.clean_dir, self.clean_images[idx])

        hazy = Image.open(hazy_path).convert("RGB")
        clean = Image.open(clean_path).convert("RGB")

        if self.transform:
            hazy = self.transform(hazy)
            clean = self.transform(clean)

        return hazy, clean
