import test_serial
import time
import os

SERIAL_PORT = "Serial Port"  # Adjust this for your system
BAUD_RATE = 115200
SAVE_DIR = "Image_Dir"
os.makedirs(SAVE_DIR, exist_ok=True)

def receive_image():
    ser = test_serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    img_count = 1

    while True:
        line = ser.readline()
        try:
            line = line.decode().strip()
        except UnicodeDecodeError:
            print("Warning: Received non-text data, ignoring...")
            continue

        if line == "START":
            # Read the image size from the next 4 bytes
            img_size_bytes = ser.read(4)  
            if len(img_size_bytes) != 4:
                print("Error: Failed to read image size, skipping...")
                continue
            img_size = int.from_bytes(img_size_bytes, 'big')

            filename = os.path.join(SAVE_DIR, f"image{img_count}.png")
            with open(filename, "wb") as img_file:
                received = 0
                while received < img_size:
                    chunk = ser.read(min(32, img_size - received))
                    if not chunk:
                        break
                    img_file.write(chunk)
                    received += len(chunk)

            print(f"Saved {filename} ({img_size} bytes)")
            img_count += 1
        elif line == "END":
            print("Image transfer complete.")

receive_image()

