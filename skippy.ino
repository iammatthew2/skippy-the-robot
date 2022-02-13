// sound setup
#include "CuteBuzzerSounds.h"

// servo driver setup
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
Adafruit_PWMServoDriver pwmServo = Adafruit_PWMServoDriver();
#define SERVO_MIN 150
#define SERVO_MAX 350
#define SERVO_MIDDLE 310
#define SERVO_FREQ 50
#define SERVO_ID_LID_LIFT 4
#define SERVO_ID_ROTATE 8
int lidState = SERVO_MIN;  // SERVO_MIN, SERVO_MIDDLE, SERVO_MAX
#define ROTATION_LEFT_MAX 150
#define ROTATION_LEFT_PARTIAL 225
#define ROTATION_RIGHT_MAX 450
#define ROTATION_RIGHT_PARTIAL 375
#define ROTATION_MIDDLE 300
int rotationState = ROTATION_MIDDLE;

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

#define TRACKING_NOT_TRACKING 0
#define TRACKING_ACQUIRED 1
#define TRACKING_LOST 2
#define SEEK_LEFT 0
#define SEEK_RIGHT 1
int motionTrackingState = 0;          // 0, 1, 2: not tracking, acquired, lost
int motionTrackingSeekDirection = 0;  // 0, 1: left, right
int distanceOfCaptive;
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
  servoGoTo(SERVO_ID_LID_LIFT, SERVO_MIN);
  servoGoTo(SERVO_ID_ROTATE, ROTATION_MIDDLE);

  // neo pixels
  pixels.begin();
  colorWipe(pixels.Color(0, 0, 0), 0);
  cute.play(S_CONNECTION);
}

void loop() {
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
    if (soundInstanceCounter == 2) {
      servoGoTo(SERVO_ID_ROTATE, ROTATION_LEFT_PARTIAL, 5);
      servoGoTo(SERVO_ID_ROTATE, ROTATION_RIGHT_PARTIAL, 5);
      servoGoTo(SERVO_ID_ROTATE, ROTATION_MIDDLE, 5);
    }
    if (soundInstanceCounter >= NUM_SOUNDS_UNTIL_OPEN) {
      delay(500);
      soundInstanceCounter = 0;

      // open the box mid-way, slowly
      servoGoTo(SERVO_ID_LID_LIFT, SERVO_MIDDLE, 15);
      cute.play(S_HAPPY);
      colorWipe(pixels.Color(0, 0, 10), 0);
    }
  }
}

void handleLidMiddleState() {
  if (getDistance() < 10) {
    closeLid();
    return;
  }

  if (digitalRead(SOUND_SENSOR_PIN) == LOW) {
    cute.playRandom(SG_JOYFUL);
    servoGoTo(SERVO_ID_LID_LIFT, SERVO_MAX, 7);
  }
}

void handleLidUpState() {
  handleMotionTracking();

  if (digitalRead(SOUND_SENSOR_PIN) == LOW) {
    digitalWrite(BUTTON_LED_PIN, HIGH);
    colorWipe(pixels.Color(random(20), random(20), random(20)), random(50));
    delay(random(250));
    cute.playRandom(SG_JOYFUL);
    digitalWrite(BUTTON_LED_PIN, LOW);
  }

  if (getDistance() < 10) {
    closeLid();
  }
}

void handleMotionTracking() {
  int targetRotationState;
  int distanceCheck = getDistance();
  if (motionTrackingState == TRACKING_NOT_TRACKING && distanceCheck < 50 && distanceCheck > 10) {
    colorWipe(pixels.Color(75, 0, 0), 0);
    cute.play(S_OHOOH2);
    motionTrackingState = TRACKING_ACQUIRED;
    return;
  }

  if (distanceCheck < 10) {
    motionTrackingState = TRACKING_NOT_TRACKING;
    closeLid();
    return;
  }

  if (motionTrackingState == TRACKING_LOST) {
    // we're seeking left: keep making the numbers smaller
    if (motionTrackingSeekDirection == SEEK_LEFT) {
      targetRotationState = rotationState - 1;
      servoGoTo(SERVO_ID_ROTATE, targetRotationState, 5);
      if (targetRotationState - 5 < ROTATION_LEFT_MAX) {
        motionTrackingSeekDirection = SEEK_RIGHT;
      }

    } else {
      //we're seeking right
      targetRotationState = rotationState + 1;
      servoGoTo(SERVO_ID_ROTATE, targetRotationState, 5);
      if (targetRotationState + 5 > ROTATION_RIGHT_MAX) {
        motionTrackingSeekDirection = SEEK_LEFT;
      }
    }
    if (distanceCheck < 50) {
      colorWipe(pixels.Color(75, 0, 0), 0);
      motionTrackingState = TRACKING_ACQUIRED;
    }
    return;
  }
  if (motionTrackingState == TRACKING_ACQUIRED && distanceCheck > 50) {
    // take our time confirming that we've lost the target
    delay(650);
    if (getDistance() > 50) {
      colorWipe(pixels.Color(0, 75, 0), 0);
      motionTrackingState = TRACKING_LOST;
    }
  }
}

void servoGoTo(int servoName, int destination) {
  servoGoTo(servoName, destination, 0);
}
void servoGoTo(int servoName, int destination, int rate) {
  int currentState = servoName == SERVO_ID_LID_LIFT ? lidState : rotationState;
  _servoGoFromTo(servoName, currentState, destination, rate);
}

void _servoGoFromTo(int servoName, int source, int destination, int rate) {
  if (servoName != SERVO_ID_LID_LIFT && servoName != SERVO_ID_ROTATE) {
    Serial.print("Error - servoName invalid for _servoGoFromTo: ");
    Serial.println(servoName);
    return;
  }

  if (servoName == SERVO_ID_ROTATE && (destination < ROTATION_LEFT_MAX || destination > ROTATION_RIGHT_MAX)) {
    Serial.print("destination value out of bounds for rotation: ");
    Serial.println(destination);
    return;
  }

  if (servoName == SERVO_ID_LID_LIFT && (destination < SERVO_MIN || destination > SERVO_MAX)) {
    Serial.print("destination value out of bounds for lid lift: ");
    Serial.println(destination);
    return;
  }

  if (rate == 0) {
    pwmServo.setPWM(servoName, 0, destination);
  } else if (source < destination) {
    // increment up to a higher servo position
    for (uint16_t pulselen = source; pulselen < destination; pulselen++) {
      pwmServo.setPWM(servoName, 0, pulselen);
      delay(rate);
    }
  } else {
    // decrement down to a lower servo position
    for (uint16_t pulselen = source; pulselen > destination; pulselen--) {
      pwmServo.setPWM(servoName, 0, pulselen);
      delay(rate);
    }
  }

  if (servoName == SERVO_ID_LID_LIFT) {
    lidState = destination;
  } else {
    rotationState = destination;
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

void closeLid() {
  colorWipe(pixels.Color(10, 0, 0), 0);

  digitalWrite(BUTTON_LED_PIN, HIGH);
  cute.play(S_DISGRUNTLED);
  delay(250);

  // close the box
  servoGoTo(SERVO_ID_LID_LIFT, SERVO_MIN);
  // move the box back to the middle
  servoGoTo(SERVO_ID_ROTATE, ROTATION_MIDDLE);

  delay(1000);
  colorWipe(pixels.Color(0, 0, 0), 0);
}

/**
 * Sets all the NeoPixels to the same color with a delay between each
 * @param color - 32 RGB color
 * @param wait - time in ms to delay each light lighting up
 */
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}