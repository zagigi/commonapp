#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <WiFiManager.h>
#include "ThingSpeak.h" 

// -------- PIN DEFINITIONS --------
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 34

// -------- YOUR THINGSPEAK CREDENTIALS --------
unsigned long myChannelNumber = 3303651; 
const char * myWriteAPIKey = "7Z24Y3OO9M27HQ8F";
WiFiClient client;

// -------- OBJECTS --------
WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- VARIABLES --------
float temperature = 0;
float humidity = 0;
int mqRaw = 0;
int aqi = 0;

unsigned long previousMillis = 0;
unsigned long lastThingSpeakMillis = 0; 
const long lcdInterval = 5000;         // LCD alternates every 5s
const long tsInterval = 20000;          // ThingSpeak updates every 20s
bool showIP = false;

// -------- WEBPAGE --------
void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='10'/>";
  html += "<style>body{font-family:sans-serif; text-align:center; padding-top:50px; background:#f4f4f4;}";
  html += ".card{display:inline-block; background:white; border:1px solid #ccc; padding:20px; border-radius:10px; box-shadow: 2px 2px 10px #eee;}</style>";
  html += "<title>Air Quality Monitor</title></head><body>";
  html += "<div class='card'><h2>ESP32 Air Quality Monitor</h2>";
  html += "<p><b>Temperature:</b> " + String(temperature, 1) + " C</p>";
  html += "<p><b>Humidity:</b> " + String(humidity, 0) + " %</p>";
  html += "<p><b>AQI:</b> " + String(aqi) + "</p>";
  html += "<p><small>Logging to ThingSpeak every 20s</small></p></div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("System Start...");

  WiFiManager wm;
  lcd.setCursor(0,1);
  lcd.print("WiFi Config...");
  
  // Connects to stored WiFi or starts "AirMonitor_Setup" AP
  if(!wm.autoConnect("AirMonitor_Setup")) {
      Serial.println("Failed to connect");
      ESP.restart();
  }

  lcd.clear();
  lcd.print("WiFi Connected!");
  
  dht.begin();
  ThingSpeak.begin(client); 
  
  server.on("/", handleRoot);
  server.begin();
  
  Serial.println("Server online.");
  delay(1500);
}

// -------- LOOP --------
void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  // Read Sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  mqRaw = analogRead(MQ135_PIN);
  aqi = map(mqRaw, 0, 4095, 0, 300);

  // LCD Logic
  if (currentMillis - previousMillis >= lcdInterval) {
    previousMillis = currentMillis;
    showIP = !showIP;
    lcd.clear();
    
    if (showIP) {
      lcd.setCursor(0,0);
      lcd.print("Web Server IP:");
      lcd.setCursor(0,1);
      lcd.print(WiFi.localIP());
    } 
    else {
      lcd.setCursor(0,0);
      lcd.print("T:"); lcd.print(temperature, 1);
      lcd.print("C H:"); lcd.print(humidity, 0); lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("AQI: "); lcd.print(aqi);
      
      if(aqi < 100) lcd.print(" GOOD");
      else if(aqi < 200) lcd.print(" FAIR");
      else lcd.print(" POOR");
    }
  }

  // ThingSpeak Logic
  if (currentMillis - lastThingSpeakMillis >= tsInterval) {
    lastThingSpeakMillis = currentMillis;

    // Mapping to your ThingSpeak Fields
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, aqi);

    int status = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(status == 200){
      Serial.println("Data sent to ThingSpeak successfully.");
    }
    else {
      Serial.print("Error sending to ThingSpeak. Code: ");
      Serial.println(status);
    }
  }
}