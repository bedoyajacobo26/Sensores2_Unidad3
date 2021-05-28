#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "HOME-ACF0";
const char* password = "26102000";
WiFiUDP udpDevice;
uint16_t localUdpPort = 4023;
uint32_t previousMillis = 0;
#define ALIVE 1000
#define D0 5
#define D8 18

void setup() {
  pinMode(D0, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(D0, HIGH);
  pinMode(D8, OUTPUT);
  digitalWrite(D8, LOW);
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
  uint8_t data;
  uint8_t packetSize = udpDevice.parsePacket();
  if (packetSize) {
    data = udpDevice.read();
    if (data == '1') {
      digitalWrite(D0, HIGH);
    } else if (data == '0') {
      digitalWrite(D0, LOW);
    }
    // send back a reply, to the IP address and port we got the packet from
    udpDevice.beginPacket(udpDevice.remoteIP(), udpDevice.remotePort());
    udpDevice.write('1');
    udpDevice .endPacket();
  }
}

void aliveTask() {
  uint32_t currentMillis;
  static uint8_t ledState = 0;
  currentMillis  = millis();
  if ((currentMillis - previousMillis) >= ALIVE) {
    previousMillis = currentMillis;
    if (ledState == 0) digitalWrite(D8, HIGH);
    else digitalWrite(D8, LOW);
  }
}

void loop() {
  networkTask();
  aliveTask();
}
