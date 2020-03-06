#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <NewPing.h>
#include "DHT.h"
#include <OneWire.h> 
#include <DallasTemperature.h>

#define MAX_DISTANCE 200
#define DEBUG

unsigned long lastUpdate = 0;
const byte light = 11, water_trigPin = 35, water_echoPin = 34, feed_trigPin = 31, feed_echoPin = 30;
const byte pumpIn = 40, pumpOut = 36, fanIn = 2, fanOut = 3, feed = 32, dhtPin = 33;
byte feedAvailable, waterLevel, lightIntensity;
int fanSpeed;
bool wiring_problem = false, maintainWaterLevel = true;
float waterDistance, waterTemp, feedDistance, humidity, airTemp, tankDistance = 10;

OneWire oneWire(52);
DallasTemperature sensors(&oneWire);
DHT dht(33, DHT22);
NewPing waterSensor(water_trigPin, water_echoPin, MAX_DISTANCE);
NewPing feedSensor(water_trigPin, water_echoPin, MAX_DISTANCE);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
//
void setup() {
  Serial.begin(9600);
  Serial3.begin(115200);
  sensors.begin(); 
  dht.begin();
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  pinMode(pumpIn, OUTPUT);
  pinMode(pumpOut, OUTPUT);
  pinMode(feed, OUTPUT);
  digitalWrite(pumpIn, LOW);
  digitalWrite(pumpOut, LOW);
  digitalWrite(feed, LOW);
}

void loop() {
  if(wiring_problem){
    Serial.println("Wiring problem");
    delay(2000);
    return;
  }
//  waterLevelControl();
  delay(100);
  if(Serial3.available()){
    parseCommand(readStr());
  }
  if(millis()-lastUpdate>=2000){
    lastUpdate = millis();
    realTime();
  }
}

void parseCommand(String str) {
  String cmd = String(str);
  Serial.println("New command: " + cmd);
//  #ifdef DEBUG
//    cmd = Serial.readString();
//  #endif
  char c = '\0';
  Serial.println("cmd len: " + String(cmd.length()));
  if(cmd.length()>10) return;
  Serial.println("Execution");
  c = cmd.charAt(0);
  if(c=='S'){
    fanSpeed = cmd.substring(1, cmd.length()).toInt();
      if(fanSpeed >=0){
        Serial.println("Fan in.");
        analogWrite(fanIn, (int)fanSpeed*2.55);
        analogWrite(fanOut, 0);
      }else if(fanSpeed <=0){
        Serial.println("Fan out.");
        analogWrite(fanOut, (int)(fanSpeed*-1)*2.55);
        analogWrite(fanIn, 0);
      }
  }else if(c=='P'){
    int val = cmd.substring(1, cmd.length()).toInt();
      Serial.println("Pump: " + String(val));
      if(val == 0){
        digitalWrite(pumpIn, LOW);
        digitalWrite(pumpOut, LOW);
      }else if(val == 1){
        digitalWrite(pumpIn, HIGH);
        digitalWrite(pumpOut, LOW);
      }else if(val == -1){
        digitalWrite(pumpIn, LOW);
        digitalWrite(pumpOut, HIGH);
      }
  }else if(c=='F'){
    int t = cmd.substring(1, cmd.length()).toInt();
      Serial.println("Feeding: " + String(100));
      digitalWrite(feed, HIGH);
      delay(t);
      digitalWrite(feed, LOW);
  }else if(c=='R'){
    advancedRead();
  }
//  switch (c) {
//    case 'W':
//      waterLevel = cmd.substring(1, cmd.length()).toInt();
//      break;
//    case 'X':
//      cmd.charAt(1)=='0' ? maintainWaterLevel = false : maintainWaterLevel = true;
//      break;
//    case 'P':
//      int val = cmd.substring(1, cmd.length()).toInt();
//      Serial.println("Pump: " + String(val));
//      if(val == 0){
//        digitalWrite(pumpIn, LOW);
//        digitalWrite(pumpOut, LOW);
//      }else if(val == 1){
//        digitalWrite(pumpIn, HIGH);
//        digitalWrite(pumpOut, LOW);
//      }else if(val == -1){
//        digitalWrite(pumpIn, LOW);
//        digitalWrite(pumpOut, HIGH);
//      }
//      break;
//    case 'S':
//      fanSpeed = cmd.substring(1, cmd.length()).toInt();
//      if(fanSpeed >=0){
//        Serial.println("Fan in.");
//        analogWrite(fanIn, (int)fanSpeed*2.55);
//        analogWrite(fanOut, 0);
//      }else if(fanSpeed <=0){
//        Serial.println("Fan out.");
//        analogWrite(fanOut, (int)(fanSpeed*-1)*2.55);
//        analogWrite(fanIn, 0);
//      }
//      break;
//    case 'F':
//      int t = cmd.substring(1, cmd.length()).toInt();
//      Serial.println("Feeding: " + String(100));
//      digitalWrite(feed, HIGH);
//      delay(t);
//      digitalWrite(feed, LOW);
//      break;
//    
//    
//    case 'L':
//      lightIntensity = cmd.substring(1, cmd.length()).toInt();
//      break;
//    case 'R':
//      advancedRead();
//      break;
//    case 'r':
//    case 'g':
//    case 'b':
//      advancedRead();
//      break;
//    default:
//      Serial.println("Unknown: " + cmd);
//      break;
//  }
}

