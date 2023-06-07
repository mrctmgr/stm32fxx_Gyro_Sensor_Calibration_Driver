import serial


ser = serial.Serial('COM3', 115200, timeout=1)

def send_data_from_file(file_path):
    with open(file_path, 'r') as file:
        data = file.read().strip()
        ser.write(data.encode())
        print("Transmit Data:", data)

while True:
    file_path = ("C:\\Users\\batma\\OneDrive\\Belgeler\\QT\\file_read_deneme_001\\foe.txt")

    send_data_from_file(file_path)

    response = ser.readline().decode().strip()
    print("Recieving Data:", response)

    if file_path.lower() == 'q':
        break

ser.close()
