#include <SoftwareSerial.h>
#include <Library_LCD.h>

Library_LCD lcd;
int controlPin = 3;
int speeds = 0;

int LedRedPin = 5;
int LedGreenPin = 6;
int LedBluePin = 9;

void setup()
{
  lcd.ClearLCD();
  
  TCCR2A = 0x23;
  TCCR2B = 0x0C;  // select timer2 clock as 16 MHz I/O clock / 64 = 250 kHz
  OCR2A = 249;  // top/overflow value is 249 => produces a 1000 Hz PWM
  pinMode(controlPin, OUTPUT);  // enable the PWM output
  OCR2B = 125;  // set the PWM to 50% duty cycle
  
  LedRGB(128,0,128);
  delay(2000);
  LedRGB(0,0,0);
}

void loop()
{
  OCR2B = 0;
  lcd.LocateLCD(0, 0);
  lcd.PrintLCD("Fan: " + (String)OCR2B + " / 249  ");
  LedFade(LedRedPin);
  
  OCR2B = 249;
  lcd.LocateLCD(0, 0);
  lcd.PrintLCD("Fan: " + (String)OCR2B + " / 249  ");
  LedFade(LedBluePin);
}

void LedFade(int RGBPin)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 255; fadeValue += 5) { 
    analogWrite(RGBPin, fadeValue);
    delay(50);                            
  } 
  delay(200);
  
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) { 
    analogWrite(RGBPin, fadeValue);
    delay(50);                          
  }
  delay(200);
}

void LedRGB(int R, int G, int B)
{
    analogWrite(LedRedPin, R);
    analogWrite(LedGreenPin, G);
    analogWrite(LedBluePin, B);
}
