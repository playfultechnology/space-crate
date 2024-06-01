/**
 * Space Crate Main Controller
 */

// https://iotespresso.com/create-captive-portal-using-esp32/
// INCLUDES
// For creating a local webpage
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// DEFINES
#define NUM_LEDS_PER_STRIP 5

// GLOBALS
DNSServer dnsServer;
AsyncWebServer server(80);
String user_name;
String proficiency;
bool name_received = false;
bool proficiency_received = false;
// LED strips
CRGB blLeds[NUM_LEDS_PER_STRIP];
CRGB flLeds[NUM_LEDS_PER_STRIP];
CRGB brLeds[NUM_LEDS_PER_STRIP];
CRGB frLeds[NUM_LEDS_PER_STRIP];
// Toggle switch input
int toggleSwitchValue = 0;
unsigned long lastChangeTime = 0;
// Tempo at which LEDs pulse
uint8_t bpm = 30;

// CONSTANTS
const byte clockPin = 26;
const byte latchPin = 25;
const byte dataPin = 33;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Captive Portal Demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Captive Portal Demo</h3>
  <br><br>
  <form action="/get">
    <br>
    Name: <input type="text" name="name">
    <br>
    ESP32 Proficiency: 
    <select name = "proficiency">
      <option value=Beginner>Beginner</option>
      <option value=Advanced>Advanced</option>
      <option value=Pro>Pro</option>
    </select>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}
    bool canHandle(AsyncWebServerRequest *request){
      //request->addInterestingHeader("ANY");
      return true;
    }
    void handleRequest(AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", index_html); 
    }
};

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html); 
      Serial.println("Client Connected");
  });
    
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      String inputParam;
  
      if (request->hasParam("name")) {
        inputMessage = request->getParam("name")->value();
        inputParam = "name";
        user_name = inputMessage;
        Serial.println(inputMessage);
        name_received = true;
      }

      if (request->hasParam("proficiency")) {
        inputMessage = request->getParam("proficiency")->value();
        inputParam = "proficiency";
        proficiency = inputMessage;
        Serial.println(inputMessage);
        proficiency_received = true;
      }
      request->send(200, "text/html", "The values entered by you have been successfully sent to the device <br><a href=\"/\">Return to Home Page</a>");
  });
}


void setup() {
  // Initialise serial connection
  Serial.begin(115200);
  delay(250);
  Serial.println(__FILE__ __DATE__);

  // Intialise secondary serial connection to Arduino controller
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  delay(250);
  // Flush the serial connection to try to prevent garbage characters being sent to Arduino
  Serial2.flush();

  // Configure GPIO pins
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);

  // For mirroring strips, all the "special" stuff happens just in setup.  We
  // just addLeds multiple times, once for each strip
  FastLED.addLeds<WS2812B, 13, GRB>(flLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 12, GRB>(blLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 27, GRB>(brLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 14, GRB>(frLeds, NUM_LEDS_PER_STRIP);

  // Configure web server
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("Space Crate");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  Serial.println("All Done!");
}

byte readToggleSwitches(){
  // Set the shift register to transmit data. CD4021 wants LOW here, HC165 wants HIGH
  digitalWrite(latchPin, HIGH);
  // Note that we DON'T use the hardware SPI interface SPI.transfer() for reading the inputs from the HC165 shift register
  // because it doesn't tristate the MISO line when it not selected (see http://www.gammon.com.au/forum/?id=11979)
  // So we'll use simple "bit bang" instead using shiftIn() instead
  byte incoming = shiftIn(dataPin, clockPin, MSBFIRST);
  // Set the shift register back to gathering input. CD4021 wants HIGH here, HC165 wants LOW
  digitalWrite(latchPin, LOW);
  // Toggle switches are wired as "pull-up", so we'll invert the result
  return 255-incoming;
}


void loop() {
  
  unsigned long now = millis();

  // Check toggle switches
  byte currentToggleSwitchValue = readToggleSwitches();
  if(currentToggleSwitchValue != toggleSwitchValue) {
    Serial.print("Toggle Switches changed to ");
    Serial.println(currentToggleSwitchValue);
    if(currentToggleSwitchValue == B11001001){
      Serial2.write("You solved the space puzzle crate!");
      Serial2.write("\n");
    }
    toggleSwitchValue = currentToggleSwitchValue;
  }
  
  // Process data received via webserver
  dnsServer.processNextRequest();
  if(name_received && proficiency_received){
      Serial.print("Hello ");
      Serial.println(user_name);
      Serial.print("You have stated your proficiency to be ");
      Serial.println(proficiency);
      name_received = false;
      proficiency_received = false;
      Serial.println("We'll wait for the next client now");
      for (int i = 0; i < user_name.length(); i++) {
       Serial2.write(user_name[i]);   // Push each char 1 by 1 on each loop pass
      }
      // Have to send a newline character to indicate the string has ended!
      Serial2.write("\n");
    }

  // If data received on serial connection, pass it through to Arduino
  while(Serial.available()){
    Serial2.write(Serial.read());
  }

  // A coloured dot sweeping back and forth, with fading trails
  fadeToBlackBy(blLeds, NUM_LEDS_PER_STRIP, 10);
  fadeToBlackBy(flLeds, NUM_LEDS_PER_STRIP, 10);
  fadeToBlackBy(brLeds, NUM_LEDS_PER_STRIP, 10);
  fadeToBlackBy(frLeds, NUM_LEDS_PER_STRIP, 10);
  int pos = beatsin8(bpm, 0, NUM_LEDS_PER_STRIP-1 );
  blLeds[pos] += CHSV(0, 255, 128);
  flLeds[pos] += CHSV(0, 255, 128);
  brLeds[pos] += CHSV(0, 255, 128);  
  frLeds[pos] += CHSV(0, 255, 128);
  FastLED.show();
}
