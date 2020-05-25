#include <Arduino.h>
#include <ESP8266WiFi.h>

const int pinnummer100 = 5;    // 100 %             PIN D1
const int pinnummer50 = 4;            // 50%        PIN D2
const int pinnummer2 = 0;             // 2%         PIN D3

const char* ssid = "---";
const char* password = "-----";

unsigned long ulReqcount;
unsigned long ulReconncount;


// Create an instance of the server on Port 80
WiFiServer server(80);

void setup() 
{
  // setup globals
  ulReqcount=0; 
  ulReconncount=0;
  
  // prepare GPIO2
  pinMode(pinnummer100, OUTPUT);
  digitalWrite(pinnummer100, 0);
  //---------------------------------------
  pinMode(pinnummer50, OUTPUT);
  digitalWrite(pinnummer50, 0);
  //---------------------------------------
  pinMode(pinnummer2, OUTPUT);
  digitalWrite(pinnummer2, 0);
  
  // start serial
  Serial.begin(1200);
  delay(1);
  
  // inital connect
  WiFi.mode(WIFI_STA);
  
}

void WiFiStart()
{
  ulReconncount++;
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() 
{
  // check if WLAN is connected
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFiStart();
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>Demo f&uumlr ESP8266 Steuerung</title></head><body>";
    sResponse += "<font color=\"#949191\"><body bgcolor=\"#000000\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>Lichtsteuerung</h1>";
    sResponse += "100% -> D1  , 50% -> D2 , 2% -> D3. <BR>";
    sResponse += "<BR>";
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<p>Licht 100% <a href=\"?pin=Ein_100%\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=Aus_100%\"><button>ausschalten</button></a></p>";
    sResponse += "<p>Licht 50% <a href=\"?pin=Ein_50%\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=Aus_50%\"><button>ausschalten</button></a></p>";
    sResponse += "<p>Licht 2% <a href=\"?pin=Ein_2%\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=Aus_2%\"><button>ausschalten</button></a></p>";
    
    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      sResponse += "Kommando:" + sCmd + "<BR>";
      //------------------------------------------------- Licht an , aus 100%--------------------------------------------------------------------
      // switch GPIO
      if(sCmd.indexOf("Ein_100%")>=0)
      {
        digitalWrite(pinnummer100, 1);
      }
      else if(sCmd.indexOf("Aus_100%")>=0)
      {
        digitalWrite(pinnummer100, 0);
      }
      //------------------------------------------------- Licht an , aus 50%--------------------------------------------------------------------
       if(sCmd.indexOf("Ein_50%")>=0)
      {
        digitalWrite(pinnummer50, 1);
      }
      else if(sCmd.indexOf("Aus_50%")>=0)
      {
        digitalWrite(pinnummer50, 0);
      }
      //------------------------------------------------- Licht an , aus 2%--------------------------------------------------------------------
       if(sCmd.indexOf("Ein_2%")>=0)
      {
        digitalWrite(pinnummer2, 1);
      }
      else if(sCmd.indexOf("Aus_2%")>=0)
      {
        digitalWrite(pinnummer2, 0);
      }
    }
    
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<BR>Aufrufz&auml;hler="; 
    sResponse += ulReqcount;
    sResponse += " - Verbindungsz&auml;hler="; 
    sResponse += ulReconncount;
    sResponse += "<BR>";
    sResponse += "Stipe Dujmovic <BR>";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  
 
}