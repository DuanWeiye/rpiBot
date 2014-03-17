int switch1 = 0;
int switch2 = 1;

int pin1 = 2;
int pin2 = 3;

void setup()
{
  Serial.begin(9600);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  
  delay(3000);
  Serial.print("sc;");
  delay(20);
  Serial.print("sd0,0;");
  delay(20);
  Serial.print("ssSwitch1:;");
  delay(20);
  Serial.print("sd1,0;");
  delay(20);
  Serial.print("ssSwitch2:;");
  delay(20);
}
 
void loop()
{
  Serial.print("sd0,9;");
  delay(20);
  if (switch1 == 1)
  {
    Serial.print("ssON ;");
    delay(20);
    digitalWrite(pin1, LOW);
    switch1 = 0;
  }
  else
  {    
    Serial.print("ssOFF;");
    delay(20);
    digitalWrite(pin1, HIGH);
    switch1 = 1;
  }

  Serial.print("sd1,9;");
  delay(20);
  if (switch2 == 1)
  {
    Serial.print("ssON ;");
    delay(20);
    digitalWrite(pin2, LOW);
    switch2 = 0;
  }
  else
  {    
    Serial.print("ssOFF;");
    delay(20);
    digitalWrite(pin2, HIGH);
    switch2 = 1;
  }
  
  delay(1000);
}
