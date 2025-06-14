import torch
import torch.nn as nn


class AODNet(nn.Module):
    def __init__(self):
        super(AODNet, self).__init__()
        self.relu = nn.ReLU(inplace=True)

        self.e_conv1 = nn.Conv2d(3, 3, 1, 1, 0, bias=True)
        self.e_conv2 = nn.Conv2d(3, 3, 3, 1, 1, bias=True)
        self.e_conv3 = nn.Conv2d(6, 3, 5, 1, 2, bias=True)
        self.e_conv4 = nn.Conv2d(6, 3, 7, 1, 3, bias=True)
        self.e_conv5 = nn.Conv2d(12, 3, 3, 1, 1, bias=True)

    def forward(self, x):
        x1 = self.relu(self.e_conv1(x))
        x2 = self.relu(self.e_conv2(x1))

        x3 = self.relu(self.e_conv3(torch.cat((x1, x2), 1)))
        x4 = self.relu(self.e_conv4(torch.cat((x2, x3), 1)))
        x5 = self.relu(self.e_conv5(torch.cat((x1, x2, x3, x4), 1)))

        return self.relu((x5 * x) - x5 + 1)
