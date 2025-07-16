#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h> //<- header to use persistent memory

const char* ssid = "wifi ssid";
const char* password = "wifi password";
const char* fixedIP = "192.168.1.100"; // Fixed IP address for everyone to use

byte LEDStatus = 0;
int LED1 = 2;      // Assign LED1 to pin GPIO2
int RESET_PIN = D8; //D8 is the reset pin
const unsigned long RESET_TIME = 5000; //5 secs to reset

ESP8266WebServer server(80);

void handleRoot() {
  String htmlContent = "<h1>Hello from NodeMCU!</h1>";
  htmlContent += "<button onclick=\"toggleLED()\">Toggle LED</button>";
  htmlContent += "<button onclick=\"changePassword()\">Change Password</button>";
  htmlContent += "<script>function toggleLED(){ fetch('/toggle').then(response => response.text())
  .then(text => {document.getElementById('ledState').innerText = text;}); }</script>";
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

void handleChangePassword() {
  String newPass = server.arg("newpassword");
  if (newPass.length() >= 8) {
    Serial.print("New password: ");
    Serial.println(newPass);
    //save new password to EEPROM
    for (int i = 0; i < newPass.length(); i++){
      EEPROM.write(i, newPass[i]); 
    }
    EEPROM.commit();//to save
    //Return
    server.send(200, "text/plain", "Password changed successfully!");
    delay(1000); // Wait for the message to be sent
    ESP.reset(); // Reset the ESP8266 to apply changes
  } else {
    server.send(400, "text/plain", "Invalid password!");
  }
}

void resetPassword(){
  for (int i = 0; i < 64; i++){
      EEPROM.write(i, 0xFF); 
  }  
  EEPROM.commit();
  Serial.println("Password reset completed! Rebooting...");
  for(int i = 0; i < 4; i++){
    digitalWrite(LED1, HIGH);
    delay(100);
    digitalWrite(LED1, LOW);
    delay(100);
  }
  delay(1000);
  ESP.reset();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT); // initialize GPIO2
  pinMode(D0, INPUT);
  EEPROM.begin(512); //initialize persistent memory take 512 byte
  delay(10); //add delay for stability

  Serial.println("Start up...");

  //Load password from EEPROM
  String pass;
  bool check = true;
  for(int i = 0; i < 64; i++){
    char c = char(EEPROM.read(i));
    check &= (c == (char)0xFF);
    if (c == 0xFF) c = (char)0x0;
    pass += c;
  }
  if (check) pass = password;

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
  server.on("/changepassword", handleChangePassword);

  // Start the server
  server.begin();
}

int cur_pin = 0;
unsigned long start_ms = 0;
unsigned long cur_ms = 0;

void loop() {
  server.handleClient();
  cur_pin = digitalRead(RESET_PIN);//d8
  if (cur_pin == LOW){
    if (start_ms > 0){
      start_ms = 0;
      Serial.print("Reset counter cleared!");
    }
  }else{
    cur_ms = millis();
    if (start_ms == 0){
      start_ms = cur_ms;
    }else{
      int diff = (int)(cur_ms - start_ms);
      Serial.print("Reset counter: ");
      Serial.println(diff);
      if (diff >= RESET_TIME){
        resetPassword();
      }
    }
  } 
}
