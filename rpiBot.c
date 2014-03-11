/*
Servo Update Rate & Freq
Rate (Hz)  50       100      200      400      500
1E6/Hz     20000    10000    5000     2500     2000
*/

#include <stdio.h>
#include <pigpio.h>

#define PARA_SHOW_MOTOR      "showmotor"
#define PARA_SHOW_SONAR      "showsonar"
#define PARA_SKIP            "skip"
#define PARA_TEST            "t"

//#define SWITCH_TRIGGER       4
#define SONAR_TRIGGER        23
#define SONAR_ECHO           24
#define SERVO_TRIGGER        18
#define SERVO_PWMRATE        50
#define SERVO_PWMFREQ        20000

#define MOTOR_LEFT_SPEED     17
#define MOTOR_LEFT_PIN1      22
#define MOTOR_LEFT_PIN2      27

#define MOTOR_RIGHT_SPEED    25
#define MOTOR_RIGHT_PIN1     8
#define MOTOR_RIGHT_PIN2     7

#define MOTOR_SPEED_MAX      255
#define MOTOR_SPEED_MED      200
#define MOTOR_SPEED_MIN      150
#define MOTOR_SPEED_STOP     0

#define MOTOR_SPEED_UP       1
#define MOTOR_SPEED_DOWN     0

#define MOTOR_FORWARD        1
#define MOTOR_BACKWARD       2
#define MOTOR_FORWARD_LEFT   4
#define MOTOR_BACKWARD_LEFT  8
#define MOTOR_STOP_LEFT      16
#define MOTOR_FORWARD_RIGHT  32
#define MOTOR_BACKWARD_RIGHT 64
#define MOTOR_STOP_RIGHT     128
#define MOTOR_STOP           256

#define SERVO_POS_LEFT       2500
#define SERVO_POS_LMIDDLE    2000
#define SERVO_POS_MIDDLE     1500
#define SERVO_POS_RMIDDLE    1000
#define SERVO_POS_RIGHT      500

#define SKIP_DISTANCE_MAX    100
#define SKIP_DISTANCE_SAFE   45
#define SKIP_DISTANCE_DANGER 30
#define SKIP_DISTANCE_STOP   15

void Init(void);
void EchoOneFrame(void);
void SonarTrigger(void);
void SonarEcho(int gpio, int level, uint32_t tick);
void UpdateAveDistance(int distance);
void MotorAction(int actionType, int speed);
int MotorSpeedChange(int origSpeed, int howToChange);
void FunctionShowMotor(void);
void FunctionSkip(void);
void FunctionTest(void);
int CalcRoute(void);
int IfEmergency(int distance);

int runMode = 0;    // 0:default | 1:show sonar | 2:show motor | 3:skip | 4:skip lite
int isEmergencyInterrupt = 0;
int aveDistance = 0;
int eachDistance[3] = { 0, 0, 0 };

void Init(void)
{
   printf("Init Raspberry Pi GPIO...\n");

   //gpioSetMode(SWITCH_TRIGGER, PI_OUTPUT);
   //gpioSetMode(SWITCH_TRIGGER, PI_OUTPUT);
   //gpioWrite  (SWITCH_TRIGGER, PI_OFF);

   gpioSetMode(SONAR_TRIGGER, PI_OUTPUT);
   gpioWrite  (SONAR_TRIGGER, PI_OFF);

   gpioSetMode(SONAR_ECHO,    PI_INPUT);
   gpioSetAlertFunc(SONAR_ECHO, SonarEcho);
   
   gpioSetMode(SERVO_TRIGGER, PI_OUTPUT);
   gpioSetPWMfrequency(SERVO_TRIGGER, SERVO_PWMRATE);
   gpioSetPWMrange(SERVO_TRIGGER, SERVO_PWMFREQ);
   gpioServo(SERVO_TRIGGER, SERVO_POS_MIDDLE);
   sleep(1);

   gpioSetMode(MOTOR_LEFT_SPEED,  PI_OUTPUT);
   gpioSetMode(MOTOR_LEFT_PIN1,   PI_OUTPUT);
   gpioSetMode(MOTOR_LEFT_PIN2,   PI_OUTPUT);
   gpioSetMode(MOTOR_RIGHT_SPEED, PI_OUTPUT);
   gpioSetMode(MOTOR_RIGHT_PIN1,  PI_OUTPUT);
   gpioSetMode(MOTOR_RIGHT_PIN2,  PI_OUTPUT);
   
   gpioPWM(MOTOR_LEFT_SPEED, 0);
   gpioPWM(MOTOR_RIGHT_SPEED, 0);
}

