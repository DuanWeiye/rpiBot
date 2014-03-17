int inputPin=2;
int outputPin=3;

void setup()
{
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);

  delay(3000);
  Serial.print("sc;");
  delay(20);
  Serial.print("sd0,0;");
  delay(20);
  Serial.print("ssDistance:;");
  delay(20);
}

void loop()
{
  int distance = sendEcho();
  char buf[17] = { 0 };

  if (distance > 500)
  {
    strcpy(buf, "ssInf.   ;");
  }
  else
  {
    sprintf(buf, "ss%d cm  ;", distance);  
  }
  
  Serial.print("sd0,10;");
  delay(20);
  Serial.print(buf);

  delay(200);
}

int sendEcho()
{
  int ret = 0;
  
  digitalWrite(outputPin, LOW);
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW);
  
  ret = pulseIn(inputPin, HIGH) / 58;
  
  Serial.println(ret);
  return ret;
}
