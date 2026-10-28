What was this project:

  Basically, in this project, I wanted to control a drone's movements with the gestures of my hand. So, for example, if I moved my hand up, the drone would follow suit and move up, and if I moved my hand left/right/up/down/forward/backward, the drone would move correspondingly.

How did I build this Project:

  I started by attaching an Adafruit BNO055 IMU to an Arduino UNO, and then I wrote a script in the Arduino IDE (IMU_drone_control_readings_with_low_pass_filter) that would take the acceleration readings from the IMU and serial print them to my computer through the serial port. Then, using Python, I wrote a program that would take the acceleration readings from the serial port in my computer (x, y, and z axes) over a 1.25-second interval, and would find the minimum, maximum, mean, and standard deviation of each x, y, and z acceleration, and then copy it down in a csv sheet (IMU_controlled_drone_data_collector.py), with each row being a 1.25 second burst and each column being either x-min, y-min, z-min, x-max, etc. Then, using this program, which ran in a loop 20 times--for each movement of my hand (up/down/forward/backward/left/right)--I moved the IMU in the corresponding direction and let the program track the data. 
  Then, using this CSV sheet, I made and trained a neural network (IMU_controlled_drone_neural_net.py)--with one hidden layer and a dropout layer to avoid overfitting + L2 regularization--which I made using TensorFlow. This model would be able to predict the motion of my hand (with the IMU) now, because it was trained on the movement data from the IMU. Then, using the model to make inferences on my hand movement, I wrote a program that would control a Ryze Tello drone's movements based on the inferences (final_product.py), and it worked, which was really cool. I also added a button for making the drone hover, and another one for making the drone land. Also, 
  I realized that the program I wrote where the acceleration readings were low-pass filtered (IMU_drone_control_readings_with_low_pass_filter) already used a complementary filter between the accelerometer and the gyroscope through a prewritten function for the BNO055, but I wanted to actually make my own just so I can understand that, so it is also included in IMU_drone_control_readings_with_CompFilterPlusLockout folder with plots. 
  The gyroscope is good for fast changes but has a drift, and the accelerometer is noisier but no drift. So fusing the two together gives smooth response without lagging. So, by giving high weighting to the gyro readings and then just like 5% to the accelerometer, I am effectively low-pass filtering the accelerometer and high-pass filtering the gyro so that they are correcting for each other's error characteristics.

Challenges:

  The first major problem I ran into was that the acceleration readings had too much noise, so I was worried that the model would get very confused between certain movements. So, what I did was I low-pass filtered the acceleration readings, which gave more weight to the immediately previous readings and made the overall curve of the acceleration vs. time graph much smoother and more reasonable. The second problem I ran into was that I found out I had made training data for all the movements I wanted, but none for just staying still, so I actually went to Claude and asked it to create synthetic data for me for holding the IMU still, and the data proved very useful. Then after that, the biggest problem I was running into was that the IMU would take so long to calibrate that eventually the drone would diconnect and the program stopped running altogther, so what I did to fix that, was I actually removed the calibration sequence from both the codes, and made sure the neural network didn't overfit to the data by using L2 regularization and a dropout layer which deactivated 10% of the neurons in the hidden layer so that the network wouldn't begin to rely solely on certain neurons and it would have an ensemble effect, and thankfully, it worked out pretty well.

Here is a video of it working: https://youtube.com/shorts/QBpUvBzy3HY

Other ways to watch:
Gathering Remote Readings: https://drive.google.com/file/d/19_Pz72nJ4QeTC7sZLrcqjH5U5IScHWr6
Model Making Inferences: https://drive.google.com/file/d/1ox5AjrlyJIbcXrpVL7GQz14OWDs-Muzk
Final Product: https://drive.google.com/file/d/1ZVLScwU0HJG88TYm8IoWIs1-Pu88Kdlp

What I am going to Improve:


  I think now that the accelerometer works well with the drone, I am also going to use the gyroscope on the remote to control the heading of the drone, which I think would be pretty cool. Also, I might integrate the camera the drone already comes with and maybe see if there is something cool I can do with that; maybe a surveillance drone controlled by my hand movements that can use computer vision to identify a threat (say, a red ball or something)
