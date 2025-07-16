#include <ESP8266WiFi.h>        //include wifi library
#include <ESP8266WebServer.h>   //include Web server library 

const char* ssid = "your wifi ssid";     
const char* password = "your wifi password";
byte LEDStatus=1;  
int LED1 = 2;      // Assign LED1 to pin GPIO2

ESP8266WebServer server(80);

//handles HTTP requests to the root URL ("/") 
void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from NodeMCU!</h1><button onclick=\"toggleLED()\">Toggle LED
  </button><script>function toggleLED() { fetch('/toggle'); }</script>");
}

void handleToggle() {     //function to toggle an LED or perform any action when the button is clicked

  server.send(200, "text/plain", "LED Toggled");
  if (LEDStatus==1){
      digitalWrite(LED1,LOW);

    LEDStatus=0;
  } else {LEDStatus=1;
        digitalWrite(LED1,HIGH);
}
  Serial.println(LEDStatus);
  
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);    // initialize GPIO2

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  // Start the server
  server.begin();
}

void loop() {   //runs repeatedly
  server.handleClient();
}
