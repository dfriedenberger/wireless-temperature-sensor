#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

#include <Crypto.h>
#include <SHA512.h>
#include <string.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 6

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

#define HASH_SIZE 64
SHA512 sha512;


#include <base64.hpp>

int counter = 0;

String id = String("did:dad:uuRCNOYlWk73a4DuHIyIHllxf-7e99Fp7--6MW6GJ1k=");
char secret[32] = "cdfvrefasafxagdfgag";

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("LoRa Sender");
  if (!LoRa.begin(868E6)) {
    //Serial.println("Starting LoRa failed!");
    while (1);
  }

  sensors.begin(); // Start up the library

}



void loop() {
 
  //Serial.println(counter);


  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  // Send the command to get temperature readings
  sensors.requestTemperatures();

  String temp = String(sensors.getTempCByIndex(0));
  //Serial.println("Temperature is: " + temp + "°C");

    // You can have more than one DS18B20 on the same bus.
    // 0 refers to the first IC on the wire


  DynamicJsonDocument doc(1024);
  
  doc["id"] = id;
  //doc["changed"]   = "2020-10-18T13:56:00.896Z";
  doc["sequence"]   = counter;
  doc["temperature"]["value"] = temp;
  doc["temperature"]["unit"] = "celsius";
  //doc["geometry"]["coordinate"]["latitude"] = 52.520008;
  //doc["geometry"]["coordinate"]["longitude"] = 13.404954;

  char data[2048];
  serializeJson(doc, data);
  Serial.println(data);

  uint8_t value[HASH_SIZE];
  sha512.reset();
  sha512.update(data, strlen(data));
  sha512.update(secret, strlen(secret));
  sha512.finalize(value, sizeof(value));
  
  unsigned char base64[2 * HASH_SIZE]; // 8 bytes for output + 1 for null terminator
  unsigned int base64_length = encode_base64(value, HASH_SIZE, base64);
  Serial.println((char*)base64);


  // send packet
  LoRa.beginPacket();
  LoRa.println(data);
  LoRa.println((char*)base64);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
