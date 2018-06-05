
#include <TimerOne.h>
#include <Servo.h>
#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define frontTrigPin 7
#define frontEchoPin 2

#define backTrigPin 19
#define backEchoPin 3

#define buzzer 49

int val = 0;

int backward = 8;
int forward = 9;
int right = 10;
int left = 12;

int timer_start = 0;
int timer_end = 0;
int timer[13];
bool _delay = false;
bool _timeout = false;
int safemode_distance = 80;

#define battery A8

#define echo_int 0

#define TIMER_US 50
#define TICK_COUNTS 4000
#define ADAFRUITBLE_REQ 53
#define ADAFRUITBLE_RDY 21     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 38

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

volatile long echo_start = 0;
volatile long echo_end = 0;
volatile long echo_duration = 0;
volatile int trigger_time_count = 0;

volatile long echo_start1 = 0;
volatile long echo_end1 = 0;
volatile long echo_duration1 = 0;

int mode = 0;

int inputs[13];

float speed_F = 1380;
float speed_B = 1600;
boolean stop = false;
float led;
int max = 40;

boolean start = true;
Servo leftMotor;
Servo rightMotor;
boolean safemode = false;

String input = "";

void setup() {
  // put your setup code here, to run once:
  pinMode(frontTrigPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);

  pinMode(backTrigPin, OUTPUT);
  pinMode(backEchoPin, INPUT);

  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerIsr);
  attachInterrupt(echo_int, echo_interrupt, CHANGE);

  leftMotor.attach(6);
  rightMotor.attach(4);

  pinMode(forward,INPUT_PULLUP);
  pinMode(backward, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);

  pinMode(39, OUTPUT);
  pinMode(20, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(20), blink, CHANGE);

  Serial.begin(9600);
  Serial.println(F("Power Mobility"));

  BTLEserial.setDeviceName("Lam"); /* 7 characters max! */

  BTLEserial.begin();

  for (int i = 0; i < 13; i++){
    inputs[i] = 0;
  }

  inputs[forward] = 1;
  inputs[backward] = 1;
  inputs[left] = 1;
  inputs[right] = 1;

  for (int i = 0; i < 13; i++){
    timer[i] = 0;
  }

  timer[forward] = 0;
  timer[backward] = 0;
  timer[left] = 0;
  timer[right] = 0;
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
char inData[50];
int index = 0;
float speed;
void loop() {
  if (start){
    digitalWrite(39,HIGH);
  } else{
    digitalWrite(39,LOW);
  }

  BTLEserial.pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    // Lets see if there's any data for us!
    if (BTLEserial.available()) {
      Serial.print("* "); Serial.print(BTLEserial.available()); Serial.println(F(" bytes available from BTLE"));
    }
    // OK while we still have something to read, get a character and print it out
  if (BTLEserial.available()) {
    char c = BTLEserial.read();
     while (c != ';') // One less than the size of the array
    {     // Read a character
        inData[index] = c; // Store it
        index++; // Increment where to write next
        inData[index] = '\0'; // Null terminate the string
        c = BTLEserial.read();
    }
    index = 0;
    Serial.println(inData);
    if (inData[0] != 'c' && inData[0] != 's' && inData[0] != 't'){
      int num = 0;
      if (inData[0] == '1'){
        speed_F = 1400;
        speed_B = 1580;
      } else if (inData[0] == '2'){
        speed_B = 1590;
        speed_F = 1390;
      } else if (inData[0] == '5'){
        speed_B = 1610;
        speed_F = 1370;
      }

      if (inData[1] == '0'){
        _timeout = false;
        _delay = false;
      } else if (inData[1] == '1'){
        _timeout = false;
        _delay = true;
      } else if (inData[1] == '2'){
        _timeout = true;
        _delay = false;
      }

      if (inData[2] == '1'){
        safemode = true;
      } else if (inData[2] == '0'){
        safemode = false;
      }

      if (inData[3] == '1'){
        num = 1;
      } else if (inData[3] == '2'){
        num = 2;
      } else if (inData[3] == '3'){
        num = 3;
      } else if (inData[3] == '4'){
        num = 4;
      }

      int i = 8;
      int m = 4;
      for (int j = 4; j < m + 2*num;j = j + 2){
        if (i == 11){
          i = 12;
        }
        if (inData[j] == 'f'){
          forward = i;
          timer[forward] = (inData[j+1] - '0') * 1000;
        } else if (inData[j] == 'b'){
          backward = i;
          timer[backward] = (inData[j+1] - '0') * 1000;
        } else if (inData[j] == 'l'){
          left = i;
          timer[left] = (inData[j+1] - '0') * 1000;
        } else if (inData[j] == 'r'){
          right = i;
          timer[right] = (inData[j+1] - '0') * 1000;
        }
        inputs[i] = 1;
        i++;
      }

      Serial.println(timer[left]);
      Serial.println(timer[right]);
      Serial.println(timer[forward]);
      Serial.println(timer[backward]);

      while (i < 13){
        inputs[i] = 0;
        i++;
      }

      int start_index = m + 2*num;

      switch (inData[start_index]){
        case '0': safemode_distance = 80; break;
        case '1': safemode_distance = 90; break;
        case '2': safemode_distance = 100; break;
        case '3': safemode_distance = 110; break;
        case '4': safemode_distance = 120; break;
        case '5': safemode_distance = 130; break;
        case '6': safemode_distance = 140; break;
        case '7': safemode_distance = 150; break;
        default: safemode_distance = 80; break;
      }
        
  } else if (inData[0] == 'c') {
      float b = ((analogRead(A8)) * (5.0 / 1023.0)/5)*100;

      digitalWrite(backTrigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(backTrigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(backTrigPin, LOW);

      // Read the signal from the sensor: a HIGH pulse whose
      // duration is the time (in microseconds) from the sending
      // of the ping to the reception of its echo off of an object.
      pinMode(backEchoPin, INPUT);
      long duration = pulseIn(backEchoPin, HIGH);

      // convert the time into a distance
      long cm = (duration/2) / 29.1;

      String motors = "00";
      if (leftMotor.attached() && rightMotor.attached()){
        motors = "11";
      } else if (!leftMotor.attached() && rightMotor.attached()){
        motors = "01";
      } else if (!rightMotor.attached() && leftMotor.attached()){
        motors = "10";
      } else {
        motors = "00";
      }
      String s = String(int(b)) + "|" + String(val) + "|" + String(cm) + "|" + String (forward) + "|" + String(backward) + "|" + String(left) + "|" + String(right) + "|" + motors;
 
      uint8_t sendbuffer[40];
      s.getBytes(sendbuffer, 40);
      char sendbuffersize = min(40, s.length());
      //char sendbuffersize = 40;
      Serial.print(F("\n* Sending -> \"")); Serial.print((char *)sendbuffer); Serial.println("\"");

      // write the data
      BTLEserial.write(sendbuffer, sendbuffersize);
    } else if (inData[0] == 's'){
      start = false;
    } else if (inData[0] == 't'){
      start = true;
    }
  }
  }


  if (start){
    leftMotor.writeMicroseconds(1510);
    rightMotor.writeMicroseconds(1510);

  if (digitalRead(backward) == LOW && inputs[backward] == 1){
    if (_delay){
      timer_start = millis();
      timer_end = millis();
      while ( (timer_end - timer_start) < timer[backward]){
        timer_end = millis();
      }

       speed = 0;
      //accelerate
      while (digitalRead(backward) == LOW && !stop){
        if (speed >= 1023){
          speed = 1023;
        }
        int right_speed = map (speed,0,1023,1480,speed_B);
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(right_speed);
        speed = speed + 0.25;
      }

      int decel_speed = map (speed,0,1023,1480,speed_B);
      //decelerate
      while (decel_speed > 1500){
        leftMotor.writeMicroseconds(decel_speed);
        rightMotor.writeMicroseconds(decel_speed);
        speed = speed - 5;
        decel_speed = map (speed,0,1023,1480,speed_B);
        Serial.println(decel_speed);
      }
    } else if (_timeout){
        speed = 0;
        //accelerate
        timer_start = millis();
        timer_end = millis();
        while (digitalRead(backward) == LOW && !stop && (timer_end - timer_start) < timer[backward]){
          if (speed >= 1023){
            speed = 1023;
          }
          int right_speed = map (speed,0,1023,1480,speed_B);
          leftMotor.writeMicroseconds(right_speed);
          rightMotor.writeMicroseconds(right_speed);
          speed = speed + 0.25;
          timer_end = millis();
        }

        int decel_speed = map (speed,0,1023,1480,speed_B);
        //decelerate
        while (decel_speed > 1500){
          leftMotor.writeMicroseconds(decel_speed);
          rightMotor.writeMicroseconds(decel_speed);
          speed = speed - 5;
          decel_speed = map (speed,0,1023,1480,speed_B);
          Serial.println(decel_speed);
        }

        while (digitalRead(backward) == LOW);
    } else {
         speed = 0;
        //accelerate
        while (digitalRead(backward) == LOW && !stop){
          if (speed >= 1023){
            speed = 1023;
          }
          int right_speed = map (speed,0,1023,1480,speed_B);
          leftMotor.writeMicroseconds(right_speed);
          rightMotor.writeMicroseconds(right_speed);
          speed = speed + 0.25;
          timer_end = millis();
        }

        int decel_speed = map (speed,0,1023,1480,speed_B);
        //decelerate
        while (decel_speed > 1500){
          leftMotor.writeMicroseconds(decel_speed);
          rightMotor.writeMicroseconds(decel_speed);
          speed = speed - 5;
          decel_speed = map (speed,0,1023,1480,speed_B);
          Serial.println(decel_speed);
        }
    }
  }

  if (digitalRead(forward) == LOW && inputs[forward] == 1){

    if (_delay){
      Serial.println("In here");
      timer_start = millis();
      timer_end = millis();
      while ( (timer_end - timer_start) < timer[forward]){
        timer_end = millis();
      }

      Serial.println("Got out");

      speed = 1023;

      while (digitalRead(forward) == LOW && !stop){
        if (speed <= 0){
          speed = 0;
        }
        int right_speed = map (speed,1023,0,1410,speed_F);
        Serial.println(right_speed);
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(right_speed);
        speed = speed - 3;
      }

      int decel_speed = map (speed,1023,0,1510,speed_F);
      //decelerate
      while (decel_speed < 1510){
        leftMotor.writeMicroseconds(decel_speed);
        rightMotor.writeMicroseconds(decel_speed);
        speed = speed + 3;
        decel_speed = map (speed,1023,0,1510,speed_F);
        Serial.println(decel_speed);
      }
    } else if (_timeout){
      speed = 1023;
      timer_start = millis();
      timer_end = millis();
      while (digitalRead(forward) == LOW && !stop && (timer_end - timer_start) < timer[forward]){
        if (speed <= 0){
          speed = 0;
        }
        int right_speed = map (speed,1023,0,1410,speed_F);
        Serial.println(right_speed);
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(right_speed);
        speed = speed - 3;
        timer_end = millis();
      }

      int decel_speed = map (speed,1023,0,1510,speed_F);
      //decelerate
      while (decel_speed < 1510){
        leftMotor.writeMicroseconds(decel_speed);
        rightMotor.writeMicroseconds(decel_speed);
        speed = speed + 3;
        decel_speed = map (speed,1023,0,1510,speed_F);
        Serial.println(decel_speed);
      }

      while(digitalRead(forward) == LOW);
    } else {
      speed = 1023;

      while (digitalRead(forward) == LOW && !stop){

        if (speed <= 0){
          speed = 0;
        }
        int right_speed = map (speed,1023,0,1410,speed_F);
        Serial.println(right_speed);
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(right_speed);
        speed = speed - 3;
      }

      int decel_speed = map (speed,1023,0,1510,speed_F);
      //decelerate
      while (decel_speed < 1510){
        leftMotor.writeMicroseconds(decel_speed);
        rightMotor.writeMicroseconds(decel_speed);
        speed = speed + 3;
        decel_speed = map (speed,1023,0,1510,speed_F);
        Serial.println(decel_speed);
      }
    }
  }

  if (digitalRead(right) == LOW && inputs[right] == 1){


    if (_delay){
      timer_start = millis();
      timer_end = millis();
      while ( (timer_end - timer_start) < timer[right]){
        timer_end = millis();
      }

            float speedL = 1023;
      float speedR = 0;

      while (digitalRead(right) == LOW && !stop){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL + 3;
        speedR = speedR - 5;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }
    } else if (_timeout){
      float speedL = 1023;
      float speedR = 0;
      timer_start = millis();
      timer_end = millis();
      while (digitalRead(right) == LOW && !stop && (timer_end - timer_start) < timer[right]){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
        timer_end = millis();
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL + 3;
        speedR = speedR - 2;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }

      while(digitalRead(right) == LOW);
    } else {
      float speedL = 1023;
      float speedR = 0;

      while (digitalRead(right) == LOW && !stop){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(left_speed);
        rightMotor.writeMicroseconds(right_speed);
        speedL = speedL + 3;
        speedR = speedR - 5;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }
    }
  }

    if (digitalRead(left) == LOW && inputs[left] == 1){

    if (_delay){
      timer_start = millis();
      timer_end = millis();
      while ( (timer_end - timer_start) < timer[left]){
        timer_end = millis();
      }

            float speedL = 1023;
      float speedR = 0;

      while (digitalRead(left) == LOW && !stop){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL + 3;
        speedR = speedR - 2;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }
    } else if (_timeout){
      float speedL = 1023;
      float speedR = 0;
      timer_start = millis();
      timer_end = millis();
      while (digitalRead(left) == LOW && !stop && (timer_end - timer_start) < timer[left]){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
        timer_end = millis();
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL + 3;
        speedR = speedR - 2;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }

      Serial.println("get here");
      while(digitalRead(left) == LOW);
    } else {
      float speedL = 1023;
      float speedR = 0;

      while (digitalRead(left) == LOW && !stop){
        if (speedR >= 1023){
          speedR = 1023;
        }

        if (speedL <= 0){
          speedL = 0;
        }

        int right_speed = map (speedR,0,1023,1480,1600);
        int left_speed = map (speedL,1023,0,1430,1390);

        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL - 3;
        speedR = speedR + 0.25;
      }

      int right_speed = map (speedR,0,1023,1480,1600);
      int left_speed = map (speedL,1023,0,1510,1390);
      while (left_speed < 1510 || right_speed > 1500){
        leftMotor.writeMicroseconds(right_speed);
        rightMotor.writeMicroseconds(left_speed);
        speedL = speedL + 3;
        speedR = speedR - 2;

        right_speed = map (speedR,0,1023,1480,1600);
        left_speed = map (speedL,1023,0,1510,1390);
      }
    }
  }
}
}

void timerIsr(){
  trigger_pulse();
}

void trigger_pulse(){
  static volatile int state = 0;

  if (!(--trigger_time_count)){
    trigger_time_count = TICK_COUNTS;
    state = 1;
  }

  switch (state) {
    case 0: break;
    case 1:
      digitalWrite(frontTrigPin,LOW);
      delayMicroseconds(5);
      digitalWrite(frontTrigPin, HIGH);
      delayMicroseconds(5);
      state = 2;
      break;
    case 2:
    default:
      digitalWrite(frontTrigPin,LOW);
      digitalWrite(backTrigPin,LOW);
      state = 0;
      break;
  }
}


void echo_interrupt(){
  switch(digitalRead(frontEchoPin)){
    case HIGH:
      echo_end = 0;
      echo_start = micros();
      break;
    case LOW:
      echo_end = micros();
      echo_duration = echo_end - echo_start;
      val = echo_duration/58;

      digitalWrite(backTrigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(backTrigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(backTrigPin, LOW);

      // Read the signal from the sensor: a HIGH pulse whose
      // duration is the time (in microseconds) from the sending
      // of the ping to the reception of its echo off of an object.
      pinMode(backEchoPin, INPUT);
      long duration = pulseIn(backEchoPin, HIGH);

      // convert the time into a distance
      long cm = (duration/2) / 29.1;
      if (echo_duration/58 < safemode_distance || cm < safemode_distance) {
        //distance in cm
        if ((echo_duration/58 < 50 || cm < 50)){
          // distance < 50 and stop the device
          if (safemode){
            tone(buzzer,1000);
            stop = true;
          } else{
            noTone(buzzer);
            stop = false;
          }
        }else if (safemode){
          // < 80 sound the alarm to alert
          tone(buzzer,500);
          stop = false;
        } else{
          noTone(buzzer);
          stop = false;
        }
      } else{
        //do nothing
        noTone(buzzer);
        stop = false;
      }
      break;
  }
}

void blink(){
  Serial.println("in here");
  if (digitalRead(20) == LOW){
    if (start){
      stop = true;
      start = false;
      safemode = false;
    } else{
      stop = false;
      start = true;
    }
  }
}
