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
     }
     
     if (Serial.available()) 
     {
       buffer = Serial.read();
       if (buffer == 'T')
       {
         Serial.print("O");
       }
       else
       {
         Serial.print("E");
       }
     }
  }
  else
  {
     delay(10);
     connectStatus = 0;
  }
}
