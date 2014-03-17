int switch1 = 1;
int switch2 = 0;

int pin1 = 2;
int pin2 = 3;

void setup()
{
  Serial.begin(9600);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  digitalWrite(pin1, switch1);
  digitalWrite(pin2, switch2);
  
  Serial.print("sc;");
  delay(20);
  Serial.print("sd0,0;");
  delay(20);
  Serial.print("ssSwitch:;");
  delay(20);
  Serial.print("sd1,0;");
  delay(20);
  Serial.print("ssLeft:;");
  delay(20);
}
 
void loop()
{ 
  Serial.print("sd0,8;");
  delay(20);
  if (switch1 == 1)
  {
    Serial.print("ssON ->;");
    delay(20);
    switch1 = 0;
  }
  else
  {    
    Serial.print("ssOFF->;");
    delay(20);
    switch1 = 1;
  }
  
  if (switch2 == 1)
  {
    Serial.print("ssON ;");
    delay(20);
    switch2 = 0;
  }
  else
  {    
    Serial.print("ssOFF;");
    delay(20);
    switch2 = 1;
  }
 
  digitalWrite(pin1, switch1);
  
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 4 second;");
  delay(1000);

  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 3 second;");
  delay(1000);
  
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 2 second;");
  delay(1000);
  
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 1 second;");
  delay(1000);
  digitalWrite(pin2, switch2);
    
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 4 second;");
  delay(1000);

  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 3 second;");
  delay(1000);
  
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 2 second;");
  delay(1000);
  
  Serial.print("sd1,6;");
  delay(20);
  Serial.print("ss 1 second;");
  delay(1000);
}
