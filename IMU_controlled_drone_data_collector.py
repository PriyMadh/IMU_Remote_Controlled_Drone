import numpy as np
import serial
import time
import csv
import os

# connect arduino through serial port
arduino = serial.Serial(port='/dev/cu.usbserial-10', baudrate=9600, timeout=1)
time.sleep(2)

#make gesture label, file name, and number of takes (examples) per lable
Gesture_label = 5 # 0 = up, 1 = down, 2 = left, 3 = right, 4 = forward, 5 = backward
Dataset_file = "gesture_data_take_3.csv"
TAKES = 20 # any amount from like 20 to 50 should be enough

if not os.path.exists(Dataset_file):
    with open(Dataset_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["X_Max", "X_Min", "X_Mean", "X_Standard_Dev", "Y_Max", "Y_Min", "Y_Mean","Y_Standard_Dev", "Z_Max", "Z_Min", "Z_Mean", "Z_Standard_Dev", "Label"]) #  the features on which model will be trained (excdept for label, thats the target)

print("Connecting to Arduino. Checking calibration status:")

# wait for arduino to calbrate
calibrated = False
while not calibrated:
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8').strip()
        print(f"Arduino calibration: {line}") # see the calibration numbers
        if "starting to stream data" in line:
            calibrated = True
            print("\n Arduino is calibrated and ready. Starting takes in 3 seconds...")
            time.sleep(3)

# the actual recording of the gestures
for take in range (TAKES):
    print(f"\nTake {take} of {TAKES}. Get in position...")
    time.sleep(2) 
    if arduino.in_waiting > 0:
        arduino.read(arduino.in_waiting)
    print("begin motion")

    x_data, y_data, z_data = [], [], []
    button_steady = 0
    button_land = 0

    start_time = time.time()
    while time.time() - start_time <= 1.25: #interval fo 1.25 seconds
        if arduino.in_waiting > 0:
            try: # dont want it to get stuck on bad reading or small error in serial communication
                line = arduino.readline().decode('utf-8').strip()
                parts = line.split(',')
                if len(parts) == 5:
                    x_data.append(float(parts[0]))
                    y_data.append(float(parts[1]))
                    z_data.append(float(parts[2]))
                    button_steady = float(parts[3]) # these can just continue to replace it self, because the second it reads a 1 for either of these, only one action is really required, and then pressing the button again reverts it back to a zero
                    button_land = float(parts[4])
            except:
                continue
    features = []
    # feature engineering
    if len(x_data) > 0:
        features = [
            np.max(x_data), np.min(x_data), np.mean(x_data), np.std(x_data),
            np.max(y_data), np.min(y_data), np.mean(y_data), np.std(y_data),
            np.max(z_data), np.min(z_data), np.mean(z_data), np.std(z_data),
        ]

        with open(Dataset_file, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(features + [Gesture_label])
    else:
        print("\n no data")

print("\n Done with this gesture, either move on to diff gesture or comlete and check csv")





    
