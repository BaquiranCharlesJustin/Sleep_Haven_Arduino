#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Adafruit_NeoPixel.h>

#define WIFI_SSID "PLDTHome"
#define WIFI_PASSWORD "p@ssw0rd"

#define API_KEY "AIzaSyCFQAp81x9NPxrayvmV2ewqZDaurLTH2W4"
#define DATABASE_URL "https://sleep-haven-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "boopboop@gmail.com"
#define USER_PASSWORD "boopboop"

#define PIN        12
#define NUMPIXELS  64
#define lightPath "Users/ItV0lwUXJ8PudsfT627g2bERwcF3/Light"
#define humidifierPath "Users/ItV0lwUXJ8PudsfT627g2bERwcF3/DeviceStatus/Humidifier"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

const int relay = 5;

void setup()
{
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
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
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    fbdo.setBSSLBufferSize(4096, 1024);

    // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino
    Firebase.begin(&config, &auth);

    // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
    // https://github.com/mobizt/Firebase-ESP32#about-firebasedata-object
    // fbdo.keepAlive(5, 5, 1);

    config.timeout.networkReconnect = 10 * 1000;
    config.timeout.serverResponse = 10 * 1000;
    config.timeout.rtdbKeepAlive = 45 * 1000;
    config.timeout.rtdbStreamReconnect = 1 * 1000;
    /** Timeout options.
     * You can also set the TCP data sending retry with
     * config.tcp_data_sending_retry = 1;
     */
}

void loop()
{
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        if (Firebase.getInt(fbdo, lightPath))
        {
            if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
            {
                int lightValue = fbdo.intData();
                // Set color based on data value
                if (lightValue == 1)
                {
                    // Set color to red
                    setColor(228, 52, 20);
                    Serial.println("Color set to: Red");
                }
                else if (lightValue == 2)
                {
                    // Set color to orange
                    setColor(255, 87, 51);
                    Serial.println("Color set to: Orange");
                }
                else if (lightValue == 3)
                {
                    // Set color to yellow
                    setColor(255, 201, 34);
                    Serial.println("Color set to: Yellow");
                }
                else if (lightValue == 4)
                {
                    // Set color to white
                    setColor(255, 255, 255);
                    Serial.println("Color set to: White");
                }
                else
                {
                    Serial.println("Invalid data value!");
                }
            }
            else
            {
                Serial.println("FAILED: " + fbdo.errorReason());
            }
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        
        delay(1000);
        
        if (Firebase.getInt(fbdo, humidifierPath))
        {
            if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
            {
                int humidifierValue = fbdo.intData();
                // Set color based on data value
                if (humidifierValue == 0)
                {
                    digitalWrite(relay, LOW);
                    Serial.println("Humidifier is turned off.");
                }
                else if (humidifierValue == 1)
                {
                    Serial.println("Humidifier is turned on.");
                    digitalWrite(relay, HIGH);
                }
                else
                {
                    Serial.println("FAILED: " + fbdo.errorReason());
                    Serial.println("Invalid data value!");
                }
            }
        }
        count++;
    }
}

void setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    // Set the color of all NeoPixels
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, red, green, blue);  
    }
    // Update the NeoPixel strip to display the color
    pixels.show();
}
