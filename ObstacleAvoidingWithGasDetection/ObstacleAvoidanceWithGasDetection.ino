#include "AFMotor.h"
#include <Servo.h>

#define echopin A4
#define trigpin A5
#define tempPin A2
#define gasPin A1
#define ledPin 9  // LED for warnings

Servo myservo;

const int MOTOR_1 = 1;
const int MOTOR_2 = 2;
const int MOTOR_3 = 3;
const int MOTOR_4 = 4;

AF_DCMotor motor1(MOTOR_1, MOTOR12_64KHZ);
AF_DCMotor motor2(MOTOR_2, MOTOR12_64KHZ);
AF_DCMotor motor3(MOTOR_3, MOTOR12_64KHZ);
AF_DCMotor motor4(MOTOR_4, MOTOR12_64KHZ);

int distance_F;

void setup() {
    Serial.begin(9600);
    myservo.attach(10);
    myservo.write(90); // Start in middle position

    pinMode(trigpin, OUTPUT);
    pinMode(echopin, INPUT);
    pinMode(ledPin, OUTPUT);

    motor1.setSpeed(155);
    motor2.setSpeed(155);
    motor3.setSpeed(155);
    motor4.setSpeed(155);
}

void loop() {
    distance_F = getDistance();
    float temp = getTemperature();
    int gasLevel = getGasLevel();

    Serial.print("Distance: ");
    Serial.print(distance_F);
    Serial.print(" cm | Temperature: ");
    Serial.print(temp);
    Serial.print(" °C | Gas Level: ");
    Serial.println(gasLevel);

    // LED Warning for High Temperature & Gas Detection
    if (temp > 85 && gasLevel > 250) {
        digitalWrite(ledPin, HIGH);
        Serial.println("WARNING: High Temperature & LPG Detected!!");
    } else {
        digitalWrite(ledPin, LOW);
    }

    // Obstacle handling
    if (distance_F > 0 && distance_F < 20) {
        Serial.println("Obstacle Detected! Stopping...");
        stopRobot();
        delay(500);

        Serial.println("Moving Backward...");
        moveBackward();
        delay(1000);

        stopRobot();
        delay(500);

        scanWithServo();  // Scan left and right

        int leftDistance = getLeftDistance();
        int rightDistance = getRightDistance();

        if (leftDistance > rightDistance) {
            Serial.println("Turning Left...");
            turnLeft();
        } else {
            Serial.println("Turning Right...");
            turnRight();
        }
    } else {
        Serial.println("No Obstacle, Moving Forward...");
        moveForward();
    }

    delay(500);
}

long getDistance() {
    digitalWrite(trigpin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigpin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigpin, LOW);

    long duration = pulseIn(echopin, HIGH, 30000); // Timeout for better accuracy
    if (duration == 0) return -1; // No echo detected
    return duration / 29 / 2;
}

float getTemperature() {
    int sensorValue = analogRead(tempPin);
    float voltage = sensorValue * 5.0 / 1023.0;
    return voltage * 100.0;
}

int getGasLevel() {
    return analogRead(gasPin);
}

// Servo scanning function
void scanWithServo() {
    myservo.write(45);  // Move Left
    delay(500);
    myservo.write(90);  // Move Mid
    delay(500);
    myservo.write(135); // Move Right
    delay(500);
    myservo.write(90);  // Move Mid again
}

// Get distance when servo is at left position
int getLeftDistance() {
    delay(500);
    return getDistance();
}

// Get distance when servo is at right position
int getRightDistance() {
    delay(500);
    return getDistance();
}

// Robot movement functions
void moveForward() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
}

void moveBackward() {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
}

void stopRobot() {
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    motor3.run(RELEASE);
    motor4.run(RELEASE);
}

void turnLeft() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
    delay(500);
}

void turnRight() {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
    delay(500);
}

