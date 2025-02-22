//
//
//  ESP8266/32 Based HTTP Web Server
//
//  Changes From Previous Version
//    Updated for managedWiFiOTASetup
//    Added JIFFS support
//
//

/*--------       Libraries        --------*/

#include <espHTTPServer.h>
#include <espHTTPUtils.h>
#include <espWiFiUtils.h>

/*-------- User-Defined Variables --------*/


/*--------          GPIO          --------*/

#ifdef ESP32
#define LED_BUILTIN 2
#endif

/*--------   Program Variables    --------*/

// Pointer for HTTP Server
espHTTPServer * httpServer;

// Placeholder for Slack Webhook API URL
String slack_url = "https://hooks.slack.com/services/TXXXXXXXXXX/BXXXXXXXXXX/XXXXXXXXXXXXXXXXXXXXXXXX";

/*--------     Main Functions     --------*/

void setup() {
  // Begin Serial for sendText
  Serial.begin(115200);

  // Setup GPIO
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef ESP8266
  digitalWrite(LED_BUILTIN, true);
#endif
#ifdef ESP32
  digitalWrite(LED_BUILTIN, false);
#endif

  // Connect to WiFi, start OTA
  managedWiFiOTASetup();

  // Load Settings
  loadServerConfig();

  // Start HTML Server, add functions and customize below
  serverSetup();
}

void loop() {
  // Webserver handler
  httpServer -> server.handleClient();

  // Arduino OTA
  ArduinoOTA.handle();

#ifdef ESP32
  // Reconnect WiFi
  autoReconnectWiFi();
#endif

  // Let the ESP8266 do its thing
  yield();
}

// Load HTTP Server Settings
void loadServerConfig() {
  // Mount SPIFFS
  if (SPIFFS.begin()) {
    // File system mounted
    if (SPIFFS.exists("/httpServerConfig.json")) {
      // File exists, reading and loading
      File configFile = SPIFFS.open("/httpServerConfig.json", "r");
      if (configFile) {
        // Allocate a buffer to store contents of the file.
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          // Successfully parsed JSON, setup HTTP server
          httpServer = new espHTTPServer( json["PAGETITLE"], json["BGCOLOR"], json["TABBGCOLOR"], json["BUTTONCOLOR"], json["TEXTCOLOR"], json["FONT"], json["TABHEIGHTEM"], (bool)json["REFRESHPAGE"], (int)json["PORT"] );

          char buffer[82];
          strcpy(buffer, json["SLACK_URL"]);
          slack_url = buffer;
        }
        configFile.close();
      }
    }
    else {
      // If no config file, create one using defaults
      httpServer = new espHTTPServer( "ESP HTTP Server", "000", "111", "222", "a40", "Helvetica", "47", false, 80 );
      saveHTTPSettings();
    }
  }
  else {
    // If no SPIFFS, use default colors
    httpServer = new espHTTPServer( "ESP HTTP Server", "000", "111", "222", "a40", "Helvetica", "47", false, 80 );
  }
}

void saveHTTPSettings() {
    // Save settings to the file system
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["PAGETITLE"] = httpServer -> returnSetting(0);
    json["BGCOLOR"] = httpServer -> returnSetting(1);
    json["TABBGCOLOR"] = httpServer -> returnSetting(2);
    json["BUTTONCOLOR"] = httpServer -> returnSetting(3);
    json["TEXTCOLOR"] = httpServer -> returnSetting(4);
    json["FONT"] = httpServer -> returnSetting(5);
    json["TABHEIGHTEM"] = httpServer -> returnSetting(6);
    json["REFRESHPAGE"] = httpServer -> returnSetting(7);
    json["PORT"] = httpServer -> returnSetting(8);
    json["SLACK_URL"] = slack_url;

    File configFile = SPIFFS.open("/httpServerConfig.json", "w");

    json.printTo(configFile);
    configFile.close();
}


/*--------    Server Functions    --------*/

// Make sure you redirect at the end of any function called directly by the server
void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  redirect();
}

