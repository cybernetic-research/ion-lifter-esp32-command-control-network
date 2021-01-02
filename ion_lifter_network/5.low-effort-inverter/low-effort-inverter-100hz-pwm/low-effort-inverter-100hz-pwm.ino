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




const int   PWM_CHAN[4]       = { 0,  1,  2,  3  }; //channels 0-15
const int   PWM_PIN[4]        = { 27, 14, 12, 13 }; //use table: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
const int   potPin[4]         = { 36, 39, 34, 35 }; // Potentiometer is connected to GPIO 27 (Analog ADC1_CH6) 
int         potVal[4]         = { 0,  0,  0,  0  };
int         oldPotVal[4]      = { 0,  0,  0,  0  };
int         ctrlIn[4]         = { 0,  0,  0,  0  };
int         oldCtrlIn[4]      = { 0,  0,  0,  0  };
#define     RXD2              16
#define     TXD2              17
#define     MODULATING_FREQ   100
bool        enableRemoteControl     = false;
bool        oldEnableRemoteControl  = false;
bool        doTransmission          = false;

/** *****************************************************
 *  
 *  /function CtrlSetup 
 *  /breif    sets up pwm params
 *  /param    void
 *  /return   N/A
 *  
 ********************************************************/
void CtrlSetup() {
 

  // Initialize channels
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(PWM_CHAN[0],           MODULATING_FREQ, 8);                 // 18 kHz PWM, 8-bit resolution
  ledcSetup(PWM_CHAN[1],           MODULATING_FREQ, 8);                 // 18 kHz PWM, 8-bit resolution
  ledcSetup(PWM_CHAN[2],           MODULATING_FREQ, 8);                 // 18 kHz PWM, 8-bit resolution
  ledcSetup(PWM_CHAN[3],           MODULATING_FREQ, 8);                 // 18 kHz PWM, 8-bit resolution

  ledcAttachPin(PWM_PIN[0],   PWM_CHAN[0]);
  ledcAttachPin(PWM_PIN[1],   PWM_CHAN[1]);
  ledcAttachPin(PWM_PIN[2],   PWM_CHAN[2]);
  ledcAttachPin(PWM_PIN[3],   PWM_CHAN[3]);
  
  ledcWrite(PWM_CHAN[0],           0);                           //signal to mosfet gate
  ledcWrite(PWM_CHAN[1],           0);                           //signal to mosfet gate
  ledcWrite(PWM_CHAN[2],           0);                           //signal to mosfet gate
  ledcWrite(PWM_CHAN[3],           0);                           //signal to mosfet gate
}


#define MAX_REG 255
/** *****************************************************
 *  
 *  /function CtrlLoop 
 *  /breif    read adc values and apply them to pwm 
 *            channels, or apply current value sent from
 *            browser remotely to current channnels
 *  /param    void
 *  /return   N/A
 *  
 ********************************************************/
