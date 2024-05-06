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
  // Serial.print(current);
  // Serial.print("\t"); 
// sensorValue = analogRead(analogInputPin);
// potValue = analogRead(potentiometerPin);
// pwmValue = map(potValue, 0, 1023, 1000, 1900);
// sensorValue = sensorValue - 520 + 37;




// float voltage_raw =   (5.0 / 1023.0)* sensorValue;
// float QOV = 0.5 * 5;
// float FACTOR = 40 / 100;// Read the voltage from sensor
// voltage =  voltage_raw - QOV + 0.007 ;// 0.007 is a value to make voltage zero when there is no current
// float current = voltage / FACTOR;


// voltage = (5.0/1023.0) * sensorValue;
// voltage = voltage - (vcc * 0.5) + 0.007;
// current = voltage / 0.02;

// voltage = (sensorValue - 2.5) * (50.0 / 2.0);
// voltage = sensorValue * (5.0 / 1023.0);
// voltage = (voltage/4);
// current = voltage / 0.15;

// current = current / 10;

// if (voltage < voltage_min || voltage > voltage_max || current < current_min || current > current_max) {
//   motor.writeMicroseconds(1000);
//   //digitalWrite(ERR, HIGH);
//   digitalWrite(LED, HIGH);
// }

w1 = scale1.get_units(10);
w2 = scale2.get_units(10);
w3 = scale3.get_units(10);
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
      String data = Serial.readStringUntil('\n');  // Read data until newline character

      // Find the index of the first space to skip the command part (e.g., "CUSTOM")
      int startIndex = data.indexOf(' ') + 1;
      String stripped = data.substring(startIndex);

      // Variables to store up to 7 extracted numbers
      float number1 = 0.0;
      float number2 = 0.0;
      float number3 = 0.0;
      float number4 = 0.0;
      float number5 = 0.0;
      float number6 = 0.0;
      float number7 = 0.0;

      // Parse the stripped string to extract numbers
      int currentIndex = 0;
      int commaIndex = stripped.indexOf(',');

      // Extract the first number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number1 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number1 = currentNumberStr.toFloat();
      }

      // Extract the second number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number2 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number2 = currentNumberStr.toFloat();
      }

      // Extract the third number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number3 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number3 = currentNumberStr.toFloat();
      }

      // Extract the fourth number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number4 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number4 = currentNumberStr.toFloat();
      }

      // Extract the fifth number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number5 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number5 = currentNumberStr.toFloat();
      }

      // Extract the sixth number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number6 = currentNumberStr.toFloat();
        currentIndex = commaIndex + 1;
        commaIndex = stripped.indexOf(',', currentIndex);
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number6 = currentNumberStr.toFloat();
      }

      // Extract the seventh number
      if (commaIndex != -1) {
        String currentNumberStr = stripped.substring(currentIndex, commaIndex);
        number7 = currentNumberStr.toFloat();
      } else {
        String currentNumberStr = stripped.substring(currentIndex);
        number7 = currentNumberStr.toFloat();
      }

      // Print the extracted numbers with two decimal places
      Serial.println("Extracted Numbers:");
      Serial.print(number1, 2);
      Serial.print(" ");
      Serial.print(number2, 2);
      Serial.print(" ");
      Serial.print(number3, 2);
      Serial.print(" ");
      Serial.print(number4, 2);
      Serial.print(" ");
      Serial.print(number5, 2);
      Serial.print(" ");
      Serial.print(number6, 2);
      Serial.print(" ");
      Serial.println(number7, 2);
    }
  }
      // (Optional) Handle potential cases with more than 5 numbers (error handling)

        // // Parse the received data
        // char* ptr = strtok((char*)data.c_str(), ",");
        // while (ptr != NULL && valueIndex < MAX_VALUES) {
        //   values[valueIndex++] = atoi(ptr);  // Convert token to integer and store in array
        //   ptr = strtok(NULL, ",");
        // }

        // // Print the received values (for debugging)
        // // Serial.println("Received values:");
        // for (int i = 0; i < valueIndex; i++) {
        //   // Serial.print("Value ");
        //   // Serial.print(i);
        //   // Serial.print(": ");
        //   Serial.println(values[i]);
        // }
      

      // Turn on ERR LED when CUSTOM command is received
    
  
}
  // Check voltage and current limits
  if (voltage < voltage_min || voltage > voltage_max || current < current_min || current > current_max) {
    // Stop the motor if any value exceeds the limits
    //digitalWrite(LED, HIGH);
  } else {
    // Clear ERR LED and run the motor with the received throttle value
    //digitalWrite(ERR, LOW);
    //digitalWrite(LED, LOW);
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
