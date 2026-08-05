#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno  = Adafruit_BNO055(55, 0x28);

const int button_steady = 8;
const int button_take_off = 9;

unsigned long last_time = 0;
float vx = 0.0, vy = 0.0 , vz = 0.0;
int state1 = 0;
int state2 = 0;


// Low pass flter vars
float filtered_ax = 0.0;
float filtered_ay = 0.0;
float filtered_az = 0.0;
const float alpha = 0.2;
  

void setup() {
  Serial.begin(9600);
  pinMode(button_steady, INPUT_PULLUP);
  pinMode(button_take_off, INPUT_PULLUP);

  if (!bno.begin()) {
    Serial.println("bno not detected");
    while (1); // Stop everything here
  }
  bno.setExtCrystalUse(true);
  delay(100);

  //last_time  = micros();
   
   uint8_t bno_system = 0, gyro = 0, accel = 0, mag = 0;
   while (accel != 3){
  
  
  // Fetch calibration state from the hardware registers
  bno.getCalibration(&bno_system, &gyro, &accel, &mag);

  // Print results to Serial Monitor
  Serial.print("CALIBRATION STATUS -> ");
  Serial.print("Sys:");   Serial.print(bno_system);
  Serial.print(" G:");    Serial.print(gyro);
  Serial.print(" A:");    Serial.print(accel);
  Serial.print(" M:");    Serial.println(mag);

  }

  Serial.println("\n calibration done, starting to stream data...");
  

}

void loop() {
  
 
  //unsigned long current_time = micros(); //the code will come back here to the top and the whole script under here will take form last time to new current time
  //float dt = (current_time - last_time)/1000000.0; // convert to sec
  //last_time = current_time; // reset last_time

  


  
  imu::Vector<3>linear_acc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL); //this integrates the accelerometer wiht the gyroscope and subtracts 9.8 m/s^2 for gravity

  float ax = linear_acc.x(); 
  float ay = linear_acc.y();// - 0.0193;
  float az = linear_acc.z(); //+ 0.393;

//integrate acceleration once to get velocity (probably better for finding features for training model later)
  //vx += ax*dt;
  //vy += ay*dt;
 //vz += az*dt;    DECIDED AGAINST USING VELOCITY BECAUSE OF THE INTEGRATION MADE THE DRIFT ACCUMULATE TOO FAST MAKING IT UNUSABLE AFTER LIKE 2-4 SECONDS

// Low pass filtering:
  filtered_ax = alpha*ax + (1-alpha)*filtered_ax;
  filtered_ay = alpha*ay + (1-alpha)*filtered_ay;
  filtered_az = alpha*az + (1-alpha)*filtered_az;


//these won't be used to train model, but overirde gestures - the fist is to mkae the drone hover and the second is to make the droen take off and land again
  if (digitalRead(button_steady) == 0){ //0 b/c used INPUT_PULLUP
    state1 = 1 - state1;
    delay(200);
  }
  if (digitalRead(button_take_off) == 0){
    state2 = 1 - state2;
    delay(200); // debouncing
  }

  


  Serial.print(filtered_ax); Serial.print(", ");
  Serial.print(filtered_ay); Serial.print(", ");
  Serial.print(filtered_az); Serial.print(",");
  Serial.print(state1); Serial.print(",");
  Serial.println(state2);

delay(20); //for like 50 rounds per sec

  
}
