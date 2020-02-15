#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
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
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  sensors.begin(); 
  dht.begin();
  if(!tsl.begin()){
    Serial.println("TSL not found");
    wiring_problem = true;
    delay(5000);
    return;
  }
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);

}

void loop() {
  if(wiring_problem){
    Serial.println("Wiring problem");
    delay(2000);
    return;
  }
  advancedRead();
  waterLevelControl();
  delay(500);
}

void parseCommand(String cmd) {
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
    default:
      Serial.println("Invalid Command");
      break;
  }
}

void advancedRead(void){
  String outStr = "";
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  float lux;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  lux = tsl.calculateLux(full, ir);
  outStr += "C{";
  outStr += "\"ir\":\"" + String(ir) + "\","; 
  outStr += "\"full\":\"" + String(full) + "\",";
  outStr += "\"lux\":\"" + String(lux,6) + "\",";
  Serial.println(outStr);  
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
