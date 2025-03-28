![image01.png](https://global.discourse-cdn.com/nvidia/original/2X/6/6f589f87224371b47e04e736ee87bff1288ba7cc.png))

![img.png](https://blog.kakaocdn.net/dn/bqCM4H/btsI40omYje/H3rKQOTgH7g12j4ivHY9d1/img.png))

핀 연결 - jetson I2C 연결
|Jetson Nano|mcu|
|---|---|
|3.3V (전원)|VCC|
|GND|GND|
|I2C SDA (Pin 3)|SDA|
|I2C SCL (Pin 5)|SCL|

Jetson의 GPIO는 3.3V로 동작하므로, 5V 센서 사용 시 레벨 변환기 필수.

- jetson sensor
    
    레이더: SPI - SPI2 사용 → PB15(SCK), PB14(MISO), PB13(MOSI)
    LED: PD12
    조도/습도 센서: I2C (PB8, PB9와 병렬 연결)
    
- mcu sensor
DC 모터		PA8 (TIM1_CH1)			
초음파 센서	Trig: PD2, Echo: PD3		
가속도 센서	I2C1 (PB8, PB9)

pip install Jetson.GPIO  # GPIO 핀 제어 (LED, 기본 입출력)

pip install pyserial  # MCU와 시리얼 통신

pip install smbus2    # I2C 통신 (센서/모터 제어)

pip install spidev    # SPI 통신 (레이더 센서)

pip install Adafruit_GPIO  # I2C 기반 환경 센서 제어
