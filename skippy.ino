// sound setup
#include "CuteBuzzerSounds.h"

const int curiousSounds[]={3,4,5,13,1,2};
const int curiousSoundsLength = 5;

// servo driver setup
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwmServo = Adafruit_PWMServoDriver();
#define SERVOMIN  150
#define SERVOMAX  350
#define SERVOMIDDLE 225
#define SERVO_FREQ 50

// reserve pins
#define NEO_PIXEL_PIN 21
#define SOUND_SENSOR_PIN 13
#define BUTTON_LED_PIN 3
#define DISTANCE_TRIG_PIN 15
#define DISTANCE_ECHO_PIN 16
#define BUZZER_PIN 11
#define KILL_ROBOT_PIN 19

#define NUM_SOUNDS_UNTIL_OPEN 3

// nep pixel setup
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define NUM_PIXELS 4
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int lidState = 9; // 0 closed, 1 half, 2 open, 9 unset
int pos = 0;
int waiter = 0;
int soundInstanceCounter = 0;
long durationCheck;
int distance;
int lastDistance = 60;
bool lightsOn = false;


void setup() {
  cute.init(BUZZER_PIN);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(DISTANCE_TRIG_PIN, OUTPUT);
  pinMode(DISTANCE_ECHO_PIN, INPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pwmServo.begin();
  pwmServo.setOscillatorFrequency(27000000);
  pwmServo.setPWMFreq(SERVO_FREQ);
  pixels.begin();
  Serial.begin(9600);
  colorWipe(pixels.Color(0,0,0), 0);
}

void loop() {
  cute.playRandom(SG_UNHAPPY);
  delay(100000);
  return;
 
  digitalWrite(BUTTON_LED_PIN, HIGH);
  if (lidState == 0) {
    if (digitalRead(SOUND_SENSOR_PIN) == LOW) {
      soundInstanceCounter += 1;
      Serial.print("sound sensor triggered, count: ");
      Serial.println(soundInstanceCounter);
      digitalWrite(BUTTON_LED_PIN, LOW);
      delay(500);
      digitalWrite(BUTTON_LED_PIN, HIGH);

      cute.play(curiousSounds[random(curiousSoundsLength)]);

      if (soundInstanceCounter >= NUM_SOUNDS_UNTIL_OPEN) {
        soundInstanceCounter = 0;

        Serial.println("wake up");
        // go to state 1
        servoGoFromTo(SERVOMIN, SERVOMIDDLE, 15);
        lidState = 1;
        delay(2000);
      }   
    }
  }

  if (lidState == 1) {
    cute.play(S_HAPPY);
     for(int i=0;i < NUM_PIXELS ;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,10));
      pixels.show();
    }
    // go to state 2
    servoGoFromTo(SERVOMIDDLE, SERVOMAX, 15);
    lidState = 2;
    delay(2000);
  }

  if (lidState == 2) {
    digitalWrite(BUTTON_LED_PIN, LOW);

     // Clears the DISTANCE_TRIG_PIN
    digitalWrite(DISTANCE_TRIG_PIN, LOW);
    delayMicroseconds(2);
    // Sets the DISTANCE_TRIG_PIN on HIGH state for 10 micro seconds
    digitalWrite(DISTANCE_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(DISTANCE_TRIG_PIN, LOW);
    
    durationCheck = pulseIn(DISTANCE_ECHO_PIN, HIGH);
    distance = durationCheck*0.034/2;

    if (distance < 10) {
      for(int i=0;i < NUM_PIXELS ;i++){
        pixels.setPixelColor(i, pixels.Color(10,0,0));
        pixels.show();
      }
          
      Serial.println("too close - distance sensor deteted less than 10 cm");

      digitalWrite(BUTTON_LED_PIN, HIGH);
      cute.play(S_DISGRUNTLED);
      delay(250);
      servoGoFromTo(SERVOMAX, SERVOMIN, 0);
      lidState = 0;
      delay(4000);
      colorWipe(pixels.Color(0,0,0), 0);  
    }

    for (waiter = 0; waiter <= 1; waiter += 1) {
      if (digitalRead(SOUND_SENSOR_PIN) == LOW && lidState == 2) {
          Serial.println("too loud - sound sensor triggered");
          digitalWrite(BUTTON_LED_PIN, HIGH);
          colorWipe(pixels.Color(random(20),random(20),random(20)), random(50));
          delay(random(250));
          cute.play(curiousSounds[random(curiousSoundsLength)]);
          colorWipe(pixels.Color(10,10,0), 0);
          digitalWrite(BUTTON_LED_PIN, LOW);
      }
      delay(10);
    }
  }

  if (lidState == 9) {
    servoGoFromTo(SERVOMAX, SERVOMIN, 10);
    lidState = 0;
    cute.play(S_JUMP);
    delay(200);
    cute.play(S_OHOOH2);
    cute.play(S_OHOOH2);
    cute.play(S_OHOOH2);
  }  
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}


void servoGoFromTo(int source, int destination, int rate) {
  if (source < destination) {
    for (uint16_t pulselen = source; pulselen < destination; pulselen++) {
      pwmServo.setPWM(4, 0, pulselen);
      delay(rate);
    } 
  } else {
    for (uint16_t pulselen = source; pulselen > destination; pulselen--) {
      pwmServo.setPWM(4, 0, pulselen);
      delay(rate);
    }
  }
}
