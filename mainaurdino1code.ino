#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int red[4] = { 24, 27, 30, 33 };
int yellow[4] = { 25, 28, 31, 34 }; 
int green[4] = { 26, 29, 32, 35 };

int emergencyBtn = 22;
int violationBtn = 23;
int buzzer = 13;

String currentMode = "NORMAL";
int emergencyLane = -1;

unsigned long previousMillis = 0;
int normalState = 0;
int currentLane = 0;

bool lastEmergencyState = HIGH;
bool lastViolationState = HIGH;

String serialBuffer = "";

int emergencyCount = 0;

void setup() {

  lcd.init();
  lcd.backlight();

  for (int i = 0; i < 4; i++) {
    pinMode(red[i], OUTPUT);
    pinMode(yellow[i], OUTPUT);
    pinMode(green[i], OUTPUT);
  }

  pinMode(emergencyBtn, INPUT_PULLUP);
  pinMode(violationBtn, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  Serial.begin(9600);
  Serial2.begin(9600);

  lcd.setCursor(0, 0);
  lcd.print("Traffic System");
  delay(2000);
  lcd.clear();
}

void loop() {

  readButtons();
  readSerialNonBlocking();

  if (currentMode == "NORMAL") {
    normalModeNonBlocking();
  }
}

void readButtons() {

  bool emergencyState = digitalRead(emergencyBtn);
  bool violationState = digitalRead(violationBtn);

  if (emergencyState != lastEmergencyState || violationState != lastViolationState) {

    Serial.print("EMERGENCY:");
    Serial.print(emergencyState == LOW ? 1 : 0);
    Serial.print(",VIOLATION:");
    Serial.println(violationState == LOW ? 1 : 0);

    lastEmergencyState = emergencyState;
    lastViolationState = violationState;
  }
}

void readSerialNonBlocking() {

  while (Serial.available()) {

    char c = Serial.read();

    if (c == '\n') {

      serialBuffer.trim();

      if (serialBuffer == "NORMAL") {
        currentMode = "NORMAL";
        emergencyLane = -1;
        digitalWrite(buzzer, LOW);
        lcd.clear();
      }

      else if (serialBuffer.startsWith("CLEAR:")) {
        emergencyCount++;
        currentMode = "EMERGENCY";
        emergencyLane = serialBuffer.substring(6).toInt() - 1;
        activateEmergency(emergencyLane);

        String UNO = "$" + String(emergencyCount) + "b";
        Serial2.println(UNO);
      }

      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}

void normalModeNonBlocking() {

  unsigned long currentMillis = millis();

  if (normalState == 0 && currentMillis - previousMillis >= 1000) {

    setGreen(currentLane);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Green: Side ");
    lcd.print(currentLane + 1);

    previousMillis = currentMillis;
    normalState = 1;
  }

  else if (normalState == 1 && currentMillis - previousMillis >= 500) {

    setYellow(currentLane);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Yellow: Side ");
    lcd.print(currentLane + 1);

    previousMillis = currentMillis;
    normalState = 0;

    currentLane++;
    if (currentLane > 3) {
      currentLane = 0;
    }
  }
}

void activateEmergency(int lane) {

  digitalWrite(buzzer, HIGH);

  for (int i = 0; i < 4; i++) {
    digitalWrite(red[i], HIGH);
    digitalWrite(yellow[i], LOW);
    digitalWrite(green[i], LOW);
  }

  if (lane >= 0 && lane < 4) {

    digitalWrite(red[lane], LOW);
    digitalWrite(green[lane], HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EMERGENCY");
    lcd.setCursor(0, 1);
    lcd.print("Lane ");
    lcd.print(lane + 1);
    lcd.print(" Clear");
  }
}

void setGreen(int lane) {

  for (int i = 0; i < 4; i++) {
    digitalWrite(red[i], HIGH);
    digitalWrite(yellow[i], LOW);
    digitalWrite(green[i], LOW);
  }

  digitalWrite(red[lane], LOW);
  digitalWrite(green[lane], HIGH);
}

void setYellow(int lane) {
  digitalWrite(green[lane], LOW);
  digitalWrite(yellow[lane], HIGH);
}
