/**********************************************************
* M5StickC Plus Trigger Cloud Function
* 
* Created January 10, 2025
* By John M. Wargo
* https://johnwargo.com
***********************************************************/

#include <WiFi.h>
#include <M5Unified.h>
#include <M5GFX.h>

#include "cert.h"
#include "config.h"

#define firstTextLine 50
#define secondTextLine 80

const String errorStr = "Error: ";
const char* ssid = WIFI_SSID;      // from `config.h`
const char* password = WIFI_PSWD;  // from `config.h`

TaskHandle_t EventLoop;

/************************************************************************
* Set the following to false to disable calling the remoate endpoint
* You would really only disable it if testing shutdown or other aspects
* of the code
*************************************************************************/
bool sendRequest = true;
// flag used to control activity in the `eventloop` thread
bool btnPressed;
// used to control/manage the shutdown timer
int shutdownDelay;                 // how long before we shutdown (in milliseconds)
unsigned long shutdownDelayStart;  // when the stutdown timer started (in milliseconds)

void setup() {
  int counter = 0;  // used to show activity while connecting to Wi-Fi

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");

  M5.begin();
  M5.Display.setTextSize(2);

  // Configuration Validation
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  centerText(firstTextLine, "Validate");
  centerText(secondTextLine, "Config");
  if (!isConfigValid()) {
    M5.Display.fillScreen(RED);
    M5.Display.setTextColor(BLACK);
    centerText(firstTextLine, "Config");
    centerText(secondTextLine, "Invalid");
    Serial.println("Configuration is invalid!");
    while (true) delay(100);  //loops forever
  }

  // Wi-Fi connection
  Serial.print("Connecting to ");
  Serial.println(ssid);

  M5.Display.fillScreen(BLUE);
  centerText(firstTextLine, "Wi-Fi");
  centerText(secondTextLine, "Connect");
  M5.Display.setCursor(0, 110);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    M5.Display.print(".");
    counter += 1;
    if (counter > 10) {
      counter = 0;
      Serial.println();
      M5.Display.println();
    }
  }
  Serial.println("\nWiFi connected");
  printWifiStatus();

  Serial.print("Setup & Loop running on core ");
  Serial.println(xPortGetCoreID());

  // Initialize the button state flag; before kicking off the threads
  // because the event loop thread uses it
  btnPressed = false;

  // Reference: https://johnwargo.com/posts/2023/arduino-running-tasks-on-multiple-cores/
  // Create a task that executes the EventLoopCode() function, with priority 1, executing on core 1
  // With this running on a second core, the sketch can do animation or other cool stuff
  // while the sketch connects to the remote host
  xTaskCreatePinnedToCore(EventLoopCode, "EventLoop", 10000, NULL, 1, &EventLoop, 1);
  delay(500);  // wait half a second for the thread to start executing

  resetPushScreen();                     // load the Push Button screen
  startShutdownTimer(DELAY_INACTIVITY);  // start the inactivity shutdown timer
}

void loop() {
  // Only check the button if it's not been pressed recently
  if (!btnPressed) {
    M5.update();
    if (M5.BtnA.wasReleased()) {
      Serial.println("Button pressed");
      cancelShutdownTimer();
      // When true, the event loop thread picks it up and runs with it,
      // calling the configured URL
      btnPressed = true;
    }
  }

  // do we have an active shutdown timer?
  if (shutdownDelay > 0) {
    // then check the timer
    if ((millis() - shutdownDelayStart) > shutdownDelay) {
      M5.Display.clear();  // clear the screen
      centerText(firstTextLine, "Shutting");
      centerText(secondTextLine, "Down");
      delay(1000);
      M5.Power.powerOff();  // turn off the device
    }
  }
}

/******************************************* 
 * Put stuff on the display
 ********************************************/

void centerText(int y, String text) {
  M5.Display.drawCentreString(text, M5.Lcd.width() / 2, y, 2);
}

void resetPushScreen() {
  // Update the display for primary activity
  M5.Display.clear();
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(WHITE);
  centerText(60, "Push");
  centerText(90, "Button");
  centerText(130, "|");
  centerText(160, "|");
  centerText(190, "\\|/");
}

void updateActionScreen(int color, int textColor, String msg) {
  M5.Display.clear();
  M5.Display.fillScreen(color);
  M5.Display.setTextColor(textColor);
  centerText(50, msg);
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();  // received signal strength:
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/******************************************* 
 * Shutdown timer
 ********************************************/

void cancelShutdownTimer() {
  Serial.println("Canceling shutdown timer");
  shutdownDelay = 0;
}

void startShutdownTimer(int duration) {
  Serial.printf("Setting shutdown timer: %s milliseconds\n", String(duration));
  shutdownDelay = duration;
  shutdownDelayStart = millis();
}

/******************************************* 
 * Configuration validation
 ********************************************/

bool isConfigPropertyValid(String prop, String name) {
  prop.trim();
  if (prop.length() < 1) {
    Serial.println(errorStr + name + " property is empty");
    return false;
  }
  return true;
}

bool isConfigValueValid(int value, String name) {
  if (value < 1) {
    Serial.println(errorStr + name + " value is < 0");
    return false;
  }
  return true;
}

bool isConfigValid() {
  bool result;
  Serial.println("\nValidating Configuration");
  result = isConfigPropertyValid(REMOTE_HOST, "Remote Host");
  result = result && isConfigPropertyValid(WIFI_SSID, "Wi-Fi SSID");
  result = result && isConfigPropertyValid(WIFI_PSWD, "Wi-Fi Password");
  result = result && isConfigValueValid(DELAY_FAILURE, "Failure Delay");
  result = result && isConfigValueValid(DELAY_SUCCESS, "Success Delay");
  result = result && isConfigValueValid(DELAY_INACTIVITY, "Shutdown Delay");
  result = result && isConfigValueValid(PARAM_ROWS, "Parameter Rows");
  return result;
}