// Example for text forms
String message;
void sendText() {
  message = httpServer -> server.arg("message");
  sendSlackMessage(message, slack_url);
  redirect();
}

// Status page, useful for returning values
// Returns status of sensor values
void status() {
  String statusOf = httpServer -> server.arg("of");
  if ( statusOf == "led" ) { httpServer -> server.send(200, "text/html", (String)(digitalRead(LED_BUILTIN))); }
  else if ( statusOf == "message" ) { httpServer -> server.send(200, "text/html", message); }
  else { handleNotFound(); }
}

// Save a new hostname and restart the device
void setHostname() {
  char buffer[32];
  strcpy(buffer, httpServer -> server.arg("hostname").c_str());
  saveHostname(buffer);
}

// Save a new Slack URL
void setSlackURL() {
  char buffer[82];
  strcpy(buffer, httpServer -> server.arg("SLACK_URL").c_str());
  slack_url = buffer;
  saveHTTPSettings();
  redirect();
}

// Save a new Slack URL
void setHTTPSettings() {
  char buffer[82];
  strcpy(buffer, httpServer -> server.arg("SLACK_URL").c_str());
  slack_url = buffer;

  httpServer -> newSettings( httpServer -> server.arg("PAGETITLE"), httpServer -> server.arg("BGCOLOR"), httpServer -> server.arg("TABBGCOLOR"), httpServer -> server.arg("BUTTONCOLOR"), httpServer -> server.arg("TEXTCOLOR"), httpServer -> server.arg("FONT"), httpServer -> server.arg("TABHEIGHTEM"), httpServer -> server.arg("REFRESHPAGE").equals("1"), httpServer -> server.arg("PORT").toInt() );

  saveHTTPSettings();
  redirect();
}

/*--------       HTTP Server      --------*/

// Server setup function
// Add void functions and lines for httpServer -> server.on
// GET allows external requests, POST prevents them?
void serverSetup() {
  httpServer -> server.on("/", handleRoot);
  httpServer -> server.on("/embed", handleEmbed);
  httpServer -> server.on("/settings", handleSettings);
  httpServer -> server.on("/setHostname", HTTP_GET, setHostname);
  httpServer -> server.on("/setSlackURL", HTTP_GET, setSlackURL);
  httpServer -> server.on("/setHTTPSettings", HTTP_GET, setHTTPSettings);
  httpServer -> server.on("/status", HTTP_GET, status);

  httpServer -> server.on("/toggleLED", HTTP_GET, toggleLED);
  httpServer -> server.on("/sendText", HTTP_GET, sendText);
  httpServer -> server.onNotFound(handleNotFound);
  httpServer -> server.begin();
}

