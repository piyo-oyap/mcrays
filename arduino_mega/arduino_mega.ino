#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <NewPing.h>
#include "DHT.h"
#include <OneWire.h> 
#include <DallasTemperature.h>

#define MAX_DISTANCE 350

const byte fan = 10, light = 11, water_trigPin = 2, water_echoPin = 3, feed_trigPin = 4, feed_echoPin = 5, pumpIn = 9, pumpOut = 8;
byte feedAvailable, waterLevel, lightIntensity, fanSpeed;
bool wiring_problem = false, maintainWaterLevel = true;
float waterDistance, waterTemp, feedDistance, humidity, airTemp;

OneWire oneWire(7);
DallasTemperature sensors(&oneWire);
DHT dht(6, DHT22);
NewPing waterSensor(water_trigPin, water_echoPin, MAX_DISTANCE);
NewPing feedSensor(water_trigPin, water_echoPin, MAX_DISTANCE);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

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

}

void loop() {
  if(wiring_problem){
    Serial.println("Wiring problem");
    delay(2000);
    return;
  }
  waterLevelControl();
  delay(100);
  if(Serial3.available()){
    parseCommand(Serial3.readString());
  }
  debug();
  Serial3.println(".");
}

void parseCommand(String cmd) {
  Serial.println(cmd);
  char c = cmd.charAt(0);
  switch (c) {
    case 'W':
      waterLevel = cmd.substring(1, cmd.length()).toInt();
      break;
    case 'X':
      cmd.charAt(1)=='0' ? maintainWaterLevel = false : maintainWaterLevel = true;
      break;
    case 'F':
      fanSpeed = cmd.substring(1, cmd.length()).toInt();
      break;
    case 'L':
      lightIntensity = cmd.substring(1, cmd.length()).toInt();
      break;
    case 'R':
      advancedRead();
      break;
    case 'r':
    case 'g':
    case 'b':
      advancedRead();
      break;
    default:
      Serial.println(cmd);
      break;
  }
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
  delay(80);
  feedDistance = (feedSensor.ping()/2) * 0.0343;
  readDHT();
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);
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
