#define BLYNK_TEMPLATE_ID "TMPL3EXPWaQwb"
#define BLYNK_TEMPLATE_NAME "LpgCar"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Motor Pins
#define ENA 22
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19
#define ENB 23
#define LED_PIN 2
#define Buzzer_pin 15

// Ultrasonic Sensor Pins
#define TRIG_PIN 5
#define ECHO_PIN 4

// Temperature Sensor
#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-6 Gas Sensor
#define MQ6_PIN 34
#define MQ6_D0 35

BlynkTimer timer;
bool forward = false, backward = false, left = false, right = false;
int Speed = 150;
bool autonomousMode = false;
unsigned long lastCommandTime = 0;
unsigned long autoModeStartTime = 0;

// WiFi Credentials
char auth[] = "ZPhFK07WDflhNdSwJJSjYPsUt3h9vjit";
char ssid[] = "iQOO Z6 p";
char pass[] = "tanmay12";

// Function Prototypes
void carForward();
void carBackward();
void carLeft();
void carRight();
void carStop();
void automaticMode();
float getDistance();
void readTemperature();
void sendDistanceToBlynk();
void readGasSensor();

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Motor Pins Setup
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // PWM Configuration for ESP32
  ledcAttachPin(ENA, 0);
  ledcAttachPin(ENB, 1);
  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);

  // Sensor Setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MQ6_PIN, INPUT);
  pinMode(MQ6_D0, INPUT);
  dht.begin();

  // Set Timers
  timer.setInterval(1000L, readTemperature);
  timer.setInterval(1000L, sendDistanceToBlynk);
  timer.setInterval(2000L, readGasSensor);
}

// Blynk Controls
BLYNK_WRITE(V0) { Speed = param.asInt(); }
BLYNK_WRITE(V1) { forward = param.asInt(); lastCommandTime = millis(); }
BLYNK_WRITE(V2) { backward = param.asInt(); lastCommandTime = millis(); }
BLYNK_WRITE(V3) { left = param.asInt(); lastCommandTime = millis(); }
BLYNK_WRITE(V4) { right = param.asInt(); lastCommandTime = millis(); }
BLYNK_WRITE(V5) { autonomousMode = param.asInt(); carStop(); autoModeStartTime = millis(); }

void loop() {
  Blynk.run();
  timer.run();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi Disconnected. Attempting Reconnect...");
    Blynk.connect();
    return;
  }

  if (!autonomousMode) {
    if (millis() - lastCommandTime > 500) {  // Debounce commands
      carStop();
    } else {
      if (forward) carForward();
      else if (backward) carBackward();
      else if (left) carLeft();
      else if (right) carRight();
    }
  } else {
    automaticMode();
  }
}

// 🚗 **Automatic Mode (Non-blocking)**
void automaticMode() {
  if (!autonomousMode) return;
  unsigned long currentTime = millis();

  if (currentTime - autoModeStartTime > 30000) { // 30s Timeout
    Serial.println("🔄 Autonomous Mode Timed Out. Switching to Manual.");
    autonomousMode = false;
    carStop();
    return;
  }

  float distance = getDistance();
  if (distance < 10) {
    Serial.println("🚧 Obstacle Detected! Stopping...");
    carStop();
    delay(500);
    carBackward();
    delay(1000);
    carLeft();
    delay(700);
    return;
  }

  carForward();
}

// 📏 **Ultrasonic Sensor Function**
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  float duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 500;
  return duration * 0.034 / 2;
}

// 📡 **Send Ultrasonic Data to Blynk**
void sendDistanceToBlynk() {
  float distance = getDistance();
  Blynk.virtualWrite(V8, distance);
  Serial.print("📡 Distance: ");
  Serial.println(distance);
}

// 🌡️ **Read Temperature & Send to Blynk**
void readTemperature() {
  float temp = dht.readTemperature();
  if (!isnan(temp)) {
    Blynk.virtualWrite(V6, temp);
    Serial.print("🌡️ Temperature: ");
    Serial.print(temp);
    Serial.println("°C");
  }
}

// 🔥 **Read Gas Sensor & Send to Blynk**
void readGasSensor() {
  int gasValue = analogRead(MQ6_PIN);
  float gasPPM = (gasValue / 4095.0) * 1000.0;
  Blynk.virtualWrite(V9, gasPPM);

  if (digitalRead(MQ6_D0) == HIGH) {
    Serial.println("⚠️ HIGH GAS DETECTED!");
  }
}

// 🚘 **Motor Control Functions**
void setMotorSpeed(int channel, int speed) {
  speed = constrain(speed, 0, 255);
  ledcWrite(channel, speed);
}

void carForward() {
  digitalWrite(LED_PIN, HIGH);
  setMotorSpeed(0, Speed);
  setMotorSpeed(1, Speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void carBackward() {
  digitalWrite(LED_PIN, HIGH);
  setMotorSpeed(0, Speed);
  setMotorSpeed(1, Speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void carLeft() {
  digitalWrite(LED_PIN, HIGH);
  setMotorSpeed(0, Speed);
  setMotorSpeed(1, Speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void carRight() {
  digitalWrite(LED_PIN, HIGH);
  setMotorSpeed(0, Speed);
  setMotorSpeed(1, Speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void carStop() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
