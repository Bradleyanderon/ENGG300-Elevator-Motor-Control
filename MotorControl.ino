#include <Servo.h>
#include <string.h>
#include "Encoder.h"
#include "Communications.h"

//TO BE CHANGED
int motorPin_onPin = 9;
int motorPin = 9;

Encoder myEnc(3, 2);

//no led pin when testing
int led = 13;

Servo Motor0;

Servo doorServo;

int Motor_max_forward = 1700;  
int Motor_max_reverse = 1300;
int Motor_stop = 1500;
int Cur_speed = Motor_stop;
int timeFromBeginning = 0;
static int timeToStop = 100; //Until we know the target estimate time to stop, will use 100

unsigned long next_step;
unsigned long last_step;

boolean emergency = false;
boolean doorState = false;

long oldPosition  = -999;
long newPosition = 0;

//Variables for sensor levels
//variables for limit switches using digital pins
//int level_1 = 31;
int level_2 = 35;
int level_3 = 39;
int level_4 = 43;
int level_5 = 47;

//pin allocations for doors
const int doorServoPin = 10;
const int doorSwitchPin = 6;

//global level state (0 = at no level)
int currFloor = 0;
int prevFloor = 0;
int elevatorStatus = 0;

void setup() {
  // put your setup code here, to run once:

 Serial.begin(9600);

 Communications.initiateConnection();
 
 pinMode(motorPin_onPin, OUTPUT); 
 pinMode(motorPin, OUTPUT); 
 pinMode(doorSwitchPin, INPUT_PULLUP);
 
 Motor0.attach(motorPin_onPin, 1000, 2000);  // This pin is the control signal for the Motor
 Motor0.writeMicroseconds(Motor_stop);
 doorServo.attach(doorServoPin);
 
 //initalisation of pins for sensros
// pinMode(level_1, INPUT);    
 pinMode(level_2, INPUT);   
 pinMode(level_3, INPUT);
 pinMode(level_4, INPUT);
 pinMode(level_5, INPUT); 
  
 Serial.println("Basic Encoder Test:");

// motorControl(3);
// delay(1000);
// motorControl(2);

//floorCheck();
//prevFloor = currFloor;
//elevatorMovement("U");
//elevatorStatus = 1;

//openDoors();
}



// Motor0's RPM should be understood to determine the speed of the elevator. 
// As it approaches its destination, slow to a stop.
// The speed of the elevator should remain a consistent max speed regardless of weight.

void ease_into(String d) { //Code to in into and out of movement.
  if(d.equals("U")){
    while(Cur_speed<Motor_max_forward){
      Cur_speed = Cur_speed + 10;
      Motor0.writeMicroseconds(Cur_speed);
      timeFromBeginning++;
    }
  }
  else if(d.equals("D")){
    while(Cur_speed>Motor_max_reverse) {
      Cur_speed = Cur_speed - 10;
      Motor0.writeMicroseconds(Cur_speed);
      timeFromBeginning++;
    }
  }
  else if(d.equals("E")){
    while(Cur_speed>Motor_stop) {
      Cur_speed = Cur_speed - 10;
      Motor0.writeMicroseconds(Cur_speed);
    }
    while(Cur_speed<Motor_stop) {
      Cur_speed = Cur_speed + 10;
      Motor0.writeMicroseconds(Cur_speed);
    }
  }
  else{
    while(Cur_speed>Motor_stop) {
      Cur_speed = Cur_speed - 10;
      Motor0.writeMicroseconds(Cur_speed);
    }
    while(Cur_speed<Motor_stop) {
      Cur_speed = Cur_speed + 10;
      Motor0.writeMicroseconds(Cur_speed);
    }
  }
}

/*
 * Motor write
 * digitalWrite LOW = on
 * digitalWrite HIGH = off
 * 
 * case 0: when the elevator is stopped at a floor
 * case 1: when the elevator is moving down
 * case 2: when the elevator has stopped due to emergency, turning off the motor
 * case3: when the elevator is moving up
 * 
 */
