int bigLPin = 5;
int LedRedPin = 2;
int LedGreenPin = 3;
int LedBluePin = 4;

void setup(){
     Serial.begin(9600);
     pinMode(bigLPin, OUTPUT);
     pinMode(LedRedPin, OUTPUT); 
     pinMode(LedGreenPin, OUTPUT); 
     pinMode(LedBluePin, OUTPUT); 
     
     LedRGB(255,0,0);
     delay(500);
     LedRGB(0,0,0);
}

void loop(){
    char input;
    if (Serial.available()) {
       input = Serial.read();
       Serial.print("character recieved: ");
       Serial.println(input);
       
       if (input == '0')
       {
         digitalWrite(bigLPin, HIGH);
       }
       else
       {
         digitalWrite(bigLPin, LOW); 
       }
   }
}

void LedFade(int R, int G, int B)
{
    for(int fadeValue = 0; fadeValue <= 255; fadeValue +=5) { 
      analogWrite(LedRedPin, (R * fadeValue / 255));
      analogWrite(LedGreenPin, (G * fadeValue / 255));
      analogWrite(LedBluePin, (B * fadeValue / 255));
      delay(30);                            
    } 
  
    for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) { 
      analogWrite(LedRedPin, (R * fadeValue / 255));
      analogWrite(LedGreenPin, (G * fadeValue / 255));
      analogWrite(LedBluePin, (B * fadeValue / 255));
      delay(30);                            
    }
}

void LedRGB(int R, int G, int B)
{
    analogWrite(LedRedPin, R);
    analogWrite(LedGreenPin, G);
    analogWrite(LedBluePin, B);
}
