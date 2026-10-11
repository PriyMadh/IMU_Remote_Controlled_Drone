#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include "MyInsKalman.h" // My Kalman experimental

MyInsKalman kalmanRoll;
MyInsKalman kalmanPitch;

Adafruit_BNO055 myIMU = Adafruit_BNO055(55, 0x28);


// Variable definitions
float thetaM, phiM;            // Raw Accelerometer Pitch & Roll
float thetaLPF = 0, phiLPF = 0;// Low-Pass Filtered values
float thetaComp = 0, phiComp = 0; // Complementary Filter values
float thetaKalman = 0, phiKalman = 0; // Kalman Filter values

unsigned long timer = 0;
int lockoutTimer = 0;
const int LOCKOUT_SAMPLES = 25; // 250 ms cooldown at 100 Hz
const float radToDeg = 180.0f / M_PI;

void setup() {
  Serial.begin(115200);
  myIMU.begin();
  delay(1000);
  int8_t temp=myIMU.getTemp();
  myIMU.setExtCrystalUse(true);
  
  uint8_t bno_system = 0, gyro = 0, accel = 0, mag = 0;
  while (accel != 3 || gyro != 3){
  
  
  // Fetch calibration state from the hardware registers
  myIMU.getCalibration(&bno_system, &gyro, &accel, &mag);

  // Print results to Serial Monitor
  Serial.print("CALIBRATION STATUS -> ");
  Serial.print("Sys:");   Serial.print(bno_system);
  Serial.print(" G:");    Serial.print(gyro);
  Serial.print(" A:");    Serial.print(accel);
  Serial.print(" M:");    Serial.println(mag);

  }

  Serial.println("\n calibration done, starting to stream data...");
  
  // Read initial accel angles to seed filters
  imu::Vector<3> acc = myIMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  thetaM = -atan2(acc.x(), acc.z()) * radToDeg;
  phiM   =  atan2(acc.y(), acc.z()) * radToDeg;

  kalmanPitch.setAngle(thetaM);
  kalmanRoll.setAngle(phiM);

  timer = micros();
}

