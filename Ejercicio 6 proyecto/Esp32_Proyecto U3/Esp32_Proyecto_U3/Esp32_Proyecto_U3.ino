#include "Wire.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>

#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Multitienda Colombia";
const char* password = "Covid2020*";
WiFiUDP udpDevice;
uint16_t localUdpPort = 777;
uint16_t UDPPort = 4023;
#define MAX_LEDSERVERS 3
const char* hosts[MAX_LEDSERVERS] = {"192.168.1.69", "192.168.1.67", "192.168.1.68"};
#define SERIALMESSAGESIZE 3

uint8_t ledState = 0;
#define D0 27

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5


#define SEALEVELPRESSURE_HPA (1013.25)
#define DS3231_I2C_ADDRESS 0x68
Adafruit_BME280 bme(BME_CS); // hardware SPI
bool status;


byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}
void setup() {
  //UDP SET-UP
  Serial.begin(115200);
  pinMode(D0, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(D0, HIGH);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  udpDevice.begin(UDPPort);

  //BME&RTC SET-UP
  Wire.begin();
  status = bme.begin();
  setDS3231time(10, 36, 22, 1, 19, 4, 21);

}

//UDP TASKS

void networkTask() {

  if (Serial.available() > 0) {
    if (Serial.read() == 0x73) {
      byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
      readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
                     &year);

      float temperature = bme.readTemperature();
      float pressure = (bme.readPressure() / 100.0F);
      float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
      float humidity = bme.readHumidity();

      uint8_t arr_temp[4] = {0};
      memcpy(arr_temp, (uint8_t *)&temperature, 4);
      uint8_t arr_pres[4] = {0};
      memcpy(arr_pres, (uint8_t *)&pressure, 4);
      uint8_t arr_alti[4] = {0};
      memcpy(arr_alti, (uint8_t *)&altitude, 4);
      uint8_t arr_humi[4] = {0};
      memcpy(arr_humi, (uint8_t *)&humidity, 4);

      uint8_t command[] = {second, minute, hour, dayOfWeek, dayOfMonth, month, year,
                           (unsigned char)arr_temp[0], (unsigned char)arr_temp[1], (unsigned char)arr_temp[2], (unsigned char)arr_temp[3],
                           (unsigned char)arr_pres[0], (unsigned char)arr_pres[1], (unsigned char)arr_pres[2], (unsigned char)arr_pres[3],
                           (unsigned char)arr_alti[0], (unsigned char)arr_alti[1], (unsigned char)arr_alti[2], (unsigned char)arr_alti[3],
                           (unsigned char)arr_humi[0], (unsigned char)arr_humi[1], (unsigned char)arr_humi[2], (unsigned char)arr_humi[3]
                          };

      udpDevice.beginPacket(hosts[0] , UDPPort);
      udpDevice.write(command, sizeof(command));
      udpDevice.endPacket();
      Serial.println("UDP");

    }
  }
  // UDP event:
  uint8_t packetSize = udpDevice.parsePacket();
  if (packetSize) {
    Serial.print("Data from: ");
    Serial.print(udpDevice.remoteIP());
    Serial.print(":");
    Serial.print(udpDevice.remotePort());
    Serial.print(' ');
    for (uint8_t i = 0; i < packetSize; i++) {
      Serial.write(udpDevice.read());
    }
  }
}

void aliveTask() {

  if (ledState == 0) {
    digitalWrite(D0, HIGH);
    ledState = 1;
    Serial.println("H");
  }
  else {
    digitalWrite(D0, LOW);
    ledState = 0;
    Serial.println("L");
  }
}


//RTC TASKS
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);

  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void loop() {
  networkTask();
  aliveTask();
  delay(1000);

}
