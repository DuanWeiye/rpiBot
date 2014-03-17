#include <Servo.h> 

Servo myservo;
int inputPin=2;
int outputPin=3;
int servoPin=4;

void setup()
{
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
  myservo.attach(servoPin);
  myservo.write(90);
}

void loop()
{
  int distance = 0;
  
  myservo.write(0);
  delay(500);
  distance = sendEcho();

  myservo.write(180);
  delay(1000);
  distance = sendEcho();
  
  myservo.write(90);
  delay(500);
  distance = sendEcho();
  
  delay(1000);
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
