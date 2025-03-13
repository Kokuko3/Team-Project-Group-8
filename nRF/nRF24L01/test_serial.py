import serial


try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    while True:
        if ser.in_waiting > 0:
            data = ser.readline()
            with open("log.txt", "w") as file:
                file.write(data)
except serial.SerialException as e:
    print(f"Error: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