int CalcRoute(void)
{
    int ret = -1;
	
	EchoOneFrame();
	
	if (eachDistance[0] < SKIP_DISTANCE_STOP ||
	    eachDistance[1] < SKIP_DISTANCE_STOP ||
	    eachDistance[2] < SKIP_DISTANCE_STOP )
	{
		ret = MOTOR_STOP;
	}
	else if (eachDistance[0] > SKIP_DISTANCE_MAX &&
		 eachDistance[1] > SKIP_DISTANCE_MAX &&
		 eachDistance[2] > SKIP_DISTANCE_MAX )
	{
		ret = MOTOR_FORWARD;
	}
	else if (eachDistance[0] < SKIP_DISTANCE_SAFE &&
		 eachDistance[1] < SKIP_DISTANCE_SAFE &&
		 eachDistance[2] < SKIP_DISTANCE_SAFE )
	{
		ret = MOTOR_STOP_LEFT;
	}
	else if (eachDistance[0] > eachDistance[1] &&
		 eachDistance[0] > eachDistance[2] )
	{
		if (eachDistance[1] > SKIP_DISTANCE_SAFE ) 
		{
		   ret = MOTOR_FORWARD_LEFT;
		}
		else
		{
		   ret = MOTOR_STOP_LEFT;
                }
        }
        else if (eachDistance[1] > eachDistance[0] &&
		 eachDistance[1] > eachDistance[2] )
	{
		//Dead End
		ret = MOTOR_STOP_LEFT;
        }
        else if (eachDistance[2] > eachDistance[0] &&
		 eachDistance[2] > eachDistance[1] )
	{
		if (eachDistance[1] > SKIP_DISTANCE_DANGER ) 
		{
		   ret = MOTOR_FORWARD_RIGHT;
		}
		else
		{
		   ret = MOTOR_STOP_RIGHT;
		}
        }
    
    printf("Calc Result: %d\n", ret);
    return ret;
}

void FunctionTest(void)
{
   MotorAction(MOTOR_FORWARD, MOTOR_SPEED_MIN);
   gpioSleep(PI_TIME_RELATIVE, 2, 0);
}

void FunctionSkip(void)
{
   int i = 0;
   gpioSetTimerFunc(0, 50, SonarTrigger);

   while(i < 20)
   {
       i++;
       int stepNext = CalcRoute();

       if (stepNext == MOTOR_STOP_LEFT || stepNext == MOTOR_STOP_RIGHT)
       {
       	   MotorAction(stepNext, MOTOR_SPEED_MED);
       	   gpioSleep(PI_TIME_RELATIVE, 0, 40 * 1000);
           continue;
       }
       else if (stepNext == MOTOR_FORWARD_LEFT || stepNext == MOTOR_FORWARD_RIGHT)
       {
           MotorAction(stepNext, MOTOR_SPEED_MAX);
       	   gpioSleep(PI_TIME_RELATIVE, 0, 500 * 1000);
      	   continue;
       }
       else 
       {
       	   MotorAction(stepNext, MOTOR_SPEED_MED);
       }

       gpioSleep(PI_TIME_RELATIVE, 0, 500 * 1000);
   }
}

void FunctionShowMotor(void)
{
   CalcRoute();
   gpioSetTimerFunc(0, 200, SonarTrigger);

   MotorAction(MOTOR_FORWARD, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_BACKWARD, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_FORWARD_LEFT, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_BACKWARD_LEFT, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_STOP_LEFT, MOTOR_SPEED_MAX);
   sleep(2);
   
   MotorAction(MOTOR_FORWARD_RIGHT, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_BACKWARD_RIGHT, MOTOR_SPEED_MED);
   sleep(2);
   
   MotorAction(MOTOR_STOP_RIGHT, MOTOR_SPEED_MAX);
   sleep(2);

   MotorAction(MOTOR_STOP, MOTOR_SPEED_STOP);
}

