#include <SoftwareSerial.h>

#define LCD_PORT_RX 12
#define LCD_PORT_TX 11
#define LIGHT_SENSOR_PORT 0

SoftwareSerial LCDSerial(LCD_PORT_RX, LCD_PORT_TX);

void setup()
{
   Serial.begin(9600); 
   LCDSerial.begin(9600);
   
   if (ClearLCD() != 0)
   {
     Serial.println("Failed to Init LCD.");
   }
}

void loop()
{
  String buf = "";
  
  while (Serial.available() > 0) 
  {
    buf += (char)Serial.read();
    delay(5);
  }
  
  if (buf.length() > 0)
  {
    Serial.println("DEBUG: loop(): buf=" + buf);
    
    ClearLCD();
    LocateLCD(1,0);
    PrintLCD(buf);
  }
  
  LocateLCD(0,0);
  PrintLCD("Analog[" + (String)LIGHT_SENSOR_PORT + "]: " + (String)GetAnalogSensor(LIGHT_SENSOR_PORT) + "  ");
  delay(500);
}

int GetAnalogSensor(int port)
{
    int val = 0;
    val = analogRead(port);
    
    Serial.println("DEBUG: GetAnalogSensor[" + (String)port + "]: " + (String)val);

    return val;
}

int InitLCD()
{
  int ret = SendLCD("L;");
   
  Serial.println((String)ret);
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
    
  Serial.print("DEBUG: SendLCD: " + buf + "[");
  
  while (retryCount < 10)
  {
    retryCount++;
    
    Serial.print((String)retryCount);
    
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
  
  Serial.println("]");
  return ret;
}

