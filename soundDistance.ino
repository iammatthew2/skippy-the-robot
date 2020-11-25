#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

Servo servo;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, 6, NEO_GRB + NEO_KHZ800);

const int trigPin = 11;
const int echoPin = 12;

long duration;
int distance;
int lastDistance = 60;
bool lightsOn = false;
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  servo.attach(10);
  pixels.begin();
  Serial.begin(9600); // Starts the serial communication
}
void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  distance= duration*0.034/2;

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
