# dataset.py

import os
from PIL import Image
from torch.utils.data import Dataset

class RESIDEDataset(Dataset):
    def __init__(self, root_dir, transform=None):
        self.hazy_dir = os.path.join(root_dir, "hazy")
        self.clean_dir = os.path.join(root_dir, "clear")
        self.transform = transform

        self.hazy_images = sorted(os.listdir(self.hazy_dir))
        self.pairs = []

        for hazy_name in self.hazy_images:
            base_id = hazy_name.split('_')[0]
            clean_name = f"{base_id}.png"
            hazy_path = os.path.join(self.hazy_dir, hazy_name)
            clean_path = os.path.join(self.clean_dir, clean_name)
            if os.path.exists(clean_path):
                self.pairs.append((hazy_path, clean_path))

    def __len__(self):
        return len(self.pairs)

    def __getitem__(self, idx):
        hazy_path, clean_path = self.pairs[idx]

        hazy = Image.open(hazy_path).convert("RGB")
        clean = Image.open(clean_path).convert("RGB")

        if self.transform:
            hazy = self.transform(hazy)
            clean = self.transform(clean)

        return hazy, clean
