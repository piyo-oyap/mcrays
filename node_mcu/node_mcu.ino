#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

const char* ssid = "Ganda & Cute Boarding House"; //Enter SSID
const char* password = "llaneta583"; //Enter Password
const char* websockets_server = "10.0.0.33"; //server adress
const int port = 5001;

unsigned long lastPing = 0;
String cmd = "";
const byte led[]={D3,D4,D7};

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
//SoftwareSerial arduino_mega(D1, D2);
StaticJsonDocument<200> doc;

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
  String json = message.data();
  DeserializationError error = deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  if (error) {
//    Serial.print(F("deserializeJson() failed: "));
//    Serial.println(error.c_str());
    return;
  }
  cmd = obj[String("content")].as<String>();
  parseCmd(cmd);
//  Serial.println(obj[String("content")].as<String>());
  statusBlink(500);
  lastPing = millis();
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if(event == WebsocketsEvent::ConnectionOpened) {
//    Serial.println("Connnection Opened");
  } else if(event == WebsocketsEvent::ConnectionClosed) {
//    Serial.println("Connnection Closed");
    reconnectToServer();
  } else if(event == WebsocketsEvent::GotPing) {
//    Serial.print("+");
    statusBlink(200);
    lastPing = millis();
  } else if(event == WebsocketsEvent::GotPong) {
//    Serial.println("Got a Pong!");
  }
}

WebsocketsClient client;
void setup() {
  Serial.begin(115200);
  pinMode(D5, OUTPUT);
  if(!tsl.begin()){
//    Serial.println("TSL not found") ;
    delay(5000);
    return;
  }
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  while( WiFi.status() != WL_CONNECTED) {
    statusBlink(1000);
//    Serial.print(".");
  }
//  Serial.println("WiFi connected");
  delay(8000);//wait some time to reset the connection from the server
  // Setup Callbacks
  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);
  
  // Connect to server
  client.addHeader("sec-websocket-protocol", "device");
  reconnectToServer();

  // Send a message
//  client.send("Hi Server!");
  // Send a ping
  client.ping();
  pinMode(led[0], OUTPUT);
  pinMode(led[1], OUTPUT);
  pinMode(led[2], OUTPUT);
}

void loop() {
  client.poll();
  if(millis()-lastPing>8000){
    reconnectToServer();
  }

  if(Serial.available()){
    pushUpdate();
  }
}

void reconnectToServer(){
  while(1){
//    Serial.println("Reconnecting");
    statusBlink(250);
    delay(500);
    statusBlink(1000);
    if(client.connect(websockets_server, port, "/")){
      lastPing = millis();
      return;
    }
  }
}

void statusBlink(int sleep){
//  digitalWrite(D5, HIGH);
  delay(sleep);
//  digitalWrite(D5, LOW);
}

void pushUpdate(){
  String content = readStr();
  client.send("{\"type\":\"push-update\", \"content\":" + content + "}");
}

String advancedRead(String color){
  if(Serial.available()) Serial.flush(); //clean the buffer
  Serial.println("R");
  String outStr = "";
  tsl.getFullLuminosity();
  delay(250); //discard the first reading for better result
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  float lux;
  ir = lum >> 16; 
  full = lum & 0xFFFF;
  lux = tsl.calculateLux(full, ir);
  outStr += "{";
  outStr += "\"ir\":\"" + String(ir) + "\","; 
  outStr += "\"full\":\"" + String(full) + "\",";
  outStr += "\"tslLux\":\"" + String(lux,6) + "\",";
  while(!Serial.available());
  outStr += readStr();
  return outStr += "}";
  
//  Serial.println(outStr);
//  client.send("{\"type\":\"coloromiter\",\"content\":" + outStr); //to fix colorimeter spelling
}

void colorimeter(){
  String ledColor[] = {"red", "green", "blue"}, outStr = "";
//  outStr += "{"
  for(int i=0; i<3; i++){
    outStr += "\"" + ledColor[i] + "\":";
    outStr += advancedRead(ledColor[i]);
    if(i<2) outStr += ",";
  }
  client.send("{\"type\":\"colorimeter\",\"content\":{" + outStr + "}");
}

void parseCmd(String cmd){
  switch(cmd.charAt(0)){
    case 'R':
    colorimeter();
      break;
    case 'r':
      Serial.println("red");
      cmd.charAt(1) == '1' ? digitalWrite(led[0], HIGH) : digitalWrite(led[0], LOW);
      break;
    case 'g':
      Serial.println("green");
      cmd.charAt(1) == '1' ? digitalWrite(led[1], HIGH) : digitalWrite(led[1], LOW);
      break;
    case 'b':
      Serial.println("blue");
      cmd.charAt(1) == '1' ? digitalWrite(led[2], HIGH) : digitalWrite(led[2], LOW);
      break;
    case 'L':
      analogWrite(D6,cmd.substring(1,cmd.length()).toInt()*10);
    default:
      Serial.print(cmd + ";");
      break;
  }
}

String readStr(){
  String out = "";
  char c = '\0';
  while(c !=';'){
    if(Serial.available()){
      c = (char)Serial.read();
      out+=c;
    }
  }
  out = out.substring(0,out.length()-1);
  return out;
}
