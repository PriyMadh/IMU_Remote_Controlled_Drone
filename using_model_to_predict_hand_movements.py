import numpy as np
import serial
import time
import csv
import os
import tensorflow as tf

# load the model
model_path = os.path.join(os.path.dirname(__file__), "IMU_controlled_drone_prediction_neural_net.keras")  
model = tf.keras.models.load_model(model_path)
# connect arduino through serial port
arduino = serial.Serial(port='/dev/cu.usbserial-10', baudrate=9600, timeout=1)
time.sleep(2)

print("Connecting to Arduino. Waiting for clean data stream...")
while True:
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8', errors='ignore').strip()
        # Check if the line has 5 comma-separated values
        if len(line.split(',')) == 5:
            print("\nArduino stream detected! Starting takeoff in 3 seconds...")
            time.sleep(3)
            break

print("Connecting to Arduino. Checking calibration status:")

# wait for arduino to calbrate
""""
calibrated = False
while not calibrated:
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8').strip()
        print(f"Arduino calibration: {line}") # see the calibration numbers
        if "starting to stream data" in line:
            calibrated = True
            print("\n Arduino is calibrated and ready. Starting takes in 3 seconds...")
            time.sleep(3)
"""
button_land = 0

while button_land == 0:
    if arduino.in_waiting > 0:
        arduino.read(arduino.in_waiting)
    print("begin motion")

    x_data, y_data, z_data = [], [], []
    button_steady = 0
    

    start_time = time.time()
    while time.time() - start_time <= 1.25: #interval fo 1.25 seconds
        if arduino.in_waiting > 0:
            try: # dont want it to get stuck on bad reading or small error in serial communication
                line = arduino.readline().decode('utf-8').strip()
                parts = line.split(',') #split at commas
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

        input_data = np.array([features])
        tensor_input = tf.convert_to_tensor(input_data, dtype=tf.float32)
        predictions = model(tensor_input)
        prediction = model.predict(tensor_input)
        prediction = predictions.numpy()
        gesture = np.argmax(prediction)
    
        if gesture == 0:
            gest_name = "up"
        elif gesture == 1:
            gest_name = "down"
        elif gesture == 2:
            gest_name = "left"
        elif gesture == 3:
            gest_name = "right"
        elif gesture == 4:
            gest_name = "forward"
        elif gesture == 5:
            gest_name = "backward"
        elif gesture == 6:
            gest_name = "stay"
        print(f"the model has predicted that the gesture is: {gest_name} ")

