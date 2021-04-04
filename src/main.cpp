#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <EspNow2MqttClient.hpp>

byte sharedKey[16] = {10,200,23,4,50,3,99,82,39,100,211,112,143,4,15,106};
byte sharedChannel = 11;
//uint8_t gatewayMac[6] = {0x3C, 0x71, 0xBF, 0xC0, 0x3E, 0x4C};  // ESP32 in box
uint8_t gatewayMac[6] = {0x24, 0x6F, 0x28, 0xAB, 0xBC, 0x48}; //

EspNow2MqttClient client = EspNow2MqttClient("bmp280", sharedKey, gatewayMac, sharedChannel);

//Create an instance of the object
BME280 bme280;

bool weHaveResponse = false;
RTC_DATA_ATTR long timeEnd = 0;
RTC_DATA_ATTR int counter = 0;
#define SLEEP_SECS 15 * 60 // 15 minutes
#define SEND_TIMEOUT 245  // 245 millis seconds timeout 
#define ANALOGPIN 36      // Analog GPIO pin

float temp, press, humi;
char tempChar[8], pressChar[8], humiChar[8];


void readBME280() {
  Serial.print("bmp280 init="); Serial.println(bme280.begin(), HEX);
  temp = bme280.readTempC();
  press = bme280.readFloatPressure() / 100.0;
  humi = bme280.readFloatHumidity();
  sprintf(tempChar, "%.1f", temp);
  sprintf(pressChar, "%.1f", press);
  sprintf(humiChar, "%.1f", humi);
  Serial.printf("Humidity = %.3f, Temperature = %.3f, Pressure = %.3f\n", humi, temp, press);
}

void gotoSleep(){
  timeEnd = millis();
  long timeMicros = SLEEP_SECS + ((uint8_t)esp_random()/2);
  esp_sleep_enable_timer_wakeup(timeMicros * 1000000);
  Serial.printf("Up for %lu ms, going to sleep for %lu secs...\n", millis(), timeMicros);
  esp_deep_sleep_start();
}
 
void setup() {
  Serial.begin(115200); Serial.println();

  Wire.begin();
  Wire.setClock(400000); //Increase to fast I2C speed!

  bme280.beginI2C();
  bme280.setI2CAddress(0x76);
  bme280.setMode(MODE_SLEEP); //Sleep for now
  
  // read sensor first before awake generates heat
  client.init();
  readBME280();
  client.doSend(tempChar,(char*)"temp");
  client.doSend(pressChar,(char*)"pres");
  client.doSend(humiChar,(char*)"humi");
}

void loop() {
    if(weHaveResponse || millis() > SEND_TIMEOUT){
      gotoSleep();
    }
    delay(10);
}


