#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include "time.h"
#include <DFRobot_MAX30102.h>

#define WIFI_SSID "PLDTHome"
#define WIFI_PASSWORD "p@ssw0rd"
#define API_KEY "AIzaSyCFQAp81x9NPxrayvmV2ewqZDaurLTH2W4"
#define DATABASE_URL "https://sleep-haven-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define EMAIL "boopboop@gmail.com"
#define PASSWORD "boopboop"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
DFRobot_MAX30102 particleSensor;

const unsigned long timeUpdateInterval = 5000;
unsigned long lastTimeUpdateMillis = 0;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

int32_t heartRate;
int8_t heartRateValid;
int32_t SPO2;
int8_t SPO2Valid;


void setup() {
  Serial.begin(115200);
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
                        /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
                        /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign in with email and password
  auth.user.email = EMAIL;
  auth.user.password = PASSWORD;
  if (EMAIL != "" && PASSWORD != "") {
    signupOK = true;
    Serial.println("Successfully Logged in");
  }
  else{
    Serial.println("Something went Wrong");
  }


  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  config.timeout.networkReconnect = 10 * 1000;
  config.timeout.serverResponse = 10 * 1000;
  config.timeout.rtdbKeepAlive = 45 * 1000;
  config.timeout.rtdbStreamReconnect = 1 * 1000;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Wait about four seconds"));
    particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);
    
    Serial.print(F("heartRate="));
    Serial.print(heartRate, DEC);
    Serial.print(F(", heartRateValid="));
    Serial.print(heartRateValid, DEC);

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 6500 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      float bpmFB = heartRate;
      String timeBPM = String(bpmFB);
      if (Firebase.RTDB.pushString(&fbdo, "Users/ItV0lwUXJ8PudsfT627g2bERwcF3/MAX30102/BPM", timeBPM)) {
        Serial.println();
        Serial.print(heartRate);
        Serial.print(" - successfully saved to: " + fbdo.dataPath());
        Serial.println("(" + fbdo.dataType() + ")");
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }
    }
  }
}
