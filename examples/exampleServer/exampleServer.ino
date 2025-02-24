//
//
//  ESP8266/32 Based HTTP Web Server
//
//  Changes From Previous Version
//    Added optional clock/schedule code
//
//

//#define USECLOCK

/*--------       Libraries        --------*/

#include <espHTTPServer.h>
#include <espHTTPUtils.h>
#include <espWiFiUtils.h>

#ifdef USECLOCK

#include "espClock.h"

#ifdef NUMOFALARMS
#undef NUMOFALARMS
#undef NUMOFFUNCS
#define NUMOFALARMS 8
#define NUMOFFUNCS 4
#endif

// Settings for espClock
Dusk2Dawn location(41.8781, -87.6298, 0);    // Chicago
#define timeZoneDST -5                       // CDT is GMT-5
#define timeZoneST  -6                       // CST is GMT-6


// Preset default function value
int defaultSetting = 70;

#endif

/*-------- User-Defined Variables --------*/



/*--------          GPIO          --------*/

#ifdef ESP32
#define LED_BUILTIN 2
#endif


/*--------   Program Variables    --------*/

// espHTTPServer
// Pointer for HTTP Server
espHTTPServer * httpServer;

// espHTTPUtils
// Placeholders for Slack Webhook API URL and message - they can be either populated with default values or left blank
String slack_url /*= "https://hooks.slack.com/services/TXXXXXXXXXX/BXXXXXXXXXX/XXXXXXXXXXXXXXXXXXXXXXXX"*/;
String message /*= "Hello ESP"*/;


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
  loadUtilConfig();
  loadSwitchConfig();

#ifdef USECLOCK
    // Set custom alarms (in the future read settings from JIFFS)
//   alarms[0] = 30600;
//   alarms[1] = 72000;
//   alarmsEnabled[0] = true;
//   alarmsEnabled[1] = true;
//   alarmFunctionToggles[0][0] = true;
//   alarmFunctionToggles[1][0] = true;
//   todFunctionToggles[0][1] = true;
//   todFunctionToggles[1][2] = true;

  // Set the initial values of the schedule functions to a default value so they're not 0
  for ( int a = 0; a < NUMOFALARMS; a++ ) {
    for ( int b = 0; b < NUMOFFUNCS; b++ ) {
      alarmFuncValues[a][b] = defaultSetting;
    }
  }
  for ( int a = 0; a < 3; a++ ) {
    for ( int b = 0; b < NUMOFFUNCS; b++ ) {
      todFuncValues[a][b] = defaultSetting;
    }
  }

  // Setup Clock
  setupClock();
#endif

  // Start HTML Server, add functions and customize below
  serverSetup();
}

void loop() {
#ifdef USECLOCK
  // Handle Clock
  handleClock();
#endif

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

#ifdef USECLOCK
// Place names of functions and the names of their values here, ex "Set Temp" and "*F" and cut to the length of NUMOFFUNCS
String nameOfFunction[2][NUMOFFUNCS] = { { "Verbose", "Function1", "Function2", "Function3"/*, "Function4", "Function5", "Function6", "Function7"*/ },
                                         { "Value0",    "Value1",    "Value2",    "Value3"/*,    "Value4",    "Value5",    "Value6",    "Value7"*/ } };

// Set whether or not a function will have an input box on the settings page
bool funcRequiresValue[NUMOFFUNCS] = { false, false, false, false /*, false, false, false, false */ };

// Place code or function calls here for schedule
// We increment thru alarmFuncValues with alarmFuncValues[alarmCounter][0]
// We need to know the alarm number, we already know what function number is running
void function0() {
  if ( trueIfAlarm ) {
    sendSlackMessage("Event " + (String)alarmCounter + " at " + epochToReadable(now()) + ". Function 0 Value: " + (String)alarmFuncValues[alarmCounter][0] + ".", slack_url);
  }
  else {
    switch ( todCounter ) {
      // Sunrise
      case 0:
        sendSlackMessage("Sunrise was at " + epochToReadable(now()) + ". Function 0 Value: " + (String)todFuncValues[todCounter][0] + ".", slack_url);
        break;
      // Sunset
      case 1:
        sendSlackMessage("Sunset was at " + epochToReadable(now()) + ". Function 0 Value: " + (String)todFuncValues[todCounter][0] + ".", slack_url);
        break;
      // Midnight
      case 2:
        sendSlackMessage("Midnight was at " + epochToReadable(now()) + ". Function 0 Value: " + (String)todFuncValues[todCounter][0] + ".", slack_url);
        break;
    }
  }
}
void function1() { sendSlackMessage("Function 1", slack_url); }
void function2() { sendSlackMessage("Function 2", slack_url); }
void function3() { sendSlackMessage("Function 3", slack_url); }
void function4() { sendSlackMessage("Function 4", slack_url); }
void function5() { sendSlackMessage("Function 5", slack_url); }
void function6() { sendSlackMessage("Function 6", slack_url); }
void function7() { sendSlackMessage("Function 7", slack_url); }
#endif


/*--------     Config Functions     --------*/

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
          httpServer = new espHTTPServer( json["PAGETITLE"], json["BGCOLOR"], json["TABBGCOLOR"],
                                          json["BUTTONCOLOR"], json["TEXTCOLOR"], json["FONT"],
                                          json["TABHEIGHTEM"], (bool)json["REFRESHPAGE"], (int)json["PORT"] );
          }
        configFile.close();
      }
    }
    else {
      // If no config file, create one using defaults
      httpServer = new espHTTPServer( "ESP HTTP Server", "000", "111", "222", "a40", "Helvetica", "47", false, 80 );
      saveServerConfig();
    }
  }
  else {
    // If no SPIFFS, use default colors
    httpServer = new espHTTPServer( "ESP HTTP Server", "000", "111", "222", "a40", "Helvetica", "47", false, 80 );
  }
}