int main(int argc, char *argv[])
{
   if (gpioInitialise() < 0) 
   {
      return 1;
   }
   
   Init();

   if (argc == 2)
   {
   	   if (!strcmp(argv[1], PARA_SHOW_MOTOR))
   	   {
                   runMode = 1;
   	           printf("Function: Show Motor\n");
   	           FunctionShowMotor();
   	   }
   	   else if (!strcmp(argv[1], PARA_SHOW_SONAR))
   	   {
                   runMode = 2;
   	           printf("Function: Show Sonar\n");
   	           CalcRoute();
   	   }
   	   else if (!strcmp(argv[1], PARA_SKIP))
   	   {
                   runMode = 3;
   	           printf("Function: Skip\n");
   	           FunctionSkip();
   	   }
   	   else if (!strcmp(argv[1], PARA_TEST))
   	   {
                   runMode = 4;
   	           printf("Function: Test\n");
   	           FunctionTest();
   	   }
   	   else
   	   {
   	   	   //
   	   }
   }
   else
   {
   	   //
   }

   gpioSetTimerFunc(0, 100, NULL);
   MotorAction(MOTOR_STOP, MOTOR_SPEED_STOP);
   printf("Waiting For Threads Stop...\n");
   sleep(2);
   //gpioWrite(SWITCH_TRIGGER, PI_ON);
   MotorAction(MOTOR_STOP, MOTOR_SPEED_STOP);
   gpioTerminate();

   return 0;
}

void UpdateAveDistance(int distance)
{
   aveDistance = (aveDistance + distance) / 2;
}

int IfEmergency(int distance)
{
        int ret = 0;

	if (distance <= SKIP_DISTANCE_STOP)
	{
                ret = 0;
	   	printf("IfEemergency: FORCE STOP\n");
                MotorAction(MOTOR_BACKWARD, MOTOR_SPEED_MIN);
                gpioSleep(PI_TIME_RELATIVE, 1, 0);
	   	MotorAction(MOTOR_STOP_LEFT, MOTOR_SPEED_MIN);
                gpioSleep(PI_TIME_RELATIVE, 0, 500 * 1000);
                MotorAction(MOTOR_STOP, MOTOR_SPEED_STOP);
	}
        else
        {
                ret = 1;
        }

        return ret;
}

