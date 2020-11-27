#include <CuteBuzzerSounds.h> // see: https://github.com/GypsyRobot/CuteBuzzerSounds
#include <Sounds.h>
// S_CONNECTION   S_DISCONNECTION S_BUTTON_PUSHED   
// S_MODE1        S_MODE2         S_MODE3     
// S_SURPRISE     S_OHOOH         S_OHOOH2    
// S_CUDDLY       S_SLEEPING      S_HAPPY     
// S_SUPER_HAPPY  S_HAPPY_SHORT   S_SAD       
// S_CONFUSED     S_FART1         S_FART2     
// S_FART3        S_JUMP 20


#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
int soundSensorPin = 9;

int lidState = 9; // 0 closed, 1 half, 2 open, 9 unset

Servo servo;
int pos = 0;
int waiter = 0;
int soundInstanceCounter = 0;
const int servoBottom = 30;
const int servoHalf = 80;
const int servoTop = 105; 

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, 6, NEO_GRB + NEO_KHZ800);

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

  servo.attach(17);
  pixels.begin();
  Serial.begin(9600);
}
void loop() {
  if (lidState == 0) {

    if (digitalRead(soundSensorPin)) {
      soundInstanceCounter += 1;
      delay(500);
       cute.play(S_CUDDLY);

      if (soundInstanceCounter >= 5) {
        soundInstanceCounter = 0;

        Serial.print("wake up");
        // go to state 1
        for (pos = servoBottom; pos <= servoHalf; pos += 1) {
          servo.write(pos);
          delay(15);
        }
        lidState = 1;
        delay(2000);
      }   
    }
  }

  if (lidState == 1) {
      cute.play(S_HAPPY);

    // go to state 2
    for (pos = servoHalf; pos <= servoTop; pos += 1) {
      servo.write(pos);             
      delay(15);                      
    }
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

    if (distance < 5) {
      servo.write(servoBottom);
      lidState = 0;
      delay(4000);
    }

    for (waiter = 0; waiter <= 10; waiter += 1) {
      if (digitalRead(soundSensorPin)) {
        Serial.print("too loud!");
        servo.write(servoBottom);
        lidState = 0;
        delay(4000);
        break;
      }
      delay(10);
    }
    


  }

  if (lidState == 9) {
    servo.write(servoBottom);
    lidState = 0;
    cute.play(S_MODE2);
    delay(200);
    cute.play(S_MODE1);
    delay(600);
    cute.play(S_MODE3);



           // S_CONNECTION   S_DISCONNECTION S_BUTTON_PUSHED   
// S_MODE1        S_MODE2         S_MODE3     
// S_SURPRISE     S_OHOOH         S_OHOOH2    
// S_CUDDLY       S_SLEEPING      S_HAPPY     
// S_SUPER_HAPPY  S_HAPPY_SHORT   S_SAD       
// S_CONFUSED     S_FART1         S_FART2     
// S_FART3        S_JUMP 20



    delay(4000);
  }




// extra stuff
  if (lidState == 99) {


    for (waiter = 0; waiter <= 10; waiter += 1) {
      Serial.println("about to listen");
      if (digitalRead(soundSensorPin)) {
        Serial.print("too loud!");
        servo.write(servoBottom);
        lidState = 0;
        delay(4000);
        break;
      }
      delay(10);
    }



    
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
      servo.write(distance * 3); //directs servo to go to position in variable 'angle'
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
