import serial
import time
import os

# Set the correct COM port (e.g., COM3 on Windows, /dev/ttyUSB0 on Linux)
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200
IMAGE_DIR = "/home/mmcdaniel/Spring 2025/Team Projects II/Images"  # Change to your image folder path

def send_image(ser, image_path):
    with open(image_path, "rb") as img_file:
        image_data = img_file.read()

    print(f"Sending {os.path.basename(image_path)} ({len(image_data)} bytes)...")
    
    ser.write(len(image_data).to_bytes(4, 'big'))  # Send file size first
    time.sleep(1)  # Small delay

    chunk_size = 32
    for i in range(0, len(image_data), chunk_size):
        chunk = image_data[i:i+chunk_size]
        ser.write(chunk)  # Send the data in 32-byte chunks
        time.sleep(0.02)  # Allow Arduino to process

    print(f"{os.path.basename(image_path)} sent!")

def main():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for Arduino to initialize

    image_files = [f for f in os.listdir(IMAGE_DIR) if f.endswith(".png")]  # List all PNG files
    if not image_files:
        print("No PNG images found in directory.")
        return

    for image in image_files:
        send_image(ser, os.path.join(IMAGE_DIR, image))
        time.sleep(2)  # Wait before sending the next image

    ser.close()
    print("All images sent!")

if __name__ == "__main__":
    main()
