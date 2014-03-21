#include <dht11.h>

int DHTPin = 5;
int LedRedPin = 4;
int LedGreenPin = 3;
int LedBluePin = 2;

dht11 DHT11;
char buffer;  
int connectStatus = 0;
int freeCount = 0;
String validChar = "RGBLPT!";

void setup(){
     Serial.begin(9600);

     pinMode(LedRedPin, OUTPUT); 
     pinMode(LedGreenPin, OUTPUT); 
     pinMode(LedBluePin, OUTPUT); 
     
     connectStatus = 0;
}

void loop()
{
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
         delay(100);
         waitCount++;
       }
 
       LedFade(10);
     }
     
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
           LedFade(5);
         }
         else if (buffer == 'T')
         {
           int hum = (int)DHT11.humidity;
           int tem = (int)DHT11.temperature;
           char buf[17] = { 0 };
           DHT11.read(DHTPin);
           sprintf(buf, "H%.2dT%.2d|", hum, tem);
           
           if (hum == 0 && tem == 0)
           {
             LedRGB(255,0,0);
           }
           else
           {
             LedRGB(0,0,255);
           }
           
           delay(50);
           LedRGB(0,0,0);
           
           Serial.print(buf);
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
        LedFade(2);
      }
      else
      {
        freeCount++;
        LoopInActive(1);
      }
    }
  }
  else
  {
     LedFade(10);
     connectStatus = 0;
  }
}

void LedFade(int fadeSpeed)
{
  LedRGB(0,0,0);
  
  for(int fadeValue = 0; fadeValue <= 250; fadeValue += fadeSpeed) { 
    if (Serial.available()) return;
    analogWrite(LedGreenPin, fadeValue);
    delay(50);                            
  } 
  delay(400);
  
  for(int fadeValue = 250 ; fadeValue >= 0; fadeValue -= fadeSpeed) { 
    if (Serial.available()) return;
    analogWrite(LedGreenPin, fadeValue);
    delay(50);                          
  }
  delay(800);
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
