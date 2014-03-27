#include <SoftwareSerial.h>
#include <Library_LCD.h>
#include <dht11.h>

#define DSM501A_WARMUP  2
#define DHT_SENSOR_ERR  -255
#define FAN_SPEED_MAX   249
#define FAN_SPEED_MIN   0

#define PIN_LED_RED     9
#define PIN_LED_GREEN   6
#define PIN_LED_BLUE    5
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
  
  starttime = millis();
}

void loop()
{
  if (true == GetHumidityAndTemperature(&hum, &tem))
  {
    lcd.LocateLCD(0, 0);
    lcd.PrintLCD((String)hum + "%/" + (String)tem + "C");
  }
  else
  {
    lcd.LocateLCD(0, 0);
    lcd.PrintLCD("---/---");
  }
  
  if (true == GetDust() && pm != 0)
  {
    lcd.LocateLCD(1, 7);
    lcd.PrintLCD((String)pm + "   ");
  }
  
  if (pm == 0)
  {
    fanSpeed = 0;
    led_r = 0;
    led_g = 0;
    led_b = 255;
    
    lcd.LocateLCD(0, 13);
    lcd.PrintLCD("MIN");    
  }
  else if (pm > 0 && pm <= 200)
  {
    fanSpeed = 0;
    led_r = 0;
    led_g = 255;
    led_b = 0;
    
    lcd.LocateLCD(0, 13);
    lcd.PrintLCD("MIN");
  }
  else if (pm > 200 && pm <= 500)
  {
    fanSpeed = 50;
    led_r = 255;
    led_g = 255;
    led_b = 0;
    
    lcd.LocateLCD(0, 13);
    lcd.PrintLCD("MED");    
  }
  else
  {
    fanSpeed = 99;
    led_r = 255;
    led_g = 0;
    led_b = 0;
    
    lcd.LocateLCD(0, 13);
    lcd.PrintLCD("MAX");
  }
  
  ControlFan(fanSpeed);
  
  LedFade(led_r, led_g, led_b, 1);
}

void DrawLCD()
{
  // |0123456789ABCDEF|
  // |----------------|
  //  xx%/xxC  Fan:xxx  
  //  PM2.5: xxx
  // |----------------|
  
  lcd.ClearLCD();
  
  lcd.LocateLCD(0, 0);
  lcd.PrintLCD("---/---");
  
  lcd.LocateLCD(0, 9);
  lcd.PrintLCD("Fan:---");
  
  lcd.LocateLCD(1, 0);
  lcd.PrintLCD("PM2.5: ---");
}

void LedFade(int R, int G, int B, int interval)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 255; fadeValue += interval) {
    LedRGB(R * fadeValue / 255, G * fadeValue / 255, B * fadeValue / 255);
    delay(10);                            
  } 
  
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -= interval) { 
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
      pm = (1.00 - offsetFix) * pm / ratio;
  
      //Serial.print(ratio);
      //Serial.print(",");
      //Serial.println(pm);
      //Serial.println(endtime - starttime);
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
