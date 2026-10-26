What was this project:
Basically, this was just a way for me to understand what a complementary filter is because I realized my other code was using it, but it was a prewritten function, so I wanted to at least understand it. So, I learned that without the complementary filter, when the IMU is tilted, it can register some sort of linear acceleration due to gravity, so the way to solve that is to combine the readings with the gyroscope readings so you can subtract the effects of gravity from the acc readings. To do this, you first compute the angle using the acc readings using trig and take a very small fraction of that and add it to a large fraction of the gyroscope's integrated angular position readings; therefore, you are low-pass filtering the accelerometer by not taking in the noise and just a fraction of the true readings, and then you are high-pass filtering the gyro because you are removing that small fraction of a drift and keeping like 90% of the readings.


What I found:

  When the IMU is just sitting still, the raw accelerometer angle (dark blue) is very noisy, and both of the filters make it a lot smoother.

![Pitch and roll while sitting still](images/ins_01_stationary_noise.png)

  When I tilted the IMU back and forth, the low pass filter (red) is smooth but it is always late and it never reaches the top of the peaks. The complementary filter (light blue) follows the real motion just as fast as the accelerometer, but with a lot less noise. So the low pass filter trades lag for smoothness, and the complementary filter doesn't have to.

![Pitch motion](images/ins_02_pitch_motion.png)

![Roll motion](images/ins_03_roll_motion.png)

  When I jerked or bumped the IMU, the complementary filter reacted much less than the accelerometer-only angle did.

![Jerk and vibration](images/ins_04_jerk_vibration.png)

  When I shook the IMU side to side, the accelerometer-only angle swung about 30 degrees either way, the low pass filter about 15 degrees, and the complementary filter much less (roughly 5 to 10 degrees).

![Shaking side to side](images/ins_05_linear_accel_xy.png)

  This plot shows the left/right acceleration. The blue line uses the low pass filter's tilt to take out gravity, and the red line uses the complementary filter's tilt. In the first part, I was only tilting the IMU (before about 341 seconds), and the blue line shows bumps of 0.3 to 0.5 g, even though I wasn't really moving it sideways. The red line barely moves. Then, when I actually move it left and right, the red line shows it clearly.

![Left and right motion compared with tilting](images/ins_09_gesture_left_right.png)

  This plot shows the "Ready for gesture" signal (green) turning off for 250 milliseconds after each motion. Without that lockout, the slow down at the end of a left motion looked like a right motion, and the other way around.

![Lockout after each motion](images/ins_10_gesture_lockout.png)