// HTML Body - header and footer are managed by espHTTPServer.assembleHTML()
// A page with tabs, useful for fitting more into a single interface. Could also create additional pages, see status() above
String body = "<div class=\"tabs\">\n"
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-0\" name=\"tab-group-1\" checked>\n"
                  "<label for=\"tab-0\">Basic</label>\n"
                  "<div class=\"content\">\n"

                    // simpleButton needs a width and passes no variables back to the server
                    "<form action=\"/toggleLED\" method=\"GET\">"
                      "<input class=\"simpleButton\" type=\"submit\" value=\"Turn LED %toggleStub%\" style=\"width: 100%;\">"
                    "</form><br><br><br>\n"

                    // Text form example
                    "<p class=\"simpleHeader\">Send text to Slack:</p>\n"
                    "<form action=\"/sendText\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input class=\"textInput\" type=\"text\" name=\"message\" value=\"%messageStub%\" style=\"width: 75%;\">"
                      "<input class=\"textInput\" type=\"submit\" value=\"Send\" style=\"width: 25%;\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">Color</label>\n"
                  "<div class=\"content\">\n"

                    // Color picker examples
                    // Hue slider, set value 0-359 (can be set to max of 255 for FastLED) with %colorHex%
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
                  "<input type=\"radio\" id=\"tab-2\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-2\">HTTP Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<form action=\"/setHTTPSettings\" style=\"\" method=\"GET\">\n"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Title:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"PAGETITLE\" value=\"%PAGETITLEStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">BG Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"BGCOLOR\" value=\"%BGCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Tab Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TABBGCOLOR\" value=\"%TABBGCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Button Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"BUTTONCOLOR\" value=\"%BUTTONCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Text Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TEXTCOLOR\" value=\"%TEXTCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Font:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"FONT\" value=\"%FONTStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Tab Height:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TABHEIGHTEM\" value=\"%TABHEIGHTEMStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Refresh:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"REFRESHPAGE\" value=\"%REFRESHPAGEStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Port:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"PORT\" value=\"%PORTStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Slack URL:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"SLACK_URL\" value=\"%SLACK_URLStub%\"></span>"
                      "</div>"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 100%;\" value=\"Set\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-3\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-3\">Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<p class=\"simpleHeader\">Set Hostname, Restart:</p>\n"
                    "<form action=\"/setHostname\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"hostname\" value=\"%hostnameStub%\" style=\"width: 75%;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 25%;\" value=\"Set\">\n"
                    "</form><br><br>\n"

                    "<p class=\"simpleHeader\">Set Slack URL:</p>\n"
                    "<form action=\"/setSlackURL\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"SLACK_URL\" value=\"%SLACK_URLStub%\" style=\"width: 75%;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 25%;\" value=\"Set\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

              "</div>\n";


// Handle Root - variables can be applies to %stubs% in deliveredHTML after the assembleHTML is written to it
// We want to do this each time the page loads as the variables may change and deliveredHTML doesn't need to sit in RAM
// This needs to be after the body
void handleRoot() {
  String deliveredHTML = httpServer -> assembleHTML(body);
#ifdef ESP8266
  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "On"); }
                          else { deliveredHTML.replace("%toggleStub%", "Off"); }
#endif
#ifdef ESP32
  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "Off"); }
                          else { deliveredHTML.replace("%toggleStub%", "On"); }
#endif

  deliveredHTML.replace("%messageStub%", message);

  deliveredHTML.replace("%hostnameStub%", WiFi.getHostname());

  deliveredHTML.replace("%PAGETITLEStub%", httpServer -> returnSetting(0));
  deliveredHTML.replace("%BGCOLORStub%", httpServer -> returnSetting(1));
  deliveredHTML.replace("%TABBGCOLORStub%", httpServer -> returnSetting(2));
  deliveredHTML.replace("%BUTTONCOLORStub%", httpServer -> returnSetting(3));
  deliveredHTML.replace("%TEXTCOLORStub%", httpServer -> returnSetting(4));
  deliveredHTML.replace("%FONTStub%", httpServer -> returnSetting(5));
  deliveredHTML.replace("%TABHEIGHTEMStub%", httpServer -> returnSetting(6));
  deliveredHTML.replace("%REFRESHPAGEStub%", httpServer -> returnSetting(7));
  deliveredHTML.replace("%PORTStub%", httpServer -> returnSetting(8));
  deliveredHTML.replace("%SLACK_URLStub%", slack_url);

  httpServer -> server.send(200, "text/html", deliveredHTML);
}

// Handle embedded page - background color and margins are changed to work well inside iframes
// Page is inline in the function in this example
void handleEmbed() {
  String deliveredHTML = "<form action=\"/toggleLED\" method=\"GET\">"
                           "<input type=\"hidden\" name=\"path\" value=\"embed\">"
                           "<input class=\"simpleButton\" type=\"submit\" value=\"Turn LED %toggleStub%\" style=\"width: 100%;\">"
                         "</form>\n";

  deliveredHTML = httpServer -> assembleHTML(deliveredHTML);

  deliveredHTML.replace("background-color: #000", "background-color: #111");
  deliveredHTML.replace("<body>", "<body style=\"margin: 0;\">");

#ifdef ESP8266
  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "On"); }
                          else { deliveredHTML.replace("%toggleStub%", "Off"); }
