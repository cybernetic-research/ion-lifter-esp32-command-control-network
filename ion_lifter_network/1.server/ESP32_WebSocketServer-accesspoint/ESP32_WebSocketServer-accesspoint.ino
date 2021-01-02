/****************************************************************************************************************************
  ESP32_WebSocketServer.ino
  For ESP32

  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license
  Version: 2.2.2

  Originally Created on: 22.05.2015
  Original Author: Markus Sattler

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  2.1.3   K Hoang      15/05/2020 Initial porting to support SAMD21, SAMD51, nRF52 boards, such as AdaFruit Feather nRF52832,
                                 nRF52840 Express, BlueFruit Sense, Itsy-Bitsy nRF52840 Express, Metro nRF52840 Express, etc.
  2.2.1   K Hoang      18/05/2020 Bump up to sync with v2.2.1 of original WebSockets library
  2.2.2   K Hoang      25/05/2020 Add support to Teensy, SAM DUE and STM32. Enable WebSocket Server for new supported boards.
*****************************************************************************************************************************/

#if !defined(ESP32)
  #error This code is intended to run only on the ESP32 boards ! Please check your Tools->Board setting.
#endif

#define _WEBSOCKETS_LOGLEVEL_     3

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsServer_Generic.h>

WiFiMulti         WiFiMulti;
WebSocketsServer  webSocket = WebSocketsServer(81);


const char*   ssid      = "LIFTER";
const char*   password  = "ionocraft";
char message[1024];
const char*   ID          = "LIFTER_SERVER";        //Access point hub provides network for entire system
//const char*   ID        = "INVERTER_PITCH";       //pitch axis pair for pitch axes
//const char*   ID        = "INVERTER_ROLL";        //roll axis pair for roll axes
//const char*   ID        = "SENSOR_ORIENTATION";   //orientation sensor
//const char*   ID        = "CONTROLLER";           //web browser to view and control system

/*
 *  Suggested Message format
 *  
 *  destination: ID code above, or ALL for broadcast
 *     
 *  controller demand to update axis pairs 
 *  
 *  {
 *    sender: CONTROLLER;
 *    destination: INVERTER_PITCH; 
 *    msg: UPDATE;
 *    payload:{
 *     AXIS0: 0;
 *     AXIS1: 255;
 *    }
 *  }
 * 
 * 
 *  status update from inverter
 * 
 *  {
 *    sender: INVERTER_PITCH;
 *    destination: CONTROLLER; 
 *    msg: STATUS;
 *    payload:{
 *     ADC0:  0;
 *     ADC1:  255;
 *    }
 *  }
 *  
 *  
 *  status update from sensor pack
 * 
 *  {
 *    sender: INVERTER_PITCH;
 *    destination: ALL; 
 *    msg: STATUS;
 *    payload:{
 *     HEADING: 180;
 *     PITCH: 45;
 *     ROLL: -45;
 *    }
 *  }
 *  
 *  
 */











void hexdump(const void *mem, uint32_t len, uint8_t cols = 16)
{
  const uint8_t* src = (const uint8_t*) mem;

  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);

  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }

    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}





#define ID_UNINITIALISED  0xff
#define ID_BROADCAST      0x7f
#define ID_SERVER         0xfe

struct whois_t
{
  uint8_t LIFTER_SERVER;       
  uint8_t INVERTER_PITCH;
  uint8_t INVERTER_ROLL;
  uint8_t SENSOR_ORIENTATION;
  uint8_t CONTROLLER;
};

whois_t WhoIs;
void InitialiseRouting()
{
  WhoIs.LIFTER_SERVER       = ID_SERVER;
  WhoIs.INVERTER_PITCH      = ID_UNINITIALISED;
  WhoIs.INVERTER_ROLL       = ID_UNINITIALISED;
  WhoIs.SENSOR_ORIENTATION  = ID_UNINITIALISED;
  WhoIs.CONTROLLER          = ID_UNINITIALISED;
}

