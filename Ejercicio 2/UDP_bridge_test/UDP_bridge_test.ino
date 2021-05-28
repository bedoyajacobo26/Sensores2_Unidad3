#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "HOME-ACF0";
const char* password = "26102000";
WiFiUDP udpDevice;
uint16_t localUdpPort = 4023;
uint16_t UDPPort = 23;
#define MAX_LEDSERVERS 3
const char* hosts[MAX_LEDSERVERS] = {"192.168.1.76","192.168.1.66","192.168.1.78"};
#define SERIALMESSAGESIZE 3
uint32_t previousMillis = 0;
#define ALIVE 1000
#define D0 5

void setup() {
  pinMode(D0, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(D0, HIGH);
  Serial.begin(115200);
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
}

void networkTask() {
  uint8_t LEDServer = 0;
  uint8_t LEDValue = 0;
  uint8_t syncChar;

  // Serial event:
  if (Serial.available() >= SERIALMESSAGESIZE) {
    LEDServer = Serial.read()- '0';
    LEDValue = Serial.read();
    syncChar = Serial.read();
    if ((LEDServer == 0) || (LEDServer > 3)) {
      Serial.println("Servidor inválido (seleccione 1,2,3)");
      return;
    }
    if (syncChar == '*') {
      udpDevice.beginPacket(hosts[LEDServer - 1] , localUdpPort);
      udpDevice.write(LEDValue);
      udpDevice.endPacket();
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
  uint32_t currentMillis;
  static uint8_t ledState = 0;
  currentMillis  = millis();
  if ((currentMillis - previousMillis) >= ALIVE) {
    previousMillis = currentMillis;
    if (ledState == 0) {
      digitalWrite(D0, HIGH);
      ledState = 1;
    }
    else {
      digitalWrite(D0, LOW);
      ledState = 0;
    }
  }
}

void loop() {
  networkTask();
  aliveTask();
}
