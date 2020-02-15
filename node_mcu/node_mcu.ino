#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

const char* ssid = "Ganda & Cute Boarding House"; //Enter SSID
const char* password = "llaneta583"; //Enter Password
const char* websockets_server = "10.0.0.4"; //server adress
const int port = 5001;

unsigned long lastPing = 0;

SoftwareSerial arduino_mega(D1, D2);
StaticJsonDocument<200> doc;

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
//  Serial.print("Got Message: ");
//  Serial.println(message.data());
  String json = message.data();
  DeserializationError error = deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  Serial.println(obj[String("content")].as<String>());
  arduino_mega.println(obj[String("content")].as<String>());
  statusBlink(500);
  lastPing = millis();
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if(event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if(event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
    reconnectToServer();
  } else if(event == WebsocketsEvent::GotPing) {
    Serial.print("+");
    statusBlink(200);
    lastPing = millis();
  } else if(event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

WebsocketsClient client;
void setup() {
  Serial.begin(115200);
  pinMode(D5, OUTPUT);
  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  while( WiFi.status() != WL_CONNECTED) {
    statusBlink(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
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
}

void loop() {
  client.poll();
  if(millis()-lastPing>8000){
    reconnectToServer();
  }

  if(arduino_mega.available()){
    pushUpdate();
  }
}

void reconnectToServer(){
  while(1){
    Serial.println("Reconnecting");
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
  digitalWrite(D5, HIGH);
  delay(sleep);
  digitalWrite(D5, LOW);
}

void pushUpdate(){
  String content = arduino_mega.readString();
  client.send("\"tpye\":\"update\", \"content\":\"" + content + "\"");
}
