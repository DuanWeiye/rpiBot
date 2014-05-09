#include <SPI.h>
#include <Ethernet.h>
#include <String.h>
#include <dht11.h>

dht11 DHT11;

#define DHT11PIN 2

int hum = 0;
int temp = 0;

boolean isDHCP = false;
String myIP = "";
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };

// Assign default IP address for the controller:
IPAddress ip(192, 168, 22, 170);
IPAddress gateway(192, 168, 22, 1);
IPAddress subnet(255, 255, 255, 0);

// Server
char serverName[] = "192.168.22.77";


// Initialize the Ethernet
EthernetServer server(80);

unsigned long requestSuccessedTimes = 0;                // Success time of send
unsigned long requestFailedTimes = 0;                   // Fail time of send
unsigned long lastReadingTime = 0;                      // last time reading the data, in milliseconds
unsigned long lastAttemptTime = 0;                      // last time connected to the server, in milliseconds

const unsigned long serverWaitInterval = 3000;
const unsigned long requestReadingInterval = 30000;      // delay between requests, in milliseconds
const unsigned long requestAttemptInterval = 600000;     // delay between requests, in milliseconds

const String maskCode = "ArduinoEnvNode";
const String SuccessReturnText = "succ_ardu_openwrt";

void setup() 
{
  Serial.begin(9600);
  
  // start the Ethernet connection and the server:
  if (Ethernet.begin(mac) == 0) 
  {
    isDHCP = false;
    Ethernet.begin(mac, ip);
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

void loop() { 
  // check for a reading no more than once a second.
  if (millis() - lastReadingTime > requestReadingInterval)
  {
    // if there's a reading ready, read it:
    // don't do anything until the data ready pin is high:
    DHT11.read(DHT11PIN);
 
    hum = (int)DHT11.humidity;
    temp = (int)DHT11.temperature;
    
    // timestamp the last time you got a reading:
    lastReadingTime = millis();
  }
  
  if (millis() - lastAttemptTime > requestAttemptInterval) 
  {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    PostToServer();
    
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
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          // send a standard http response header
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println();
          
          // print the current readings, in HTML format:
          webClient.print("== Arduino temperature station ==");
          webClient.println("<br /><br />");
          
          webClient.print("Local IP Address: ");
          webClient.print(Ethernet.localIP());
          webClient.print(" (");
          webClient.print(isDHCP == true ? "DHCP)" : "STATIC)");
          webClient.println("<br />");
          
          webClient.print("Server IP Address: ");
          webClient.println(serverName);
          webClient.println("<br />");
          webClient.print("Request Send Success/Fail Count: ");
          webClient.print(requestSuccessedTimes);
          webClient.print(" / ");
          webClient.print(requestFailedTimes);
          webClient.println("<br /><br />");
          
          webClient.print("Temperature: ");
          webClient.print(temp);
          webClient.print(" C");
          webClient.println("<br />");
          
          webClient.print("Humidity: ");
          webClient.print(hum);
          webClient.print(" %");
          webClient.println("<br />");  
          
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(20);
    
    // close the connection:
    webClient.stop();
  }
} 

void PostToServer()
{
  EthernetClient client;
  
  // attempt to connect, and wait a millisecond:
  if (client.connect(serverName, 80)) 
  {
    String postText = maskCode + "," + myIP + "," + (String)temp + "," + (String)hum;
    
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
        
        requestSuccessedTimes++;
      }
      else
      {
        Serial.println("Failed");
        
        requestFailedTimes++;
      }
    } 
    
    if (isGotReturn == false)
    {
      Serial.println("Timeout");
      requestFailedTimes++;
    }
    
    client.stop();
  }
  else
  {
    Serial.println('connection failed');
  }
}   

