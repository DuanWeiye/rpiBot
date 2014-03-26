#ifndef LIBRARY_LCD_H_
#define LIBRARY_LCD_H_

#include <Arduino.h>
#include <..\SoftwareSerial\SoftwareSerial.h>

class Library_LCD
{
public:
  Library_LCD();
  Library_LCD(int rx, int tx);
  int PrintLCD(int text);
  int PrintLCD(String text);
  int LocateLCD(int x, int y);
  int ClearLCD();
  int swBackLightLCD(int isTurnOn);
private:  
  int SendLCD(String buf);
};

#endif
