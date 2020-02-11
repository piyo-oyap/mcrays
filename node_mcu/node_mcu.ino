#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

#define LED D0
const char* ssid = "Ganda & Cute Boarding House"; //Enter SSID
const char* password = "llaneta583"; //Enter Password
const char* websockets_server_host = "10.0.0.4"; //Enter server adress
const int websockets_server_port = 5001; // Enter server port

using namespace websockets;

WebsocketsClient client;

DynamicJsonDocument doc(1024);
JsonObject msg = doc.as<JsonObject>();
void setup() {
    Serial.begin(115200);
    Serial1.begin(9600);

    // Connect to wifi
    WiFi.begin(ssid, password);
    delay(3000); //needed delay to make sure the 
    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }
//    Serial.println(WiFi.localIP);
    Serial.println("Connected to Wifi, Connecting to server.");
    delay(3000);
    pinMode(LED, OUTPUT);
    // try to connect to Websockets server
    client.addHeader("sec-websocket-protocol","device");
    bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
    if(connected) {
        delay(1000);
        Serial.println("Connecetd!");
        digitalWrite(LED, LOW);
    } else {
        Serial.println("Not Connected!");
    }
    
    // run callback when messages are received
    client.onMessage([&](WebsocketsMessage message) {
        deserializeJson(doc, message.data());
        Serial.println(msg["type"].as<String>() + " : " + msg["content"].as<String>());
        Serial1.println(msg["content"].as<String>());
        
        
    });
}

void loop() {
    // let the websockets client check for incoming messages
    if(client.available()) {
        client.poll();
    }
    delay(500);
}
