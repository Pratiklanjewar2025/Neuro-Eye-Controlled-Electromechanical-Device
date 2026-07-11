import serial
import time
import pyautogui

# Set up the serial port
COM_PORT = 'COM5'  # Change to your COM port
BAUD_RATE = 9600    # Must match Arduino's baud rate

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for the connection to establish

    while True:
        # Read a line from the serial port
        line = ser.readline().decode('utf-8').strip()

        # Process the line only if it contains the blink count or command
        if "Blink Count:" in line:
            print(line)
        elif "Command:" in line:
            print(line)
            if "Move Right" in line:
                pyautogui.keyDown('d')
                time.sleep(1)  # You can adjust the duration the key is pressed
                pyautogui.keyUp('d')
                print("Pressed 'd' for Move Right")
            elif "Move Left" in line:
                pyautogui.keyDown('a')
                time.sleep(1)  # You can adjust the duration the key is pressed
                pyautogui.keyUp('a')
                print("Pressed 'a' for Move Left")

except serial.SerialException as e:
    print(f"Error: {e}")
except KeyboardInterrupt:
    print("Exiting...")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()  # Ensure the serial connection is closed on exit