// Load HTTP Utility Settings
void loadUtilConfig() {
  // Mount SPIFFS
  if (SPIFFS.begin()) {
    // File system mounted
    if (SPIFFS.exists("/httpUtilConfig.json")) {
      // File exists, reading and loading
      File configFile = SPIFFS.open("/httpUtilConfig.json", "r");
      if (configFile) {
        // Allocate a buffer to store contents of the file.
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          // Successfully parsed JSON, copy data
          char buffer[255];
          strcpy(buffer, json["SLACK_URL"]);
          slack_url = buffer;

          strcpy(buffer, json["MESSAGE"]);
          message = buffer;
        }
        configFile.close();
      }
    }
    else {
      // If no config file, create one using blank defaults
      saveUtilConfig();
    }
  }
  else {
    // If no SPIFFS, use defaults placeholders
    slack_url = "https://hooks.slack.com/services/TXXXXXXXXXX/BXXXXXXXXXX/XXXXXXXXXXXXXXXXXXXXXXXX";
    message = "Hello ESP";
  }
}

// Load Switch Settings
void loadSwitchConfig() {
  // Mount SPIFFS
  if (SPIFFS.begin()) {
    // File system mounted
    if (SPIFFS.exists("/switchConfig.json")) {
      // File exists, reading and loading
      File configFile = SPIFFS.open("/switchConfig.json", "r");
      if (configFile) {
        // Allocate a buffer to store contents of the file.
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          digitalWrite(LED_BUILTIN, (bool)json["SWITCH_STATE"]);
        }
        configFile.close();
      }
    }
    else {
      // If no config file, create one using blank defaults
      saveSwitchConfig();
    }
  }
  else {
    // No SPIFFS
  }
}


void saveServerConfig() {
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

    File configFile = SPIFFS.open("/httpServerConfig.json", "w");

    json.printTo(configFile);
    configFile.close();
}

void saveUtilConfig() {
    // Save settings to the file system
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["SLACK_URL"] = slack_url;
    json["MESSAGE"] = message;

    File configFile = SPIFFS.open("/httpUtilConfig.json", "w");

    json.printTo(configFile);
    configFile.close();
}

void saveSwitchConfig() {
    // Save settings to the file system
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["SWITCH_STATE"] = digitalRead(LED_BUILTIN);

    File configFile = SPIFFS.open("/switchConfig.json", "w");

    json.printTo(configFile);
    configFile.close();
}


/*--------    Server Functions    --------*/

// Make sure you redirect at the end of any function called directly by the server
void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  saveSwitchConfig();
  redirect();
}

// Example for text forms
void sendText() {
  message = httpServer -> server.arg("message");
  sendSlackMessage(message, slack_url);
  saveUtilConfig();
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
  if ( httpServer -> server.arg("hostname").length() < 33 ) {
    char buffer[32];
    strcpy(buffer, httpServer -> server.arg("hostname").c_str());
    saveHostname(buffer);
  }
}

// Save a new Slack URL and message
void setUtilSettings() {
  slack_url = httpServer -> server.arg("SLACK_URL");
  message = httpServer -> server.arg("MESSAGE");

  saveUtilConfig();
  redirect();
}

