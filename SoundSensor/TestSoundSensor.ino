#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//#define WIFI_SSID "Fibr_Baquiran"
//#define WIFI_PASSWORD "wifin0og!Ecyj"
#define WIFI_SSID "PLDTHome"
#define WIFI_PASSWORD "p@ssw0rd"
#define API_KEY "AIzaSyCFQAp81x9NPxrayvmV2ewqZDaurLTH2W4"
#define DATABASE_URL "https://sleep-haven-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define PIN 12
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define USER_EMAIL "boopboop@gmail.com"
#define USER_PASSWORD "boopboop"
#define soundSensorPath "Users/ItV0lwUXJ8PudsfT627g2bERwcF3/DeviceStatus/SoundSensor"

const int relay = 5;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
int soundSensorVal = 0;

void setup() {
  Serial.begin(115200);  // enable serial monitor
  pinMode(PIN, INPUT);  // define arduino pin
  
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

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096, 1024);

  Firebase.begin(&config, &auth);

  config.timeout.networkReconnect = 10 * 1000;
  config.timeout.serverResponse = 10 * 1000;
  config.timeout.rtdbKeepAlive = 45 * 1000;
  config.timeout.rtdbStreamReconnect = 1 * 1000;
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    soundSensorVal = digitalRead(PIN);
    
    if (soundSensorVal == 0) {
      Serial.println("Sound Detected");  // print serial monitor "ON"
    } else if (soundSensorVal == 1) {
      Serial.println("No Sound Detected");  // print serial monitor "OFF"
    }
    if (Firebase.RTDB.setInt(&fbdo, soundSensorPath, soundSensorVal)) {
      Serial.println(soundSensorVal);
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
}
