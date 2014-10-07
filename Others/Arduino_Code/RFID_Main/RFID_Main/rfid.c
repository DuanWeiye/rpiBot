// make: cc ./rfid.c -o rfid -lwiringPi

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

#define RFID_DEVICE				"/dev/ttyUSB0"
//#define RFID_DEVICE 				"/dev/ttyAMA0"

#define RFID_BITRATE				9600
#define SERIAL_TIMEOUT_RET			-1
#define BUFFER_LENGTH				64

#define STRING_EMPTY 				"E#"
#define STRING_NEW 				"N#"
#define STRING_ALIVING 				"A#"
#define STRING_PAIR 				"!#"

#define RFID_PROC_RET_EMPTY			1
#define RFID_PROC_RET_NEW			2
#define RFID_PROC_RET_ALIVING			3
#define RFID_PROC_RET_ERROR			0

int rfidPort = -1;
char activingCard[BUFFER_LENGTH] = { 0 };

int initRFID();
int RFIDProc();
int file_exist(char *filename);
int GetCurrentTime(char* text, int length);

int main (void)
{
	if (wiringPiSetup() == -1) return 1;
	
	while(1)
	{
		initRFID();
		if (rfidPort >= 0)
		{
			int ret = RFID_PROC_RET_ERROR;
			
			ret = RFIDProc();
			
			switch (ret)
			{
				case RFID_PROC_RET_EMPTY:
					// Waitting for card
					break;
				case RFID_PROC_RET_NEW:
					// New card
					printf("New Card Found: %s\n", activingCard);
					break;
				case RFID_PROC_RET_ALIVING:
					// Keep touching
					break;
				case RFID_PROC_RET_ERROR:
				default:
					// Failed reading and ...
					break;
			}
		}
		
		delay(2000);
	}
	
	serialClose(rfidPort);
	
	return 0;
}

int file_exist(char *filename)
{
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

int initRFID()
{
	if (file_exist(RFID_DEVICE))
	{
		if (rfidPort < 0)
		{
			rfidPort = serialOpen(RFID_DEVICE, RFID_BITRATE);
			
			if (rfidPort < 0)
			{
				//printf("Error: Unable to open RFID device.\n");
				return 1;
			}
			else
			{
				//printf("Open RFID device successed.\n");
				serialFlush(rfidPort);
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		rfidPort = -1;
		return 1;
	}
}

int RFIDProc()
{
	int ret = RFID_PROC_RET_ERROR;
	int isNewCard = 0;
	char buf;
	char full_buf[BUFFER_LENGTH] = { 0 };
	char *ptr = &full_buf[0];
	
	serialFlush(rfidPort);
	
	while ((buf = serialGetchar(rfidPort)) != SERIAL_TIMEOUT_RET)
	{
		if (buf == '#')
		{
			strcpy(ptr, &buf);
			ptr = &full_buf[0];
			
			if (strcmp(full_buf, STRING_EMPTY) < 0)
			{
				ret = RFID_PROC_RET_EMPTY;
			}
			else if (strcmp(full_buf, STRING_NEW) < 0)
			{
				isNewCard = 1;
				memset(full_buf, 0, BUFFER_LENGTH);
				continue;
			}
			else if (strcmp(full_buf, STRING_ALIVING) < 0)
			{
				ret = RFID_PROC_RET_ALIVING;
			}
			else
			{
				if (isNewCard == 1)
				{
					memset(activingCard, 0, BUFFER_LENGTH);
					strncpy(activingCard, full_buf, BUFFER_LENGTH);
					
					ret = RFID_PROC_RET_NEW;
				}
				else
				{
					ret = RFID_PROC_RET_ERROR;
				}
			}
			
			break;
		}
		else
		{
			if (ptr - &full_buf[0] < BUFFER_LENGTH)
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
	
	serialFlush(rfidPort);
	
	return ret;
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

