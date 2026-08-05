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


void setup() {
  Serial.begin(115200);
  pinMode(button_steady, INPUT_PULLUP);
  pinMode(button_take_off, INPUT_PULLUP);

  bno.setExtCrystalUse(true);
  delay(100);

  last_time  = micros();

}

void loop() {
  
  unsigned long current_time = micros(); //the code will come back here to the top and the whole script under here will take form last time to new current time
  float dt = (current_time - last_time)/1000000.0; // convert to sec
  last_time = current_time; // reset last_time

  


  
  imu::Vector<3>linear_acc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  float ax = linear_acc.x();
  float ay = linear_acc.y();
  float az = linear_acc.z();

//integrate acceleration once to get velocity (probably better for finding features for training model later)
  vx += ax*dt;
  vy += ay*dt;
  vz += az*dt;


//these won't be used to train model, but overirde gestures - the fist is to mkae the drone hover and the second is to make the droen take off and land again
  if digitalRead(button_steady == 0){ //0 b/c used INPUT_PULLUP
    state1 = 1 - state1;
  }
  if digitalRead(button_take_off == 0){
    state2 = 1 - state2;
  }

  


  Serial.print(vx); Serial.print(", ");
  Serial.print(vy); Serial.print(", ");
  Serial.print(vz); Serial.print(",");
  Serial.print(state1); Serial.print(",");
  Serial.println(state2);


  
}
