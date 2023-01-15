#include <VirtualButton.h>
#include <EEPROM.h>

#define STR_ADDR 0  // адрес хранения строки в EEPROM
#define BTN_PIN 2
#define LedCOLD_PIN 5
#define LedWARM_PIN 6

uint8_t powerColdLed = 100;
uint8_t powerWarmLed = 100;
uint8_t clickCount = 0;
uint32_t holdMillis = 0;

bool needSavePower = false;
bool isLedsTurnON = false;
bool isHold = false;

VButton btn;

//----------------------------------------------------------
void setup() {
  pinMode(LedCOLD_PIN, OUTPUT);
  pinMode(LedWARM_PIN, OUTPUT);
  digitalWrite(LedCOLD_PIN, 0);
  digitalWrite(LedWARM_PIN, 0);
  
  Serial.begin(115200);

  btn.setHoldTimeout(250);

  loadEEPROM();
}

//----------------------------------------------------------
void loop() {
  btn.poll(digitalRead(BTN_PIN));

  if (btn.click()) {
    clickCount++;
    Serial.print(F("click: Count = ")); Serial.println(clickCount);
  }

  if (btn.hold()) {
    if (isLedsTurnON) {
      isHold = true;
      if (millis() - holdMillis > 20) { holdMillis = millis(); holdAction(); }
    }
  }

  if (btn.timeout(500)) {
    Serial.print(F("timeout:"));
    if (isHold) {
      isHold = false;
      clickCount = 0;
      if (needSavePower) saveEEPROM();
    }
    else {
      Serial.print(F(" clickCount = ")); Serial.println(clickCount);
      switch (clickCount) {
        case 1: toggleLeds(); break;
        case 2: defaultLight01(); break;
        case 3: defaultLight50(); break;
        case 4: defaultLight127(); break;
      }
      clickCount = 0;
      Serial.print(F("reset clickCount = ")); Serial.println(clickCount);
    }
  }
}


//----------------------------------------------------------
void defaultLight127() {
  Serial.print(F("defaultLight"));
  powerWarmLed = 127;
  powerColdLed = 127;
  onLeds();
  saveEEPROM();
}

//----------------------------------------------------------
void defaultLight01() {
  Serial.print(F("defaultLight"));
  powerWarmLed = 1;
  powerColdLed = 1;
  onLeds();
  saveEEPROM();
}

//----------------------------------------------------------
void defaultLight50() {
  Serial.print(F("defaultLight"));
  powerWarmLed = 50;
  powerColdLed = 50;
  onLeds();
  saveEEPROM();
}

//----------------------------------------------------------
void holdAction() {
//  Serial.print(F("holdAction: clickCount = ")); Serial.println(clickCount);
  switch (clickCount) {
    case 1: incrementLight(); break;
    case 2: decrementLight(); break;
    case 3: coldLight(); break;
    case 4: warmLight(); break;
  }
}

//----------------------------------------------------------
void warmLight() {
  // Cold- Warm+
  if (powerColdLed > 0 && powerWarmLed < 255) {
    Serial.print(F("Cold- Warm+"));
    powerColdLed--;
    powerWarmLed++;
    Serial.print(F(": powerColdLed = ")); Serial.print(powerColdLed);
    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
    setPower(powerColdLed, powerWarmLed);
    needSavePower = true;
  }
}

//----------------------------------------------------------
void coldLight() {
  // Cold+ Warm-
  if (powerColdLed < 255 && powerWarmLed > 0) {
    Serial.print(F("Cold+ Warm-"));
    powerColdLed++;
    powerWarmLed--;
    Serial.print(F(": powerColdLed = ")); Serial.print(powerColdLed);
    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
    setPower(powerColdLed, powerWarmLed);
    needSavePower = true;
  }
}

//----------------------------------------------------------
void incrementLight() {
  // Cold+ Warm+
  if (powerColdLed < 255 && powerWarmLed < 255) {
    Serial.print(F("Cold+ Warm+"));
    powerColdLed++;
    powerWarmLed++;
    Serial.print(F(": powerColdLed = ")); Serial.print(powerColdLed);
    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
    setPower(powerColdLed, powerWarmLed);
    needSavePower = true;
  }
}

//----------------------------------------------------------
void decrementLight() {
  // Cold- Warm-
  if (powerColdLed > 1 && powerWarmLed > 1) {
    Serial.print(F("Cold- Warm-"));
    powerColdLed--;
    powerWarmLed--;
    Serial.print(F(": powerColdLed = ")); Serial.print(powerColdLed);
    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
    setPower(powerColdLed, powerWarmLed);
    needSavePower = true;
  }
}

//----------------------------------------------------------
void toggleLeds() {
  if (isLedsTurnON) { offLeds(); } else { onLeds(); }
}

//----------------------------------------------------------
void setPower(uint8_t powerColdLed, uint8_t powerWarmLed) {
  analogWrite(LedCOLD_PIN, ((uint16_t)powerColdLed * powerColdLed + 255) >> 8);
  analogWrite(LedWARM_PIN, ((uint16_t)powerWarmLed * powerWarmLed + 255) >> 8);

//  analogWrite(LedCOLD_PIN, powerColdLed);
//  analogWrite(LedWARM_PIN, powerWarmLed);
}

//----------------------------------------------------------
void saveEEPROM() {
  Serial.println(F("saveEEPROM"));
  EEPROM.put(0, powerColdLed);
  EEPROM.put(1, powerWarmLed);
  needSavePower = false;
}

//----------------------------------------------------------
void loadEEPROM() {
  EEPROM.get(0, powerColdLed);
  EEPROM.get(1, powerWarmLed);
  Serial.print(F("loadEEPROM: powerColdLed = ")); Serial.print(powerColdLed);
  Serial.print(F("; powerWarmLed = ")); Serial.println(powerWarmLed);
}

//----------------------------------------------------------
void onLeds() {
  uint8_t cold = 0;
  uint8_t warm = 0;
  uint8_t count = max(powerColdLed, powerWarmLed);
  for (uint8_t i = count; i > 0; i--) {
    if (cold < powerColdLed) cold++;
    if (warm < powerWarmLed) warm++;
    setPower(cold, warm);
    delay(5);
  }
  isLedsTurnON = true;
}

//----------------------------------------------------------
void offLeds() {
  uint8_t cold = powerColdLed;
  uint8_t warm = powerWarmLed;
  uint8_t count = max(powerColdLed, powerWarmLed);
  for (uint8_t i = count; i > 0; i--) {
    if (cold > 0) cold--;
    if (warm > 0) warm--;
    setPower(cold, warm);
    delay(5);
  }
  isLedsTurnON = false;
}
