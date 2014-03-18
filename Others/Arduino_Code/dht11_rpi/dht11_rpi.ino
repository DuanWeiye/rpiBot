
#include <dht11.h>

dht11 DHT11;

#define DHT11PIN 2

char buffer;
int connectStatus = 0;

void setup(){
     Serial.begin(9600);
     
     connectStatus = 0;
}

void loop()
{
  if (Serial)
  {
     if (connectStatus == 0)
     {
       Serial.print("A");
       connectStatus = 1;
       
       return;
     }
     
     if (Serial.available()) 
     {
       buffer = Serial.read();
       if (buffer == 'T')
       {
         char buf[17] = { 0 };
         DHT11.read(DHT11PIN);
         sprintf(buf, "H%.2dT%.2d|", (int)DHT11.humidity, (int)DHT11.temperature);
         Serial.print(buf);
       }
       else
       {
         Serial.print("E");
       }
       //Serial.print("O");
     }
  }
  else
  {
     connectStatus = 0;
  }
  
  delay(10);
}
