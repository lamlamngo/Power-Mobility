
#include <TimerOne.h>
#include <Servo.h>
//#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define frontTrigPin 4
#define frontEchoPin 2

#define backTrigPin 5
#define backEchoPin 3

#define buzzer 53

//#define ir 18

#define backward 11
#define forward 12
#define right 13

#define ring_speed 8
#define ring_battery 12 
#define ring_status 52

#define frontAnalog A1
#define backAnalog A2

#define NUMPIXELS_1 8
#define NUMPIXELS_2 10
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

Servo leftMotor;
Servo rightMotor;

Adafruit_NeoPixel pixels_1 = Adafruit_NeoPixel(16, 8, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(frontTrigPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);

  Timer1.initialize(TIMER_US);
  Timer1.attachInterrupt(timerIsr);
  attachInterrupt(echo_int, echo_interrupt, CHANGE);

  leftMotor.attach(6);
  rightMotor.attach(7);

  pinMode(forward,INPUT_PULLUP);
  pinMode(backward, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);

  pixels_1.begin();
  pixels_1.show();

  Serial.begin(9600);
}

void loop() {
//
  leftMotor.writeMicroseconds(1510);
  rightMotor.writeMicroseconds(1510);

  if (digitalRead(forward) == LOW){

    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(0,0,255));
        pixels_1.show();
    }
    speed = 1560;
    while (digitalRead(forward) == LOW){
      if (speed >= 1600){
        speed = 1600;
      }
      leftMotor.writeMicroseconds(speed+2);
      rightMotor.writeMicroseconds(speed);
      speed = speed + 0.0005;
    }

    
    while (speed > 1525){
      leftMotor.writeMicroseconds(speed);
      rightMotor.writeMicroseconds(speed);
      speed = speed - 0.005;
    }
    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(0,0,0));
        pixels_1.show();
    }
    pixels_1.show();
  }

  if (digitalRead(backward) == LOW){
    speed = 1440;

    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(255,0,0));
        pixels_1.show();
    }
//    
    while (digitalRead(backward) == LOW && !stop){
      if (speed <= 1380){
        speed = 1380;
      }
      leftMotor.writeMicroseconds(speed);
      rightMotor.writeMicroseconds(speed);
      speed = speed - 0.0005;

    }

    while (speed < 1475){
      leftMotor.writeMicroseconds(speed);
      rightMotor.writeMicroseconds(speed);
      speed = speed + 0.001;
    } 
    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(0,0,0));
        pixels_1.show();
    }
    pixels_1.show();
  }

  if (digitalRead(right) == LOW){

    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(0,255,0));
        pixels_1.show();
    }
    float speedL = 1560;
    float speedR = 1440;

    while (digitalRead(right) == LOW && !stop){
      if (speedL >= 1600){
        speedL = 1600; 
      }

      if (speedR <= 1400){
        speedR = 1400;
      }

      leftMotor.writeMicroseconds(speedL);
      rightMotor.writeMicroseconds(speedR);
      speedL = speedL + 0.002;
      speedR = speedR - 0.002;
    }
    
    while (speedL > 1525 && speedR < 1475){
      leftMotor.writeMicroseconds(speedL);
      rightMotor.writeMicroseconds(speedR);
      speedL = speedL - 0.001;
      speedR = speedR + 0.001;      
    }
    for( int i = 0; i<16; i++){
        pixels_1.setPixelColor(i, pixels_1.Color(0,0,0));
        pixels_1.show();
    }
    pixels_1.show();
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

