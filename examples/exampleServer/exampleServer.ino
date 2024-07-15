//
//
//  ESP HTTP Server - Version 1.1.0
//    This version was deployed 2024.02.02
//
//  ESP8266/32 Based
//    HTTP Web Server
//    Basic and tabbed sites
//    Customizable colors and styles
//
//  Changes From Previous Version
//    Comments, cleanup
//    Library-ified!
//
//

/*--------       Libraries        --------*/

#include <espHTTPServer.h>
#include <espWiFiUtils.h>


/*--------    WiFi Credentials    --------*/

#define WiFiHostname "espHTTPServer"

#ifndef STASSID
  #define STASSID "Your_WiFi_SSID"
  #define STAPSK  "Your_WiFi_Pass"
#endif


/*-------- User-Defined Variables --------*/

// Define BASICPAGE or TABBEDPAGE - NOTE: For example only, please remove as it's only used to wrap the body in this file
#define TABBEDPAGE

// Webpage User Settings
#define PAGETITLE "ESP HTTP Server"
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"
#define REFRESHPAGE false
#define PORT 80

// espHTTPServer Object
espHTTPServer httpServer( PAGETITLE, BGCOLOR, TABBGCOLOR, BUTTONCOLOR, TEXTCOLOR, FONT, TABHEIGHTEM, REFRESHPAGE, PORT );


/*--------          GPIO          --------*/


/*--------   Program Variables    --------*/


/*--------     Main Functions     --------*/

void setup() {
  // Begin Serial for sendText
  Serial.begin(115200);

  // Setup GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);

  // Connect to WiFi, start OTA
  connectWiFi(STASSID, STAPSK, WiFiHostname);
  initializeOTA(WiFiHostname, STAPSK);

  // Start HTML Server, add functions and customize below
  serverSetup();
}

void loop() {
  // Webserver handler
  httpServer.server.handleClient();

  // Arduino OTA
  ArduinoOTA.handle();

  // Let the ESP8266 do its thing
  yield();
}


/*--------    Server Functions    --------*/

// Make sure you redirect at the end of any function called directly by the server
void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  httpServer.redirect();
}

// Example for text forms
String message;
void sendText() {
  message = httpServer.server.arg("message");
  Serial.println(message);
  httpServer.redirect();
}

// Status page, useful for returning values
// Returns status of sensor values
void status() {
  String statusOf = server.arg("of");
  if ( statusOf == "led" ) { server.send(200, "text/html", (String)(digitalRead(LED_BUILTIN))); }
  else if ( statusOf == "message" ) { server.send(200, "text/html", message); }
  else { handleNotFound(); }
}


/*--------       HTTP Server      --------*/

// Server setup function
// Add void functions and lines for httpServer.server.on
// GET allows external requests, POST prevents them?
void serverSetup() {
  httpServer.server.on("/", handleRoot);
  httpServer.server.on("/toggleLED", HTTP_GET, toggleLED);
  httpServer.server.on("/sendText", HTTP_GET, sendText);
  httpServer.server.on("/status", HTTP_GET, status);
  httpServer.server.onNotFound(handleNotFound);
  httpServer.server.begin();
}

// HTML Body - header and footer are managed by espHTTPServer.assembleHTML()
#ifdef BASICPAGE
// A basic page, everything is centered horizontally and vertically inside the divs
String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"

                  // simpleButton needs a width and passes no variables back to the server
                  "<form action=\"/toggleLED\" method=\"GET\">"
                    "<input class=\"simpleButton\" type=\"submit\" value=\"Turn LED %toggleStub%\" style=\"width: 100%;\">"
                  "</form>\n"

                "</div>\n"
              "</div>\n";
#endif