void EchoOneFrame(void)
{
	gpioServo(SERVO_TRIGGER, SERVO_POS_LMIDDLE);
	gpioSleep(PI_TIME_RELATIVE, 0, 300000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	eachDistance[0] = aveDistance;
	printf("Left Position Distance: %d\n", aveDistance);
	
	gpioServo(SERVO_TRIGGER, SERVO_POS_RMIDDLE);
	gpioSleep(PI_TIME_RELATIVE, 0, 500000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	eachDistance[2] = aveDistance;
	printf("Right Position Distance: %d\n", aveDistance);
	
	gpioServo(SERVO_TRIGGER, SERVO_POS_MIDDLE);
	gpioSleep(PI_TIME_RELATIVE, 0, 300000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	SonarTrigger();
	gpioSleep(PI_TIME_RELATIVE, 0, 50000);
	eachDistance[1] = aveDistance;
	printf("Middle Position Distance: %d\n", aveDistance);
}

int MotorSpeedChange(int origSpeed, int howToChange)
{
    int ret = 0;
    
    if (howToChange == MOTOR_SPEED_UP)
	{
		if (origSpeed == MOTOR_SPEED_MAX)
		{
			//
		}
		else if (origSpeed == MOTOR_SPEED_MED)
		{
			ret = MOTOR_SPEED_MAX;
		}
		else if (origSpeed == MOTOR_SPEED_MIN)
		{
			ret = MOTOR_SPEED_MED;
		}
		else
		{
			ret = MOTOR_SPEED_MIN;
		}
	}
	else
	{
		if (origSpeed == MOTOR_SPEED_MAX)
		{
			ret = MOTOR_SPEED_MED;
		}
		else if (origSpeed == MOTOR_SPEED_MED)
		{
			ret = MOTOR_SPEED_MIN;
		}
		else if (origSpeed == MOTOR_SPEED_MIN)
		{
			ret = MOTOR_SPEED_STOP;
		}
		else
		{
			//
		}
	}
}

void MotorAction(int actionType, int speed)
{
	int valueLeftSpeed = 0;
	int valueLeftPin1 = 0;
	int valueLeftPin2 = 0;
	int valueRightSpeed = 0;
	int valueRightPin1 = 0;
	int valueRightPin2 = 0;
	
	if (actionType == MOTOR_FORWARD)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 0;
		valueLeftPin2 = 1;
		valueRightSpeed = speed;
		valueRightPin1 = 1;
		valueRightPin2 = 0;
	}
	else if (actionType == MOTOR_BACKWARD)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 1;
		valueLeftPin2 = 0;
		valueRightSpeed = speed;
		valueRightPin1 = 0;
		valueRightPin2 = 1;
	}
	else if (actionType == MOTOR_FORWARD_LEFT)
	{
		valueLeftSpeed = MOTOR_SPEED_MIN; //MotorSpeedChange(speed, MOTOR_SPEED_DOWN);
		valueLeftPin1 = 0;
		valueLeftPin2 = 1;
		valueRightSpeed = speed;
		valueRightPin1 = 1;
		valueRightPin2 = 0;	
	}
	else if (actionType == MOTOR_BACKWARD_LEFT)
	{
		valueLeftSpeed = MOTOR_SPEED_MIN; //MotorSpeedChange(speed, MOTOR_SPEED_DOWN);
		valueLeftPin1 = 1;
		valueLeftPin2 = 0;
		valueRightSpeed = speed;
		valueRightPin1 = 0;
		valueRightPin2 = 1;	
	}
	else if (actionType == MOTOR_STOP_LEFT)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 1;
		valueLeftPin2 = 0;
		valueRightSpeed = speed;
		valueRightPin1 = 1;
		valueRightPin2 = 0;	
	}	
	else if (actionType == MOTOR_FORWARD_RIGHT)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 0;
		valueLeftPin2 = 1;
		valueRightSpeed = MOTOR_SPEED_MIN; //MotorSpeedChange(speed, MOTOR_SPEED_DOWN);
		valueRightPin1 = 1;
		valueRightPin2 = 0;	
	}
	else if (actionType == MOTOR_BACKWARD_RIGHT)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 1;
		valueLeftPin2 = 0;
		valueRightSpeed = MOTOR_SPEED_MIN; //MotorSpeedChange(speed, MOTOR_SPEED_DOWN);
		valueRightPin1 = 0;
		valueRightPin2 = 1;	
	}
	else if (actionType == MOTOR_STOP_RIGHT)
	{
		valueLeftSpeed = speed;
		valueLeftPin1 = 0;
		valueLeftPin2 = 1;
		valueRightSpeed = speed;
		valueRightPin1 = 0;
		valueRightPin2 = 1;	
	}
	else //MOTOR_STOP
	{
		valueLeftSpeed = 0;
		valueLeftPin1 = 0;
		valueLeftPin2 = 0;
		valueRightSpeed = 0;
		valueRightPin1 = 0;
		valueRightPin2 = 0;	
	}
	
	gpioPWM(MOTOR_LEFT_SPEED, valueLeftSpeed);
	gpioWrite(MOTOR_LEFT_PIN1, valueLeftPin1);
	gpioWrite(MOTOR_LEFT_PIN2, valueLeftPin2);
	
	gpioPWM(MOTOR_RIGHT_SPEED, valueRightSpeed);
	gpioWrite(MOTOR_RIGHT_PIN1, valueRightPin1);
	gpioWrite(MOTOR_RIGHT_PIN2, valueRightPin2);
}

void SonarTrigger(void)
{
   gpioWrite(SONAR_TRIGGER, PI_ON);
   gpioDelay(10);
   gpioWrite(SONAR_TRIGGER, PI_OFF);
}

void SonarEcho(int gpio, int level, uint32_t tick)
{
   static uint32_t startTick, firstTick=0;

   int diffTick;

   if (!firstTick) firstTick = tick;

   if (level == PI_ON)
   {
      startTick = tick;
   }
   else if (level == PI_OFF)
   {
      diffTick = tick - startTick;

      int distance = diffTick / 58;
      
      UpdateAveDistance(distance);

      IfEmergency(distance);
   }
}