void loop() {
  // delta time (dt)
  float dt = (micros() - timer) / 1000000.0;
  timer = micros();

  // This gets bias compensated acc, but gravity is still present
  imu::Vector<3> acc = myIMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

  // This gets bias compensated gyro meas, but gravity is still present
  imu::Vector<3> gyr = myIMU.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE); // deg/s

  // 3. Compute raw accelerometer pitch and roll
  thetaM = -atan2(acc.x(), acc.z()) * radToDeg;
  phiM   =  atan2(acc.y(), acc.z()) * radToDeg;

  // --- FILTER 1: Low-Pass Filter (LPF) ---
  thetaLPF = 0.90 * thetaLPF + 0.10 * thetaM;
  phiLPF   = 0.90 * phiLPF   + 0.10 * phiM;

  // --- FILTER 2: Complementary Filter ---
  float gyroBias = 0;
  thetaComp = 0.95 * (thetaComp + (gyr.y()+gyroBias) * dt) + 0.05 * thetaM;
  phiComp   = 0.95 * (phiComp   + (gyr.x()+gyroBias) * dt) + 0.05 * phiM;

  // --- FILTER 3: My experimental 2 state kalman filter for roll and pitch
  float phiKalman  = kalmanRoll.predictAndUpdate(phiM, gyr.x()+gyroBias, dt);
  float thetaKalman = kalmanPitch.predictAndUpdate(thetaM, gyr.y()+gyroBias, dt);

  // --- FILTER 4: BNO055 Onboard Hardware Fusion ---
  /*imu::Vector<3> bnoEuler = myIMU.getVector(Adafruit_BNO055::VECTOR_EULER);
  float bnoRoll  = -bnoEuler.y();
  float bnoPitch = bnoEuler.z();*/

  // Convert Fused Angles to Radians for Trigonometry
  float thetaRad = thetaComp * DEG_TO_RAD;
  float phiRad   = phiComp   * DEG_TO_RAD;

  // Calculate Expected Gravity Components on Body Axes (m/s^2)
  float gx = -9.81 * sin(thetaRad);
  float gy =  9.81 * sin(phiRad) * cos(thetaRad);
  float gz =  9.81 * cos(phiRad) * cos(thetaRad);

  float gxlpf = -9.81 * sin(thetaLPF*DEG_TO_RAD);
  // linear acc from lpf
  float linAccelXlpf = (acc.x() - gxlpf)/9.81;

  // ISOLATE TRUE LINEAR ACCELERATION (Feed these to ML model)
  float linAccelX = (acc.x() - gx)/9.81; // Pure Left/Right dynamic acceleration
  float linAccelY = (acc.y() - gy)/9.81; // Pure Forward/Backward dynamic acceleration
  float linAccelZ = (acc.z() - gz)/9.81; // Pure Up/Down dynamic acceleration

  // Lockout Handling for Classification Logic to prevent wrong gesture on decceleration
  bool readyForNewGesture = true;
  if (lockoutTimer > 0) {
    lockoutTimer--;
    readyForNewGesture = false;
  }

  // Motion detection using acceleration values, use a threshold on 0.2g
  if (readyForNewGesture) {
    if (linAccelX > 0.15) {
      Serial.println("MOTION DETECTED: RIGHT (+X)");
      lockoutTimer = LOCKOUT_SAMPLES; // Lockout for 250 ms to absorb deceleration phase
    } 
    else if (linAccelX < -0.2) {
      Serial.println("MOTION DETECTED: LEFT (-X)");
      lockoutTimer = LOCKOUT_SAMPLES;
    }
    else if (linAccelY > 0.2) {
      Serial.println("MOTION DETECTED: FORWARD (+Y)");
      lockoutTimer = LOCKOUT_SAMPLES;
    }
    else if (linAccelY < -0.2) {
      Serial.println("MOTION DETECTED: BACKWARD (-Y)");
      lockoutTimer = LOCKOUT_SAMPLES;
    }
    else if (linAccelZ > 0.2) {
      Serial.println("MOTION DETECTED: UP (+Z)");
      lockoutTimer = LOCKOUT_SAMPLES;
    }
    else if (linAccelZ < -0.2) {
      Serial.println("MOTION DETECTED: DOWN (-Z)");
      lockoutTimer = LOCKOUT_SAMPLES;
    }
  }

  // 4. Output values formatted for Arduino Serial Plotter
  // Format: "Label1:Val1,Label2:Val2,..."
  Serial.print(timer); Serial.print(",");
  /*Serial.print(thetaM);      Serial.print(",");
  Serial.print(thetaLPF);    Serial.print(",");
  Serial.print(thetaComp);   Serial.print(",");
  Serial.print(thetaKalman); Serial.print(",");*/
  //Serial.print(bnoPitch); Serial.print(",");
  Serial.print(phiM);        Serial.print(",");
  Serial.print(phiLPF);    Serial.print(",");
  Serial.print(phiComp);   Serial.print(",");
  Serial.print(phiKalman); Serial.print(",");
  //Serial.println(bnoRoll); Serial.println(",");

  // Serial output for debugging/plotting linear motion
  Serial.print(linAccelXlpf); Serial.print(",");
  Serial.print(linAccelX); Serial.print(",");
  Serial.print(linAccelY); Serial.print(",");
  Serial.print(linAccelZ); Serial.print(",");
  Serial.print(readyForNewGesture ? 1:0); Serial.println(",");

  /*Serial.print("RawAccelPitch:");     Serial.print(thetaM);      Serial.print(",");
  Serial.print("LPFPitch:");          Serial.print(thetaLPF);    Serial.print(",");
  Serial.print("ComplementaryPitch:");Serial.print(thetaComp);   Serial.print(",");
  Serial.print("KalmanPitch:");       Serial.print(thetaKalman); Serial.print(",");
  Serial.print("BNO055_FusedPitch:"); Serial.print(bnoPitch); Serial.println(",");

  Serial.print("RawAccelRoll:");     Serial.print(phiM);        Serial.print(",");
  Serial.print("LPFRoll:");          Serial.print(phiLPF);    Serial.print(",");
  Serial.print("ComplementaryRoll:");Serial.print(phiComp);   Serial.print(",");
  Serial.print("KalmanRoll:");       Serial.print(phiKalman); Serial.print(",");
  Serial.print("BNO055_FusedRoll:"); Serial.println(bnoRoll);*/

  delay(10); //for like 100 rounds per sec
}
