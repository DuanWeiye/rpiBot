#include <SoftwareSerial.h>
#include <Library_LCD.h>
#include <dht11.h>

#define DHT_SENSOR_ERR  -255
#define FAN_SPEED_MAX   249
#define FAN_SPEED_MIN   0

#define PIN_LED_RED     9
#define PIN_LED_GREEN   6
#define PIN_LED_BLUE    5
#define PIN_FAN         3
#define PIN_DHT         4

dht11 dhtSensor;
Library_LCD lcd;

int fanSpeed = 0;
int led_r = 0;
int led_g = 0;
int led_b = 0;

int hum = 0;
int tem = 0;

void setup()
{
  Serial.begin(9600);
  
  lcd.ClearLCD();
  
  TCCR2A = 0x23;
  TCCR2B = 0x0C;  // select timer2 clock as 16 MHz I/O clock / 64 = 250 kHz
  
  OCR2A = FAN_SPEED_MAX;
  ControlFan(0);
  
  pinMode(PIN_FAN, OUTPUT);
  
  lcd.ClearLCD();
  
  LedFade(255,0,0,25);
  LedFade(0,255,0,25);
  LedFade(0,0,255,25);
  
  DrawLCD();
}

void loop()
{
  if (true == GetHumidityAndTemperature(&hum, &tem))
  {
    lcd.LocateLCD(0, 4);
    lcd.PrintLCD((String)hum + "/" + (String)tem);
  }
  else
  {
    lcd.LocateLCD(0, 4);
    lcd.PrintLCD("--/--");
  }
  
  if (hum == DHT_SENSOR_ERR || tem ==DHT_SENSOR_ERR)
  {
    fanSpeed = 99;
    led_r = 255;
    led_g = 0;
    led_b = 0;
  }
  else if (hum > 30 && hum < 40 && tem > 18 && tem < 32)
  {
    fanSpeed = 25;
    led_r = 0;
    led_g = 255;
    led_b = 0;
  }
  /*
  else
  {
    fanSpeed = 50;
    led_r = hum < 50 ? hum * 255 / 50 : 255;
    led_g = tem < 50 ? tem * 255 / 50 : 255;
    led_b = tem < hum ? ( hum - tem > 10 ? 255 : ( hum - tem ) * 255 / 10 ) : ( tem - hum > 10 ? 255 : ( tem - hum ) * 255 / 10 );
  }
  */
  else
  {
    fanSpeed = 50;
    led_r = 255;
    led_g = 255;
    led_b = 0;
  }

  ControlFan(fanSpeed);  
  lcd.LocateLCD(0, 14);
  lcd.PrintLCD(fanSpeed);
  lcd.LocateLCD(1, 5);
  lcd.PrintLCD("           ");
  lcd.LocateLCD(1, 5);
  lcd.PrintLCD((String)led_r + "/" + (String)led_g + "/" + (String)led_b);
  LedFade(led_r, led_g, led_b, 5);
}

void DrawLCD()
{
  // |0123456789ABCDEF|
  // |----------------|
  //  H/T:xx/xx Fan:xx  
  //  Col: 255/255/255
  // |----------------|
  
  lcd.ClearLCD();
  lcd.LocateLCD(0, 0);
  lcd.PrintLCD("H/T:");
  
  lcd.LocateLCD(0, 10);
  lcd.PrintLCD("Fan:");
  
  lcd.LocateLCD(1, 0);
  lcd.PrintLCD("Col:");
}

void LedFade(int R, int G, int B, int interval)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 255; fadeValue += interval) { 
    LedRGB(R * fadeValue / 255, G * fadeValue / 255, B * fadeValue / 255);
    delay(50);                            
  } 
  delay(200);
  
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -= interval) { 
    LedRGB(R * fadeValue / 255, G * fadeValue / 255, B * fadeValue / 255);
    delay(50);                          
  }
  delay(200);
  
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
