#include "Library_LCD.h"

#define LCD_PORT_RX_DEFAULT 12
#define LCD_PORT_TX_DEFAULT 11

SoftwareSerial LCDSerial(LCD_PORT_RX_DEFAULT, LCD_PORT_TX_DEFAULT);

Library_LCD::Library_LCD()
{
    LCDSerial.begin(9600);
}

Library_LCD::Library_LCD(int rx, int tx)
{
    LCDSerial = SoftwareSerial(rx, tx);
    LCDSerial.begin(9600);
}

int Library_LCD::PrintLCD(int text)
{
	return PrintLCD((String)text);
}

int Library_LCD::PrintLCD(String text)
{
  String buf = "ss" + text + ";";
  return SendLCD(buf);
}

int Library_LCD::LocateLCD(int x, int y)
{
  if (x < 0 || y < 0 || x > 15 || y > 15) return 1;

  String buf = "sd" + (String)x + "," + (String)y + ";";
  return SendLCD(buf);
}

int Library_LCD::ClearLCD()
{
  return SendLCD("sc;");
}

int Library_LCD::swBackLightLCD(int isTurnOn)
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

int Library_LCD::SendLCD(String buf)
{
  int ret = 0;
  int retryCount = 0;
  char readBuf = NULL;
  
  LCDSerial.flush();
  LCDSerial.print(buf);
    
  //Serial.print("DEBUG: SendLCD: " + buf + "[");
  
  while (retryCount < 10)
  {
    retryCount++;
    
    //Serial.print((String)retryCount);
    
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
  
  //Serial.println("]");
  return ret;
}

