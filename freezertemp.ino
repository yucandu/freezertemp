#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "time.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
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

char auth[] = "VnFlJdW3V0uZQaqslqPJi6WPA9LaG1Pk";

AsyncWebServer server(80);

WidgetTerminal terminal(V10);

#define every(interval) \
    static uint32_t __every__##interval = millis(); \
    if (millis() - __every__##interval >= interval && (__every__##interval = millis()))

BLYNK_WRITE(V10) {
  if (String("help") == param.asStr()) {
    terminal.println("==List of available commands:==");
    terminal.println("wifi");
    terminal.println("temps");
    terminal.println("==End of list.==");
  }
  if (String("wifi") == param.asStr()) {
    terminal.print("Connected to: ");
    terminal.println(ssid);
    terminal.print("IP address:");
    terminal.println(WiFi.localIP());
    terminal.print("Signal strength: ");
    terminal.println(WiFi.RSSI());
    printLocalTime();
  }
  if (String("temps") == param.asStr()) {
    aht.getEvent(&humidity, &temp);
    terminal.print("DSTemp: ");
    terminal.println(temp.temperature);
    terminal.print("ESPTemp:");
    terminal.println(temperatureRead());
  }
    terminal.flush();
}

BLYNK_WRITE(V11)
{
  if (param.asInt() == 1) {buttonstart = true;}
  if (param.asInt() == 0) {buttonstart = false;}
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V11);
}


void printLocalTime() {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  terminal.print(asctime(timeinfo));
}


void gotoSleep(int sleeptimeSecs) {
      //WiFi.disconnect();
      delay(1);
      esp_sleep_enable_timer_wakeup(sleeptimeSecs * 1000000ULL);
      delay(1);
      esp_deep_sleep_start();
      delay(1000);
}


void setup(void) {
  esptemp = temperatureRead();
  volts = analogReadMilliVolts(1) / 500.0;
  //pinMode(8, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //WiFi.setTxPower(WIFI_POWER_19dBm);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    //digitalWrite(8, HIGH);
    //delay(250);
    //digitalWrite(8, LOW);
  }
  //digitalWrite(8, LOW);
  //pinMode(8, INPUT);

  Blynk.config(auth, IPAddress(192, 168, 50, 197), 8080);
  Blynk.connect();
  while (!Blynk.connected()){delay(250);}

  
  if (! aht.begin()) {
    terminal.println("Could not find AHT? Check wiring");
  }
  //terminal.println("AHT10 or AHT20 found");
  
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);
  bmp.takeForcedMeasurement();
  float presread = bmp.readPressure() / 100.0;
  delay(100);
        aht.getEvent(&humidity, &temp);
      Blynk.virtualWrite(V1, temp.temperature); //temperatureRead();
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V2, esptemp);
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V3, humidity.relative_humidity); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V4, WiFi.RSSI()); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V5, presread); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V6, volts); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V7, 0); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      Blynk.virtualWrite(V7, 0); 
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      //delay(1000);

  if (!buttonstart){
    gotoSleep(300);
  }
  else{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hi! I am ESP32.");
    });

    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started");
    delay(250);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(250);
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    hours = timeinfo.tm_hour;
    mins = timeinfo.tm_min;
    secs = timeinfo.tm_sec;
    terminal.println("***FREEZER TEMP 2.0 STARTED***");
    terminal.print("Connected to ");
    terminal.println(ssid);
    terminal.print("IP address: ");
    terminal.println(WiFi.localIP());
    printLocalTime();
  }
terminal.flush();
}

void loop() {
  
      if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
      every(5000){
      aht.getEvent(&humidity, &temp);
      bmp.takeForcedMeasurement();
      volts = analogReadMilliVolts(1) / 500.0;
      float presread = bmp.readPressure() / 100.0;
      Blynk.virtualWrite(V1, temp.temperature); //temperatureRead();
      Blynk.virtualWrite(V2, temperatureRead());
      Blynk.virtualWrite(V3, humidity.relative_humidity); 
      Blynk.virtualWrite(V4, WiFi.RSSI()); 
      Blynk.virtualWrite(V5, presread); 
      Blynk.virtualWrite(V6, volts); 
      }

}