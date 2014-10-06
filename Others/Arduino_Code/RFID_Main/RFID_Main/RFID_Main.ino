/*
 ----------------------------------------------------------------------------- 
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               52                MOSI
 * SPI MISO   12               51                MISO
 * SPI SCK    13               50                SCK
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

#define RETRY_MAX 3

MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.
String activingCard = "";
int livingCount = 0;

void setup() 
{ 
  Serial.begin(9600);        // Initialize serial communications with the PC
  SPI.begin();                // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
}

void loop() 
{
  delay(1000);
  
  // Look for new cards
  if ( !mfrc522.PICC_IsNewCardPresent())
  {
    livingCount--;
    if (livingCount <= 0)
    {
      Serial.print("E#");
      livingCount = 0;
      activingCard = "";
    }
    //Serial.println("No PICC_IsNewCardPresent");
    return;
  }
  
  // Select one of the cards
  if ( !mfrc522.PICC_ReadCardSerial())
  {
    livingCount--;
    if (livingCount <= 0)
    {
      Serial.print("E#");
      livingCount = 0;
      activingCard = "";
    }
    //Serial.println("No PICC_ReadCardSerial");
    return;
  }
  
  //Serial.print("Card Found: ");    //Dump UID
  String newCard = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //Serial.print(mfrc522.uid.uidByte[i], HEX);
    newCard += String(mfrc522.uid.uidByte[i], HEX);
  }
  newCard.toUpperCase();
  
  if (activingCard != newCard)
  {
    activingCard = newCard;
    Serial.print("N#");
    Serial.print(activingCard);
    Serial.print("#");
  }

  livingCount = RETRY_MAX;
}

