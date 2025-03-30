import test_serial
import time
import os

# Set the correct COM port (e.g., COM3 on Windows, /dev/ttyACM0 on Linux)
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
IMAGE_DIR = "/home/mmcdaniel/Spring 2025/Team Projects II/text"  # Change to your image folder path

def send_image(ser, image_path):
    with open(image_path, "rb") as img_file:
        image_data = img_file.read()

    print(f"Sending {os.path.basename(image_path)} ({len(image_data)} bytes)...")
    
    ser.write(len(image_data).to_bytes(4, 'big'))  # Send file size first
    print(len(image_data).to_bytes(4, 'big'))
    time.sleep(1)  

    chunk_size = 32
    for i in range(0, len(image_data), chunk_size):
        chunk = image_data[i:i+chunk_size]
        ser.write(chunk)  # Send the data in 32-byte chunks
        print(chunk)
        time.sleep(0.02)  

    print(f"{os.path.basename(image_path)} sent!")

def main():
    ser = test_serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  

    image_files = [f for f in os.listdir(IMAGE_DIR) if f.endswith(".txt")]
    if not image_files:
        print("No JPG images found in directory.")
        return

    for image in image_files:
        send_image(ser, os.path.join(IMAGE_DIR, image))
        time.sleep(2)  
    ser.close()
    print("All images sent!")

if __name__ == "__main__":
    main()