char comp[50];
/**
 * void AssignIDToNodes(uint8_t num, uint8_t * payload)
 * shall build a lookup table linking the reported name of a node to its ID code in the array
 */
void RefreshNodeIDLookup(uint8_t num, char * payload)
{
  char* ret;
  sprintf(comp,"\"sender\":\"INVERTER_PITCH\"");
  ret = strstr(payload, comp);
  if(ret!=NULL)
  {
    WhoIs.INVERTER_PITCH = num;  
    return;
  }
  ret = strstr(payload, "\"sender\":\"INVERTER_ROLL\"");
  if(ret!=NULL)
  {
    WhoIs.INVERTER_ROLL = num;  
    return;
  }
  ret = strstr(payload, "\"sender\":\"INVERTER_PITCH\"");
  if(ret!=NULL)
  {
    WhoIs.INVERTER_PITCH = num;  
    return;
  }
  ret = strstr(payload, "\"sender\":\"SENSOR_ORIENTATION\"");
  if(ret!=NULL)
  {
    WhoIs.SENSOR_ORIENTATION = num;  
    return;
  }
  ret = strstr(payload, "\"sender\":\"CONTROLLER\"");
  if(ret!=NULL)
  {
    WhoIs.CONTROLLER = num;  
    return;
  }
}



uint8_t GetDestination(uint8_t num, char * payload)
{
  uint8_t result = ID_UNINITIALISED;
  char* ret;
  ret = strstr(payload, "\"destination\":\"ALL\"");
  if(ret!=NULL)
  {
    result = ID_BROADCAST;  
  }
  ret = strstr(payload, "\"destination\":\"LIFTER_SERVER\"");
  if(ret!=NULL)
  {
    result = WhoIs.LIFTER_SERVER;  
  }
  ret = strstr(payload, "\"destination\":\"INVERTER_PITCH\"");
  if(ret!=NULL)
  {
    result = WhoIs.INVERTER_PITCH;  
  }
  ret = strstr(payload, "\"destination\":\"INVERTER_ROLL\"");
  if(ret!=NULL)
  {
    result = WhoIs.INVERTER_ROLL;  
  }
  ret = strstr(payload, "\"destination\":\"SENSOR_ORIENTATION\"");
  if(ret!=NULL)
  {
    result = WhoIs.SENSOR_ORIENTATION;  
  }
  ret = strstr(payload, "\"destination\":\"CONTROLLER\"");
  if(ret!=NULL)
  {
    result = WhoIs.CONTROLLER;  
  }
  return(result);
}




void RouteMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  uint8_t destination = GetDestination(num,(char *)payload);
  if(ID_BROADCAST==destination)
  {
    webSocket.broadcastTXT(payload);
  }else if(ID_UNINITIALISED==destination)
  {
    //silent discard
  }
  else if(WhoIs.LIFTER_SERVER==destination)
  {
  }
  else
  {
    webSocket.sendTXT(destination,payload);
  }
}





void MessageRouter(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
  RefreshNodeIDLookup(num,(char *)payload);
  RouteMessage(num,type,payload,length);  
}


bool toggle=false;
uint32_t it = 0;
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        sprintf(message,"{\"sender\":\"%s\",\"msg\":\"CONNECTED\"}",ID,it);
        webSocket.sendTXT(num, message);
      }
      break;
      
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      MessageRouter(num, type, payload, length);
      break;
      
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);

      // send message to client
      webSocket.sendBIN(num, payload, length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;

    default:
      break;  
  }

}



WiFiServer server(80);

void setup()
{
  // Serial.begin(921600);
  Serial.begin(115200);
  InitialiseRouting();
  Serial.print("setting up acces point");
  WiFi.softAP(ssid,password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address:");
  Serial.println(IP);
  server.begin();
  Serial.println("\nStart ESP32_WebSocketServer");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() 
{
  webSocket.loop();
}
