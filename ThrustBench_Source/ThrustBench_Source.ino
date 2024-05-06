#include "HX711.h"
#include <Servo.h>
#include <Robojax_WCS.h>
#include <String.h>

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 700
#define servoPin 9

#define MAX_VALUES 10  // Maximum number of values to store in the list (adjust as needed)

int values[MAX_VALUES];  // Array to store the parsed values
int valueIndex = 0;  // Index to keep track of the next available position in the array

// byte potentiometerPin = A4;
Servo motor;
HX711 scale1;
HX711 scale3;
HX711 scale2;

String msg;

int i;
float voltage, current;

int abc = 0;

float voltage_min = 0;
float voltage_max = 1000;
float current_min = 0;
float current_max = 1000;
// int param3_min, param3_max;

int sensorValue, potValue, pwmValue;
bool HALT = false;
float vcc = 5.0;

uint8_t dataPin1 = 2;
uint8_t clockPin1 = 3;

uint8_t dataPin2 = 4;
uint8_t clockPin2 = 5;

uint8_t dataPin3 = 6;
uint8_t clockPin3 = 7;

int LED = 10;
int ERR = 11;
// const int analogInputPin = A0;  // Connect ACS758 output to A7

float w1, w2, w3;
int delta;

float a, b, c, d;
float throttleValue = 1000.0;       // Initial throttle value
float receivedThrottleValue = 0.0;  // Variable to store received throttle value

// Define  limits
float voltage_limit_min = 0.0;
float voltage_limit_max = 0.0;  // Adjust the maximum limit based on your application
float current_limit_min = 0.0;
float current_limit_max = 0.0;  // Adjust the maximum limit based on your application


//WCS1700 Variables
#define MODEL 11 //see list above
#define SENSOR_PIN A0 //pin for reading sensor
#define SENSOR_VCC_PIN 8 //pin for powring up the sensor
#define ZERO_CURRENT_LED_PIN 13 //zero current LED pin

#define ZERO_CURRENT_WAIT_TIME 5000 //wait for 5 seconds to allow zero current measurement
#define CORRECTION_VLALUE 2150 //mA
#define MEASUREMENT_ITERATION 100
#define VOLTAGE_REFERENCE  5000.0 //5000mv is for 5V
#define BIT_RESOLUTION 10
#define DEBUT_ONCE true
Robojax_WCS sensor(
        MODEL, SENSOR_PIN, SENSOR_VCC_PIN, 
        ZERO_CURRENT_WAIT_TIME, ZERO_CURRENT_LED_PIN,
        CORRECTION_VLALUE, MEASUREMENT_ITERATION, VOLTAGE_REFERENCE,
        BIT_RESOLUTION, DEBUT_ONCE           
        );
char msgBuffer[50]; // Buffer to hold the converted string
char *token;

// Function to process LIMITS message

void processLimitsMessage(String msg) {
// Serial.println("Received message: " + msg);

// Convert String to char array
char msgBuffer[50];
msg.toCharArray(msgBuffer, sizeof(msgBuffer));

// Tokenize the message
char *token = strtok(msgBuffer, " ");
float tokens[4]; // Array to hold tokens as floats
int i = 0;
while (token != NULL && i <= 4) {
  // Convert token to float and store it in the tokens array
  tokens[i] = atof(token);
  // Serial.print("Token ");
  // Serial.print(i);
  // Serial.print(": ");
  // Serial.println(tokens[i], 3); // Print with 3 decimal places
  token = strtok(NULL, " ");
  i++;
}



voltage_limit_min = tokens[1];
voltage_limit_max = tokens[2];
current_limit_min = tokens[3];
current_limit_max = tokens[4];
// Serial.print(voltage_limit_max);

// Now 'tokens' array holds the tokens as floats, you can use it as needed
}


void finddata() {
  current = analogRead(A0);
  current = (current - 625);

w1 = scale1.get_units(10);
w2 = scale2.get_units(10);
w3 = scale3.get_units(10);
}

