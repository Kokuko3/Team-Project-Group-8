import serial
import time
import os

SERIAL_PORT = "/dev/ttyUSB0"  # Adjust this based on your system
BAUD_RATE = 115200
SAVE_DIR = "/home/mmcdaniel/Spring 2025/Team Projects II/received_images"
os.makedirs(SAVE_DIR, exist_ok=True)

def receive_image():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Allow time for connection
    img_count = 1

    while True:
        line = ser.readline().decode().strip()  # Read text commands
        if line == "START":
            img_size = int.from_bytes(ser.read(4), 'big')  # Read image size
            filename = os.path.join(SAVE_DIR, f"image{img_count}.png")
            with open(filename, "wb") as img_file:
                received = 0
                while received < img_size:
                    chunk = ser.read(min(32, img_size - received))
                    img_file.write(chunk)
                    received += len(chunk)
            print(f"Saved {filename} ({img_size} bytes)")
            img_count += 1
        elif line == "END":
            print("Image transfer complete.")

receive_image()