#ifdef TABBEDPAGE
// A page with tabs, useful for fitting more into a single interface. Could also create additional pages, see status() below
String body = "<div class=\"tabs\">\n"
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-0\" name=\"tab-group-1\" checked>\n"
                  "<label for=\"tab-0\">Tab 0</label>\n"
                  "<div class=\"content\">\n"

                    // simpleButton needs a width and passes no variables back to the server
                    "<form action=\"/toggleLED\" method=\"GET\">"
                      "<input class=\"simpleButton\" type=\"submit\" value=\"Turn LED %toggleStub%\" style=\"width: 100%;\">"
                    "</form>\n"

                    // Form with inputButtons
                    "<form action=\"/sendText\" method=\"GET\">\n"
                      // Hidden input can be sent along with the form
                      // "<input type=\"hidden\" name=\"hiddenInput\" value=\"0\"></input>\n"
                      // Button press submits form and sends value
                      "<button class=\"inputButton\" onclick=\"this.form.submit()\" name=\"message\" value=\"1\" style=\"width: 33.33%;\">1</button>"
                      "<button class=\"inputButton\" onclick=\"this.form.submit()\" name=\"message\" value=\"2\" style=\"width: 33.33%;\">2</button>"
                      "<button class=\"inputButton\" onclick=\"this.form.submit()\" name=\"message\" value=\"3\" style=\"width: 33.33%;\">3</button>\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">Tab 1</label>\n"
                  "<div class=\"content\">\n"

                    // Text form example
                    "<form action=\"/sendText\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input class=\"textInput\" type=\"text\" name=\"message\" value=\"%messageStub%\" style=\"width: 75%;\">"
                      "<input class=\"textInput\" type=\"submit\" value=\"Send\" style=\"width: 25%;\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-2\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-2\">Tab 2</label>\n"
                  "<div class=\"content\">\n"

                    // Color picker examples
                    // Hue slider, set value 0-359 with %colorHex%
                    "<form action=\"/sendText\" method=\"GET\">"
                      "<input class=\"simpleButton\" type=\"range\" onchange=\"this.form.submit()\" min=\"0\" max=\"359\" step=\"1\" name=\"message\" value=\"180\" style=\"width: 100%;\">"
                    "</form>\n"
                    // OS color picker, set value to #XXXXXX with %colorHex%
                    "<form action=\"/sendText\" method=\"GET\">"
                      "<input class=\"simpleButton\" type=\"color\" onchange=\"this.form.submit()\" name=\"message\" value=\"#000000\" style=\"border: 0; width: 100%;\">"
                    "</form>\n"
                    // Multiple simpleButtons in one form, whichever gets pressed gets sent
                    "<form action=\"/sendText\" method=\"GET\" style=\"font-size: 1em;\">\n"
                      "<input class=\"simpleButton\" type=\"submit\" name=\"message\" value=\"#FFF200\" style=\"width: 20%; border: 0; background-color: #FFF200; color: #FFF200;\">"
                      "<input class=\"simpleButton\" type=\"submit\" name=\"message\" value=\"#F7941E\" style=\"width: 20%; border: 0; background-color: #F7941E; color: #F7941E;\">"
                      "<input class=\"simpleButton\" type=\"submit\" name=\"message\" value=\"#ED145B\" style=\"width: 20%; border: 0; background-color: #ED145B; color: #ED145B;\">"
                      "<input class=\"simpleButton\" type=\"submit\" name=\"message\" value=\"#92278F\" style=\"width: 20%; border: 0; background-color: #92278F; color: #92278F;\">"
                      "<input class=\"simpleButton\" type=\"submit\" name=\"message\" value=\"#0072BC\" style=\"width: 20%; border: 0; background-color: #0072BC; color: #0072BC;\">\n"
                    "</form>\n"

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

// Handle Root - variables can be applies to %stubs% in deliveredHTML after the assembleHTML is written to it
// We want to do this each time the page loads as the variables may change and deliveredHTML doesn't need to sit in RAM
// This needs to be after the body
void handleRoot() {
  String deliveredHTML = httpServer.assembleHTML(body);
  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "On"); }
                           else { deliveredHTML.replace("%toggleStub%", "Off"); }

  deliveredHTML.replace("%messageStub%", message);

  httpServer.server.send(200, "text/html", deliveredHTML);
}

// Simple call back to espHTTPServer object for reasons
void handleNotFound() {
  httpServer.handleNotFound();
}