void advancedRead(void){
  String outStr = "";
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.getRawData(&r, &g, &b, &c);
  delay(250); //discard the first reading for better result
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);
  outStr += "\"r\":\"" + String(r) + "\","; 
  outStr += "\"g\":\"" + String(g) + "\",";
  outStr += "\"b\":\"" + String(b) + "\",";
  outStr += "\"c\":\"" + String(c) + "\",";  
  outStr += "\"colorTemp\":\"" + String(colorTemp) + "\",";
  outStr += "\"tcsLux\":\"" + String(lux) + "\"";
  Serial.println(outStr);
  Serial3.println(outStr);  
}

void realTime(){
  waterDistance = (waterSensor.ping()/2) * 0.0343;
  delay(100);
  readDHT();
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);
  feedDistance = (feedSensor.ping()/2) * 0.0343;
  String out = "";
  out+="{\"type\":\"realtime\",\"content\":{";
  out+="\"WaterLevelAquarium\":"+String(waterDistance, 2) + ",";
  out+="\"WaterLevelTank\":"+String(tankDistance, 2) + ",";
  out+="\"WaterTemp\":"+String(waterTemp, 2) + ",";
  out+="\"AirTemp\":"+String(airTemp, 2) + ",";
  out+="\"AirHumidity\":"+String(humidity, 2) + ",";
  out+="\"Feeds\":"+String(feedDistance, 2);
  out+="}}";
  Serial3.print(out+";");
  Serial.println(out);
}

void waterLevelControl(){
  if(maintainWaterLevel){
    if(waterLevel > waterDistance){
      digitalWrite(pumpOut, LOW);
      digitalWrite(pumpIn, HIGH);
    }else if(waterLevel < waterDistance){
      digitalWrite(pumpIn, LOW);
      digitalWrite(pumpOut, HIGH);
    }else{
      digitalWrite(pumpIn, HIGH);
      digitalWrite(pumpOut, HIGH);
    }
  }
}

void readDHT(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    return;
  }
  humidity = h;
  airTemp = t;
}

void debug(){
  if(Serial.available()){
    Serial3.println(Serial.readString());
  }
}

String readStr(){
  String out = "";
  char c = '\0';
  while(c !=';'){
    if(Serial3.available()){
      c = (char)Serial3.read();
      out+=String(c);
    }
  }
  out = out.substring(0,out.length()-1);
  Serial3.flush();
  #ifdef DEBUG
  Serial.println(out);
  #endif
  return out;
}
