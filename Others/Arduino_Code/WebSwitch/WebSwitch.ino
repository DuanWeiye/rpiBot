#include <SPI.h>
#include <Ethernet.h>
#include <String.h>

#define PIN_SWITCH 5

static boolean isDHCP = false;
static String myIP = "";
static char myUPTime[100] = { 0 };
String requestBuffer = "BUF ";
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };

// Assign default IP address for the controller:
IPAddress ip(192, 168, 254, 2);
IPAddress gateway(192, 168, 254, 1);
IPAddress subnet(255, 255, 255, 0);

// Server
char serverName[] = "192.168.1.1";

// Initialize the Ethernet
EthernetServer server(80);

unsigned long lastAttemptTime = 0;                       // last time connected to the server, in milliseconds

const unsigned long serverWaitInterval = 10000;
const unsigned long requestAttemptInterval = 600000;     // delay between requests, in milliseconds

const String maskCode = "ArduinoEnvNode";
const String SuccessReturnText = "succ_ardu_openwrt";

void setup() 
{
  Serial.begin(9600);

  pinMode(PIN_SWITCH, OUTPUT);
  digitalWrite(PIN_SWITCH, HIGH);

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

void loop()
{ 
  if (millis() - lastAttemptTime > requestAttemptInterval) 
  {
    CheckReverseServer();

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
          uptime();
          Serial.println(requestBuffer);
          Serial.println(requestBuffer.indexOf("GET /?SetSwitch=ON"));

          if (requestBuffer.indexOf("GET /?SetSwitch=ON") > 0)
          {
            digitalWrite(PIN_SWITCH, LOW);
            Serial.println("Switch: ON");
          }
          else if (requestBuffer.indexOf("GET /?SetSwitch=OFF") > 0)
          {
            digitalWrite(PIN_SWITCH, HIGH);
            Serial.println("Switch: OFF");
          }
          else
          {
            //
          }
          
          requestBuffer = "BUF ";
          
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println("Connection: close");
	  //webClient.println("Refresh: 5");
          webClient.println();
          webClient.println("<!DOCTYPE HTML>");
          
          webClient.println("<HTML>");
          webClient.println("<HEAD>");

          // print the current readings, in HTML format:
          webClient.println("<TITLE>Arduino Air Switch</TITLE>");
          webClient.println("</HEAD>");
          webClient.println("<BODY>");
          webClient.println("<H2>Arduino Air Switch</H2><br />");
          
          webClient.print("<H4>UPTime: ");
          Serial.println(myUPTime);
          webClient.println("</H4>");
          
          webClient.print("<H4>Local IP Address: ");
          webClient.print(Ethernet.localIP());
          webClient.print(" (");
          webClient.print(isDHCP == true ? "DHCP)" : "STATIC)");
          webClient.println("</H4>");

          webClient.print("<H4>Server IP Address: ");
          webClient.println(serverName);
          webClient.println("</H4>");

          webClient.print("<H4>Switch Status: ");
          if (digitalRead(PIN_SWITCH))
          {
            webClient.println("<font color='red'>OFF</font>");
          }
          else
          {
            webClient.println("<font color='green'>ON</font>");
          }
          
          webClient.println("<FORM action=\"/\">");
          webClient.println("<P> <INPUT type=\"radio\" name=\"SetSwitch\" value=\"ON\">ON");
          webClient.println("<P> <INPUT type=\"radio\" name=\"SetSwitch\" value=\"OFF\">OFF");
          webClient.println("<P> <INPUT type=\"submit\" value=\"Submit\"></FORM></H4>");
          
          webClient.println("</BODY>");
          webClient.println("</HTML>");
          
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

void CheckReverseServer()
{
  EthernetClient client;

  // attempt to connect, and wait a millisecond:
  if (client.connect(serverName, 80)) 
  {
    String postText = maskCode + "," + myIP;

    Serial.print("Request String: ");
    Serial.println(postText);

    Serial.print("Sending HTTP request...");

    // make HTTP request
    client.print("GET /bin/main.lua?");
    client.print(postText);
    client.println(" HTTP/1.1");

    client.print("HOST: ");
    client.println(serverName);

    //client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");

    //client.print("Content-Length: ");
    //client.println(postText.length());

    client.println();

    boolean isGotReturn = false;
    delay(serverWaitInterval);

    if (client.available())
    {
      String currentLine = "";

      int waitStartTime = millis();
      while(client.available())
      {
        char recv = client.read();

        // add incoming byte to end of line:
        currentLine += recv;
      }

      Serial.println(currentLine);
      isGotReturn = true;

      if (currentLine.indexOf(SuccessReturnText) > 0)
      {
        Serial.println("OK");
      }
      else
      {
        Serial.println("Failed");
      }
    } 

    if (isGotReturn == false)
    {
      Serial.println("Timeout");
    }

    client.stop();
  }
  else
  {
    Serial.println('connection failed');
  }
}   

unsigned long millisRollover() 
{
  static unsigned long numRollovers = 0; // variable that permanently holds the number of rollovers since startup
  static boolean readyToRoll = false; // tracks whether we've made it halfway to rollover
  unsigned long now = millis(); // the time right now
  unsigned long halfwayMillis = 2147483647; // this is halfway to the max millis value (17179868 for earlier versions of Arduino)

  if (now > halfwayMillis) { // as long as the value is greater than halfway to the max
    readyToRoll = true; // you are ready to roll over
  }

  if (readyToRoll == true && now < halfwayMillis) {
    // if we've previously made it to halfway
    // and the current millis() value is now _less_ than the halfway mark
    // then we have rolled over
    numRollovers++; // add one to the count the number of rollovers
    readyToRoll = false; // we're no longer past halfway
  } 
  return numRollovers;
}

void uptime()
{
  unsigned long days = 0;
  unsigned long hours = 0;
  unsigned long mins = 0;
  unsigned long secs = 0;
  
  unsigned long maxHours = 24;
  unsigned long maxMinutes = 60;
  unsigned long maxSeconds = 60;
  unsigned long maxMillis = 1000;
  unsigned long maxRounds = 4294967295;
  
  //millis() MAX:4294967295 | about 50 days each [numRollovers]
  
  secs = millis() / maxMillis + (maxRounds / maxMillis) * millisRollover(); //convect milliseconds to seconds
  mins = secs / maxSeconds; //convert seconds to minutes
  hours = mins / maxMinutes; //convert minutes to hours
  days = hours / maxHours; //convert hours to days
  
  secs = secs - (mins * maxSeconds); //subtract the coverted seconds to minutes in order to display 59 secs max 
  mins = mins - (hours * maxMinutes); //subtract the coverted minutes to hours in order to display 59 minutes max
  hours = hours - (days * maxHours); //subtract the coverted hours to days in order to display 23 hours max
  
  memset(myUPTime, NULL, 100);
  sprintf(myUPTime, "%lu days %lu hours %lu minutes %lu seconds", days, hours, mins, secs);
}

