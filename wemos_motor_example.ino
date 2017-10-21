#include "WEMOS_Motor.h"
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WiFiMulti.h>
#include <Arduino.h>

#define MESSAGE_INTERVAL 400

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor MA(0x30,_MOTOR_A, 1000);//Motor A
Motor MB(0x30,_MOTOR_B, 1000);//Motor B

uint8_t aDir;
uint8_t bDir;
float pwm = 0.0;
float aPwr = 1.1;
float bPwr = 1.1;
uint64_t messageTimestamp = 0;
bool isConnected = false;

WebSocketsServer webSocket = WebSocketsServer(81);
const char* ssid     = "Enter Your Router SSID Here";
const char* password = "Enter Your Router Password Here";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    switch(type) {
        case WStype_DISCONNECTED:
            isConnected = false;           
            break;
        case WStype_CONNECTED:
         {
                IPAddress ip = webSocket.remoteIP(num);  
                isConnected = true;           
            }
            break;
        case WStype_TEXT:
        {
          Serial.printf("\r\n[%u] get Text: %s\n", num, payload);
           String text = String((char *) payload);         
           if(text.startsWith("p")){
            String motorA=(text.substring(text.indexOf("p")+1,text.indexOf(",")));
            String motorB=(text.substring(text.indexOf(",")+1,text.length()));
            Serial.print("A: "); Serial.print(motorA); Serial.print(" B: "); Serial.print(motorB);
            String motorADir = motorA.substring(0,1);
            String motorAPwr = motorA.substring(1,motorA.length());
            String motorBDir = motorB.substring(0,1);
            String motorBPwr = motorB.substring(1,motorB.length());               
            aPwr = motorAPwr.toFloat();
            bPwr = motorBPwr.toFloat();
            if(motorADir == "f"){
              aDir = _CCW;  
            }
            if( motorADir == "r"){
              aDir = _CW;
            }
            if(motorBDir == "f"){
              bDir = _CCW;  
            }
            if( motorBDir == "r"){
              bDir = _CW;
            }
           }                 
        }   
        webSocket.sendTXT(num, payload, lenght);
        webSocket.broadcastTXT(payload, lenght);
        break;
        
        case WStype_BIN:
            hexdump(payload, lenght);
            // echo data back to browser
            webSocket.sendBIN(num, payload, lenght);
            break;
    }
}

void setup() {
  Serial.begin(250000);
   WiFi.begin(ssid, password);
   Serial.println("");
   Serial.print("Connecting");

    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    
  for (pwm = 30.3; pwm <= 100; pwm+=12.1)
  {
   MA.setmotor( _CCW, pwm);
   MB.setmotor( _CCW, pwm);
  }
  pwm=0.0;

    MA.setmotor( _STOP);
    MB.setmotor( _STOP);
    
  Serial.println(""); 
  Serial.println("copy following line into ip field of local html file");
  Serial.print("ws://"); Serial.print(WiFi.localIP()); Serial.println(":81");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}
void loop() {
   webSocket.loop();


   if(isConnected && (millis() > (messageTimestamp + MESSAGE_INTERVAL))){
    Serial.println("moved");
    uint8_t aDirtwo = aDir;
    uint8_t bDirtwo = bDir;
    float aPower = (float)aPwr;
    float bPower = (float)bPwr;
    Serial.print("dirA: "); Serial.println(aDirtwo); Serial.print(" dirB: ");Serial.println(bDirtwo); Serial.print(" powerA: "); Serial.println(aPwr); Serial.print(" powerB: "); Serial.println(bPwr);
    messageTimestamp = millis();
    MA.setmotor( aDirtwo, aPower);
    MB.setmotor( bDirtwo, bPower);
  } 

}
