import sys, os, serial, datetime
from collections import deque
import time

MAX_BUFFER_SIZE = 100
CAPTURE_INTERVAL = 0.0001

def capture():
    print("Start capture")
    ser = serial.Serial("COM3", 115200, timeout=0)
    data_queue = deque(maxlen=MAX_BUFFER_SIZE)
    while True:
        line = ser.readline().decode('utf-8') 
        if line != "":
            print(line[:-1])
            
            filename = str(datetime.date.today()) + ".log"
            
            with open("deneme.txt", "a") as text_file:
                text_file.write(line)
            
            data_queue.append(line)
            
            if len(data_queue) == MAX_BUFFER_SIZE:
                removed_data = data_queue.popleft()
                remove_first_lines("deneme.txt", removed_data.strip())
            
        time.sleep(CAPTURE_INTERVAL)

def remove_first_lines(filename, line):
    with open(filename, "r") as file:
        lines = file.readlines()
    
    with open(filename, "w") as file:
        found_line = False
        for file_line in lines:
            if not found_line and file_line.strip() == line:
                found_line = True
                continue
            if file_line.strip() != "":
                file.write(file_line)

if __name__ == '__main__':
    capture()
