#include <dht11.h>

dht11 DHT11;

#define DHT11PIN       4
#define SONARINPUTPIN  2
#define SONAROUTPUTPIN 3
#define BACKLIGHTMAX   10
#define SHOWDIST       100

int hum = 0;
int temp = 0;
int blight = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(SONARINPUTPIN, INPUT);
  pinMode(SONAROUTPUTPIN, OUTPUT);
  
  Serial.print("sc;");
  delay(20);
  Serial.print("sd0,0;");
  delay(20);
  Serial.print("ssHum/Temp:;");
  delay(20);
  Serial.print("sd1,0;");
  delay(20);
  Serial.print("ssDis/CD:;");
  delay(20);
}
 
void loop()
{
  int backlight = 3;
  int tmp_hum = 0;
  int tmp_temp = 0;
  int distance = sendEcho();
  char buf[17] = { 0 };
  int chk = DHT11.read(DHT11PIN);
 
  tmp_hum = (int)DHT11.humidity;
  tmp_temp = (int)DHT11.temperature;
 
  if (tmp_hum != hum || tmp_temp != temp)
  {
    hum = tmp_hum;
    temp = tmp_temp;
    
    Serial.print("sd0,9;");
    delay(20);
    sprintf(buf, "ss%.2d%%/%.2dC;", hum, temp);
    Serial.print(buf);
    delay(20);
  }

  memcpy(buf, 0, 17);
  
  if (distance > 500)
  {
    if (blight > 0) blight--;
    
    if (blight == 0)
    {
      Serial.print("sb0;");
      delay(20);
    }
    
    sprintf(buf, "ssInf./%ds  ;", blight);
  }
  else if (distance <= 500 && distance > SHOWDIST)
  {
    if (blight > 0) blight--;
    
    if (blight == 0)
    {
      Serial.print("sb0;");
      delay(20);
    }

    sprintf(buf, "ss%dcm/%ds  ;", distance, blight);
  }
  else
  {
    if (blight == 0)
    {
      Serial.print("sb1;");
      delay(20);
    }
    blight = BACKLIGHTMAX;
    
    sprintf(buf, "ss%dcm/%ds  ;", distance, blight);
  }
  
  Serial.print("sd1,7;");
  delay(20);
  Serial.print(buf);
  
  delay(1000);
}


int sendEcho()
{
  int ret = 0;
  
  digitalWrite(SONAROUTPUTPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(SONAROUTPUTPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONAROUTPUTPIN, LOW);
  
  ret = pulseIn(SONARINPUTPIN, HIGH) / 58;
  
  Serial.println(ret);
  return ret;
}
