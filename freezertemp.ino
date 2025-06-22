#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ==================== CONFIGURATION ====================
#define DEVICE_ID 1002                    // Unique ID for this device (change for each C3)
#define TWO_WAY_COMM false                 // Set to false for broadcast-only mode
#define TRANSMISSION_INTERVAL 30000       // Send data every 30 seconds
#define MAX_RETRIES 3                     // Only used in two-way mode
#define ACK_TIMEOUT 1000                  // Timeout for ACK in two-way mode (ms)

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

sensors_event_t humidity, temp;

const char* ssid = "mikesnet";
const char* password = "springchicken";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  //Replace with your GMT offset (secs)
const int daylightOffset_sec = 0;   //Replace with your daylight offset (secs)
int hours, mins, secs;
bool buttonstart = false;
float dstemp, esptemp, volts;
float presread;
char auth[] = "VnFlJdW3V0uZQaqslqPJi6WPA9LaG1Pk";


// ==================== DATA STRUCTURES ====================
typedef struct {
  uint16_t device_id;
  float sensor_data[7];  // Array for up to 7 sensor values
  bool request_response; // True if device wants a response
  uint32_t timestamp;    // For debugging/tracking
} sensor_message_t;

typedef struct {
  uint16_t target_device_id;
  float response_data[7]; // Array for response data
  uint32_t timestamp;
} response_message_t;

// ==================== GLOBAL VARIABLES ====================
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast to all
esp_now_peer_info_t peerInfo;
bool waiting_for_response = false;
unsigned long response_timeout = 0;
unsigned long last_transmission = 0;

// ==================== SENSOR DATA FUNCTIONS ====================
void collectSensorData(float* data) {
      data[0] = temp.temperature; // Temperature in Celsius
      data[1] = humidity.relative_humidity;  // Unused
      data[2] = presread;  // Pressure in hPa
      data[3] = volts; // Battery voltage in Volts
      data[4] = 0.0;   // Unused
      data[5] = 0.0;   // Unused
      data[6] = 0.0;   // Unused
}

void sendSensorData() {
  sensor_message_t message;
  message.device_id = DEVICE_ID;
  message.request_response = TWO_WAY_COMM;
  message.timestamp = millis();
  
  // Collect sensor data
  collectSensorData(message.sensor_data);
  
  // Send the message
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
  
  if (result == ESP_OK) {
    Serial.printf("Sending data from device %d...\n", DEVICE_ID);
    
    // Print sensor data being sent
    Serial.println("Sensor data:");
    for(int i = 0; i < 7; i++) {
      if(message.sensor_data[i] != 0.0) {
        Serial.printf("  Sensor[%d]: %.2f\n", i, message.sensor_data[i]);
      }
    }
  } else {
    Serial.println("Error sending the data");
  }
}

// ==================== ESP-NOW CALLBACKS ====================
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(TWO_WAY_COMM) {
    if(status == ESP_NOW_SEND_SUCCESS) {
      Serial.println("Data sent successfully, waiting for response...");
      waiting_for_response = true;
      response_timeout = millis() + ACK_TIMEOUT;
    } else {
      Serial.println("Failed to send data");
      waiting_for_response = false;
    }
  } else {
    // Broadcast mode - just log status
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Data broadcasted" : "Broadcast failed");
  }
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {

}



#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))





void gotoSleep(int sleeptimeSecs) {
      //WiFi.disconnect();
      delay(1);
      esp_sleep_enable_timer_wakeup(sleeptimeSecs * 1000000ULL);
      delay(1);
      esp_deep_sleep_start();
      delay(1000);
}


void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello.");
  esptemp = temperatureRead();
  volts = analogReadMilliVolts(1) / 500.0;
  //pinMode(8, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); 
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  WiFi.setTxPower(WIFI_POWER_8_5dBm); 
  
  Serial.println("ESP-NOW initialized successfully");

  aht.begin();
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);
  bmp.takeForcedMeasurement();
   presread = bmp.readPressure() / 100.0;
  delay(100);
        aht.getEvent(&humidity, &temp);
     
  sendSensorData();
  WiFi.setTxPower(WIFI_POWER_19_5dBm); 
  sendSensorData();
  gotoSleep(300);
}

void loop() {
  
delay(1);
gotoSleep(300);
}
