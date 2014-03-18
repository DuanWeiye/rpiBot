// make: cc ./lcd.c -o lcd -lwiringPi

#include <wiringPi.h>
#include <wiringSerial.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#define ARDUINO_DEVICE	"/dev/ttyUSB0"
#define LCD_DEVICE 		"/dev/ttyAMA0"

#define LCD_BITRATE		9600
#define ARDUINO_BITRATE	9600

#define LCD_MAX_WIDTH 	16
#define LCD_CMD_ADDON 	5

#define BUFFER_LENGTH 	256

int lcdPort = -1;
int arduinoPort = -1;

int initLCD();
int clearLCD();
int sendLCD(char* buf);
int locateLCD(int x, int y);
int printLCD(char* text);
int swBackLightLCD(int isTurnOn);
int GetCurrentTime(char* text, int length);

int initArduino();
int GetHumAndTemp(int* humidity, int* temperature);

int main (void)
{
	if (wiringPiSetup() == -1) return 1;
	int hu = 0;
	int tm = 0;
	int displayLength = LCD_MAX_WIDTH + 1;
	char displayBuffer[LCD_MAX_WIDTH + 1] = { 0 };
	
	initLCD();
	initArduino();
	
	while(1)
	{
		GetCurrentTime(displayBuffer, displayLength);

		locateLCD(0, 1);
		printLCD(displayBuffer);
		
		memset(displayBuffer, 0, displayLength);
		hu = 0;
		tm = 0;
		GetHumAndTemp(&hu, &tm);
		if (hu != 0 && tm != 0)
		{
			sprintf(displayBuffer, "Hum/Temp:%d%%/%dC", hu, tm);
		}
		else
		{
			sprintf(displayBuffer, "Hum/Temp:--/--");
		}
		locateLCD(1, 0);
		printLCD(displayBuffer);
		
		delay(900);
	}
	
	serialClose(lcdPort);
	serialClose(arduinoPort);
	
	return 0;
}

int initLCD()
{
	if (lcdPort < 0)
	{
		lcdPort = serialOpen(LCD_DEVICE, LCD_BITRATE);
		
		if (lcdPort < 0)
		{
			printf("Error: Unable to open serial device.\n");
			return 1;
		}
		else
		{
			return sendLCD("sc;");
		}
	}
	else
	{
		return 0;
	}
}

int initArduino()
{
	if (arduinoPort < 0)
	{
		arduinoPort = serialOpen(ARDUINO_DEVICE, ARDUINO_BITRATE);
		
		if (arduinoPort < 0)
		{
			printf("Error: Unable to open Arduino device.\n");
			return 1;
		}
		else
		{
			serialFlush(arduinoPort);
			if (serialGetchar(arduinoPort) == 'A')
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	else
	{
		return 0;
	}
}

int printLCD(char* text)
{
	if (initLCD() != 0 || text == NULL) return 1;
	
	int bufLen = strlen(text);
	char buf[LCD_MAX_WIDTH + LCD_CMD_ADDON] = { 0 };

	sprintf(buf, "ss%s;", text);
	return sendLCD(buf);
}

int locateLCD(int x, int y)
{
	if (initLCD() != 0 || x < 0 || y < 0 || x > 15 || y > 15) return 1;

	char buf[3 * LCD_CMD_ADDON] = { 0 };

	sprintf(buf, "sd%d,%d;", x, y);
	
	return sendLCD(buf);
}

int clearLCD()
{
	if (initLCD() != 0) return 1;

	return sendLCD("sc;");
}

int swBackLightLCD(int isTurnOn)
{
	if (initLCD() != 0) return 1;

	if (isTurnOn == 1)
	{
		return sendLCD("sb1;");
	}
	else
	{
		return sendLCD("sb0;");
	}

}

int sendLCD(char* buf)
{
	if (initLCD() != 0 || buf == NULL) return 1;

	int ret = 0;
	int retryCount = 0;

	while (retryCount < 10)
	{
		retryCount++;

		serialFlush(lcdPort);
		serialPuts(lcdPort, buf);
		
		// printf("Send To LCD: [%s]\n", buf);
		
		if (serialGetchar(lcdPort) == 'O')
		{
			ret = 0;
			break;
		}
		else
		{
			ret = 1;
			delay(10);
		}
	}
	
	return ret;
}

int GetHumAndTemp(int* humidity, int* temperature)
{
	char *ptr;
	char buf;
	char hum_buf[BUFFER_LENGTH] = { 0 };
	char temp_buf[BUFFER_LENGTH] = { 0 };	

	serialFlush(arduinoPort);
	serialPuts(arduinoPort, "T");
	
	while ((buf = serialGetchar(arduinoPort)) != -1)
	{
		// printf("get: %c\n", buf);
		
		if (buf == '|' || buf == '\n' || buf == 'O' || buf == 'E')
		{
			break;
		}
		else if (buf == 'H')
		{
			ptr = &hum_buf[0];
		}
		else if (buf == 'T')
		{
			ptr = &temp_buf[0];
		}
		else
		{
			strcpy(ptr, &buf);
			ptr++;
		}
	}
	
	serialFlush(arduinoPort);
	
	if (strlen(hum_buf) > 0 && strlen(temp_buf) > 0)
	{
		*humidity = atoi(hum_buf);
		*temperature = atoi(temp_buf);
		
		return 0;
	}
	else
	{
		return 1;
	}
}

int GetCurrentTime(char* text, int length)
{
	if (length == 0 || text == NULL) return 1;
	
	size_t ret = 0;
	struct tm *ptr;
	time_t lt;
	
	lt=time(NULL);
	ptr=localtime(&lt);
	
	ret = strftime(text, length, "%m/%d %T", ptr);
	
	if (ret == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
	
}
