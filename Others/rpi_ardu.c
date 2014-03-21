// make: cc ./lcd.c -o lcd -lwiringPi

#include <wiringPi.h>
#include <wiringSerial.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#define ARDUINO_DEVICE		"/dev/ttyUSB0"
#define LCD_DEVICE 			"/dev/ttyAMA0"

#define LCD_BITRATE			9600
#define ARDUINO_BITRATE		9600

#define LCD_MAX_WIDTH 		16
#define LCD_CMD_ADDON 		5

#define BUFFER_LENGTH 		256
#define SENSOR_ERROR_FLAG 	-255
#define IP_ADDR_LENGTH	 	15
#define IP_ADDR_SHOWTIME 	5

int lcdPort = -1;
int arduinoPort = -1;

int file_exist(char *filename);
int initLCD();
int clearLCD();
int sendLCD(char* buf);
int locateLCD(int x, int y);
int printLCD(char* text);
int swBackLightLCD(int isTurnOn);
int GetCurrentTime(char* text, int length);

int initArduino();
int GetHumAndTemp(int* humidity, int* temperature);

float GetCPULoad();
void GetIPAndMAC(char* ipaddr, int* ipCount);

int main (void)
{
	if (wiringPiSetup() == -1) return 1;
	//printf("\nSyncing system time online...\n");
	//system("sudo sntp -s s1a.time.edu.cn");
	
	int hu = 0;
	int tm = 0;
	int ipArrow = 1;
	int ipTimeLeft = 0;

	int displayLength = LCD_MAX_WIDTH + 1;
	char ip[IP_ADDR_LENGTH + 1] = { 0 };
	char displayBuffer[LCD_MAX_WIDTH + 1] = { 0 };
	
	initLCD();
	initArduino();
	
	while(1)
	{
		// show time [hh:mm]
		memset(displayBuffer, 0, displayLength);
		GetCurrentTime(displayBuffer, displayLength);
		locateLCD(0, 0);
		printLCD(displayBuffer);
		
		// show cpu load
		memset(displayBuffer, 0, displayLength);
		sprintf(displayBuffer, "| CPU:%.2f", GetCPULoad());
		locateLCD(0, 6);
		printLCD(displayBuffer);
		
		if (arduinoPort >= 0)
		{
			// show sensor
			GetHumAndTemp(&hu, &tm);
			memset(displayBuffer, 0, displayLength);
			if (hu == SENSOR_ERROR_FLAG || tm == SENSOR_ERROR_FLAG)
			{
				sprintf(displayBuffer, "Hum/Temp:  N/A  ");
			}
			else
			{
				sprintf(displayBuffer, "Hum/Temp:%d%%/%dC", hu, tm);
			}
			locateLCD(1, 0);
			printLCD(displayBuffer);
		}
		else
		{
			ipTimeLeft--;
			if (ipTimeLeft <= 0)
			{
				ipTimeLeft = IP_ADDR_SHOWTIME;
				int ipCount = 0;
				
				GetIPAndMAC(NULL, &ipCount);
				if (ipCount == 0)
				{
					locateLCD(1, 0);
					printLCD("> No IP Address ");
				}
				else if (ipArrow <= ipCount)
				{
					memset(ip, 0, IP_ADDR_LENGTH + 1);
					GetIPAndMAC(ip, &ipArrow);
					ipArrow++;
					
					locateLCD(1, 0);
					printLCD(">               ");
					locateLCD(1, 1);
					printLCD(ip);
				}
				else
				{
					ipArrow = 1;
				}
			}
		}
		
		delay(2000);
	}

	serialClose(lcdPort);
	serialClose(arduinoPort);
	
	return 0;
}

int file_exist(char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
	//return 1;
}

int initLCD()
{
	if (file_exist(LCD_DEVICE))
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
	else
	{
		lcdPort = -1;
		return 0;
	}
}

int initArduino()
{
	if (file_exist(ARDUINO_DEVICE))
	{
		if (arduinoPort < 0)
		{
			arduinoPort = serialOpen(ARDUINO_DEVICE, ARDUINO_BITRATE);
			
			if (arduinoPort < 0)
			{
				//printf("Error: Unable to open Arduino device.\n");
				return 1;
			}
			else
			{
				serialFlush(arduinoPort);
				if (serialGetchar(arduinoPort) == 'A')
				{
					serialPuts(arduinoPort, "A");
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
	else
	{
		arduinoPort = -1;
		return 1;
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
	
	if (initArduino() == 0)
	{
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
	}	
	
	*humidity = SENSOR_ERROR_FLAG;
	*temperature = SENSOR_ERROR_FLAG;

	return 1;
}

int GetCurrentTime(char* text, int length)
{
	if (length == 0 || text == NULL) return 1;
	
	size_t ret = 0;
	struct tm *ptr;
	time_t lt;
	
	lt=time(NULL);
	
	if (lt > 946080000) // 30 years, 1970 + 30 = 2000
	{
		ptr=localtime(&lt);
		
		ret = strftime(text, length, "%R", ptr);
		
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		sprintf(text, "Offline");
	}
}

float GetCPULoad()
{
	int FileHandler = 0;
	char FileBuffer[BUFFER_LENGTH] = { 0 };
	float load;

	FileHandler = open("/proc/loadavg", O_RDONLY);

	if(FileHandler < 0) 
	{
		return 1; 
	}
	else
	{
		read(FileHandler, FileBuffer, sizeof(FileBuffer) - 1);
		sscanf(FileBuffer, "%f", &load);
		close(FileHandler);
	}
	//return (int)(load * 100);
	return load;
}

void GetIPAndMAC(char* ipaddr, int* ipCount)
{
	int copyCount = 0;
	int isFirstRun = (*ipCount == 0) ? 1 : 0;
	
	struct ifaddrs *addrs, *tmp;
	
	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp)
	{
	    if (tmp->ifa_addr && 
	    	tmp->ifa_addr->sa_family == AF_PACKET && 
	    	!(tmp -> ifa_flags & IFF_LOOPBACK))
	    {
	    	//printf("%s\n", tmp->ifa_name);
			char mac[18] = { 0 };
			char ip[IP_ADDR_LENGTH + 1] = { 0 };

	    	struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
            int i;
            int len = 0;
            for(i = 0; i < 6; i++)
            {
            	len+=sprintf(mac+len,"%02X%s",s->sll_addr[i],i < 5 ? ":":"");
        	}
            
            //printf("%s: %s\n", tmp->ifa_name, mac);
            
    	    int inet_sock;
		    struct ifreq ifr;
		    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

		    strcpy(ifr.ifr_name, tmp->ifa_name);
		    if (ioctl(inet_sock, SIOCGIFADDR, &ifr) < 0)
		    {
		    	printf("ERROR ON GET\n");
		    }
		    else
		    {
		    	sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
		    	//printf("%s\n",ip);
			}
			
			if (strlen(ip) > 0)
			{
				copyCount++;
				
				if (isFirstRun == 1)
				{
					*ipCount = copyCount;
				}
				else
				{
					if (*ipCount == copyCount) 
					{
						strcpy(ipaddr, ip);
					}
				}
			}
	    }
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
}

