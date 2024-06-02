#include <SPI.h>
#include <MFRC522.h>
#include <Keyboard.h> // Include the Keyboard library for HID functionality

#define SS_PIN 10
#define RST_PIN 9
#define BUTTON_PIN 2 // Define the pin where the button is connected

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Editable known NUID in hex format
byte knownNUID[4] = {0xFA, 0x1E, 0xFE, 0xB0}; // The NUID tag in hex
bool isLoggedIn = false; // Track login status

void setup() { 
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Keyboard.begin(); // Initialize the keyboard emulation
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set the button pin as input with internal pull-up resistor

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("Setup complete. Ready to scan for the correct RFID tag or monitor the button."));
}

void loop() {
  // Button and RFID logic
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;
  bool cardPresent = rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();

  if (buttonPressed && cardPresent) {
    if (memcmp(rfid.uid.uidByte, knownNUID, 4) == 0 && !isLoggedIn) {
      logIn();
      isLoggedIn = true;
    }
  } else if (!buttonPressed && isLoggedIn) {
    logOut();
    isLoggedIn = false;
  }

  if (cardPresent) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void logIn() {
  Keyboard.press(KEY_RETURN); // Tab to the password field
  Keyboard.releaseAll();
  delay(1000);
  Keyboard.println("Password");
  delay(1000);
  Keyboard.press(KEY_RETURN); // Press Enter to log in
  Keyboard.releaseAll();
  Serial.println(F("Login sequence executed."));
}



void logOut() {
  //Windows shortcut to lock the computer (Windows Key + L)
  Keyboard.press(KEY_LEFT_GUI); // Windows Key
  Keyboard.press('l');
  Keyboard.releaseAll();
  Serial.println(F("Logout sequence executed."));
}

void printTagDetails(MFRC522 &rfid) {
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}
