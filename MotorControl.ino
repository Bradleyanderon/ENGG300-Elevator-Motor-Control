#include <Servo.h>
#include <string.h>
#include "Encoder.h"
#include "Communications.h"
#include<PriorityQueue.h>

int motorPin_onPin = 9;
int motorPin = 9;

Encoder myEnc(3, 2);

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
int level_1 = 31;
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
int queueStatus = 0;

#define LENGTH (10)

//Library used is a priority queue library found here:
//https://github.com/CollinDietz/PriorityQueue

//sort by ASCII value
bool queueAscending(int a, int b){
  return a < b;
}

bool queueDescending(int a, int b){
  return a > b;
}

//defines the two priority queues, one that's used when the elevator ascends and the other while it descends
PriorityQueue<int> queueFloorsAbove = PriorityQueue<int>(queueAscending);
PriorityQueue<int> queueFloorsBelow = PriorityQueue<int>(queueDescending);

//Array for floors present in either priority queue
bool inQueue[] = {false, false, false, false, false};

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
 pinMode(level_1, INPUT);    
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

void updateElevatorQueues(String itemToAdd){
  int floorToAdd = 0;
  
  //checks if the input is a value from 1 - 5, to ensure the input is a valid floor number
  for(int i = 1; i <= 5; i++){
    if(itemToAdd.equals(String(i))){
      floorToAdd = itemToAdd.toInt();
      break;
    }
  }

  //if floorToAdd = 0 by this point, input wasn't a valid floor, so just return
  if(floorToAdd == 0){
    return;
  }
  
  //Check if the floor already exists in either one of the two queues. If true, ignore this input
  if(inQueue[floorToAdd - 1] == false){
    
    //Check if the floor is above where the carriage is currently located. If true, add it to the ascending queue
    //and set the boolean in inQueue at the index corrosponding to the floor number - 1 to true
    if(floorToAdd > currFloor || currFloor == 1 || floorToAdd == currFloor && elevatorStatus == -1){
      queueFloorsAbove.push(floorToAdd);
      inQueue[floorToAdd - 1] = true;

    //Else put it in the descending queue and set the boolean in inQueue at the same index to true.
    } else if(floorToAdd < currFloor || currFloor == 5 || floorToAdd == currFloor && elevatorStatus == 1){
      queueFloorsBelow.push(floorToAdd);
      inQueue[floorToAdd - 1] = true;
    }
  }
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
  } else if(data.equals("S")){
    ease_into(data);
    motorControl(2);
  } 
}

//Reads the sensors present at every level and updates currFloor based on
void floorCheck(){
  if(digitalRead(level_1) == LOW) {
    currFloor = 1;
    Serial.println(currFloor);
    //Serial.println(prevFloor); 
  } else if (digitalRead(level_2) == LOW) {             
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

//When called opens the elevator doors
void openDoors(){
  doorServo.write(0);
  delay(1200);
  doorServo.write(95);
}

//When called closes the elevator doors
void closeDoors(){
  //while(digitalRead(doorSwitchPin) != LOW){
  doorServo.write(190);
  delay(1400);
  doorServo.write(95);
}

void loop() {
  String data = Communications.receive();
    if (data != "") { 
      Serial.println(data);
      updateElevatorQueues(data);
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

  //Where neither queue has anything in it
  if(queueFloorsBelow.count() == 0 && queueFloorsAbove.count() == 0 || queueStatus == 0){
    elevatorMovement("S");
    elevatorStatus = 0;
  }

  //Where queueFloorsBelow is the only queue that is filled
    if(queueFloorsBelow.count() != 0 || queueStatus == 1){
      queueStatus = 1;
      elevatorMovement("D");
      elevatorStatus = -1;

      if(queueFloorsBelow.peek() == currFloor){
        elevatorMovement("S");
        queueFloorsBelow.pop();
        elevatorStatus = 0;
        openDoors();
        delay(2000);
        closeDoors();
      }  
    }

    //Where queueFloorsAbove is the only queue that is filled
    if(queueFloorsAbove.count() != 0 || queueStatus == 2){
      queueStatus = 2;
      elevatorMovement("U");
      elevatorStatus = 1;

      if(queueFloorsAbove.peek() == currFloor){
        elevatorMovement("S");
        queueFloorsAbove.pop();
        elevatorStatus = 0;
        openDoors();
        delay(2000);
        closeDoors();
      }
    }

    //Where queueFloorsAbove was the first queue filled, then queueFloorsBelow
    if(queueFloorsAbove.count() != 0 && queueFloorsBelow != 0 && queueStatus == 2 || queueStatus == 3){
      queueStatus = 3;
      elevatorMovement("U");
      elevatorStatus = 1;

      if(queueFloorsAbove.peek() == currFloor){
        elevatorMovement("S");
        queueFloorsAbove.pop();
        elevatorStatus = 0;
        openDoors();
        delay(2000);
        closeDoors();
      }

      if(queueFloorsAbove.count() == 0){
        queueStatus = 1;
      }
    }

    //Where queueFloorsBelow was the first queue filled, then queueFloorsAbove
    if(queueFloorsBelow.count() != 0 && queueFloorsAbove.count() != 0 && queueStatus == 1 || queueStatus == 4){
      queueStatus = 4;
      elevatorMovement("D");
      elevatorStatus = -1;

      if(queueFloorsBelow.peek() == currFloor){
        elevatorMovement("S");
        queueFloorsBelow.pop();
        elevatorStatus = 0;
        openDoors();
        delay(2000);
        closeDoors();
      }

      if(queueFloorsBelow.count() == 0){
        queueStatus = 2;
      }
    }

  //Serial.println(data);
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}
