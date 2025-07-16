int LED1 = 2;      // Assign LED1 to pin GPIO2

void setup() {
  pinMode(LED1, OUTPUT);// initialize GPIO2
}

void loop() {
  digitalWrite(LED1, HIGH);     // turn the LED off
  delay(1000);                // wait for a second
  digitalWrite(LED1, LOW);  // turn the LED on
  delay(1000); 

}
