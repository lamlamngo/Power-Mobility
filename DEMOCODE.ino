//Author: Lam Ngo

#include <TimerOne.h>
#include <Servo.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

//define pins
#define frontTrigPin 4
#define frontEchoPin 2
#define backTrigPin 5
#define backEchoPin 3

#define buzzer 24

//define four installed sockets
#define first 11
#define second 12
#define third 13
#define fourth 22

//Statiscal LEDs
#define ring_speed 8
#define ring_battery 9
#define ring_status 10

//IR sensors
#define frontAnalog A1
#define backAnalog A2

#define battery_level A2
#define ir 18

//additional constants
#define NUMPIXELS_1 16
#define NUMPIXELS_2 24
#define echo_int 0

#define TIMER_US 50
#define TICK_COUNTS 4000

volatile long echo_start = 0;
volatile long echo_end = 0;
volatile long echo_duration = 0;
volatile int trigger_time_count = 0;

float speed;
boolean stop = true;
float led;
int max = 40;
int speedprofile = -1;

int[][] forward_speed = {{1540,1580},{1540,1600},{1530,1650}}
int[][] backward_speed = {{1460,1420},{1460,1400},{1460,1350}}

int forward;
int backward;
int left;
int right;

bool run = true;
int light_state = 0;

Servo leftMotor;
Servo rightMotor;

