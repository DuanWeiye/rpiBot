 
#include <dht11.h>

dht11 DHT11;

#define DHT11PIN 2

int hum = 0;
int temp = 0;

void setup()
{
  Serial.begin(9600);
  Serial.print("sc;");
  delay(20);
  Serial.print("sd0,0;");
  delay(20);
  Serial.print("ssHumidity:;");
  delay(20);
  Serial.print("sd1,0;");
  delay(20);
  Serial.print("ssTemperature:;");
  delay(20);
}
 
void loop()
{
  int tmp_hum = 0;
  int tmp_temp = 0;
  char buf[17] = { 0 };
  int chk = DHT11.read(DHT11PIN);
 
  tmp_hum = (int)DHT11.humidity;
  tmp_temp = (int)DHT11.temperature;
 
  if (tmp_hum != hum)
  {
    hum = tmp_hum;
    Serial.print("sd0,9;");
    delay(20);
    sprintf(buf, "ss%.2d%% ;", hum);
    Serial.print(buf);
    delay(20);
  }
  
  if (tmp_temp != temp)
  {
    temp = tmp_temp;
    Serial.print("sd1,12;");
    delay(20);
    sprintf(buf, "ss%.2dC ;", temp);
    Serial.print(buf);
    delay(20);
  }
  
  delay(2000);
}
