/*********************************************************
* Event Loop
* 
* Handles all of the action logic in the sketch
* This code runs on a separate thread, just in case you 
* wanted to do animation or other interesting stuff in 
* the main `loop()`.
**********************************************************/

#include <WiFiClientSecure.h>
#include <HTTPClient.h>

WiFiClientSecure* client = new WiFiClientSecure;
HTTPClient https;

void EventLoopCode(void* pvParameters) {
  // Thread initialization
  Serial.print("EventLoop running on core ");
  Serial.println(xPortGetCoreID());
  // Now, this infinite loop waits for button press
  // and called the remote host
  for (;;) {
    // check to see if the button's been pressed
    if (btnPressed) {
      Serial.println("\nButton pressed");
      if (sendRequest) {
        cancelShutdownTimer();
        updateActionScreen(BLUE, WHITE, "Connecting");
        if (callRemoteAPI()) {
          updateActionScreen(GREEN, BLACK, "Success!");
          delay(DELAY_SUCCESS);
        } else {
          updateActionScreen(RED, BLACK, "Failure");
          delay(DELAY_FAILURE);
        }
        startShutdownTimer(DELAY_INACTIVITY);
        resetPushScreen();
        // Renable the button for another press (but only after the
        // previous one completed)
        btnPressed = !btnPressed;
      }
    }
    // https://randomerrors.dev/posts/2023/esp32-watchdog-got-triggered/
    delay(100);
  }
}

// https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/BasicHttpsClient/BasicHttpsClient.ino
bool callRemoteAPI() {

  const char* host = REMOTE_HOST;  // from `config.h`
  int responseCode;

  if (client) {
    // secure client with a CA certificate
    client->setCACert(caCert);

    // https://arduinogetstarted.com/tutorials/arduino-http-request
    Serial.print("Connecting to ");
    Serial.println(host);

    https.begin(*client, host);
    https.addHeader("content-type", "application/json");
    if (USE_PARAMS) {
      Serial.println("Building POST body");
      String httpRequestData = buildRequestBodyJSON();
      Serial.print("Body: ");
      Serial.println(httpRequestData);
      Serial.println("Executing POST request");
      responseCode = https.POST(httpRequestData);
    } else {
      Serial.println("Executing GET request");
      responseCode = https.GET();
    }
    Serial.print("HTTPS response: ");
    Serial.println(responseCode);
    if (responseCode > 0) {
      if (responseCode == HTTP_CODE_OK) {
        Serial.printf("Response code: %u\n", responseCode);
        Serial.printf("Content length: %u\n", https.getSize());
        String payload = https.getString();
        Serial.printf("Payload: %s\n", payload.c_str());
        https.end();
      } else {
        return false;
      }
    } else {
      https.end();
      Serial.println("\nConnection failed");
      Serial.print("Message: ");
      Serial.println(https.errorToString(responseCode));
      return false;
    }
  } else {
    Serial.println("Unable to create client");
    return false;
  }
  return true;
}

String buildRequestBodyJSON() {
  String returnStr = "{ ";
  for (int row = 0; row < PARAM_ROWS; row++) {
    returnStr += "\"";
    returnStr += paramsArray[row][0];
    returnStr += "\": \"";
    returnStr += paramsArray[row][1];
    returnStr += "\"";
    if (row < PARAM_ROWS - 1) returnStr += ", ";
  }
  returnStr += " }";
  return returnStr;
}