Adafruit_NeoPixel pixels_speed = Adafruit_NeoPixel(NUMPIXELS_2, ring_speed, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_battery = Adafruit_NeoPixel(NUMPIXELS_2, ring_battery, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_tail = Adafruit_NeoPixel(NUMPIXELS_1, ring_status, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(frontTrigPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);

  pinMode(backTrigPin, OUTPUT);
  pinMode(backEchoPin, INPUT);

  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerIsr);
  attachInterrupt(echo_int, echo_interrupt, CHANGE);

  attachInterrupt(ir, decode);

  leftMotor.attach(6);
  leftMotor.write(90);
  rightMotor.attach(7);
  rightMotor.write(90);

  pixels_speed.begin();
  pixels_speed.show();

  pixels_battery.begin();
  pixels_battery.show();

  pixels_tail.begin();
  pixels_tail.show();

  Serial.begin(9600);

  setup_stage();
}

void loop() {

  if (run){
    //neutral state
    setup_stage();
    leftMotor.writeMicroseconds(1510);
    rightMotor.writeMicroseconds(1510);

    if (digitalRead(forward) == LOW){
      speed = forward_speed[speed_profile][0];

      //slowly accelarating
      while (digitalRead(forward) == LOW){
        if (speed >= forward_speed[speed_profile][1]){
          speed = forward_speed[speed_profile][1];
        }
        leftMotor.writeMicroseconds(speed+2);
        rightMotor.writeMicroseconds(speed);
        speed = speed + 0.0005;

        proportial_light(speed, forward_speed[speed_profile][1] - forward_speed[speed_profile][0], 0)
      }

      //slow decelerating
      while (speed > 1525){
        leftMotor.writeMicroseconds(speed);
        rightMotor.writeMicroseconds(speed);
        speed = speed - 0.005;
      }

      reset_tail_light();
    }

    if (digitalRead(backward) == LOW){
      speed = backward_speed[speed_profile][0];

      while (digitalRead(backward) == LOW && !stop){
        if (speed <= backward_speed[speed_profile][1]){
          speed = backward_speed[speed_profile][1];
        }
        leftMotor.writeMicroseconds(speed);
        rightMotor.writeMicroseconds(speed);
        speed = speed - 0.0005;
        proportial_light(speed, backward_speed[speed_profile][0] - backward_speed[speed_profile][1], 1)
      }

      while (speed < 1475){
        leftMotor.writeMicroseconds(speed);
        rightMotor.writeMicroseconds(speed);
        speed = speed + 0.001;
      }

      reset_tail_light();
    }

    if (digitalRead(right) == LOW){
      float speedL = forward_speed[speed_profile][0];
      float speedR = backward_speed[speed_profile][0];

      while (digitalRead(right) == LOW && !stop){
        if (speedL >= forward_speed[speed_profile][1]){
          speedL = forward_speed[speed_profile][1];
        }

        if (speedR <= backward_speed[speed_profile][1]){
          speedR = backward_speed[speed_profile][1];
        }

        leftMotor.writeMicroseconds(speedL);
        rightMotor.writeMicroseconds(speedR);
        speedL = speedL + 0.002;
        speedR = speedR - 0.002;
        proportial_light(speedL, forward_speed[speed_profile][1] - forward_speed[speed_profile][0], 0)
      }

      while (speedL > 1525 && speedR < 1475){
        leftMotor.writeMicroseconds(speedL);
        rightMotor.writeMicroseconds(speedR);
        speedL = speedL - 0.001;
        speedR = speedR + 0.001;
      }

      reset_tail_light();
    }

    if (digitalRead(left) == LOW){
      float speedL = backward_speed[speed_profile][0];
      float speedR = forward_speed[speed_profile][0];

      while (digitalRead(right) == LOW && !stop){
        if (speedR >= forward_speed[speed_profile][1]){
          speedR = forward_speed[speed_profile][1];
        }

        if (speedL <= backward_speed[speed_profile][1]){
          speedL = backward_speed[speed_profile][1];
        }

        leftMotor.writeMicroseconds(speedL);
        rightMotor.writeMicroseconds(speedR);
        speedR = speedR + 0.002;
        speedL = speedL - 0.002;

        proportial_light(speedL, backward_speed[speed_profile][0] - backward_speed[speed_profile][1], 1)
      }

      while (speedR > 1525 && speedL < 1475){
        leftMotor.writeMicroseconds(speedL);
        rightMotor.writeMicroseconds(speedR);
        speedR = speedR - 0.001;
        speedL = speedL + 0.001;
      }

      reset_tail_light();
    }
  }
}

void timerIsr(){
  trigger_pulse();
}

void proportial_light(int currentspeed, int max, int direction){
  int to_lit = (currentspeed/max)*NUMPIXELS_1;
  if (direction == 0){
      pixels_tail.setPixelColor(to_lit, pixels_tail.Color(0,150,0));
  } else if (direction == 1){
      pixels_tail.setPixelColor(to_lit, pixels_tail.Color(150,0,0));
  }
  pixels_tail.show();
}

void reset_tail_light(){
  for (int i = 0; i < NUMPIXELS_1; i++){
    pixels_tail.setPixelColor(i, pixels_tail.Color(0,0,0));
  }
  pixels_tail.show();
}

void tail_blink(int state){
  if (state == 0){
    for (int i = 0; i < NUMPIXELS_1; i++){
      pixels_tail.setPixelColor(i, pixels_tail.Color(0,0,0));
    }
    pixels_tail.show();
    delay(500);
  } else{
    for (int i = 0; i < NUMPIXELS_1; i++){
      pixels_tail.setPixelColor(i, pixels_tail.Color(150,0,0));
    }
    pixels_tail.show();
    delay(500);
  }
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
      state = 2;
      break;
    case 2:
    default:
      digitalWrite(frontTrigPin,LOW);
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
      Serial.println(echo_duration/58);
      if (echo_duration/58 < 80  && echo_duration/58 != 0) {
        //distance in cm
        if (echo_duration/58 < 50 && echo_duration/58 != 0){
          // distance < 50 and stop the device
          tone(buzzer,1000);
          if (light_state == 0){
            light_state = 1;
            tail_blink(light_state);
          } else{
            light_state = 0;
            tail_blink(light_state);
          }
          stop = true;
        }else{
          // < 80 sound the alarm to alert
          tone(buzzer,500);
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
