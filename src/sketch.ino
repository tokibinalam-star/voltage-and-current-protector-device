#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80); // Creates an HTTP server on standard port 80

// ESP32 Pin Mapping
const int VOLT_SENSOR = 34; // ESP32 ADC pin for Voltage
const int CURR_SENSOR = 35; // ESP32 ADC pin for Current
const int RELAY_PIN = 4;    // ESP32 GPIO for Relay
const int SAFE_LED = 18;    // ESP32 GPIO for Green LED
const int FAULT_LED = 19;   // ESP32 GPIO for Red LED

// Default protection thresholds
float overVoltageTrip = 250.0;
float underVoltageTrip = 180.0;
float overCurrentTrip = 15.0; 

bool systemFault = false;

// --- API Endpoint Handler ---
void handleSetLimits() {
  if (server.hasArg("vmax")) {
    overVoltageTrip = server.arg("vmax").toFloat();
  }
  if (server.hasArg("vmin")) {
    underVoltageTrip = server.arg("vmin").toFloat();
  }
  if (server.hasArg("imax")) {
    overCurrentTrip = server.arg("imax").toFloat();
  }
  if (server.hasArg("reset")) {
    systemFault = false; // Sending ?reset=true clears the fault
  }
  
  // JSON response
  String response = "{\n";
  response += "  \"status\": \"success\",\n";
  response += "  \"vmax\": " + String(overVoltageTrip) + ",\n";
  response += "  \"vmin\": " + String(underVoltageTrip) + ",\n";
  response += "  \"imax\": " + String(overCurrentTrip) + "\n";
  response += "}";
  
  server.send(200, "application/json", response);
  Serial.println("--> SUCCESS: Settings updated via API!");
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SAFE_LED, OUTPUT);
  pinMode(FAULT_LED, OUTPUT);
  
  // Connect appliance load on startup
  digitalWrite(RELAY_PIN, HIGH);
  
  lcd.init();
  lcd.backlight();
  
  Serial.println("--- Voltage Protector Dual-Mode (ESP32) ---");
  Serial.print("Connecting to Wokwi-GUEST WiFi...");
  
  // Connect to Wokwi virtual Wi-Fi
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("Device IP Address: ");
  Serial.println(WiFi.localIP());

  // Bind REST API routes
  server.on("/api/set", HTTP_GET, handleSetLimits);
  server.begin();
  Serial.println("API Server is listening on Port 80.");
  Serial.println("Serial Commands: SET_VMAX <val>, SET_VMIN <val>, SET_IMAX <val>, RESET");
}

void loop() {
  // 1. Process HTTP Web Server API Requests (Hardware/IoT Mode)
  server.handleClient();

  // 2. Process USB-Serial CLI Commands (Wokwi Terminal / Local Mode)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("SET_VMAX ")) {
      overVoltageTrip = command.substring(9).toFloat();
      Serial.print("--> SUCCESS: Over-voltage trip updated to: ");
      Serial.print(overVoltageTrip, 0); Serial.println("V");
    } else if (command.startsWith("SET_VMIN ")) {
      underVoltageTrip = command.substring(9).toFloat();
      Serial.print("--> SUCCESS: Under-voltage trip updated to: ");
      Serial.print(underVoltageTrip, 0); Serial.println("V");
    } else if (command.startsWith("SET_IMAX ")) {
      overCurrentTrip = command.substring(9).toFloat();
      Serial.print("--> SUCCESS: Over-current trip updated to: ");
      Serial.print(overCurrentTrip, 1); Serial.println("A");
    } else if (command == "RESET") {
      systemFault = false;
      Serial.println("--> SUCCESS: System faults cleared.");
    }
  }

  // 3. Read Analog Inputs (ESP32 12-bit ADC: 0-4095)
  int rawV = analogRead(VOLT_SENSOR);
  float gridVoltage = (rawV / 4095.0) * 300.0; 
  
  int rawI = analogRead(CURR_SENSOR);
  float loadCurrent = (rawI / 4095.0) * 30.0;  

  // 4. Fault Evaluation
  if (gridVoltage > overVoltageTrip || gridVoltage < underVoltageTrip || loadCurrent > overCurrentTrip) {
    systemFault = true;
  } else {
    systemFault = false;
  }

  // 5. Hardware Control (Relay & Status LEDs)
  if (systemFault) {
    digitalWrite(RELAY_PIN, LOW);  // Disconnect appliance
    digitalWrite(FAULT_LED, HIGH); // Red LED ON
    digitalWrite(SAFE_LED, LOW);   // Green LED OFF
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Appliance connected
    digitalWrite(FAULT_LED, LOW);  // Red LED OFF
    digitalWrite(SAFE_LED, HIGH);  // Green LED ON
  }

  // 6. Update LCD Interface
  lcd.setCursor(0, 0);
  lcd.print("V:" + String(gridVoltage, 0) + "V  I:" + String(loadCurrent, 1) + "A   ");
  lcd.setCursor(0, 1);
  lcd.print(systemFault ? "STATUS: TRIPPED " : "STATUS: SAFE    ");

  delay(10);
}