void motorControl(int state){
  unsigned long in_step;  // how long we are in the current step
  in_step = (millis() - last_step)/2;
  if (in_step > 1000) {
    in_step = 1000;  // don't go beyond the step time
  }
    switch(state) {
    case 0:
      //emergency stops lift
      digitalWrite(led, HIGH);
      digitalWrite(motorPin_onPin, LOW);
      Motor0.writeMicroseconds(Motor_stop);
      Serial.println("Emergency");
      break;
    case 1:
      //runs lift downwards
      digitalWrite(led, HIGH);
      digitalWrite(motorPin_onPin,LOW);
      Motor0.writeMicroseconds(Motor_max_forward);
      Serial.println("Down");
      break;
    case 2:
      //stops lift
      digitalWrite(led, LOW);
      digitalWrite(motorPin_onPin, HIGH);
      Motor0.writeMicroseconds(Motor_stop);
      Serial.println("Stop");
      break;
    case 3:
      //runs lift upwards
      digitalWrite(motorPin_onPin, HIGH);
      Motor0.writeMicroseconds(Motor_max_reverse);
      Serial.println("Up");
      break;
  }
}
/*
 * Inputs from the buttons:
 *   "E" = Emergency stop
 *   "U" = Motor going forward i.e. upwards
 *   "D" = Motor going reverse i.e. downwards
 *   "S" = Motor is stopped
 *   "O" = Doors opening
 *   "C" = Closing doors
 *
 */

void elevatorMovement(String data) {
  if(data.equals("E")){
    ease_into(data);
    motorControl(0);
  } else if(data.equals("U")){
    timeFromBeginning = 0;
    ease_into(data);
    motorControl(3);
  } else if(data.equals("D")){
    timeFromBeginning = 0;
    ease_into(data);
    motorControl(1);
  }
  // To be calibrated
  else if(data.equals("S")){
    ease_into(data);
    motorControl(2);
  } 
  // To be calibrated
  else if(data.equals("O")){
   doorServo.write(0); // clock-wise
  delay(3000);
  doorServo.write(95); // stop
  } else if(data.equals("C")){
      doorServo.write(190); //anti-clockwise
      delay(3000);
       doorServo.write(95); //stop
  }
}

void floorCheck(){
//  if(digitalRead(level_1) == LOW) {
//    currFloor = 1;
//    Serial.println(currFloor);
//    //Serial.println(prevFloor); 
//  } else 
  if (digitalRead(level_2) == LOW) {             
    currFloor = 2;  
    Serial.println(currFloor);
    //Serial.println(prevFloor);  
  } else if (digitalRead(level_3) == LOW) {             
    currFloor = 3;    
    Serial.println(currFloor);
    //Serial.println(prevFloor); 
  } else if (digitalRead(level_4) == LOW) {              
    currFloor = 4;   
    Serial.println(currFloor);
    //Serial.println(prevFloor); 
  } else if(digitalRead(level_5) == LOW) {
    currFloor = 5;
    Serial.println(currFloor);
    //Serial.println(prevFloor); 
  } 
}

void openDoors(){
  doorServo.write(0);
  delay(1200);
  doorServo.write(95);
}

void closeDoors(){
  //while(digitalRead(doorSwitchPin) != LOW){
  doorServo.write(190);
  delay(1400);
  doorServo.write(95);
}

void loop() {
  
//Testing
  String data = Communications.receive();
    if (data != "") { 
      Serial.println(data);
//      if (data.equals("E") || data.equals("D") || data.equals("U")){
//        Communications.send("MEGA OK");
//      } else {
//        Communications.send("MEGA NOK")
//      }
    }

  if(data.equals("E")){
    elevatorMovement(data);
    elevatorStatus = 0;
  }
    
  floorCheck();

  if(data.equals("S")){
    elevatorMovement(data);
    elevatorStatus = 0;
  }

  if(elevatorStatus == 0){
    if(data.equals("U")){
      elevatorStatus = 1;
      closeDoors();
      floorCheck();
      prevFloor = currFloor;
      elevatorMovement(data);
    } else if(data.equals("D")){
      elevatorStatus = -1;
      closeDoors();
      floorCheck();
      prevFloor = currFloor;
      elevatorMovement(data);
    }
  }

  if(elevatorStatus == 1){
    if(currFloor > prevFloor){
      elevatorMovement("S");
      elevatorStatus = 0;
      openDoors();
      closeDoors();
    }
  }

  if(elevatorStatus == -1){
    if(currFloor < prevFloor){
      elevatorMovement("S");
      elevatorStatus = 0;
      openDoors();
      closeDoors();
    }
  }

  //Serial.println(data);
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
  
}
