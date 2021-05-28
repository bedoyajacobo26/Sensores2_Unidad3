#define DS3231_I2C_ADDRESS 0x68
#include "Wire.h"
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "HOME-ACF0";
const char* password = "26102000";
WiFiUDP udpDevice;
uint16_t localUdpPort = 777;
uint16_t UDPPort = 888;
#define MAX_LEDSERVERS 3
const char* hosts[MAX_LEDSERVERS] = {"192.168.1.77", "192.168.1.67", "192.168.1.68"};
#define SERIALMESSAGESIZE 3

uint8_t ledState = 0;
#define D0 27

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
  udpDevice.begin(localUdpPort);

  //RTC SET-UP
  Wire.begin();
  setDS3231time(10, 30, 4, 1, 24, 5, 21);
}

byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

void networkTask() {

  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
                 &year);

  uint8_t command[] = {second, minute, hour, dayOfWeek, dayOfMonth, month, year};

  udpDevice.beginPacket(hosts[0] , UDPPort);
  udpDevice.write(command, sizeof(command));
  udpDevice.endPacket();
  for (uint8_t i = 0; i < 7; i++) {
    Serial.println(command[i]);
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

void aliveTask() {

  if (ledState == 0) {
    digitalWrite(D0, HIGH);
    ledState = 1;
  }
  else {
    digitalWrite(D0, LOW);
    ledState = 0;
  }
}

void loop() {
  networkTask();
  aliveTask();
  delay(1000);
}
