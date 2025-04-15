# Wiper 🚗☔️

> A lightweight dehazing-based RC car project for enhancing autonomous driving in adverse weather conditions.

---

## 📌 Project Overview

**Wiper** is an autonomous RC car system designed to improve driving performance under degraded weather conditions like fog and rain.  
The project uses a dual-board architecture combining **STM32 (MCU)** and **NVIDIA Jetson**, integrating multiple sensors, motor control, and image-based object detection enhanced by a **lightweight AOD-Net** dehazing model.

---

## 🧠 Key Features

To be added

### 🔧 Hardware

- **Main Controller (MCU: STM32)**:
  - 3x Ultrasonic Sensors (data fusion for obstacle detection)
  - IMU (Inertial Measurement Unit)
  - Light sensor (for ambient light detection)
  - DC Motor Wheels (motor control)
  - Bluetooth module (communication)

- **Jetson Platform**:
  - Camera module for real-time video input
  - Lightweight AOD-Net for image dehazing
  - YOLOv5s for object detection after dehazing

### 💻 Software

- **MCU Side**: C (via STM32CubeIDE)
- **Jetson Side**: Python, OpenCV, PyTorch
- **Model**: AOD-Net (trained from scratch for this project)

---

## 🏗️ Project Structure

To be added