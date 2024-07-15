//
// espHTTP v1.0.2
// 2023.12.20
//

/*--------       Libraries        --------*/

#include "espHTTPServerUtils.h"
#include <espWiFiUtils.h>


/*--------    WiFi Credentials    --------*/

#define WiFiHostname "espHTTP"

#ifndef STASSID
#define STASSID "Your_WiFi_SSID"
#define STAPSK  "Your_WiFi_Pass"
#endif


/*-------- User-Defined Variables --------*/

// Define BASICPAGE or TABBEDPAGE
#define BASICPAGE

// Webpage Hex Colors
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"


/*--------          GPIO          --------*/


/*--------   Program Variables    --------*/


/*--------     Main Functions     --------*/

void setup() {
  // Setup GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);

  // Connect to WiFi, start OTA
  connectWiFi(STASSID, STAPSK, WiFiHostname);
  initializeOTA(WiFiHostname, STAPSK);

  // Start HTML Server
  serverSetup();
}

void loop() {
  // Webserver
  server.handleClient();

  // Arduino OTA
  ArduinoOTA.handle();

  // Let the ESP8266 do its thing
  yield();
}


/*--------    Server Functions    --------*/

void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  redirect();
}


/*--------         Webpage        --------*/

void serverSetup() {
  server.on("/", handleRoot);
  server.on("/toggleLED", toggleLED);
  server.onNotFound(handleNotFound);
  server.begin();
}

String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"
                  "<form action=\"/toggleLED\" method=\"GET\"><input type=\"submit\" value=\"Turn LED %toggleStub%\" class=\"simpleButton\"></form>\n"
                "</div>\n"
              "</div>\n";

void handleRoot() {
  String deliveredHTML = assembleHTML(body);

  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "On"); }
                           else { deliveredHTML.replace("%toggleStub%", "Off"); }

  server.send(200, "text/html", deliveredHTML);
}
