
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ======================================================
// AQUAGUARD - COMPLETE ESP32 FISH FARM CONTROL SYSTEM
// ======================================================

// ---------------- PIN CONFIGURATION ------------------

#define FLOAT_PIN       32
#define TEMP_PIN        4
#define PH_PIN          34
#define TURBIDITY_PIN   35

#define BUZZER_PIN      25

#define RED_LED         19
#define GREEN_LED       17
#define YELLOW_LED      16

#define INLET_RELAY     26
#define OUTLET_RELAY    27

// ---------------- RELAY CONFIGURATION ----------------

// Your relay modules are ACTIVE LOW
#define RELAY_ON        LOW
#define RELAY_OFF       HIGH

// Normally-open solenoid valves:
// Relay ON  = Valve CLOSED
// Relay OFF = Valve OPEN

// ---------------- FLOAT SWITCH -----------------------

#define TANK_FULL       HIGH
#define TANK_LOW        LOW

// ---------------- LCD CONFIGURATION ------------------

LiquidCrystal_I2C lcd(0x27, 20, 4);

// ---------------- TEMPERATURE SENSOR -----------------

OneWire oneWire(TEMP_PIN);
DallasTemperature temperatureSensor(&oneWire);

// ======================================================
// WATER QUALITY SETTINGS
// ======================================================

// Maximum permitted temperature (adjust for fish species)
const float TEMP_MAX = 30.0;

// Turbidity calibration settings.
//
// This initial configuration assumes that a lower ADC
// reading means more turbidity, as with common analog
// turbidity modules.
//
// IMPORTANT: Calibrate these two values with your
// actual sensor before relying on automatic flushing.

const bool TURBIDITY_DIRTY_WHEN_RAW_BELOW = true;

const int TURBIDITY_START_RAW = 2200;
const int TURBIDITY_CLEAR_RAW = 2500;

// The two thresholds provide hysteresis to prevent
// rapid relay switching around one threshold.

// ======================================================
// SYSTEM VARIABLES
// ======================================================

int floatState = HIGH;

int phRaw = 0;
int turbidityRaw = 0;

float tempC = 0.0;

bool tankIsFull = false;
bool tankIsLow = false;

bool temperatureFault = false;
bool temperatureHigh = false;

bool turbidityHigh = false;

bool inletIsOpen = false;
bool outletIsOpen = false;

bool alarmActive = false;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastBeepTime = 0;

const unsigned long SENSOR_INTERVAL = 1000;
const unsigned long LCD_INTERVAL = 2000;
const unsigned long BEEP_INTERVAL = 350;

// Buzzer state
bool buzzerState = false;

// ======================================================
// LCD HELPER FUNCTION
// ======================================================

void printLCDLine(int row, String message) {

  lcd.setCursor(0, row);

  lcd.print("                    ");

  lcd.setCursor(0, row);

  lcd.print(message.substring(0, 20));
}

// ======================================================
// RELAY CONTROL FUNCTION
// ======================================================

void controlRelay(int pin, bool turnOn) {

  digitalWrite(pin, turnOn ? RELAY_ON : RELAY_OFF);
}

// ======================================================
// 1. READ ALL SENSORS
// ======================================================

void readSensors() {

  // Read float switch
  floatState = digitalRead(FLOAT_PIN);

  tankIsFull = (floatState == TANK_FULL);
  tankIsLow = (floatState == TANK_LOW);

  // Read pH sensor
  phRaw = analogRead(PH_PIN);

  // Read turbidity sensor
  turbidityRaw = analogRead(TURBIDITY_PIN);

  // Read temperature sensor
  temperatureSensor.requestTemperatures();

  tempC = temperatureSensor.getTempCByIndex(0);

  // Detect temperature sensor failure
  temperatureFault =
      (tempC == DEVICE_DISCONNECTED_C ||
       tempC < -55.0 ||
       tempC > 125.0);

  // Detect high temperature
  temperatureHigh =
      (!temperatureFault && tempC > TEMP_MAX);
}

// ======================================================
// 2. TURBIDITY DETECTION
// ======================================================

