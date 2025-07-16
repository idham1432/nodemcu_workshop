#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "Firebase URL"            //remove the "https://" and "/"at the end
#define FIREBASE_AUTH "Firebase authentication"
#define WIFI_SSID "your wifi ssid"
#define WIFI_PASSWORD "your wifi password"

// Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

const int LED_PIN = D4; // Pin connected to the LED

unsigned long previousMillis = 0;  // Stores the last time data was fetched
const long interval = 5000;        // Interval at which to fetch data (milliseconds)

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println("IP address: " + WiFi.localIP().toString());


  // Initialize Firebase
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;  // If using a legacy token
  Firebase.begin(&config, &auth);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Implement a non-blocking delay to call fetchData every 5 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Check WiFi connection before attempting to fetch data
    if (WiFi.status() == WL_CONNECTED) {
      fetchData();
    } else {
      Serial.println("WiFi not connected, attempting to reconnect...");
      WiFi.reconnect();
    }
  }
}

void fetchData() {
  //  ("/FirebaseIOT/led") should have the same name as the collection in your firebase
  if (Firebase.get(firebaseData, "/FirebaseIOT/led")) {
    if (firebaseData.dataType() == "string") {
      String value = firebaseData.stringData();
      Serial.println("Received LED state from Firebase: " + value);
      
      // Toggle LED based on fetched value
      if (value == "1") {
        digitalWrite(LED_PIN, HIGH);
      } else if (value == "0") {
        digitalWrite(LED_PIN, LOW);
      } else {
        Serial.println("Invalid LED state value received from Firebase.");
      }
    } else {
      Serial.println("Data fetched is not a string.");
    }
  } else {
    Serial.println("Failed to fetch data: " + firebaseData.errorReason());
  }
  delay (500);
}
