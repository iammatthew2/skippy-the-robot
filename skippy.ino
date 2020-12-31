// sound setup
#include "CuteBuzzerSounds.h"

// servo driver setup
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwmServo = Adafruit_PWMServoDriver();
#define SERVO_MIN  150
#define SERVO_MAX  350
#define SERVO_MIDDLE 270
#define SERVO_FREQ 50
int lidState = SERVO_MIN; // SERVO_MIN, SERVO_MIDDLE, SERVO_MAX

// reserve pins
#define NEO_PIXEL_PIN 21
#define SOUND_SENSOR_PIN 13
#define BUTTON_LED_PIN 3
#define DISTANCE_TRIG_PIN 15
#define DISTANCE_ECHO_PIN 16
#define BUZZER_PIN 9
#define BUZZER_ALT_PIN 14
#define KILL_ROBOT_PIN 19

#define NUM_SOUNDS_UNTIL_OPEN 3

// nep pixel setup
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define NUM_PIXELS 4
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int waiter = 0;
int soundInstanceCounter = 0;

void setup() {
  Serial.begin(9600);
  cute.init(BUZZER_PIN);

  // pins
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(DISTANCE_TRIG_PIN, OUTPUT);
  pinMode(DISTANCE_ECHO_PIN, INPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);

  // servo
  pwmServo.begin();
  pwmServo.setOscillatorFrequency(27000000);
  pwmServo.setPWMFreq(SERVO_FREQ);
  servoGoTo(SERVO_MIN);

  // neo pixels
  pixels.begin();
  colorWipe(pixels.Color(0,0,0), 0);
  cute.play(S_CONNECTION);
  pwmServo.setPWM(8, 0, 100 );
  delay(1000);
  pwmServo.setPWM(8, 0, 450);
  delay(1000);
    pwmServo.setPWM(8, 0, 500);


}

void loop() {
  digitalWrite(BUTTON_LED_PIN, HIGH);
  if (lidState == SERVO_MIN) {
    handleLidDownState();
  }
  if (lidState == SERVO_MIDDLE) {
    handleLidMiddleState();
  }
  if (lidState == SERVO_MAX) {
    handleLidUpState();
  }
}

void handleLidDownState() {
  if (digitalRead(SOUND_SENSOR_PIN) == LOW) {
    soundInstanceCounter += 1;
    Serial.print("sound sensor triggered, count: ");
    Serial.println(soundInstanceCounter);

    digitalWrite(BUTTON_LED_PIN, LOW);
    cute.playRandom(SG_JOYFUL);
    digitalWrite(BUTTON_LED_PIN, HIGH);
    delay(250);
  
    if (soundInstanceCounter >= NUM_SOUNDS_UNTIL_OPEN) {
      delay(500);
      soundInstanceCounter = 0;

      // open the box mid-way, slowly
      servoGoTo(SERVO_MIDDLE, 15);
      cute.play(S_HAPPY);
      for(int i=0;i < NUM_PIXELS ;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,10));
        pixels.show();
      }
    }   
  }
}

void handleLidMiddleState() {
  if (digitalRead(SOUND_SENSOR_PIN) == LOW) {
    cute.playRandom(SG_JOYFUL);
    servoGoTo(SERVO_MAX, 7);
  }
}

void handleLidUpState() {
  if (getDistance() < 10) {
    for(int i=0;i < NUM_PIXELS ;i++){
      pixels.setPixelColor(i, pixels.Color(10,0,0));
      pixels.show();
    }
        
    Serial.println("too close - distance sensor deleted less than 10 cm");

    digitalWrite(BUTTON_LED_PIN, HIGH);
    cute.play(S_DISGRUNTLED);
    delay(250);

    // close the box
    servoGoTo(SERVO_MIN);

    delay(1000);
    colorWipe(pixels.Color(0,0,0), 0);  
  }

  for (waiter = 0; waiter <= 1; waiter += 1) {
    if (digitalRead(SOUND_SENSOR_PIN) == LOW && lidState == SERVO_MAX) {
        Serial.println("too loud - sound sensor triggered");
        digitalWrite(BUTTON_LED_PIN, HIGH);
        colorWipe(pixels.Color(random(20),random(20),random(20)), random(50));
        delay(random(250));
        cute.playRandom(SG_JOYFUL);
        colorWipe(pixels.Color(10,10,0), 0);
        digitalWrite(BUTTON_LED_PIN, LOW);
    }
    delay(10);
  }
}

float getDistance() {
  // Clears the DISTANCE_TRIG_PIN
  digitalWrite(DISTANCE_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the DISTANCE_TRIG_PIN on HIGH state for 10 micro seconds
  digitalWrite(DISTANCE_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(DISTANCE_TRIG_PIN, LOW);

  // calc distance based on assumed value of speed of sound 
  return pulseIn(DISTANCE_ECHO_PIN, HIGH) * 0.034 / 2;
}

void servoGoTo(int destination) {
  servoGoTo(destination, 0);
}
void servoGoTo(int destination, int rate) {
  servoGoFromTo(lidState, destination, rate);
}

void servoGoFromTo(int source, int destination, int rate) {
  if (rate == 0) {
    pwmServo.setPWM(4, 0, destination);
  } else if (source < destination) {
    // increment up to a higher servo position
    for (uint16_t pulselen = source; pulselen < destination; pulselen++) {
      pwmServo.setPWM(4, 0, pulselen);
      delay(rate);
    } 
  } else {
    // decrement down to a lower servo position
    for (uint16_t pulselen = source; pulselen > destination; pulselen--) {
      pwmServo.setPWM(4, 0, pulselen);
      delay(rate);
    }
  }
  lidState = destination;
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}
