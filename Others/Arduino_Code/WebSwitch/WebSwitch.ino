#include <SPI.h>
#include <Ethernet.h>
#include <String.h>

#define PIN_1_SWITCH A0
#define PIN_2_SWITCH A1

static boolean isDHCP = false;
static String myIP = "";
String requestBuffer = "BUF ";
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };

// Assign default IP address for the controller:
IPAddress ip(172, 0, 0, 2);
IPAddress gateway(172, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// Server
char serverName[] = "www.baidu.com";

// Initialize the Ethernet
EthernetServer server(80);

unsigned long lastAttemptTime = 0;                       // last time connected to the server, in milliseconds
unsigned long totalFailedCount = 0;

const unsigned long serverWaitInterval = 10000;
const unsigned long requestAttemptInterval = 60000;     // delay between requests, in milliseconds

//const String maskCode = "ArduinoEnvNode";
//const String SuccessReturnText = "succ_ardu_openwrt";

void setup() 
{
  Serial.begin(9600);
  
  pinMode(PIN_1_SWITCH, OUTPUT);
  pinMode(PIN_2_SWITCH, OUTPUT);
    
  digitalWrite(PIN_1_SWITCH, LOW);
  digitalWrite(PIN_2_SWITCH, LOW);
  
  // start the Ethernet connection and the server:
  if (Ethernet.begin(mac) == 0) 
  {
    isDHCP = false;
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  else
  {
    isDHCP = true;
  }

  server.begin();

  myIP  = String(Ethernet.localIP()[0], DEC) + ".";
  myIP += String(Ethernet.localIP()[1], DEC) + ".";
  myIP += String(Ethernet.localIP()[2], DEC) + ".";
  myIP += String(Ethernet.localIP()[3], DEC);

  Serial.println(myIP);

  // give the sensor and Ethernet shield time to set up:
  delay(1000);
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop()
{ 
  if (millis() - lastAttemptTime > requestAttemptInterval) 
  {
    CheckInternetConnection();

    lastAttemptTime = millis();
  }

  ListenForEthernetClients();
}

void ListenForEthernetClients()
{
  // listen for incoming clients
  EthernetClient webClient = server.available();

  if (webClient) 
  {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;

    while (webClient.connected()) 
    {
      if (webClient.available()) 
      {
        char c = webClient.read();
        // Serial.print(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          String strSW1 = "";
          String strSW2 = "";
          String paraALLText = "GET /?SetSwitch=";
          String paraReboot = "GET /?Reboot";
          Serial.println(requestBuffer);
          
          int paraPosALL = requestBuffer.indexOf(paraALLText);
          int paraPosReboot = requestBuffer.indexOf(paraReboot);
          int paraPosMono = requestBuffer.indexOf("MONO");
          Serial.print("paraPosALL=");
          Serial.println(paraPosALL);
          
          if (paraPosALL > 0)
          {
            int switchZeroPos = paraPosALL + paraALLText.length();
            strSW1 = requestBuffer.substring(switchZeroPos + 0, switchZeroPos + 2);
            strSW2 = requestBuffer.substring(switchZeroPos + 3, switchZeroPos + 5);
            
            if (strSW1 == "ON")
            {
              digitalWrite(PIN_1_SWITCH, LOW);
              Serial.println("Switch_1: ON");
            }
            else if (strSW1 == "OF")
            {
              digitalWrite(PIN_1_SWITCH, HIGH);
              Serial.println("Switch_1: OFF");
            }
            else
            {
              Serial.print("Switch_1: Unknown: ");
              Serial.println(strSW1);
            }
            
            if (strSW2 == "ON")
            {
              digitalWrite(PIN_2_SWITCH, LOW);
              Serial.println("Switch_2: ON");
            }
            else if (strSW2 == "OF")
            {
              digitalWrite(PIN_2_SWITCH, HIGH);
              Serial.println("Switch_2: OFF");
            }
            else
            {
              Serial.print("Switch_2: Unknown: ");
              Serial.println(strSW2);
            }
          }
          
          if (paraPosReboot > 0)
          {
            ReBootSW();
          }
          
          requestBuffer = "BUF ";
          
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println("Connection: close");
	  //webClient.println("Refresh: 5");
          webClient.println();
          
          if (paraPosMono == -1)
          {
            
            webClient.println("<!DOCTYPE HTML>");
            
            webClient.println("<HTML>");
            webClient.println("<HEAD>");
  
            // print the current readings, in HTML format:
            webClient.println("<TITLE>Arduino Air Switch</TITLE>");
            webClient.println("</HEAD>");
            webClient.println("<BODY>");
            webClient.println("<H2>Arduino Air Switch</H2><br />");
            
            webClient.print("<H4>Local IP Address: ");
            webClient.print(Ethernet.localIP());
            webClient.print(" (");
            webClient.print(isDHCP == true ? "DHCP)" : "STATIC)");
            webClient.println("</H4>");
  
            webClient.print("<H4>Server IP Address: ");
            webClient.println(serverName);
            webClient.println("</H4>");
  
            webClient.print("<H4>Switch Status: ");
            
            webClient.print("SW1: ");
            if (digitalRead(PIN_1_SWITCH))
            {
              strSW1 = "OF";
              webClient.print("<font color='red'>OFF</font>");
            }
            else
            {
              strSW1 = "ON";
              webClient.print("<font color='green'>ON</font>");
            }
              
            webClient.print(" SW2: ");
            if (digitalRead(PIN_2_SWITCH))
            {
              strSW2 = "OF";
              webClient.print("<font color='red'>OFF</font>");
            }
            else
            {
              strSW2 = "ON";
              webClient.print("<font color='green'>ON</font>");
            }
            webClient.println("</H4>");
            
            /*
            webClient.print("<H4>Change: ");
            
            webClient.print("<a href='http://");
            webClient.print(myIP);
            webClient.print("/?SetSwitch=");
            if (strSW1 == "OF")
            {
              webClient.print("ON");
            }
            else
            {
              webClient.print("OF");
            }
            webClient.print(",");
            webClient.print(strSW2);
            webClient.println("'><input type='button' name='SWITCH1' value='SWITCH1'/></a>");
            
            webClient.print("<a href='http://");
            webClient.print(myIP);
            webClient.print("/?SetSwitch=");
            webClient.print(strSW1);
            webClient.print(",");
            if (strSW2 == "OF")
            {
              webClient.print("ON");
            }
            else
            {
              webClient.print("OF");
            }
            webClient.println("'><input type='button' name='SWITCH2' value='SWITCH2'/></a>");           
            webClient.println("</H4>");
            */
            
            
            webClient.println("</BODY>");
            webClient.println("</HTML>");
          }
          else
          {
            webClient.println("OK");
          }
          
          break;
        }
 
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
          requestBuffer += " ";
        } 
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
          requestBuffer += c;
        }
      }
    }

    // give the web browser time to receive the data
    delay(20);

    // close the connection:
    webClient.stop();
  }
} 

void CheckInternetConnection()
{
  EthernetClient client;
  
  // attempt to connect, and wait a millisecond:
  if (client.connect(serverName, 80))
  {
    Serial.println("connection success");
    client.stop();
  }
  else
  {
    totalFailedCount++;
    Serial.println("connection failed");
  }
  
  if (totalFailedCount > 5)
  {
    ReBootSW();
  }
}

void ReBootSW()
{
  Serial.println("Reboot!");
  digitalWrite(PIN_1_SWITCH, HIGH);
  digitalWrite(PIN_2_SWITCH, HIGH);
  
  delay(10000);
  resetFunc();
}




