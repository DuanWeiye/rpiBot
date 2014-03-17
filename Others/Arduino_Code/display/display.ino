void setup()
{
   Serial.begin(9600); 
}

void loop()
{
  Serial.print("sc;");
  delay(1000);
  Serial.print("sd1,0;");
  delay(100);
  Serial.print("ssABCDEFG;");
  /*
  if(Serial.available() > 0)
  {
    char input = Serial.read();
    showText("text",(String)input);
  }
  */
  delay(1000);
}

int showText(String textA, String textB)
{
  char textBuf[16] = { 0 };
  //if (textA.length > 15 || textB.length > 15) return 0;

  Serial.print("sc;");
  getRet();
  Serial.print("sd0,0;");
  getRet();
  
  sprintf(textBuf, "ss %s;", textA);
  Serial.print(textBuf);
  getRet();
  
  Serial.print("sd1,0;");
  getRet();
  sprintf(textBuf, "ss %s;", textB);
  Serial.print(textBuf);
}

int getRet()
{
  while (Serial.available() > 0)
  {
    char ret = Serial.read();
    if (ret == 'O')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}
