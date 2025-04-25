# spi.py (Jetson)
import spidev
import time

spi = spidev.SpiDev()
spi.open(0, 0)                # Bus 0, Device 0 (CS0)
spi.max_speed_hz = 1000000    # 1MHz
spi.mode = 0b00               # SPI mode 0

def send_jetson_signals(human, red_light, car):
    # 각 신호를 1바이트 안에 담아서 보내기 (예: 비트 조합)
    # 예: 0b00000101 → 사람 O, 신호등 X, 차량 O
    tx_byte = (int(human) << 2) | (int(red_light) << 1) | int(car)
    rx_data = spi.xfer2([tx_byte])
    print(f"[Jetson] TX: {bin(tx_byte)} | RX: {rx_data}")
    time.sleep(0.05)

if __name__ == "__main__":
    try:
        human = 0
        red_light = 0
        car = 0
        
        cnt = 0
        while True:
            send_jetson_signals(human, red_light, car)
            time.sleep(0.5)
            cnt += 1
            if(cnt == 10):
                human = 1

    except KeyboardInterrupt:
        spi.close()