void processTurbidity() {

  if (TURBIDITY_DIRTY_WHEN_RAW_BELOW) {

    // Enter dirty state
    if (!turbidityHigh &&
        turbidityRaw < TURBIDITY_START_RAW) {

      turbidityHigh = true;
    }

    // Leave dirty state when water becomes clear
    else if (turbidityHigh &&
             turbidityRaw > TURBIDITY_CLEAR_RAW) {

      turbidityHigh = false;
    }

  } else {

    // Alternative sensor direction:
    // Higher ADC means more turbidity.

    if (!turbidityHigh &&
        turbidityRaw > TURBIDITY_START_RAW) {

      turbidityHigh = true;
    }

    else if (turbidityHigh &&
             turbidityRaw < TURBIDITY_CLEAR_RAW) {

      turbidityHigh = false;
    }
  }
}

// ======================================================
// 3. AUTOMATIC WATER LEVEL AND VALVE CONTROL
// ======================================================

void controlWaterLevel() {

  // TURBIDITY FLUSHING HAS PRIORITY

  if (turbidityHigh) {

    // Open inlet valve
    controlRelay(INLET_RELAY, false);

    // Open outlet valve
    controlRelay(OUTLET_RELAY, false);

    inletIsOpen = true;
    outletIsOpen = true;
  }

  // NORMAL WATER LEVEL CONTROL

  else {

    // Keep outlet CLOSED
    controlRelay(OUTLET_RELAY, true);

    outletIsOpen = false;

    // Tank FULL
    if (tankIsFull) {

      // Close inlet valve
      controlRelay(INLET_RELAY, true);

      inletIsOpen = false;
    }

    // Tank LOW
    else if (tankIsLow) {

      // Open inlet valve
      controlRelay(INLET_RELAY, false);

      inletIsOpen = true;
    }
  }
}

// ======================================================
// 4. ALERT DETECTION
// ======================================================

bool isAlarmActive() {

  return (
      tankIsLow ||
      temperatureHigh ||
      temperatureFault ||
      turbidityHigh
  );
}

// ======================================================
// 5. LED INDICATORS
// ======================================================

void handleLEDs() {

  alarmActive = isAlarmActive();

  // RED LED:
  // Water low, high temperature,
  // temperature fault or high turbidity.

  digitalWrite(RED_LED,
      alarmActive ? HIGH : LOW);

  // GREEN LED:
  // Tank full and no active warnings.

  digitalWrite(GREEN_LED,
      (tankIsFull && !alarmActive) ? HIGH : LOW);

  // YELLOW LED:
  // Indicates active turbidity flushing.

  digitalWrite(YELLOW_LED,
      turbidityHigh ? HIGH : LOW);
}

// ======================================================
// 6. BUZZER CONTROL
// ======================================================

