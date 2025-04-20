import spidev
import time

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1000000
spi.mode = 0b00

def send_jetson_signals(human, red_light, car):
    tx_data = [int(human), int(red_light), int(car)]
    rx_data = spi.xfer2(tx_data)
    print(f"TX → {tx_data} | RX ← {rx_data}")

if __name__ == "__main__":
    try:
        while True:
            send_jetson_signals(human=True, red_light=False, car=True)
            time.sleep(0.5)  # 충분한 간격 두기
    except KeyboardInterrupt:
        spi.close()

