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

int input_list[] = {11,12,13,22};
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

int forward_speed[][] = {{1540,1580},{1540,1600},{1530,1650}}
int backward_speed[][] = {{1460,1420},{1460,1400},{1460,1350}}

int forward = -1;
int backward = -1;
int left = -1;
int right = -1;

bool run = true;
boolean safemode = true;
int light_state = 0;

Servo leftMotor;
Servo rightMotor;

Adafruit_NeoPixel pixels_speed = Adafruit_NeoPixel(NUMPIXELS_2, ring_speed, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_battery = Adafruit_NeoPixel(NUMPIXELS_2, ring_battery, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_tail = Adafruit_NeoPixel(NUMPIXELS_1, ring_status, NEO_GRB + NEO_KHZ800);

IRrecv irr(ir);
decode_results results;

int number_of_inputs = 0;
void setup() {
  // initialize inputs
  pinMode(frontTrigPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);

  pinMode(backTrigPin, OUTPUT);
  pinMode(backEchoPin, INPUT);

  pinMode(first,INPUT_PULLUP);
  pinMode(second,INPUT_PULLUP);
  pinMode(third,INPUT_PULLUP);
  pinMode(fourth,INPUT_PULLUP);

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
   time_t t = now();
}

void loop() {
  if (minute(now()) - minute(t) >= 10){
    battery_light(analogRead(battery_level));
    t = now();
  }
  if (run){
    //neutral state
    setup_stage();
    leftMotor.writeMicroseconds(1510);
    rightMotor.writeMicroseconds(1510);

    //move forward
    if (forward != -1 && digitalRead(forward) == LOW){
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

    //move backward
    if (backward != -1 && digitalRead(backward) == LOW){
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

    //move left
    if (left != -1 && digitalRead(right) == LOW){
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

    //move right
    if (right != -1 && digitalRead(left) == LOW){
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

//Read from serial and setup
void setup_stage(){
  if (Serial.avialable() > 0){
    String message = Serial.readString();

    String v = message;
    int count = 1;

    while (v.indexOf("|") != -1){
      if (count == 1){
        number_of_inputs = int(v.substring(0,v.indexOf("|")));
      } else if (count == 2){
        speed_profile =  int(v.substring(0,v.indexOf("|")));
        if (speed_profile == 1){
          speed_profile = 0;
        } else if (speed_profile == 2){
          speed_profile = 1;
        } else{
          speed_profile = 2;
        }
        speed_light(speed_profile);
      } else if (count == 3){
        r =  int(v.substring(0,v.indexOf("|")));
        if (r == "On"){
          safemode = true;
        } else {
          safemode = false;
        }
      } else if (count == 4){
        String a = v.substring(0,v.indexOf("|"));
        for (int i = 0; i < number_of_inputs; i++){
          if (a.charAt(i) == 'F'){
            forward = input_list[i];
          }else if (a.charAt(i) == "B"){
            backward = input_list[i];
          } else if (a.charAt(i) == "L"){
            left = input_list[i];
          } else if (a.charAt(i) == "R"){
            right = input_list[i];
          }
        }
      }
    }
  }
}

void timerIsr(){
  trigger_pulse();
}

//incrementally light up the tail light neopixel
void proportial_light(int currentspeed, int max, int direction){
  int to_lit = (currentspeed/max)*NUMPIXELS_1;
  if (direction == 0){
      pixels_tail.setPixelColor(to_lit, pixels_tail.Color(0,150,0));
  } else if (direction == 1){
      pixels_tail.setPixelColor(to_lit, pixels_tail.Color(150,0,0));
  }
  pixels_tail.show();
}

//turn off neopixel
void reset_tail_light(){
  for (int i = 0; i < NUMPIXELS_1; i++){
    pixels_tail.setPixelColor(i, pixels_tail.Color(0,0,0));
  }
  pixels_tail.show();
}

//light up speed light
void speed_light(int speed_profile){
  if (speed_profile == 0){
    int color[] = {0,0,255};
  } else if (speed_profile == 1){
    int color[] = {0,150,0};
  } else if (speed_profile == 2) {
    int color[] = {238,130,238}
  }
  for (int i = 0; i < NUMPIXELS_2; i++){
    pixels_speed.setPixelColor(i, pixels_speed.Color(color[0],color[1],color[2]));
  }
  pixels_speed.show();
}

void battery_light(int batterylevel){
  if (batterylevel > 4.47){
    for (int i = 0; i < NUMPIXELS_2; i++){
      pixels_battery.setPixelColor(i, pixels_speed.Color(0,150,0));
    }
    pixels_battery.show();
  } else if (batterylevel > 4.32 and batterylevel <= 4.47){
    for (int i = 0; i < NUMPIXELS_2; i++){
      pixels_battery.setPixelColor(i, pixels_speed.Color(255,255,0));
    }
    pixels_battery.show();
  } else if (batterylevel <= 4.32){
    for (int i = 0; i < NUMPIXELS_2; i++){
      pixels_battery.setPixelColor(i, pixels_speed.Color(150,0,0));
    }
    pixels_battery.show();
  }
}

//blink red
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

//decode upcoming signal from IR sensors
void decode(){
  if (irr.decode(&results)){
    if (results.value == "C573E684" && run){
      run = false;
    } else if (results.value == "C573E684" && !run){
      run = true;
    }
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
      if (safemode){
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
      }
      break;
  }
}
