#include <CuteBuzzerSounds.h> // see: https://github.com/GypsyRobot/CuteBuzzerSounds
#include <Sounds.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwmServo = Adafruit_PWMServoDriver();
#define SERVOMIN  150
#define SERVOMAX  350
#define SERVOMIDDLE 225
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, 6, NEO_GRB + NEO_KHZ800);

int soundSensorPin = 13;
int lidState = 9; // 0 closed, 1 half, 2 open, 9 unset
const int curiousSounds[]={3,4,5,13,1,2};
const int curiousSoundsLength = 5;
int pos = 0;
int waiter = 0;
int soundInstanceCounter = 0;

const int trigPin = 15;
const int echoPin = 16;

long durationCheck;
int distance;
int lastDistance = 60;
bool lightsOn = false;
#define BUZZER_PIN 14

void setup() {
  cute.init(BUZZER_PIN);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(soundSensorPin, INPUT);
  pwmServo.begin();
  pwmServo.setOscillatorFrequency(27000000);
  pwmServo.setPWMFreq(SERVO_FREQ);
  pixels.begin();
  Serial.begin(9600);
  Serial.println("setup");
}

void loop() {      
  if (lidState == 0) {
    if (digitalRead(soundSensorPin)) {
      Serial.println("sound sensor triggered one time - when count is 5 then action happens");
      soundInstanceCounter += 1;
      delay(500);
      cute.play(curiousSounds[random(curiousSoundsLength)]);

      if (soundInstanceCounter >= 5) {
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

    // go to state 2
    servoGoFromTo(SERVOMIDDLE, SERVOMAX, 15);
    lidState = 2;
    delay(2000);
  }

  if (lidState == 2) {
     // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    durationCheck = pulseIn(echoPin, HIGH);
    distance = durationCheck*0.034/2;

    if (distance < 10) {
      Serial.println("too close - distance sensor deteted less than 10 cm");
      servoGoFromTo(SERVOMAX, SERVOMIN, 0);
      lidState = 0;
      delay(4000);
    }

    for (waiter = 0; waiter <= 1; waiter += 1) {
      if (digitalRead(soundSensorPin)) {
        Serial.println("too loud - sound sensor triggered");
        servoGoFromTo(SERVOMAX, SERVOMIN, 0);
        lidState = 0;
        delay(4000);
        break;
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
    delay(3000);
    cute.play(S_HAPPY_SHORT);
  }





















  
// extra stuff ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 
  if (lidState == 99) {
    
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  durationCheck = pulseIn(echoPin, HIGH);
  
  distance= durationCheck*0.034/2;

  if (distance < 50 ) {
    if (abs(distance - lastDistance) > 5) {
//      servo.write(distance * 3); //directs servo to go to position in variable 'angle'
      lastDistance = distance;
  
     
    }
//    tone(14, map(distance, 0, 50, 2000, 500));
  }
  if (distance < 70 && !lightsOn) {
     for(int i=0;i<24;i++){
      pixels.setPixelColor(i, pixels.Color(255,100,0));
      pixels.show();
    }
    lightsOn = true;
  } else if (lightsOn) {
    for(int i=0;i<24;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0));
      pixels.show();
    }
        lightsOn = false;
  }
  
  
  delay(300);
  noTone(14);
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
