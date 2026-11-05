
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// =====================================
// AQUAGUARD - ESP32 SENSOR TEST
// =====================================

// Pin configuration
#define FLOAT_PIN 32
#define TEMP_PIN 4
#define PH_PIN 34
#define TURBIDITY_PIN 35

#define BUZZER_PIN 25
#define RED_LED 19
#define GREEN_LED 17
#define YELLOW_LED 16

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Temperature sensor
OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

void setup() {
  Serial.begin(115200);

  pinMode(FLOAT_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  lcd.init();
  lcd.backlight();

  temperatureSensor.begin();

  analogReadResolution(12);

  lcd.setCursor(0, 0);
  lcd.print("AQUAGUARD SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("Sensor Diagnostics");
  delay(2000);
  lcd.clear();

  Serial.println("AQUAGUARD SENSOR TEST");
  Serial.println("---------------------");
}

void loop() {

  // Read float switch
  int floatState = digitalRead(FLOAT_PIN);

  // Read analog sensors
  int phRaw = analogRead(PH_PIN);
  int turbidityRaw = analogRead(TURBIDITY_PIN);

  // Read temperature
  temperatureSensor.requestTemperatures();
  float tempC = temperatureSensor.getTempCByIndex(0);

  // Serial monitor
  Serial.println("\n--- AQUAGUARD READINGS ---");

  Serial.print("Float switch: ");
  Serial.println(floatState == LOW ? "LOW" : "HIGH");

  Serial.print("Temperature: ");
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor disconnected");
  } else {
    Serial.print(tempC);
    Serial.println(" C");
  }

  Serial.print("pH raw: ");
  Serial.println(phRaw);

  Serial.print("Turbidity raw: ");
  Serial.println(turbidityRaw);

  // LCD display
  lcd.setCursor(0, 0);
  lcd.print("AquaGuard Monitoring");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  if (tempC == DEVICE_DISCONNECTED_C) {
    lcd.print("ERROR       ");
  } else {
    lcd.print(tempC, 1);
    lcd.print(" C       ");
  }

  lcd.setCursor(0, 2);
  lcd.print("pH raw: ");
  lcd.print(phRaw);
  lcd.print("       ");

  lcd.setCursor(0, 3);
  lcd.print("Float:");
  lcd.print(floatState == LOW ? "LOW " : "HIGH");
  lcd.print(" Turb:");
  lcd.print(turbidityRaw);
  lcd.print("   ");

  delay(1500);
}
