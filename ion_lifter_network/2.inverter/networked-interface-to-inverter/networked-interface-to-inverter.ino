/****************************************************************************************************************************
  ESP32_WebSocketClient.ino
  For ESP32

  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license
  Version: 2.2.2

  Originally Created on: 24.05.2015
  Original Author: Markus Sattler

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  2.1.3   K Hoang      15/05/2020 Initial porting to support SAMD21, SAMD51, nRF52 boards, such as AdaFruit Feather nRF52832,
                                 nRF52840 Express, BlueFruit Sense, Itsy-Bitsy nRF52840 Express, Metro nRF52840 Express, etc.
  2.2.1   K Hoang      18/05/2020 Bump up to sync with v2.2.1 of original WebSockets library
  2.2.2   K Hoang      25/05/2020 Add support to Teensy, SAM DUE and STM32. Enable WebSocket Server for new supported boards.
*****************************************************************************************************************************/
//https://arduinojson.org/v6/assistant/
#include "ArduinoJson.h"
#include "CircularBuffer.h"
#if !defined(ESP32)
  #error This code is intended to run only on the ESP32 boards ! Please check your Tools->Board setting.
#endif

#define _WEBSOCKETS_LOGLEVEL_     3

#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsClient_Generic.h>

WiFiMulti         WiFiMulti;
WebSocketsClient  webSocket;

// Select the IP address according to your local network

IPAddress serverIP(192, 168, 4, 1);
IPAddress clientIP(192, 168, 4, 254);

bool _connected = false;

const char*   ssid      = "LIFTER";
const char*   password  = "ionocraft";
char message[1024];
//const char*   ID        = "LIFTER_SERVER";        //Access point hub provides network for entire system
const char*     ID1       = "INVERTER_PITCH";       //pitch axis pair for pitch axes
const char*     ID2       = "INVERTER_ROLL";        //roll axis pair for roll axes
//const char*   ID        = "SENSOR_ORIENTATION";   //orientation sensor
//const char*   ID        = "CONTROLLER";           //web browser to view and control system
char id[25];

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
 *    sender: SENSOR_ORIENTATION;
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




#define RXD2 16
#define TXD2 17

int idPin = 15;
int         potVal[4]     = { 0,  0,  0,   0  };
int         ctrl[2]       = {0};
int         remoteCtrlEn  = 0;
int         oldTime       = 0;
int _update=0;
int ct=0;




void GetID()
{
  int val = digitalRead(idPin);
  if(val)
  {
    sprintf(id,"%s",ID1);
  }
  else
  {
    sprintf(id,"%s",ID2);
  }
}
        


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













void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      _connected=false;
      Serial.printf("[WSc] Disconnected!\n");
      setup();
      break;
    case WStype_CONNECTED:
      _connected=true;
      Serial.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      GetID();
      sprintf(message,"{\"sender\":\"%s\",\"msg\":\"CONNECTED\"}",id);
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      break;



      
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      webSocket.sendBIN(payload, length);
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





volatile int interruptCounter;
int totalInterruptCounter;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);


  
}





void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("\nStart ESP32_WebSocketClient");

  //Serial.setDebugOutput(true);
  Serial.setDebugOutput(true);

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

   WiFiMulti.addAP(ssid, password);

  WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    delay(100);
  }

  // server address, port and URL
  Serial.print("WebSockets Server IP address: ");
  Serial.println(serverIP);

  // server address, port and URL
  webSocket.begin(serverIP, 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  
  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);



  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 500, true);
  timerAlarmEnable(timer);
 
}


void SerialLoop()
{
  bool error=false;
  while (Serial2.available()) {
    AddToBuffer(0,char(Serial2.read()),&error);
  }
  Decoder();
}


