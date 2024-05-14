/// @file    MultiArrays.ino
/// @brief   Demonstrates how to use multiple LED strips, each with their own data
/// @example MultiArrays.ino

// MultiArrays - see https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples for more info on
// using multiple controllers.  In this example, we're going to set up three NEOPIXEL strips on three
// different pins, each strip getting its own CRGB array to be played with

// https://iotespresso.com/create-captive-portal-using-esp32/
// INCLUDES
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

CRGB blLeds[NUM_LEDS_PER_STRIP];
CRGB flLeds[NUM_LEDS_PER_STRIP];
CRGB brLeds[NUM_LEDS_PER_STRIP];
CRGB frLeds[NUM_LEDS_PER_STRIP];

// CONSTANTS
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


// For mirroring strips, all the "special" stuff happens just in setup.  We
// just addLeds multiple times, once for each strip
void setup() {
    // tell FastLED there's 60 NEOPIXEL leds on pin 11
  FastLED.addLeds<WS2812B, 13, GRB>(flLeds, NUM_LEDS_PER_STRIP);
  // tell FastLED there's 60 NEOPIXEL leds on pin 10
  FastLED.addLeds<WS2812B, 12, GRB>(blLeds, NUM_LEDS_PER_STRIP);
  // tell FastLED there's 60 NEOPIXEL leds on pin 12
  FastLED.addLeds<WS2812B, 27, GRB>(brLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 14, GRB>(frLeds, NUM_LEDS_PER_STRIP);


  Serial.begin(115200);
  Serial2.begin(9600);

Serial.println();
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("esp-captive");
  Serial.print("AP IP address: ");Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  Serial.println("All Done!");


}

void loop() {


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

  while(Serial.available()){
    Serial2.write(Serial.read());
  }


  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    // set our current dot to red, green, and blue
    blLeds[i] = CRGB::Blue;
    flLeds[i] = CRGB::Blue;
    brLeds[i] = CRGB::Blue;
    frLeds[i] = CRGB::Blue;
    FastLED.show();
    // clear our current dot before we move on
    blLeds[i] = CRGB::Black;
    flLeds[i] = CRGB::Black;
    brLeds[i] = CRGB::Black;
    frLeds[i] = CRGB::Black;
    delay(100);
  }

  for(int i = NUM_LEDS_PER_STRIP-1; i >= 0; i--) {
    // set our current dot to red, green, and blue
    blLeds[i] = CRGB::Red;
    flLeds[i] = CRGB::Red;
    brLeds[i] = CRGB::Red;
    frLeds[i] = CRGB::Red;
    FastLED.show();
    // clear our current dot before we move on
    blLeds[i] = CRGB::Black;
    flLeds[i] = CRGB::Black;
    brLeds[i] = CRGB::Black;
    frLeds[i] = CRGB::Black;
    delay(100);
  }
}
