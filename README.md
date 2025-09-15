# ECG-Signal-Processing

This project focuses on **ECG (Electrocardiogram) signal processing and feature extraction** using STM32-based embedded systems.  
The goal is to acquire, process, and display ECG signals in real time while extracting clinically relevant features.

## 🖥️ System Architecture
The developed architecture employs two STM32 boards:

- **STM32F469I-DISCO**: Acts as the **master controller** and provides a **graphical user interface (GUI)** for visualization.  
- **STM32H7**: Dedicated to **ECG signal processing** tasks.  

Communication between the two modules is implemented via **SPI Protocol**, ensuring fast and efficient data transfer.

## ⚙️ Key Features
- Real-time ECG signal acquisition and visualization  
- **Pan–Tompkins algorithm** for detecting **QRS complexes**  
- Feature extraction:
  - Heart Rate (BPM)
  - PR, RR, and QT intervals  
- **TouchGFX-based GUI** for user interaction  
- **FreeRTOS** for task scheduling, managing SPI communication, and handling display tasks without blocking states  

## 📖 Detailed Documentation
The complete mathematical derivations, STM32CubeIDE setup, and CubeMonitor test results  
are provided in the documentation: 
[ECG_Signal.pdf](https://github.com/user-attachments/files/22311942/ECG_Signal.pdf)

## 📊 Demo
🎥 Watch the real-time ECG demo here: [Video Link](https://screenapp.io/app/#/shared/dbaA_IyuB5)
