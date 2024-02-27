#include "HX711.h"
#include <Servo.h>

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 700
#define servoPin 9
byte potentiometerPin = A0;
Servo motor;
HX711 scale1;
HX711 scale3;
HX711 scale2;

int i;
float voltage, current;

int voltage_min, voltage_max;
int current_min, current_max;
int param3_min, param3_max;

int sensorValue, potValue, pwmValue;
bool HALT = false;

uint8_t dataPin1 = 2;
uint8_t clockPin1 = 3;

uint8_t dataPin2 = 4;
uint8_t clockPin2 = 5;

uint8_t dataPin3 = 6;
uint8_t clockPin3 = 7;

int LED = 10;
int ERR = 11;
const int analogInputPin = A7; // Connect ACS758 output to A7

float w1, w2, w3;
int delta;

float throttleValue = 1000.0; // Initial throttle value
float receivedThrottleValue = 0.0; // Variable to store received throttle value

void finddata() {
  sensorValue = analogRead(analogInputPin);
  potValue = analogRead(potentiometerPin);
  pwmValue = map(potValue, 0, 1023, 1000, 1900);
  sensorValue = sensorValue - 520 + 37;

  if (sensorValue < 0) {
    delta = abs(sensorValue);
    sensorValue = sensorValue + (2 * delta);
  }
  voltage = sensorValue * (5.0 / 1023.0);
  current = voltage / 0.08;

  w1 = scale1.get_units(10);
  w2 = scale2.get_units(10);
  w3 = scale3.get_units(10);
}

void usedata() {
  if (pwmValue <= 1120) {
    motor.writeMicroseconds(1000);
    voltage = 0;
    current = 0;
  } else {
    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\t');
      if (command == "SET_THROTTLE") {
        // Receive and process throttle value from the serial port
        String throttleString = Serial.readStringUntil('\n');
        receivedThrottleValue = throttleString.toFloat();
        if (receivedThrottleValue >= 1049 && receivedThrottleValue <= 1900) {
          motor.writeMicroseconds(receivedThrottleValue);
        }
        else if (receivedThrottleValue >= 1050){
          motor.writeMicroseconds(1000);
        }
        else if (receivedThrottleValue >= 1900){
          motor.writeMicroseconds(1900);
        }
          // Serial.print("Received Throttle Value: ");
          // Serial.println(receivedThrottleValue);
        
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  motor.attach(servoPin);
  delay(3000);

  motor.writeMicroseconds(1000);
  // delay(5000);
  // delay(2000);

  scale1.begin(dataPin1, clockPin1);
  scale2.begin(dataPin2, clockPin2);
  scale3.begin(dataPin3, clockPin3);

  pinMode(LED, OUTPUT);
  pinMode(ERR, OUTPUT);

  // Load cell factor calibration; thrust bench
  scale1.set_scale(125830);
  scale1.tare();

  scale2.set_scale(-878);
  scale2.tare();

  scale3.set_scale(150.645);
  scale3.tare();
}

void loop() {
  if (Serial.available() > 0) {
    String msg = Serial.readString();
    if (msg == "HALT") {
      HALT = !HALT;
    }
  }

  finddata();
  if (HALT == false) {
    digitalWrite(ERR, LOW);
    digitalWrite(LED, HIGH);
    usedata();
  } else {
    motor.writeMicroseconds(1000);
    digitalWrite(ERR, HIGH);
    digitalWrite(LED, HIGH);
  }

  float savedThrottleValue = throttleValue;

  if(receivedThrottleValue <= 1050){
    current = 0.0;
    voltage = 0.0;
  }
  Serial.print(voltage, 2);
  Serial.print("\t");
  Serial.print(current, 2);
  Serial.print("\t");
  Serial.print(w1);
  Serial.print("\t");
  Serial.print(w2);
  Serial.print("\t");
  Serial.println(w3);
  delay(1000);
}