void CtrlLoop()
{

  potVal[0]=(analogRead(potPin[0])>>4);
  potVal[1]=(analogRead(potPin[1])>>4);
  potVal[2]=(analogRead(potPin[2])>>4);
  potVal[3]=(analogRead(potPin[3])>>4);

  if( (oldPotVal[0] = potVal[0])||
      (oldPotVal[1] = potVal[1])||
      (oldPotVal[2] = potVal[2])||
      (oldPotVal[3] = potVal[3]))
  {
    doTransmission = true;
  }
  oldPotVal[0] = potVal[0];
  oldPotVal[1] = potVal[1];
  oldPotVal[2] = potVal[2];
  oldPotVal[3] = potVal[3];

    
 // Serial.printf("\n adc0, %d, adc1, %d, adc2, %d, adc3, %d",potVal[0],potVal[1],potVal[2],potVal[3]);

  if(enableRemoteControl)
  {
    ledcWrite(PWM_CHAN[0],           uint8_t(ctrlIn[0]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[1],           uint8_t(ctrlIn[1]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[2],           uint8_t(ctrlIn[2]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[3],           uint8_t(ctrlIn[3]));                           //signal to mosfet gate
  }
  else
  {
    ledcWrite(PWM_CHAN[0],           uint8_t(potVal[0]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[1],           uint8_t(potVal[1]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[2],           uint8_t(potVal[2]));                           //signal to mosfet gate
    ledcWrite(PWM_CHAN[3],           uint8_t(potVal[3]));                           //signal to mosfet gate
    
    ctrlIn[0] = 0;
    ctrlIn[1] = 0;
    ctrlIn[2] = 0;
    ctrlIn[3] = 0;
  }
}








/** *****************************************************
 *  
 *  /function hexdump(const void *mem, uint32_t len, uint8_t cols = 16)trlLoop 
 *  /breif    display hex dump of input
 *  /param    void
 *  /return   N/A
 *  
 ********************************************************/
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















void DecodeMessage(WStype_t type, char * payload, size_t length)
{
  
   Serial.printf("get Text: %s\r\n", payload);
/*
  ","payload":{"REMOTE_CONTROL_ENABLE":false}}
get Text: {"sender":"CONTROLLER","destination":"INVERTER_PITCH","msg":"UPDATE","payload":{"REMOTE_CONTROL_ENABLE":true}}
get Text: {"sender":"CONTROLLER","destination":"INVERTER_PITCH","msg":"UPDATE","payload":{"AXIS0":0,"AXIS1":0}}
*/
  char* ret;
  ret = strstr(payload, "REMOTE_CONTROL_ENABLE");
  if(ret!=NULL)
  {
    ret = strstr(payload, "false");
    if(ret!=NULL)
    {
      enableRemoteControl       = false;
      if(oldEnableRemoteControl != enableRemoteControl)
      {
        doTransmission = true;  
      }
      oldEnableRemoteControl    = enableRemoteControl;
      Serial.printf("enableRemoteControl: %d\r\n", enableRemoteControl);
    }
    else
    {
      enableRemoteControl       = true;
      oldEnableRemoteControl    = enableRemoteControl;
      if(oldEnableRemoteControl != enableRemoteControl)
      {
        doTransmission = true;  
      }
      Serial.printf("enableRemoteControl: %d\r\n", enableRemoteControl);
    }
  }

  

  ret = strstr(payload, "AXIS0");
  if(ret!=NULL)
  {
      ret = strstr(payload, "INVERTER_PITCH");
      if(ret!=NULL)
      {
      
        DynamicJsonDocument doc(length);
        deserializeJson(doc, payload);
        ctrlIn[0] = doc["payload"]["AXIS0"];
        ctrlIn[1] = doc["payload"]["AXIS1"];
        Serial.printf("INVERTER_PITCH: \r\n");

        if((oldCtrlIn[0] != ctrlIn[0]) ||
           (oldCtrlIn[1] != ctrlIn[1]))
        {
          doTransmission = true;
        }
        oldCtrlIn[0] = ctrlIn[0];
        oldCtrlIn[1] = ctrlIn[1];
      }
      
      ret = strstr(payload, "INVERTER_ROLL");
      if(ret!=NULL)
      {
        DynamicJsonDocument doc(length);
        deserializeJson(doc, payload);
        ctrlIn[2] = doc["payload"]["AXIS0"];
        ctrlIn[3] = doc["payload"]["AXIS1"];
        Serial.printf("INVERTER_ROLL:\r\n");

        if((oldCtrlIn[2] != ctrlIn[2]) ||
           (oldCtrlIn[3] != ctrlIn[3]))
        {
          doTransmission = true;
        }

        oldCtrlIn[2] = ctrlIn[2];
        oldCtrlIn[3] = ctrlIn[3];
      }
  }
}





/** *****************************************************
 *  
 *  /function webSocketEvent 
 *  /breif    
 *  /param    void
 *  /return   N/A
 *  
 ********************************************************/
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
      
      sprintf(message,"{\"sender\":\"INVERTER\",\"msg\":\"CONNECTED\"}");
      doTransmission=true;
      break;
     case WStype_TEXT:
      //Serial.printf("get Text: %s\r\n", payload);
      DecodeMessage(type, (char *)payload, length);
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




void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("\nStart ESP32_WebSocketClient");
  CtrlSetup();
  WifiSetup();
}


void WifiSetup()
{
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
}
/*

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



 
}
*/

bool oneTimeSetup = false;


int ctr1=0;
bool toggle=false;
unsigned long currTime =0;
unsigned long oldTime =0;
unsigned long lastTransmissionTime =0;
#define WIFI_SETUP_1  0
#define WIFI_SETUP_2  1
#define NORMAL_RUN    2
int wifiState = WIFI_SETUP_1;
void loop()
{
  currTime = millis();

  if(currTime>=oldTime+200)
  {
    oldTime = currTime;
    CtrlLoop();
    webSocket.loop();
    TransmissionLoop();
  }
}

void TransmissionLoop()
{
  switch(wifiState)
  {
    case WIFI_SETUP_1:
    {
      if(WiFiMulti.run() == WL_CONNECTED)
      {
        wifiState = WIFI_SETUP_2;
      }
    }
    break;

    case WIFI_SETUP_2:
    {
      // server address, port and URL
      Serial.print("WebSockets Server IP address: ");
      Serial.println(serverIP);

      // server address, port and URL
      webSocket.begin(serverIP, 81, "/");

      // event handler
      webSocket.onEvent(webSocketEvent);

  
      // try ever 5000 again if connection has failed
      webSocket.setReconnectInterval(5000);
      wifiState = NORMAL_RUN;
    }
    break;

    
    case NORMAL_RUN:
    {
      if(currTime-lastTransmissionTime>=5000)
      {
        doTransmission = true;
      }
      if(doTransmission)
      {
        doTransmission = false;
        if(_connected)
        {
          lastTransmissionTime = currTime;
          sprintf(message,"{\"sender\":\"%s\",\"destination\":\"CONTROLLER\",\"msg\":\"STATUS\",\"payload\":{\"ADC0\":\"%d\",\"ADC1\":\"%d\",\"CTRL0\":\"%d\",\"CTRL1\":\"%d\",\"REMOTE_CONTROL\":\"%d\"}}",
              ID1,
              potVal[0],
              potVal[1],
              ctrlIn[0],
              ctrlIn[1],
              enableRemoteControl);
          webSocket.sendTXT(message);

          sprintf(message,"{\"sender\":\"%s\",\"destination\":\"CONTROLLER\",\"msg\":\"STATUS\",\"payload\":{\"ADC0\":\"%d\",\"ADC1\":\"%d\",\"CTRL0\":\"%d\",\"CTRL1\":\"%d\",\"REMOTE_CONTROL\":\"%d\"}}",
              ID2,
              potVal[2],
              potVal[3],
              ctrlIn[2],
              ctrlIn[3],
              enableRemoteControl);
          webSocket.sendTXT(message);
        }
      }
    }
    break;
  }

}
