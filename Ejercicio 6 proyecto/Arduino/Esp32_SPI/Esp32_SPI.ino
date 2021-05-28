#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>

#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "HOME-ACF0";
const char* password = "26102000";
WiFiUDP udpDevice;
uint16_t localUdpPort = 888;
uint16_t UDPPort = 5999;
#define MAX_LEDSERVERS 3
const char* hosts[MAX_LEDSERVERS] = {"192.168.1.66", "192.168.1.67", "192.168.1.68"};
#define SERIALMESSAGESIZE 3

uint8_t ledState = 0;
uint32_t previousMillis = 0;
#define ALIVE 1000
#define D0 27

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5


#define SEALEVELPRESSURE_HPA (1013.25)


Adafruit_BME280 bme(BME_CS); // hardware SPI
bool status;

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

  //BME&RTC SET-UP
  status = bme.begin();
}

//UDP TASKS
void networkTask() {
  uint8_t i2c_data[7];
  uint8_t packetSize = udpDevice.parsePacket();
  //Serial.println(packetSize);
  if (packetSize >= 7) {

 for (int i = 0 ; i <= 7; i++) {
     i2c_data[i]= udpDevice.read();
    // Serial.print(i2c_data[i]);
    }
   
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

    uint8_t command[] = {i2c_data[0], i2c_data[1], i2c_data[2], i2c_data[3], i2c_data[4], i2c_data[5], i2c_data[6],
                         (unsigned char)arr_temp[0], (unsigned char)arr_temp[1], (unsigned char)arr_temp[2], (unsigned char)arr_temp[3],
                         (unsigned char)arr_pres[0], (unsigned char)arr_pres[1], (unsigned char)arr_pres[2], (unsigned char)arr_pres[3],
                         (unsigned char)arr_alti[0], (unsigned char)arr_alti[1], (unsigned char)arr_alti[2], (unsigned char)arr_alti[3],
                         (unsigned char)arr_humi[0], (unsigned char)arr_humi[1], (unsigned char)arr_humi[2], (unsigned char)arr_humi[3]
                        };

    udpDevice.beginPacket(hosts[0] , UDPPort);
    udpDevice.write(command, sizeof(command));
    udpDevice .endPacket();
    for (int i = 0 ; i < 23; i++) {
      Serial.print(command[i]);
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