#endif
#ifdef ESP32
  if (digitalRead(LED_BUILTIN)) { deliveredHTML.replace("%toggleStub%", "Off"); }
                          else { deliveredHTML.replace("%toggleStub%", "On"); }
#endif

  httpServer -> server.send(200, "text/html", deliveredHTML);
}

// Handle settings page
// Page is inline in the function in this example
void handleSettings() {
  String deliveredHTML = "<div class=\"tabs\">\n"
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-0\" name=\"tab-group-1\" checked>\n"
                  "<label for=\"tab-0\">HTTP Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<form action=\"/setHTTPSettings\" style=\"\" method=\"GET\">\n"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Title:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"PAGETITLE\" value=\"%PAGETITLEStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">BG Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"BGCOLOR\" value=\"%BGCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Tab Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TABBGCOLOR\" value=\"%TABBGCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Button Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"BUTTONCOLOR\" value=\"%BUTTONCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Text Color:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TEXTCOLOR\" value=\"%TEXTCOLORStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Font:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"FONT\" value=\"%FONTStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Tab Height:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"TABHEIGHTEM\" value=\"%TABHEIGHTEMStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Refresh:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"REFRESHPAGE\" value=\"%REFRESHPAGEStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Port:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"PORT\" value=\"%PORTStub%\"></span>"
                      "</div>"
                      "<div class=\"table\">"
                        "<span class=\"settingTitle\">Slack URL:</span>"
                        "<span><input type=\"text\" class=\"textInput settingText\" name=\"SLACK_URL\" value=\"%SLACK_URLStub%\"></span>"
                      "</div>"
                      "<input type=\"hidden\" name=\"path\" value=\"settings\">"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 100%;\" value=\"Set\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<p class=\"simpleHeader\">Set Hostname, Restart:</p>\n"
                    "<form action=\"/setHostname\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"hostname\" value=\"%hostnameStub%\" style=\"width: 75%;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 25%;\" value=\"Set\">\n"
                    "</form><br><br>\n"

                    "<p class=\"simpleHeader\">Set Slack URL:</p>\n"
                    "<form action=\"/setSlackURL\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"hidden\" name=\"path\" value=\"settings\">"
                      "<input type=\"text\" class=\"textInput\" name=\"SLACK_URL\" value=\"%SLACK_URLStub%\" style=\"width: 75%;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 25%;\" value=\"Set\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

              "</div>\n";

  deliveredHTML = httpServer -> assembleHTML(deliveredHTML);

  deliveredHTML.replace("%hostnameStub%", WiFi.getHostname());

  deliveredHTML.replace("%PAGETITLEStub%", httpServer -> returnSetting(0));
  deliveredHTML.replace("%BGCOLORStub%", httpServer -> returnSetting(1));
  deliveredHTML.replace("%TABBGCOLORStub%", httpServer -> returnSetting(2));
  deliveredHTML.replace("%BUTTONCOLORStub%", httpServer -> returnSetting(3));
  deliveredHTML.replace("%TEXTCOLORStub%", httpServer -> returnSetting(4));
  deliveredHTML.replace("%FONTStub%", httpServer -> returnSetting(5));
  deliveredHTML.replace("%TABHEIGHTEMStub%", httpServer -> returnSetting(6));
  deliveredHTML.replace("%REFRESHPAGEStub%", httpServer -> returnSetting(7));
  deliveredHTML.replace("%PORTStub%", httpServer -> returnSetting(8));
  deliveredHTML.replace("%SLACK_URLStub%", slack_url);

  httpServer -> server.send(200, "text/html", deliveredHTML);
}

// Simple call back to espHTTPServer object for reasons
void handleNotFound() {
  httpServer -> handleNotFound();
}

// Handles redirection for other pages, requires hidden path element in forms
void redirect() {
  if ( httpServer -> server.arg("path") == "embed" ) { httpServer -> redirect("embed"); }
  else if ( httpServer -> server.arg("path") == "settings" ) { httpServer -> redirect("settings"); }
  else { httpServer -> redirect(); }
}
