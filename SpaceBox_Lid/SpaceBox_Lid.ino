/**
 * Space Create Lid controller
 */

// DEFINES
// Maximum length of the input string received over serial connection
#define MAX_STRING_LENGTH 50 
#define LED_INTENSITY 8
// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES	4
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS
// We always wait a bit between updates of the display
#define  DELAYTIME  50  // in milliseconds

// INCLUDES
#include <MD_MAX72xx.h>

// GLOBALS
// Array to store the input string
char inputString[MAX_STRING_LENGTH]; 
// Flag to indicate whether the string is complete
boolean stringComplete = false; 
// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
unsigned long lastInputTime;

void scrollText(const char *p) {
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  Serial.println("\nScrolling text");
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, LED_INTENSITY);
  while (*p != '\0') {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
    // Allow space between characters
    for (uint8_t i=0; i<=charWidth; i++)	{
      mx.transform(MD_MAX72XX::TSL);
      mx.setColumn(0, (i < charWidth) ? cBuf[i] : 0); 
      delay(DELAYTIME);
    }
  }

  // Hold the last frame
  delay(500);
  // Fade
  for (int8_t i=LED_INTENSITY; i>0; i--){
    mx.control(MD_MAX72XX::INTENSITY, i);
    delay(DELAYTIME*3);
  }
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, LED_INTENSITY);
}

void bullseye() {
  Serial.println("Bullseye");
  mx.clear();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  byte  b = 0x00;
  int   i = 4;
  // Expand
  while (b != 0xff) {
    mx.control(MD_MAX72XX::INTENSITY, 4-i);    
    for (uint8_t j=0; j<MAX_DEVICES+1; j++) {
      mx.setRow(j, i, b);
      mx.setColumn(j, i, b);
      mx.setRow(j, ROW_SIZE-1-i, b);
      mx.setColumn(j, COL_SIZE-1-i, b);
    }
    mx.update();
    delay(3*DELAYTIME);
    for (uint8_t j=0; j<MAX_DEVICES+1; j++) {
      mx.setRow(j, i, 0);
      mx.setColumn(j, i, 0);
      mx.setRow(j, ROW_SIZE-1-i, 0);
      mx.setColumn(j, COL_SIZE-1-i, 0);
    }

    i--;
    bitSet(b, i);
    bitSet(b, 7-i);
  }  
  // Contract
  while (b != 0x00) {
    mx.control(MD_MAX72XX::INTENSITY, 4-i);  
    // Repeat across all displays
    for (uint8_t j=0; j<MAX_DEVICES+1; j++) {
      mx.setRow(j, i, b);
      mx.setColumn(j, i, b);
      mx.setRow(j, ROW_SIZE-1-i, b);
      mx.setColumn(j, COL_SIZE-1-i, b);
    }
    mx.update();
    delay(3*DELAYTIME);
    for (uint8_t j=0; j<MAX_DEVICES+1; j++) {
      mx.setRow(j, i, 0);
      mx.setColumn(j, i, 0);
      mx.setRow(j, ROW_SIZE-1-i, 0);
      mx.setColumn(j, COL_SIZE-1-i, 0);
    }

    bitClear(b, i);
    bitClear(b, 7-i);
    i++;
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup() {

  // Initialise the serial interface
  Serial.begin(9600);
  Serial.println(__FILE__ __DATE__);

  // Clear the input string array
  memset(inputString, 0, sizeof(inputString)); 

  //Initialise connection to MAX7219
 if(!mx.begin()) {
  Serial.println("MAX72XX initialization failed");
 }
 mx.control(MD_MAX72XX::INTENSITY, LED_INTENSITY);
}

void loop() {

  unsigned long now = millis();

  while (Serial.available()) {
    char inChar = (char)Serial.read(); // Read the incoming byte
    Serial.print(inChar);

    if (inChar == '\n') { // Check if newline character is received
      inputString[strcspn(inputString, "\r\n")] = '\0'; // Terminate the string
      stringComplete = true; // Set flag to indicate string is complete
      break; // Exit the loop
    }
    else {
      // Append the character to the input string if it's not a newline
      strncat(inputString, &inChar, 1);
      
      // Check if the input string exceeds maximum length
      if (strlen(inputString) >= MAX_STRING_LENGTH - 1) {
        inputString[MAX_STRING_LENGTH - 1] = '\0'; // Terminate the string
        stringComplete = true; // Set flag to indicate string is complete
        break; // Exit the loop
      }
    }
  }
  
  // If a complete string is received, do something with it
  if (stringComplete) {
    Serial.println("Received: " + String(inputString)); // Print the received string
    // Do something with the received string here
    scrollText(inputString);
    // Clear the input string array for the next input
    memset(inputString, 0, sizeof(inputString));
    stringComplete = false; // Reset the flag
    lastInputTime = now;
  }

  if(now - lastInputTime > 3000) {
    // If no serial data available
    bullseye();
    lastInputTime = now;
  }
}