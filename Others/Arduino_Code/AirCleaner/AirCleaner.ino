#include <SoftwareSerial.h>
#include <Library_LCD.h>
#include <dht11.h>

#define DSM501A_WARMUP  2
#define DHT_SENSOR_ERR  -255
#define FAN_SPEED_MAX   249
#define FAN_SPEED_MIN   0

#define PIN_LED_RED     5
#define PIN_LED_GREEN   6
#define PIN_LED_BLUE    9
#define PIN_FAN         3
#define PIN_DHT         4
#define PIN_DSM501A     10

dht11 dhtSensor;
Library_LCD lcd;

int fanSpeed = 0;
int led_r = 0;
int led_g = 0;
int led_b = 0;

int hum = 0;
int tem = 0;
int pm = 0; // pcs/0.01cf
int pm_warmup_count = 0;

unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;

void setup()
{
  Serial.begin(9600);
  
  lcd.ClearLCD();
  
  TCCR2A = 0x23;
  TCCR2B = 0x0C;  // select timer2 clock as 16 MHz I/O clock / 64 = 250 kHz
  
  OCR2A = FAN_SPEED_MAX;
  ControlFan(0);
  
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_DSM501A,INPUT);
  
  DrawLCD();
   
  LedFade(255,0,0,15);
  LedFade(0,255,0,15);
  LedFade(0,0,255,15);
  
  fanSpeed = 0;
  led_r = 0;
  led_g = 0;
  led_b = 255;
  
  starttime = millis();
}

void loop()
{
  String buf;
  bool retDHT = GetHumidityAndTemperature(&hum, &tem);
  bool retDSM = GetDust();
  
  if (retDHT == true)
  {
    buf = (String)hum + "%/" + (String)tem + "C";
  }
  else
  {
    buf = "---/---";
  }
  
  lcd.LocateLCD(0, 9);
  lcd.PrintLCD(buf);
  
  if (retDSM == true && pm != 0)
  {
    if (pm < 10)
    {
      buf = "----- ";
      
      fanSpeed = 0;
      led_r = 0;
      led_g = 0;
      led_b = 255; 
    }    
    else if (pm >= 1 && pm < 100)
    {
      buf = "  " + (String)pm + "  ";
      
      fanSpeed = 0;
      led_r = 0;
      led_g = 255;
      led_b = 0;
    }
    else if (pm >= 100 && pm < 500)
    {
      buf = " " + (String)pm + "  ";
      
      fanSpeed = 0;
      led_r = 0;
      led_g = 255;
      led_b = 0;
    }
    else if (pm >= 500 && pm < 1000)
    {
      buf = " " + (String)pm + "  ";
      
      fanSpeed = 50;
      led_r = 255;
      led_g = 255;
      led_b = 0;
    }
    else if (pm >= 1000 && pm < 10000)
    {
      buf = (String)pm + "  ";
      
      fanSpeed = 100;
      led_r = 255;
      led_g = 0;
      led_b = 0;
    }
    else
    {
      buf = " " + (String)pm + " ";
      
      fanSpeed = 100;
      led_r = 255;
      led_g = 0;
      led_b = 0;
    }
    
    lcd.LocateLCD(1, 10);
    lcd.PrintLCD(buf);
    
    ControlFan(fanSpeed);
  }
  
  LedFade(led_r, led_g, led_b, 1);
  BTDataTransfer();
}

void DrawLCD()
{
  // |0123456789ABCDEF|
  // |----------------|
  //  Hum/Temp:xx%/xxC  
  //  PM(>2.5): xxxx 
  // |----------------|
  
  lcd.ClearLCD();
  
  lcd.LocateLCD(0, 0);
  lcd.PrintLCD("Hum/Temp:---/---");
  
  lcd.LocateLCD(1, 0);
  lcd.PrintLCD("PM(>2.5): ----- ");    //particulate matter
}

void LedFade(int R, int G, int B, int interval)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 255 && Serial.available() <= 0; fadeValue += interval) 
  {
    LedRGB(R * fadeValue / 255, G * fadeValue / 255, B * fadeValue / 255);
    delay(10);
  } 
  
  for(int fadeValue = 255 ; fadeValue >= 0 && Serial.available() <= 0; fadeValue -= interval) 
  { 
    LedRGB(R * fadeValue / 255, G * fadeValue / 255, B * fadeValue / 255);
    delay(10);
  }
  
  LedRGB(0,0,0);
}

void LedRGB(int R, int G, int B)
{
  analogWrite(PIN_LED_RED, R);
  analogWrite(PIN_LED_GREEN, G);
  analogWrite(PIN_LED_BLUE, B);
}

bool GetHumidityAndTemperature(int* hum, int* tem)
{
  int ret = dhtSensor.read(PIN_DHT);
  
  if (DHTLIB_OK == ret)
  {
    *hum = (int)dhtSensor.humidity;
    *tem = (int)dhtSensor.temperature;
    ret = true;
  }
  else
  {
    *hum = DHT_SENSOR_ERR;
    *tem = DHT_SENSOR_ERR;
    ret = false;
  }
  return ret;
}

bool ControlFan(int speedPercent)
{
  if (speedPercent >= 0 && speedPercent <= 100)
  {
    int totalStep = FAN_SPEED_MAX + (1 - FAN_SPEED_MIN);
    int targetStep = FAN_SPEED_MIN + ( totalStep * speedPercent / 100 );
    
    OCR2B = targetStep;
    
    return true;
  }
  else
  {
    return false;
  }
}

bool GetDust()
{
  duration = pulseIn(PIN_DSM501A, LOW);
  lowpulseoccupancy += duration;
  endtime = millis();
  
  if ((endtime - starttime) > sampletime_ms)
  {
    if (pm_warmup_count < DSM501A_WARMUP)
    {
      pm_warmup_count++;
    }
    else
    {
      ratio = (lowpulseoccupancy - endtime + starttime + sampletime_ms) / (sampletime_ms * 10.0);
      pm = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;
      float offsetFix = (endtime - starttime - 30000) / 30000.0;
      pm = (1.00 - offsetFix) * pm ;/// ratio;
  
      //Serial.print(ratio);
      //Serial.print(", OLD pm: " + (String)pm);
      
      //pm = (1.00 - offsetFix) * pm / ratio;
      
      //Serial.println(", NEW pm: " + (String)pm);
    }
    
    lowpulseoccupancy = 0;
    starttime = millis();
    
    return true;
  }
  else
  {
    //Serial.println(endtime - starttime);
    return false;
  }
}

void BTDataTransfer()
{
   if (Serial.available() > 0) 
   {
      char buffer = Serial.read();   
      
      if (buffer == NULL)
      {
        //
      }   
      else if (buffer == '!')
      {
        LedRGB(0,0,0);
      }         
      else if (buffer == 'A')
      {
        LedRGB(255,0,255);
        delay(300);
        LedRGB(0,0,0);
        
        Serial.print('A');
      }
      else if (buffer == 'V')
      {
        Serial.print("Arduino AirCleaner HW.V1|");
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
        LedFade(255,255,255,5);
      }
      else if (buffer == 'T')
      {
        char buf[255] = { 0 };
        sprintf(buf, "H%.2dT%.2dS%.2d|", hum, tem, pm);
        
        Serial.print(buf);
      }
      else
      {
        Serial.print("E");
      }
   }
}
