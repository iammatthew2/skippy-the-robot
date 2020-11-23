#include <Servo.h>
Servo servo;

const int trigPin = 11;
const int echoPin = 12;

long duration;
int distance;
int lastDistance = 60;
void setup() {
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input
servo.attach(10);
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
Serial.print("Distance: ");
Serial.print(distance);
Serial.print(" - tone: ");

Serial.println(map(distance, 0, 50, 2000, 500));
//tone(14, 500);
if (distance < 50 ) {
  if (abs(distance - lastDistance) > 5) {
    servo.write(distance * 3); //directs servo to go to position in variable 'angle'
    lastDistance = distance;
  }
  tone(14, map(distance, 0, 50, 2000, 500));
  }
//tone(14, map(distance, 0, 50, 2000, 500));

//
delay(300);
noTone(14);
}
