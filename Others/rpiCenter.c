// make: cc ./rpiCenter.c -o rpiCenter -lwiringPi

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

#define ARDUINO_DEVICE		"/dev/ttyAMA0"
#define ARDUINO_BITRATE		9600

#define BUFFER_LENGTH 		256
#define SENSOR_ERROR_FLAG 	-255
#define IP_ADDR_LENGTH	 	15
#define IP_ADDR_SHOWTIME 	5

int arduinoPort = -1;

int file_exist(char *filename);
int GetCurrentTime(char* text, int length);

int initArduino();
int GetEnvironment(int* humidity, int* temperature, int* smoke);

float GetCPULoad();
void GetIPAndMAC(char* ipaddr, int* ipCount);
int KeepAlive();

int main(void)
{
	if (wiringPiSetup() == -1) return 1;
	
	int hu = 0;
	int tm = 0;
    int sm = 0;
	
	while(1)
	{
		// show time [hh:mm]
		//memset(displayBuffer, 0, displayLength);
		//GetCurrentTime(displayBuffer, displayLength);
		
		// show cpu load
		//memset(displayBuffer, 0, displayLength);
		//sprintf(displayBuffer, "| CPU:%.2f", GetCPULoad());
		
		hu = 0;
		tm = 0;
		sm = 0;
		GetEnvironment(&hu, &tm, &sm);
		
		printf("hu %d, tm %d, sm %d\n", hu,tm,sm);
		
		delay(15000);
	}

	serialClose(arduinoPort);
	
	return 0;
}

int KeepAlive()
{
	if (arduinoPort >= 0)
	{
		serialFlush(arduinoPort);
		serialPutchar(arduinoPort, 'E');
		
		char buf = serialGetchar(arduinoPort);
		
		if (buf == 'E')
		{
			return 0;
		}
		else
		{
			arduinoPort = -1;
			return 1;
		}
	}
	arduinoPort = -1;
	return 1;
}

int file_exist(char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
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
				printf("Error: Unable to open Arduino device.\n");
				return 1;
			}
			else
			{
				printf("Waitting for device...");
				
				serialFlush(arduinoPort);
				serialPutchar(arduinoPort, 'A');
				
				char buf = serialGetchar(arduinoPort);
				
				if (buf == 'A')
				{
					printf("OK\n");
				}
				else
				{
					printf("Failed\n");
					arduinoPort = -1;
					return 1;
				}
				
				serialPutchar(arduinoPort, 'V');
				char textBuf[BUFFER_LENGTH] = { 0 };
				
				while ((buf = serialGetchar(arduinoPort)) != -1)
				{
					if (buf == '|' || buf == '\n' || buf == 'O' || buf == 'E')
					{
						break;
					}
					else
					{
						if (strlen(textBuf) < BUFFER_LENGTH)
						{
							textBuf[strlen(textBuf)] = buf;
						}
						else
						{
							break;
						}
					}
				}
				
				serialFlush(arduinoPort);
				
				if (strlen(textBuf) > 0)
				{
					printf("Device: [%s]\n", textBuf);
					return 0;
				}
				else
				{
					printf("Device: Not Found\n");
					arduinoPort = -1;
					return 1;
				}
			}
		}
		else
		{
			return KeepAlive();
		}
	}
	else
	{
		arduinoPort = -1;
		return 1;
	}
}

int GetEnvironment(int* humidity, int* temperature, int* smoke)
{
	char *ptr = NULL;
	char buf;
	char hum_buf[BUFFER_LENGTH] = { 0 };
	char temp_buf[BUFFER_LENGTH] = { 0 };
    char smoke_buf[BUFFER_LENGTH] = { 0 };
	
	if (initArduino() == 0)
	{
		serialFlush(arduinoPort);
		serialPutchar(arduinoPort, 'T');
		
		while ((buf = serialGetchar(arduinoPort)) != -1)
		{
			//printf("GetEnvironment: BUF: %c\n", buf);
			
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
			else if (buf == 'S')
			{
				ptr = &smoke_buf[0];
			}
			else
			{
				if (ptr != NULL)
				{
					strcpy(ptr, &buf);
					ptr++;
				}
				else
				{
					break;
				}
			}
		}
		
		serialFlush(arduinoPort);
		
		if (strlen(hum_buf) == 0 || strlen(temp_buf) == 0 || strlen(smoke_buf) == 0)
		{
			*humidity = SENSOR_ERROR_FLAG;
			*temperature = SENSOR_ERROR_FLAG;
		    *smoke = SENSOR_ERROR_FLAG;
		    
		    return 1;
		}
		else
		{
			if (atoi(hum_buf) != 0 && atoi(temp_buf) != 0)
			{
				*humidity = atoi(hum_buf);
				*temperature = atoi(temp_buf);
		        *smoke = SENSOR_ERROR_FLAG;
				return 0;
			}
			else if (atoi(smoke_buf) > 0)
			{
	            *humidity = SENSOR_ERROR_FLAG;
	            *temperature = SENSOR_ERROR_FLAG;
				*smoke = atoi(smoke_buf);
				return 0;
			}
			else
			{
		        *humidity = SENSOR_ERROR_FLAG;
		        *temperature = SENSOR_ERROR_FLAG;
	    		*smoke = SENSOR_ERROR_FLAG;
				return 1;
			}
		}
	}

	*humidity = SENSOR_ERROR_FLAG;
	*temperature = SENSOR_ERROR_FLAG;
    *smoke = SENSOR_ERROR_FLAG;

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