bool add = false;
bool decode = false;
int adc0Idx=0;
char adc0[3];
int adc1Idx=0;
char adc1[3];
int ctrl0Idx=0;
char ctrl0[3];
int ctrl1Idx=0;
char ctrl1[3];
int remIdx=0;
char rem[3];
int errIdx=0;
char err[3];
int commaCount=0;
void Decoder()
{
  bool error=false;
  while(!IsEmpty(0,&error))
  { 
    char s = RemoveFromBuffer(0,&error);
    if(add)
    {
      if(','==s)
      {
        commaCount+=1;
      }

      if((1==commaCount)&&(','!=s))
      {
        if(adc0Idx<3)
        {
          adc0[adc0Idx] =   s;
          adc0Idx       +=  1;
        }
      }
      if((3==commaCount)&&(','!=s))
      {
        if(adc1Idx<3)
        {
          adc1[adc1Idx] =   s;
          adc1Idx       +=  1;
        }
      }
      if((5==commaCount)&&(','!=s))
      {
        if(ctrl0Idx<3)
        {
          ctrl0[ctrl0Idx] =   s;
          ctrl0Idx       +=  1;
        }
      }
      if((7==commaCount)&&(','!=s))
      {
        if(ctrl1Idx<3)
        {
          ctrl1[ctrl1Idx] =   s;
          ctrl1Idx       +=  1;
        }
      }
      if((9==commaCount)&&(','!=s))
      {
        if(remIdx<3)
        {
          rem[remIdx] =   s;
          remIdx       +=  1;
        }
      }
      if((11==commaCount)&&(','!=s))
      {
        if(errIdx<3)
        {
          err[errIdx] =   s;
          errIdx       +=  1;
        }
      }
    }
    if('*'==s)
    {
      //start adding
      add=true;
      commaCount=0;
      adc0Idx=0;
      memset(adc0,'\0',3);
      adc1Idx=0;
      memset(adc1,'\0',3);
      ctrl0Idx=0;
      memset(ctrl0,'\0',3);
      ctrl1Idx=0;
      memset(ctrl1,'\0',3);
      remIdx=0;
      memset(rem,'\0',3);
      errIdx=0;
      memset(err,'\0',3);
      commaCount=0;
    }
    if('\n'==s)
    {
      add=false;
      decode=true;
    }
   

    if(decode)
    {
      decode = false;
      GetID();
        sprintf(message,"{\"sender\":\"%s\",\"destination\":\"CONTROLLER\",\"msg\":\"STATUS\",\"payload\":{\"ADC0\":\"%s\",\"ADC1\":\"%s\",\"CTRL0\":\"%s\",\"CTRL1\":\"%s\",\"REMOTE_CONTROL\":\"%s\",\"ERROR\":\"%s\"}}\n",
                id,
                adc0,
                adc1,
                ctrl0,
                ctrl1,
                rem,
                err);
       Serial.printf(message);
       if(_connected)
       {
          webSocket.sendTXT(message);
       }
    }
  }
}


int ctr1=0;
void loop()
{
  
  if (interruptCounter > 0) {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;
    ctr1+=1;
    if(ctr1>99)
    {
      
      ctr1 = 0;
    }
  }
  bool webloop=false;
  switch(ctr1)
  {
    default:break;
    case 0:
    case 10:
    case 20:
    case 30:
    case 40:
    case 50:
    case 60:
    case 70:
    case 80:
    case 90:
    {
     
    }
    break;
    case 5:
    case 15:
    case 25:
    case 35:
    case 45:
    case 55:
    case 65:
    case 75:
    case 85:
    case 95:
    {
       webSocket.loop();
       webloop=true;
    }
    break;

    case 7:
    case 27:
    case 57:
    case 77:
    {
      /*
      if(_update){
        _update = false;
        GetID();
        sprintf(message,"{\"sender\":\"%s\",\"destination\":\"CONTROLLER\",\"msg\":\"STATUS\",\"payload\":{\"ADC0\":\"%d\",\"ADC1\":\"%d\",\"CTRL0\":\"%d\",\"CTRL1\":\"%d\",\"REMOTE_CONTROL\":\"%d\"}}",
                id,
                potVal[0],
                potVal[1],
                ctrl[0],
                ctrl[1],
                remoteCtrlEn);
                
        webSocket.sendTXT(message);
        Serial.printf("%s\n",message);
      }
      */
    }
    break;
  }
   if(!webloop)
   {
    SerialLoop();
    }
}