// Save new HTTP server settings
void setServerSettings() {
  httpServer -> newSettings( httpServer -> server.arg("PAGETITLE"),
                             httpServer -> server.arg("BGCOLOR"),
                             httpServer -> server.arg("TABBGCOLOR"),
                             httpServer -> server.arg("BUTTONCOLOR"),
                             httpServer -> server.arg("TEXTCOLOR"),
                             httpServer -> server.arg("FONT"),
                             httpServer -> server.arg("TABHEIGHTEM"),
                             httpServer -> server.arg("REFRESHPAGE").equals("1"),
                             httpServer -> server.arg("PORT").toInt() );
  saveServerConfig();
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
  httpServer -> server.on("/setUtilSettings", HTTP_GET, setUtilSettings);
  httpServer -> server.on("/setServerSettings", HTTP_GET, setServerSettings);
  httpServer -> server.on("/status", HTTP_GET, status);
#ifdef USECLOCK
  httpServer -> server.on("/saveSchedule", HTTP_GET, saveSchedule);
#endif

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
#ifdef USECLOCK
                    "<p style=\"font-size: 5em; text-align: center;\">%currentTimeStub%</p>\n"
#endif
                    // simpleButton needs a width and passes no variables back to the server
                    "<form action=\"/toggleLED\" method=\"GET\">"
                      "<input class=\"simpleButton\" type=\"submit\" value=\"%PAGETITLEStub% %toggleStub%\" style=\"width: 100%;\">"
                    "</form><br><br><br>\n"

                    // Text form example
                    "<p class=\"simpleHeader\">Send text to Slack:</p>\n"
                    "<form action=\"/sendText\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input class=\"textInput\" type=\"text\" name=\"message\" value=\"%MESSAGEStub%\" style=\"width: 75%;\">"
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
#ifdef USECLOCK
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-2\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-2\">Schedule</label>\n"
                  "<div class=\"content\">\n"

                    "%alarmScheduleStub%"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-3\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-3\">Horiz Schedule</label>\n"
                  "<div class=\"content\">\n"

                    "%horizScheduleStub%"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-4\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-4\">Day Schedule</label>\n"
                  "<div class=\"content\">\n"

                    "%todScheduleStub%"

                  "</div>\n"
                "</div>\n"
#endif
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

#ifdef USECLOCK
  deliveredHTML.replace("%currentTimeStub%", epochToReadable(now()));

  deliveredHTML.replace("%alarmScheduleStub%", alarmScheduleStub);
  deliveredHTML.replace("%horizScheduleStub%", horizScheduleStub);
  setAlarmScheduleValues(deliveredHTML);

  deliveredHTML.replace("%todScheduleStub%", todScheduleStub);
  setTodScheduleValues(deliveredHTML);
#endif

  deliveredHTML.replace("%PAGETITLEStub%", httpServer -> returnSetting(0));
  deliveredHTML.replace("%MESSAGEStub%", message);

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
                  "<label for=\"tab-0\">Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<p class=\"simpleHeader\">Set Hostname, Restart:</p>\n"
                    "<form action=\"/setHostname\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"hostname\" value=\"%hostnameStub%\" style=\"width: 75%;\">\n"
                      "<input type=\"hidden\" name=\"path\" value=\"settings\">"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 25%;\" value=\"Set\">\n"
                    "</form><br><br>\n"

                    "<form action=\"/setUtilSettings\" style=\"display: flex; flex-wrap: wrap;\" method=\"GET\">\n"
                      "<p class=\"simpleHeader\">Slack URL:</p>\n"
                      "<input type=\"text\" class=\"textInput\" name=\"SLACK_URL\" value=\"%SLACK_URLStub%\" style=\"width: 100%;\">\n"
                      "<p class=\"simpleHeader\" style=\"padding-top: 1em;\">Message:</p>\n"
                      "<input type=\"text\" class=\"textInput\" name=\"MESSAGE\" value=\"%MESSAGEStub%\" style=\"width: 100%;\">\n"
                      "<input type=\"hidden\" name=\"path\" value=\"settings\">"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 100%;\" value=\"Set\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">HTTP Settings</label>\n"
                  "<div class=\"content\">\n"

                    "<form action=\"/setServerSettings\" style=\"\" method=\"GET\">\n"
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
                      "<input type=\"hidden\" name=\"path\" value=\"settings\">"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 100%;\" value=\"Set\">\n"
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
  deliveredHTML.replace("%MESSAGEStub%", message);

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
