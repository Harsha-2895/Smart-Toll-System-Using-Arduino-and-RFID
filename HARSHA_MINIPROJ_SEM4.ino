#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 5
#define SERVO_PIN 6
 
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust if needed
Servo gateServo;

byte validUIDs[][4] = {
  {0x33, 0x22, 0x0D, 0xF5},
  {0x43, 0xB6, 0xAC, 0xFD},
  {0xa3, 0xa0, 0xda, 0x6b},
  {0xDD, 0x33, 0xA8, 0xE3},
  {0xF3, 0x08, 0x5B, 0x3A}
};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  RFID Toll Gate");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card...");

  pinMode(BUZZER_PIN, OUTPUT);
  gateServo.attach(SERVO_PIN);
  gateServo.write(0); // Closed gate

  Serial.println("System Ready. Waiting for card...");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID Detected: ");
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidStr += String(mfrc522.uid.uidByte[i], HEX);
      if (i != mfrc522.uid.size - 1) uidStr += "-";
    }
    Serial.println(uidStr);

    bool matched = false;
    for (int i = 0; i < sizeof(validUIDs) / sizeof(validUIDs[0]); i++) {
      if (memcmp(mfrc522.uid.uidByte, validUIDs[i], mfrc522.uid.size) == 0) {
        matched = true;
        break;
      }
    }

    if (matched) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACCESS GRANTED");
      lcd.setCursor(0, 1);
      lcd.print("OPENING...");
      tone(BUZZER_PIN, 1000, 200);
      Serial.println("Buzzer: ON (Beep)");
      gateServo.write(90);
      Serial.println("Servo: OPEN (90°)");
      delay(3000);
      gateServo.write(0);
      Serial.println("Servo: CLOSED (0°)");
      lcd.clear();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACCESS DENIED");
      lcd.setCursor(0, 1);
      lcd.print("INVALID CARD");
      Serial.println("Access Denied: Invalid UID");
      for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 1000, 100);
        Serial.println("Buzzer: Warning Beep");
        delay(200);
      }
    }

    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  RFID Toll Gate");
    lcd.setCursor(0, 1);
    lcd.print("Scan your card...");
    Serial.println("Ready for next card...\n");

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}
