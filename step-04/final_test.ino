#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define FIREBASE_HOST "fir-8e392-default-rtdb.asia-southeast1.firebasedatabase.app" //remove the "https://" and "/" at the end
#define FIREBASE_AUTH "HhKLdMxjUdJ1sRVSLfek9XqS73kaa9ZpaGeJ3IYt"

// Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

ESP8266WebServer server(80); // Create a web server on port 80

const int LED_PIN = D4; // Pin connected to the LED
#define FACTORY_RESET_PIN D8
#define FACTORY_RESET_DURATION 5000 // 5 seconds in milliseconds

bool isFactoryResetTriggered = false;

unsigned long previousMillis = 0; // Stores the last time data was fetched
const long interval = 5000;       // Interval at which to fetch data (milliseconds)

String homeSSID = "";     // Variable to store home Wi-Fi SSID
String homePassword = ""; // Variable to store home Wi-Fi password

bool isConnectedToHomeWiFi = false;

void setup()
{
    Serial.begin(9600);

    // Check if D8 pin is connected to VCC (factory reset)
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

    // Initialize Firebase
        config.database_url = FIREBASE_HOST;
        config.signer.tokens.legacy_token = FIREBASE_AUTH;  // If using a legacy token
        Firebase.begin(&config, &auth); // Initialize Firebase with authentication token

    // Set LED pin as output
        pinMode(LED_PIN, OUTPUT);

    // Start the web server
        server.on("/", handleRoot);
        server.on("/update", handleUpdate);
        server.begin();

    // Set up the NodeMCU as an access point if not connected to home Wi-Fi
        if (!isConnectedToHomeWiFi) {
              WiFi.mode(WIFI_AP);
              WiFi.softAP("NodeMCU_AP", "Password123"); // Set the SSID and password for the access point
              Serial.println("Access Point Mode Enabled");
              Serial.println("IP address: " + WiFi.softAPIP().toString());
          }
      }

void loop()
      {
          server.handleClient(); // Handle web server requests

          // Check if factory reset is triggered
          if (!isFactoryResetTriggered && digitalRead(FACTORY_RESET_PIN) == LOW) {
              unsigned long startTime = millis();
              while (millis() - startTime <= FACTORY_RESET_DURATION) {
                  // Check if D8 pin remains connected to VCC for specified duration
                  if (digitalRead(FACTORY_RESET_PIN) == HIGH) {
                      isFactoryResetTriggered = true;
                      Serial.println("Factory reset triggered.");
                      break;
                  }
              }
          }

          // Perform factory reset if triggered
          if (isFactoryResetTriggered) {
              performFactoryReset();
          }

          // If connected to home Wi-Fi, fetch data from Firebase
          if (isConnectedToHomeWiFi) {
              fetchData();
          } else {
              handleWiFiReconnection(); // If not connected to home Wi-Fi, handle reconnection
          }
      }

void performFactoryReset()
{
    // Reset home Wi-Fi credentials to default values
    homeSSID = "NodeMCU_AP";
    homePassword = "Password123";
    isConnectedToHomeWiFi = false;
    Serial.println("Factory reset successful.");
    isFactoryResetTriggered = false; // Reset the factory reset trigger flag
}

void fetchData()
{
    // ("/FirebaseIOT/led") should have the same name as the collection in your firebase
    if (Firebase.get(firebaseData, "/FirebaseIOT/led"))
    {
        if (firebaseData.dataType() == "string")
        {
            String value = firebaseData.stringData();
            Serial.println("Received LED state from Firebase: " + value);

            // Toggle LED based on fetched value
            if (value == "1")
            {
                digitalWrite(LED_PIN, HIGH);
            }
            else if (value == "0")
            {
                digitalWrite(LED_PIN, LOW);
            }
            else
            {
                Serial.println("Invalid LED state value received from Firebase.");
            }
            delay(500);
        }
        else
        {
            Serial.println("Data fetched is not a string.");
        }
    }
    else
    {
        Serial.println("Failed to fetch data: " + firebaseData.errorReason());
    }
}

void handleWiFiReconnection()
{
    // If not connected to home Wi-Fi, attempt to connect
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Attempting to connect to home WiFi...");
        WiFi.begin(homeSSID.c_str(), homePassword.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            // If unable to connect within 10 seconds, continue with access point mode
            if (millis() - startAttemptTime > 10000)
            {
                Serial.println("Failed to connect to home WiFi within 10 seconds. Continuing in access point mode.");
                return;
            }
            delay(500);
        }

        isConnectedToHomeWiFi = true;
        Serial.println("Connected to home WiFi");
        Serial.println("IP address: " + WiFi.localIP().toString());
    }
}

void handleRoot()
{
    // HTML form to input home Wi-Fi SSID and password
    String html = "<form action='/update' method='POST'>"
                  "<label for='ssid'>SSID:</label><br>"
                  "<input type='text' id='ssid' name='ssid'><br>"
                  "<label for='password'>Password:</label><br>"
                  "<input type='password' id='password' name='password'><br><br>"
                  "<input type='submit' value='Submit'>"
                  "</form>";
    server.send(200, "text/html", html);
}

void handleUpdate()
{
    if (server.hasArg("ssid") && server.hasArg("password"))
    {
        // Update home Wi-Fi credentials
        homeSSID = server.arg("ssid");
        homePassword = server.arg("password");

        // Connect to the new Wi-Fi network
        WiFi.begin(homeSSID.c_str(), homePassword.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            // If unable to connect within 10 seconds, continue in access point mode
            if (millis() - startAttemptTime > 10000)
            {
                Serial.println("Failed to connect to home WiFi within 10 seconds. Continuing in access point mode.");
                return;
            }
            delay(500);
        }

        isConnectedToHomeWiFi = true;
        Serial.println("Connected to new WiFi");
        Serial.println("IP address: " + WiFi.localIP().toString());
    }
    server.send(200, "text/plain", "Wi-Fi credentials updated successfully.");
}
