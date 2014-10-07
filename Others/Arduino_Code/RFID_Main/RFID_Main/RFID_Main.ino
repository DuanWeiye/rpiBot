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
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9

#define RETRY_MAX 4

#define LCD_PORT_RX 2
#define LCD_PORT_TX 3

#define STRING_EMPTY "E#"
#define STRING_NEW "N#"
#define STRING_ALIVING "A#"
#define STRING_PAIR "!#"

SoftwareSerial LCDSerial(LCD_PORT_RX, LCD_PORT_TX);
MFRC522 mfrc522(SS_PIN, RST_PIN);

String activingCard = "";
int livingCount = 0;
unsigned long lastReadingTime = 0;
unsigned long requestReadingInterval = 1000;

void setup() 
{ 
  Serial.begin(9600);        // Initialize serial communications with the PC
  LCDSerial.begin(9600);
  ClearLCD();
  
  SPI.begin();                // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
}

void loop() 
{
  if (Serial)
  {
     if (Serial.available()) 
     {
       if (Serial.read() == '!')
       {
         Serial.print(STRING_PAIR);
       }
       Serial.flush();
     }
  }
  
  if (millis() - lastReadingTime > requestReadingInterval || millis() - lastReadingTime < 0)
  {
    // Look for new cards
    if ( !mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
      livingCount--;
      if (livingCount <= 0)
      {
        Serial.print(STRING_EMPTY);
        livingCount = 0;
        activingCard = "";
        
        ClearLCD();
        LocateLCD(0,0);
        PrintLCD(STRING_EMPTY);
      }
      else
      {
        Serial.print(STRING_ALIVING);
        
        LocateLCD(1,0);
        PrintLCD(String(livingCount));
      }
    }
    else
    { 
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
        Serial.print(STRING_NEW);
        Serial.print(activingCard);
        Serial.print("#");
      }
      else
      {
        Serial.print(STRING_ALIVING);
      }
      
      livingCount = RETRY_MAX;
      
      ClearLCD();
      LocateLCD(0,0);
      PrintLCD(STRING_NEW + activingCard + "#");
      LocateLCD(1,0);
      PrintLCD(String(livingCount));
    }
    
    lastReadingTime = millis();
  }
}


int InitLCD()
{
  int ret = SendLCD("L;");
  
  if (ret == 1) // Force Error
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


int PrintLCD(String text)
{
  String buf = "ss" + text + ";";
  return SendLCD(buf);
}

int LocateLCD(int x, int y)
{
  if (x < 0 || y < 0 || x > 15 || y > 15) return 1;

  String buf = "sd" + (String)x + "," + (String)y + ";";
  return SendLCD(buf);
}

int ClearLCD()
{
  return SendLCD("sc;");
}

int swBackLightLCD(int isTurnOn)
{
  if (isTurnOn == 1)
  {
    return SendLCD("sb1;");
  }
  else
  {
    return SendLCD("sb0;");
  }
}

int SendLCD(String buf)
{
  int ret = 0;
  int retryCount = 0;
  char readBuf = NULL;
  
  LCDSerial.flush();
  LCDSerial.print(buf);
  
  while (retryCount < 10)
  {
    retryCount++;
    
    if (LCDSerial.available() > 0) 
    {
      readBuf = (char)LCDSerial.read();
      
      if (readBuf == 'O')
      {
        ret = 0;
        break;
      }
      else if (readBuf == 'E')
      {
        ret = 1;
        delay(20);
      }
      else
      {
        ret = 2;
        delay(20);
      }
    }
    else
    {
      ret = 3;
      delay(20);
    }
  }
  
  return ret;
}

