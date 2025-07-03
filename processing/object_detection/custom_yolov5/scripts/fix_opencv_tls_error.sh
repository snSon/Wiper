#!/bin/bash

echo "removing existing opencv-python packages..."
pip uninstall -y opencv-python opencv-contrib-python

echo "Installing headless version..."
pip install opencv-python-headless

echo "All set! OpenCV has been replaced with the headless build."
