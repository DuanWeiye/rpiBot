#include <EEPROM.h>
#include <avr/wdt.h>

#define EEPROMADDR     0
#define LedRedPin      5
#define LedGreenPin    6
#define LedBluePin     9

char buffer;
int connectStatus = 0;
int freeCount = 0;
String validChar = "RGBLPT!F";
byte flashMode = 'N';
int flashCount = 0;

void setup()
{
  flashMode = EEPROM.read(EEPROMADDR);
  
  if (flashMode == '1')
  {
    Serial.begin(9600);
    Serial.print("AT+BAUD7");
    
    EEPROM.write(EEPROMADDR, '2');
   
    LedRGB(255,255,255);
    wdt_enable(WDTO_1S);
    delay(10000);
    
  }
  else if (flashMode == '2')
  {
    Serial.begin(57600);
    Serial.print("AT+BAUD4");
    
    EEPROM.write(EEPROMADDR, '3');
  }
  else
  {
    Serial.begin(9600);
  }
  
  pinMode(LedRedPin, OUTPUT); 
  pinMode(LedGreenPin, OUTPUT); 
  pinMode(LedBluePin, OUTPUT); 
   
  connectStatus = 0;
}

void loop()
{
  if (flashMode == '2')
  {
    flashCount++;
    
    if (flashCount < 120)
    {
      LedRGB(0,0,255);
      delay(10);
      LedRGB(0,0,0);
      
      delay(1000);
      return;
    }
    else
    {
      LedRGB(255,255,255);
      delay(100);
      LedRGB(0,0,0);
      delay(100);
      LedRGB(255,255,255);
      wdt_enable(WDTO_1S);
      delay(10000);
    }
  }
  
  if (Serial)
  {
     if (connectStatus == 0)
     {
       int waitCount = 0;
       Serial.print("A");
       
       while (waitCount < 3)
       {
         if (Serial.available()) 
         {
           buffer = Serial.read();
           if (buffer == 'A')
           {
             LedRGB(255,0,255);
             delay(300);
             LedRGB(0,0,0);
             connectStatus = 1;
             return;
           }
         }
         delay(500);
         waitCount++;
       }
 
       LedFade(LedGreenPin, 10);
     }
     else
     {
        if (Serial.available()) 
        {
           freeCount =0;
           buffer = Serial.read();
           
           if (validChar.indexOf(buffer) >= 0)
           {
             if (buffer == '!')
             {
               LedRGB(0,0,0);
             }         
             else if (buffer == 'R')
             {
               LedRGB(255,0,0);
             }
             else if (buffer == 'G')
             {
               LedRGB(0,255,0);
             }
             else if (buffer == 'B')
             {
               LedRGB(0,0,255);
             }
             else if (buffer == 'P')
             {
               LedFade(LedRedPin, 5);
             }
             else if (buffer == 'F')
             {
               LedRGB(0,0,255);
               delay(1000);
               EEPROM.write(EEPROMADDR, '1');
               
               wdt_enable(WDTO_1S);
               delay(10000);
             }
             Serial.print("O");
           }
           else
           {
             Serial.print("E");
           }
        }
        else
        {
          if (freeCount >= 10)
          {
            LedFade(LedRedPin, 2);
          }
          else
          {
            freeCount++;
            LoopInActive(1);
          }
        }
     }
  }
  else
  {
     LedFade(LedBluePin, 10);
     connectStatus = 0;
  }
}

void LedFade(int colorPin, int fadeSpeed)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 250; fadeValue += fadeSpeed) { 
    if (Serial.available()) return;
    analogWrite(colorPin, fadeValue);
    delay(50);                            
  } 
  
  for(int fadeValue = 250 ; fadeValue >= 0; fadeValue -= fadeSpeed) { 
    if (Serial.available()) return;
    analogWrite(colorPin, fadeValue);
    delay(50);    
  }
  
  LedRGB(0,0,0);
}

void LoopInActive(int seconds)
{
  for(int rounds = 0 ; rounds < seconds * 100; rounds++) 
  { 
    if (Serial.available()) return;

    delay(10);
  }
}

void LedRGB(int R, int G, int B)
{
    analogWrite(LedRedPin, R);
    analogWrite(LedGreenPin, G);
    analogWrite(LedBluePin, B);
}
