#include <VirtualButton.h>
#include <EEPROM.h>

#define STR_ADDR 0  // адрес хранения строки в EEPROM
#define BTN_PIN 2
#define LedCOLD_PIN 9 //5
#define LedWARM_PIN 6

uint8_t powerColdLed = 100;
uint8_t powerWarmLed = 100;
uint8_t clickCount = 0;
uint32_t holdMillis = 0;
uint32_t ledMillis = 0;

bool needSavePower = false;
bool isLedsTurnON = false;
bool isHold = false;
bool isAutoStart = false;

VButton btn;

//----------------------------------------------------------
void setup() {
//  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LedCOLD_PIN, OUTPUT);
  pinMode(LedWARM_PIN, OUTPUT);
  digitalWrite(LedCOLD_PIN, 0);
  digitalWrite(LedWARM_PIN, 0);
  
//  Serial.begin(115200);

  btn.setHoldTimeout(350);

  loadEEPROM();
  if (isAutoStart) onLeds();
}

//----------------------------------------------------------
void loop() {
//  if (millis() - ledMillis > 1000) { ledMillis = millis(); digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); }

  btn.poll(digitalRead(BTN_PIN));

  if (btn.click()) {
    clickCount++;
//    Serial.print(F("click: Count = ")); Serial.println(clickCount);
  }

  if (btn.hold()) {
    if (isLedsTurnON) {
      isHold = true;
      if (millis() - holdMillis > 20) { holdMillis = millis(); holdAction(); }
    }
  }

  if (btn.timeout(500)) {
//    Serial.print(F("timeout:"));
    if (isHold) {
      isHold = false;
      clickCount = 0;
      if (needSavePower) savePowerToEEPROM();
    }
    else {
//      Serial.print(F(" clickCount = ")); Serial.println(clickCount);
      switch (clickCount) {
        case 1: toggleLeds(); break;
        case 2: setLightPower01(); break;
        case 3: setLightPower100(); break;
        case 4: setLightPower200(); break;
        case 6: toggleAutoStart(); break;
      }
      clickCount = 0;
//      Serial.print(F("reset clickCount = ")); Serial.println(clickCount);
    }
  }
}

//----------------------------------------------------------
void toggleAutoStart() {
//  Serial.print(F("toggleAutoStart"));
  isAutoStart = !isAutoStart;
  
  uint8_t count = isAutoStart ? 2 : 1;
  for (uint8_t i = count; i > 0; i--) {
    delay(200);
    setPower(255, 255);
    delay(100);
    setPower(0, 0);
    delay(200);
  }
  setPower(powerColdLed, powerWarmLed);
  
  saveAutoStartToEEPROM();
}

//----------------------------------------------------------
void setLightPower200() {
//  Serial.print(F("defaultLight"));
  powerWarmLed = 200;
  powerColdLed = 200;
  onLeds();
  savePowerToEEPROM();
}

//----------------------------------------------------------
void setLightPower01() {
//  Serial.print(F("defaultLight"));
  powerWarmLed = 1;
  powerColdLed = 1;
  onLeds();
  savePowerToEEPROM();
}

//----------------------------------------------------------
void setLightPower100() {
//  Serial.print(F("defaultLight"));
  powerWarmLed = 100;
  powerColdLed = 100;
  onLeds();
  savePowerToEEPROM();
}

//----------------------------------------------------------
void holdAction() {
  switch (clickCount) {
    case 1: incrementLight(); break;
    case 2: decrementLight(); break;
    case 3: incrementColdLight(); break;
    case 4: incrementWarmLight(); break;
  }
}

//----------------------------------------------------------
void incrementWarmLight() {
  // Cold- Warm+
  if (powerColdLed > 0) { powerColdLed--; needSavePower = true; }
  if (powerWarmLed < 255) { powerWarmLed++; needSavePower = true; }
  setPower(powerColdLed, powerWarmLed);
//  Serial.print(F("Cold- Warm+: powerColdLed = ")); Serial.print(powerColdLed);
//  Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
}

//----------------------------------------------------------
void incrementColdLight() {
  // Cold+ Warm-
  if (powerColdLed < 255) { powerColdLed++; needSavePower = true; }
  if (powerWarmLed > 0) { powerWarmLed--; needSavePower = true; }
  setPower(powerColdLed, powerWarmLed);
//    Serial.print(F("Cold+ Warm-: powerColdLed = ")); Serial.print(powerColdLed);
//    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
}

//----------------------------------------------------------
void incrementLight() {
  // Cold+ Warm+
  if (powerColdLed < 255) { powerColdLed++; needSavePower = true; }
  if (powerWarmLed < 255) { powerWarmLed++; needSavePower = true; }
  setPower(powerColdLed, powerWarmLed);
//    Serial.print(F("Cold+ Warm+: powerColdLed = ")); Serial.print(powerColdLed);
//    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
}

//----------------------------------------------------------
void decrementLight() {
  // Cold- Warm-
  if (powerColdLed > 0) { powerColdLed--; needSavePower = true; }
  if (powerWarmLed > 0) { powerWarmLed--; needSavePower = true; }
  setPower(powerColdLed, powerWarmLed);
//    Serial.print(F("Cold- Warm-: powerColdLed = ")); Serial.print(powerColdLed);
//    Serial.print(F(", powerWarmLed = ")); Serial.println(powerWarmLed);
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
void savePowerToEEPROM() {
//  Serial.println(F("savePowerToEEPROM"));
  EEPROM.put(0, powerColdLed);
  EEPROM.put(1, powerWarmLed);
  needSavePower = false;
}

//----------------------------------------------------------
void saveAutoStartToEEPROM() {
//  Serial.println(F("saveAutoStartToEEPROM"));
  EEPROM.put(2, isAutoStart);
}

//----------------------------------------------------------
void loadEEPROM() {
  EEPROM.get(0, powerColdLed);
  EEPROM.get(1, powerWarmLed);
  EEPROM.get(2, isAutoStart);
//  Serial.print(F("loadEEPROM: powerColdLed = ")); Serial.print(powerColdLed);
//  Serial.print(F("; powerWarmLed = ")); Serial.print(powerWarmLed);
//  Serial.print(F("; isAutoPower = ")); Serial.println(isAutoPower);
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
