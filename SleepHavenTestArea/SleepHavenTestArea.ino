#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "time.h"

// #define WIFI_SSID "PLDTHome"
// #define WIFI_PASSWORD "p@ssw0rd"
#define WIFI_SSID "Fibr_Baquiran"
#define WIFI_PASSWORD "wifin0og!Ecyj"
#define API_KEY "AIzaSyCFQAp81x9NPxrayvmV2ewqZDaurLTH2W4"
#define DATABASE_URL "https://sleep-haven-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
MAX30105 particleSensor;

const unsigned long timeUpdateInterval = 5000;  // Update time every 60 seconds
unsigned long lastTimeUpdateMillis = 0;

const char* ntpServer = "asia.pool.ntp.org";
const long gmtOffset_sec = 28800;
const int daylightOffset_sec = 0;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;         // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

String currentTime;

void setup() {
  Serial.begin(115200);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {  // Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();                               // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);            // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);             // Turn off Green LED
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
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s,\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true) {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;  // Store this reading in the array
      rateSpot %= RATE_SIZE;                    // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);

    if (irValue < 50000) {
      Serial.print(" No finger?");
    }
    if (WiFi.status() == WL_CONNECTED) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastTimeUpdateMillis >= timeUpdateInterval) {
        lastTimeUpdateMillis = currentMillis;

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          char timeStringBuff[50];  // 50 chars should be enough
          strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
          String asString(timeStringBuff);
          asString.replace(" ", "-");

          currentTime = asString;
        }
      }
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        float bpmFB = beatsPerMinute;
        String timeBPM = String(bpmFB) + " --- " + currentTime;
        if (Firebase.RTDB.pushString(&fbdo, "MAX30102/BPM", timeBPM)) {
          Serial.println();
          Serial.print(beatsPerMinute);
          Serial.print(" - successfully saved to: " + fbdo.dataPath());
          Serial.println("(" + fbdo.dataType() + ")");
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
      }
    }
  }
}
