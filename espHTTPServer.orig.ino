//
//
//  ESP HTTP Server - Version 1.0.4
//    This version was not deployed [2024.01.10]
//
//  ESP8266/32 Based
//    HTTP Web Server
//    Basic and tabbed sites
//    Customizable colors and styles
//
//  Changes From Previous Version
//    Comments, cleanup
//
//  To Do
//    Convert to true library (see espIRRemote)
//    Put body in flash
//    Remove TABBEDPAGE and merge css
//
//

/*--------       Libraries        --------*/

#include "espHTTPServerUtils.h"
#include <espWiFiUtils.h>


/*--------    WiFi Credentials    --------*/

#define WiFiHostname "espHTTPServer"

#ifndef STASSID
#define STASSID "Your_WiFi_SSID"
#define STAPSK  "Your_WiFi_Pass"
#endif


/*-------- User-Defined Variables --------*/

// Define BASICPAGE or TABBEDPAGE
#define BASICPAGE

// Define REFRESHPAGE or not
//#define REFRESHPAGE

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

#ifdef BASICPAGE
String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"
                  "<form action=\"/toggleLED\" method=\"GET\"><input type=\"submit\" value=\"Turn LED %toggleStub%\" class=\"simpleButton\" style=\"width: 100%;\"></form>\n"
                "</div>\n"
              "</div>\n";
#endif

#ifdef TABBEDPAGE
String body = "<div class=\"tabs\">\n"
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-0\" name=\"tab-group-1\" checked>\n"
                  "<label for=\"tab-0\">Tab 0</label>\n"
                  "<div class=\"content\">\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">Tab 1</label>\n"
                  "<div class=\"content\">\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-2\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-2\">Tab 2</label>\n"
                  "<div class=\"content\">\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-3\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-3\">Tab 3</label>\n"
                  "<div class=\"content\">\n"

                  "</div>\n"
                "</div>\n"

              "</div>\n";
#endif

void handleRoot() {
  String deliveredHTML = assembleHTML(body);

  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "On"); }
                           else { deliveredHTML.replace("%toggleStub%", "Off"); }

  server.send(200, "text/html", deliveredHTML);
}
