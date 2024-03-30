#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Adafruit_NeoPixel.h>

#define WIFI_SSID "Fibr_Baquiran"
#define WIFI_PASSWORD "wifin0og!Ecyj"
#define API_KEY "AIzaSyCFQAp81x9NPxrayvmV2ewqZDaurLTH2W4"
#define DATABASE_URL "https://sleep-haven-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define PIN        12
#define NUMPIXELS  64

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
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
  
  // Initialize the NeoPixel strip
  pixels.begin();
  
  // Set brightness to around 60% (approximately)
  pixels.setBrightness(153); // 60% of 255
  
  // Update the NeoPixel strip to display the color
  pixels.show();
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    if (Firebase.RTDB.getInt(&fbdo, "/MAX30102/Light")) {
      if(fbdo.dataType() == "int"){
        int data = fbdo.intData();
        Serial.println("Data successfully retrieved from Firebase!" + fbdo.dataPath() + ": " + data + " (" + fbdo.dataType() + ")");
        // Set color based on data value
        if (data == 1) {
        // Set color to red
        setColor(228, 52, 20);
        Serial.println("Color set to: Red");
        } else if (data == 2) {
        // Set color to orange
        setColor(255, 87, 51);
        Serial.println("Color set to: Orange");
        } else if (data == 3) {
        // Set color to yellow
        setColor(255, 201, 34);
        Serial.println("Color set to: Yellow");
        } else if (data == 4) {
        // Set color to white
        setColor(255, 255, 255);
        Serial.println("Color set to: White");
        } else {
        Serial.println("Invalid data value!");
        }
      } 
      else {
      Serial.println("FAILED: "+ fbdo.errorReason());
    }
    sendDataPrevMillis = millis();

     }
  }
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  // Set the color of all NeoPixels
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, red, green, blue);  
  }
  // Update the NeoPixel strip to display the color
  pixels.show();
}
