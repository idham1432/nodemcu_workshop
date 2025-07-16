#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "your wifi ssid";
const char* pass = "Your wifi password";
// Fixed IP address for everyone to use
const char* fixedIP = "192.168.1.100";
byte LEDStatus = 0;
// Assign LED1 to pin GPIO2
int LED1 = 2;

ESP8266WebServer server(80);

void handleRoot() {
  String htmlContent = "<h1>Hello from NodeMCU!</h1>";
  htmlContent += "<button onclick=\"toggleLED()\">Toggle LED</button>";
  htmlContent += "<script>function toggleLED(){ fetch('/toggle').then(response => response.text()).then(text => {document.getElementById('ledState').innerText = text;}); }</script>";
  htmlContent += "<p>LED State: <span id='ledState'>";
  htmlContent += (LEDStatus == 1) ? "ON" : "OFF";
  htmlContent += "</span></p>";
  
  server.send(200, "text/html", htmlContent);
}

void handleToggle() {
  // Your code to toggle an LED or perform any action when the button is clicked
  if (LEDStatus == 1) {
    digitalWrite(LED1, HIGH);
    LEDStatus = 0;
    server.send(200, "text/plain", "OFF");
  } else {
    digitalWrite(LED1, LOW);
    LEDStatus = 1;
    server.send(200, "text/plain", "ON");
  }
  Serial.println(LEDStatus);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  Serial.println("Start up...");

  // Set up NodeMCU as an access point with fixed IP
  IPAddress IP(192, 168, 1, 100);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(IP, gateway, subnet);
  WiFi.softAP(ssid, pass);
  Serial.print("Access Point SSID: ");
  Serial.println(ssid);
  Serial.print("Access Point Password: ");
  Serial.println(pass);
  Serial.print("Access Point IP address: ");
  Serial.println(IP);

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  server.begin();
}

void loop() {
  server.handleClient();
}