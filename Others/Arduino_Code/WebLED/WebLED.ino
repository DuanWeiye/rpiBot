#include <SPI.h>
#include <Ethernet.h>
#include <String.h>

int LedRedPin = 3;
int LedGreenPin = 5;
int LedBluePin = 6;

int ValueR = 0;
int ValueG = 0;
int ValueB = 0;

static boolean isDHCP = false;
static String myIP = "";
String requestBuffer = "BUF ";
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };

// Assign default IP address for the controller:
IPAddress ip(172, 0, 0, 150);
IPAddress gateway(172, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// Server
char serverName[] = "172.0.0.1";

// Initialize the Ethernet
EthernetServer server(80);

unsigned long lastAttemptTime = 0;

const unsigned long serverWaitInterval = 10000;
const unsigned long requestAttemptInterval = 1000;

const String maskCode = "ArduinoEnvNode";
const String SuccessReturnText = "succ_ardu_openwrt";

void setup() 
{
  Serial.begin(9600);

  pinMode(LedRedPin, OUTPUT); 
  pinMode(LedGreenPin, OUTPUT); 
  pinMode(LedBluePin, OUTPUT); 

  LedFade(255, 0, 0, 10);
  LedFade(255, 255, 0, 10);
  LedFade(255, 255, 255, 10);
  LedFade(0, 0, 0, 20);
  
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
        // Serial.print(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          String R = "";
          String G = "";
          String B = "";
          boolean isMono = false;
          Serial.println(requestBuffer);
         
          if (requestBuffer.indexOf("Mono") > 0) isMono = true;
          
          String paraALLText = "GET /?SetColor=";
          int paraPosALL = requestBuffer.indexOf(paraALLText);
          if (paraPosALL > 0)
          {
            int colorZeroPos = paraPosALL + paraALLText.length();
            R = requestBuffer.substring(colorZeroPos + 0, colorZeroPos + 3);
            G = requestBuffer.substring(colorZeroPos + 4, colorZeroPos + 7);
            B = requestBuffer.substring(colorZeroPos + 8, colorZeroPos + 11);
            
            Serial.println(R + ',' + G + ',' + B);
            
            if (R.toInt() >= 0 && R.toInt() <= 255 && 
                G.toInt() >= 0 && G.toInt() <= 255 && 
                B.toInt() >= 0 && B.toInt() <= 255)
            {
              LedFade(R.toInt(), G.toInt(), B.toInt(), 30);
            }
          }
          
          String paraRText = "GET /?SetR=";
          int paraPosR = requestBuffer.indexOf(paraRText);
          if (paraPosR > 0)
          {
            int colorZeroPos = paraPosR + paraRText.length();
            R = requestBuffer.substring(colorZeroPos + 0, colorZeroPos + 3);
            
            Serial.println("R:" + R);
            
            if (R.toInt() >= 0 && R.toInt() <= 255)
            {
              LedFade(R.toInt(), ValueG, ValueB, 30);
            }
          }
          
          String paraGText = "GET /?SetG=";
          int paraPosG = requestBuffer.indexOf(paraGText);
          if (paraPosG > 0)
          {
            int colorZeroPos = paraPosG + paraGText.length();
            G = requestBuffer.substring(colorZeroPos + 0, colorZeroPos + 3);
            
            Serial.println("G:" + G);
            
            if (G.toInt() >= 0 && G.toInt() <= 255)
            {
              LedFade(ValueR, G.toInt(), ValueB, 30);
            }
          }
          
          String paraBText = "GET /?SetB=";
          int paraPosB = requestBuffer.indexOf(paraBText);
          if (paraPosB > 0)
          {
            int colorZeroPos = paraPosB + paraBText.length();
            B = requestBuffer.substring(colorZeroPos + 0, colorZeroPos + 3);
            
            Serial.println("B:" + B);
            
            if (B.toInt() >= 0 && B.toInt() <= 255)
            {
              LedFade(ValueR, ValueG, B.toInt(), 30);
            }
          }
          
          requestBuffer = "BUF ";
          
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println("Connection: close");
	  //webClient.println("Refresh: 5");
          webClient.println();
          
          if (isMono == false)
          {
            webClient.println("<!DOCTYPE HTML>");
            
            webClient.println("<HTML>");
            webClient.println("<HEAD>");
  
            // print the current readings, in HTML format:
            webClient.println("<TITLE>Arduino Air LED</TITLE>");
            webClient.println("</HEAD>");
            webClient.println("<BODY>");
            webClient.println("<H2>Arduino Air LED</H2><br />");
            
            webClient.print("<H4>Local IP Address: ");
            webClient.print(Ethernet.localIP());
            webClient.print(" (");
            webClient.print(isDHCP == true ? "DHCP)" : "STATIC)");
            webClient.println("</H4>");
  
            webClient.print("<H4>Server IP Address: ");
            webClient.println(serverName);
            webClient.println("</H4>");
  
            webClient.print("<H4>LED Status: R:" + (String)ValueR + ",G:" + (String)ValueG + ",B:" + (String)ValueB + "</H4>");
            
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

void SyncWithServer()
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
    client.print("GET /bin/queryCommand.lua?");
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

void LedFade(int R, int G, int B, int interval)
{
  for(int fadeStep = 0; fadeStep <= 100; fadeStep += 1) 
  {
    int stepR = R > ValueR ? ValueR + (R - ValueR) * fadeStep / 100 : ValueR - ((ValueR - R) * fadeStep / 100);
    int stepG = G > ValueG ? ValueG + (G - ValueG) * fadeStep / 100 : ValueG - ((ValueG - G) * fadeStep / 100);
    int stepB = B > ValueB ? ValueB + (B - ValueB) * fadeStep / 100 : ValueB - ((ValueB - B) * fadeStep / 100);
    
    LedRGB(stepR, stepG, stepB);
    
    delay(interval);
  }
  
  LedRGB(R, G, B);
  
  ValueR = R;
  ValueG = G;
  ValueB = B;
}

void LedRGB(int R, int G, int B)
{
  analogWrite(LedRedPin, R);
  analogWrite(LedGreenPin, G);
  analogWrite(LedBluePin, B);
}