void handleBuzzer() {

  alarmActive = isAlarmActive();

  unsigned long currentTime = millis();

  if (alarmActive) {

    // Intermittent beeping
    if (currentTime - lastBeepTime >= BEEP_INTERVAL) {

      lastBeepTime = currentTime;

      buzzerState = !buzzerState;

      digitalWrite(BUZZER_PIN,
          buzzerState ? HIGH : LOW);
    }

  } else {

    // Turn buzzer OFF when all warnings clear
    buzzerState = false;

    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ======================================================
// 7. LCD WARNING MESSAGES
// ======================================================

void updateLCD() {

  static int messageIndex = 0;

  static unsigned long lastMessageChange = 0;

  unsigned long currentTime = millis();

  // Count active warning conditions
  int warningCount = 0;

  if (tankIsLow) warningCount++;

  if (temperatureHigh) warningCount++;

  if (temperatureFault) warningCount++;

  if (turbidityHigh) warningCount++;

  // Rotate through warning messages
  if (currentTime - lastMessageChange >= LCD_INTERVAL) {

    lastMessageChange = currentTime;

    messageIndex++;

    if (messageIndex >= warningCount) {
      messageIndex = 0;
    }
  }

  // ROW 1: SYSTEM AND WATER LEVEL
  String levelText = tankIsFull ? "FULL" : "LOW";

  printLCDLine(0, "AQUAGUARD " + levelText);

  // ROW 2: TEMPERATURE
  if (temperatureFault) {

    printLCDLine(1, "Temp: SENSOR ERROR");

  } else {

    printLCDLine(1,
        "Temp: " + String(tempC, 1) + " C");
  }

  // ROW 3: pH AND TURBIDITY RAW READINGS
  printLCDLine(2,
      "PH:" + String(phRaw) +
      " TUR:" + String(turbidityRaw));

  // ROW 4: SHOW THE REASON FOR BUZZER

  if (warningCount > 0) {

    int currentWarning = 0;

    if (tankIsLow) {

      if (currentWarning == messageIndex) {
        printLCDLine(3, "ALARM: WATER LOW");
        return;
      }

      currentWarning++;
    }

    if (temperatureHigh) {

      if (currentWarning == messageIndex) {
        printLCDLine(3, "ALARM: TEMP HIGH");
        return;
      }

      currentWarning++;
    }

    if (temperatureFault) {

      if (currentWarning == messageIndex) {
        printLCDLine(3, "ALARM: TEMP ERROR");
        return;
      }

      currentWarning++;
    }

    if (turbidityHigh) {

      if (currentWarning == messageIndex) {
        printLCDLine(3, "ALARM: WATER TURBID");
        return;
      }

      currentWarning++;
    }

  } else {

    if (tankIsFull) {

      printLCDLine(3, "STATUS: WATER FULL");

    } else {

      printLCDLine(3, "STATUS: MONITORING");
    }
  }
}

// ======================================================
// 8. SERIAL MONITOR INFORMATION
// ======================================================

void printSerialData() {

  Serial.println();
  Serial.println("================================");
  Serial.println("       AQUAGUARD SYSTEM");
  Serial.println("================================");

  Serial.print("Water Level: ");

  Serial.println(tankIsFull ? "FULL" : "LOW");

  Serial.print("Temperature: ");

  if (temperatureFault) {

    Serial.println("SENSOR ERROR");

  } else {

    Serial.print(tempC, 2);
    Serial.println(" C");
  }

  Serial.print("pH ADC: ");
  Serial.println(phRaw);

  Serial.print("Turbidity ADC: ");
  Serial.println(turbidityRaw);

  Serial.print("Turbidity Status: ");

  Serial.println(turbidityHigh ? "TURBID" : "CLEAR");

  Serial.print("Inlet Valve: ");

  Serial.println(inletIsOpen ? "OPEN" : "CLOSED");

  Serial.print("Outlet Valve: ");

  Serial.println(outletIsOpen ? "OPEN" : "CLOSED");

  Serial.print("Red LED: ");

  Serial.println(alarmActive ? "ON" : "OFF");

  Serial.print("Green LED: ");

  Serial.println((tankIsFull && !alarmActive) ? "ON" : "OFF");

  Serial.print("Buzzer: ");

  Serial.println(alarmActive ? "BEEPING" : "OFF");

  Serial.println("================================");
}

// ======================================================
// 9. SYSTEM SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // Initialize input
  pinMode(FLOAT_PIN, INPUT_PULLUP);

  // Initialize output pins
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  pinMode(INLET_RELAY, OUTPUT);
  pinMode(OUTLET_RELAY, OUTPUT);

  // Initialize buzzer and LEDs
  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  // Close both normally-open valves at startup
  digitalWrite(INLET_RELAY, RELAY_ON);
  digitalWrite(OUTLET_RELAY, RELAY_ON);

  inletIsOpen = false;
  outletIsOpen = false;

  // ADC configuration
  analogReadResolution(12);

  // I2C configuration
  Wire.begin(21, 22);

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Temperature sensor initialization
  temperatureSensor.begin();

  // Startup screen
  printLCDLine(0, "AQUAGUARD SYSTEM");
  printLCDLine(1, "Initializing...");
  printLCDLine(2, "Sensors Starting");
  printLCDLine(3, "Please Wait");

  Serial.println();
  Serial.println("AQUAGUARD SYSTEM");
  Serial.println("Initializing...");

  delay(2000);

  lcd.clear();

  Serial.println("SYSTEM READY");
}

// ======================================================
// 10. MAIN PROGRAM LOOP
// ======================================================

void loop() {

  unsigned long currentTime = millis();

  // Read sensors at regular intervals
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {

    lastSensorRead = currentTime;

    // STEP 1: READ SENSORS
    readSensors();

    // STEP 2: CHECK TURBIDITY
    processTurbidity();

    // STEP 3: CONTROL WATER LEVEL AND VALVES
    controlWaterLevel();

    // STEP 4: CONTROL LED INDICATORS
    handleLEDs();

    // STEP 5: UPDATE SERIAL MONITOR
    printSerialData();
  }

  // STEP 6: CONTROL BUZZER
  handleBuzzer();

  // STEP 7: UPDATE LCD
  updateLCD();
}