void extractNumbers(String data, float& num1, float& num2, float& num3, float& num4, float& num5, float& num6, float& num7) {
  // Find the index of the first space to skip the command part (e.g., "CUSTOM")
  int startIndex = data.indexOf(' ') + 1;
  String stripped = data.substring(startIndex);

  // Parse the stripped string to extract numbers
  int currentIndex = 0;
  int commaIndex;
  int numIndex = 0;

  // Extract up to 7 numbers
  while (numIndex < 7 && currentIndex < stripped.length()) {
    commaIndex = stripped.indexOf(',', currentIndex);

    if (commaIndex == -1) {
      commaIndex = stripped.length(); // No more commas, take till end
    }

    String currentNumberStr = stripped.substring(currentIndex, commaIndex);
    float extractedNumber = currentNumberStr.toFloat();

    // Assign the extracted number to the appropriate variable
    switch (numIndex) {
      case 0:
        num1 = extractedNumber;
        break;
      case 1:
        num2 = extractedNumber;
        break;
      case 2:
        num3 = extractedNumber;
        break;
      case 3:
        num4 = extractedNumber;
        break;
      case 4:
        num5 = extractedNumber;
        break;
      case 5:
        num6 = extractedNumber;
        break;
      case 6:
        num7 = extractedNumber;
        break;
    }

    numIndex++;
    currentIndex = commaIndex + 1; // Move past the comma
  }
}

void throttleControl(float min_throttle, float max_throttle, float total_duration, float rise_time, float fall_time, float step_count, float step_duration) {
 
  
  float throttleStep = (max_throttle - min_throttle) / step_count;

  for (float throttle = min_throttle; throttle <= max_throttle; throttle += throttleStep) {
    motor.writeMicroseconds(throttle);
    delay(step_duration * 1000); // Convert step_duration from seconds to milliseconds
  }

  // Hold maximum throttle for rise_time (if specified)
  if (rise_time > 0) {
    motor.writeMicroseconds(max_throttle);
    delay(rise_time * 1000); // Convert rise_time from seconds to milliseconds
  }

  // Gradually decrease throttle
  for (float throttle = max_throttle; throttle >= min_throttle; throttle -= throttleStep) {
    motor.writeMicroseconds(throttle);
    delay(step_duration * 1000); // Convert step_duration from seconds to milliseconds
  }

  // Hold minimum throttle for fall_time (if specified)
  if (fall_time > 0) {
    motor.writeMicroseconds(min_throttle);
    delay(fall_time * 1000); // Convert fall_time from seconds to milliseconds
  }

  // Stop ESC (set throttle to minimum)
  motor.writeMicroseconds(1000);
  finddata();
  Serial.print(voltage, 2);
  Serial.print("\t");
  Serial.print(current, 2);
  Serial.print("\t");
  Serial.print(w1);
  Serial.print("\t");
  Serial.print(w2);
  Serial.print("\t");
  Serial.println(w3);
  
}

void usedata() {
if (Serial.available() > 0) {
    String command = Serial.readStringUntil(' '); // Read command until space

    if (command == "SET_THROTTLE") {
      // Receive and process throttle value from the serial port
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
    else if (command == "CUSTOM") {
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);

    if (Serial.available() > 0) {
      String data = Serial.readStringUntil('\n');
      float min_throttle = 0.0;
      float max_throttle = 0.0;
      float total_duration = 0.0;
      float rise_time = 0.0;
      float fall_time = 0.0;
      float step_count = 0.0;
      float step_duration = 0.0;
      extractNumbers(data, min_throttle, max_throttle, total_duration, rise_time, fall_time, step_count, step_duration);
      throttleControl(min_throttle, max_throttle, total_duration, rise_time, fall_time, step_count, step_duration);
      }
    
    }

}
  // Check voltage and current limits
  if (voltage < voltage_min || voltage > voltage_max || current < current_min || current > current_max) {
    // Stop the motor if any value exceeds the limits
    digitalWrite(LED, HIGH);
  } else {
    // Clear ERR LED and run the motor with the received throttle value
    digitalWrite(ERR, LOW);
    digitalWrite(LED, LOW);
    motor.writeMicroseconds(receivedThrottleValue);
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

  // Load cell factor calibration; thrust bench
  scale1.set_scale(114980);
  scale1.tare();

  scale2.set_scale(-878);
  scale2.tare();

  scale3.set_scale(95);  // 150.645 for the older sensor
  scale3.tare();
}

void loop() {
  

  if (Serial.available() > 0) {
    String command = Serial.readString();
    
    if (command == "HALT") {
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
    ////digitalWrite(ERR, HIGH);
  }
  else if (HALT == false) {
    ////digitalWrite(ERR, LOW);
    //digitalWrite(LED, HIGH);
    usedata();
  

    // Check if values exceed limits


  float savedThrottleValue = throttleValue;


  current = (abs(current) / 10);
  if (receivedThrottleValue < 1500){
    current = abs(current - 3);
  }
  // else if (receivedThrottleValue >= 1500 && receivedThrottleValue < 1700){
  //   current = abs(current) + 3;
  // }
  else if (receivedThrottleValue >= 1700){
    current = abs(current) + 6;
  }
  // current = abs(current - 2);
  // current = abs(current-27);
  voltage = current * 0.33;

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
  delay(200);
}
}
