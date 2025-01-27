  #include "HX711.h"
  #include <Servo.h>
  #include <String.h>

  #define MAX_SIGNAL 2000
  #define MIN_SIGNAL 700
  #define servoPin 9
  byte potentiometerPin = A0;
  Servo motor;
  HX711 scale1;
  HX711 scale3;
  HX711 scale2;

  String msg;

  int i;
  float voltage, current;

  float voltage_min = 0;
  float voltage_max = 1000;
  float current_min = 0;
  float current_max = 1000;
  // int param3_min, param3_max;

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
  const int analogInputPin = A7;  

  float w1, w2, w3;
  int delta;

  float a, b, c, d;
  float throttleValue = 1000.0;       
  float receivedThrottleValue = 0.0;  

  // Define  limits
  float voltage_limit_min = 0.0;
  float voltage_limit_max = 0.0;  
  float current_limit_min = 0.0;
  float current_limit_max = 0.0;  

  char msgBuffer[50];
  char *token;


void processLimitsMessage(String msg) {
  
  // Convert String to char array
  char msgBuffer[50];
  msg.toCharArray(msgBuffer, sizeof(msgBuffer));

  // Tokenize message
  char *token = strtok(msgBuffer, " ");
  float tokens[4]; 
  int i = 0;
  while (token != NULL && i <= 4) {
    // Convert token to float 
    tokens[i] = atof(token);
    // Serial.print("Token ");
    // Serial.print(i);
    // Serial.print(": ");
    // Serial.println(tokens[i], 3); 
    token = strtok(NULL, " ");
    i++;
  }

  voltage_limit_min = tokens[1];
  voltage_limit_max = tokens[2];
  current_limit_min = tokens[3];
  current_limit_max = tokens[4];
  // Serial.print(voltage_limit_max);
  
}





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
  voltage = voltage / 4;
  current = voltage / 0.15;

  // current = current / 10;

  if (voltage < voltage_min || voltage > voltage_max || current < current_min || current > current_max) {
    motor.writeMicroseconds(1000);
    digitalWrite(ERR, HIGH);
    digitalWrite(LED, HIGH);
  }

  w1 = scale1.get_units(10);
  w2 = scale2.get_units(10);
  w3 = scale3.get_units(10);
}

void usedata() {
  if (pwmValue <= 1120) {
    motor.writeMicroseconds(1000);
    voltage = 0;
    current = 0;
  }

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil(' ');
    if (command == "SET_THROTTLE") {
      String throttleString = Serial.readStringUntil('\n');
      receivedThrottleValue = throttleString.toFloat();
      if (receivedThrottleValue >= 1049 && receivedThrottleValue <= 1900) {
        motor.writeMicroseconds(receivedThrottleValue);
      } else if (receivedThrottleValue <= 1050) {
        motor.writeMicroseconds(1000);
      } else if (receivedThrottleValue >= 1900) {
        motor.writeMicroseconds(1900);
      }
    }
  }
  if (voltage < voltage_min || voltage > voltage_max || current < current_min || current > current_max) {
    digitalWrite(ERR, HIGH);
    digitalWrite(LED, HIGH);
  }

  else {
    motor.writeMicroseconds(receivedThrottleValue);
    digitalWrite(ERR, LOW);
    digitalWrite(LED, HIGH);
  }
}


  void setup() {
    Serial.begin(9600);
    motor.attach(servoPin);
    delay(3000);

    motor.writeMicroseconds(1000);

    scale1.begin(dataPin1, clockPin1);
    scale2.begin(dataPin2, clockPin2);
    scale3.begin(dataPin3, clockPin3);

    pinMode(LED, OUTPUT);
    pinMode(ERR, OUTPUT);

    scale1.set_scale(114980);
    scale1.tare();

    scale2.set_scale(-878);
    scale2.tare();

    scale3.set_scale(95);  // 150.645 for the older sensor
    scale3.tare();
  }

  void loop() {
    if (Serial.available() > 0) {
      String msg = Serial.readString();
      // Serial.print("Received message: ");
      // Serial.println(msg);  // Debug print

      if (msg == "HALT") {
        HALT = !HALT;
      }

      else if (msg.startsWith("LIMITS")) {
        // Serial.println("entered LIMITS");
        processLimitsMessage(msg);
      }

    }
    finddata();

    if (HALT == true){
      motor.writeMicroseconds(1000);
      digitalWrite(ERR, HIGH);
    }
    else if (HALT == false) {
      digitalWrite(ERR, LOW);
      digitalWrite(LED, HIGH);
      usedata();
    

    float savedThrottleValue = throttleValue;

    if (receivedThrottleValue <= 1050) {
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
  }
