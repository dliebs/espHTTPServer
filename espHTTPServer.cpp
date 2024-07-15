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

#include "espHTTPServer.h"


/*--------    Server Functions    --------*/

// Instanciate server on port PORT upon creation of espHTTPServer object
espHTTPServer::espHTTPServer(String PAGETITLE, String BGCOLOR, String TABBGCOLOR, String BUTTONCOLOR, String TEXTCOLOR, String FONT, String TABHEIGHTEM, bool REFRESHPAGE, int PORT) : server(PORT) {
  _PAGETITLE = PAGETITLE;
  _BGCOLOR = BGCOLOR;
  _TABBGCOLOR = TABBGCOLOR;
  _BUTTONCOLOR = BUTTONCOLOR;
  _TEXTCOLOR = TEXTCOLOR;
  _FONT = FONT;
  _TABHEIGHTEM = TABHEIGHTEM;
  _REFRESHPAGE = REFRESHPAGE;
}

// Returns full webpage HTML with header and footer
String espHTTPServer::assembleHTML(String &body) {
  String webpage;
  addHead(webpage);
  webpage += body;
  addFooter(webpage);
  return webpage;
}

// Add header to webpage
void espHTTPServer::addHead(String &webpage) {
  webpage = "<!DOCTYPE html>\n"
              "<html>\n"
              "<head>\n"
                "<title>" + _PAGETITLE + "</title>\n"
                "<meta name=\"mobile-web-app-capable\" content=\"yes\" />\n"
                "<meta name=\"viewport\" content=\"width=device-width\" />\n";
   if ( _REFRESHPAGE ) {
     webpage += "<meta http-equiv=\"refresh\" content=\"10\" />\n"; }
     webpage += "<style>\n"
                  "body {background-color: #" + _BGCOLOR + "; color: #" + _TEXTCOLOR + "; font-family: " + _FONT + "; }\n"
                  "p { font-size: 1.25em; }\n"
                  "input.simpleButton { height: 2.5em; padding: 0; font-size: 2em; background-color: #" + _BUTTONCOLOR + "; border-color: #" + _BUTTONCOLOR + "; color: #" + _TEXTCOLOR + "; font-family: " + _FONT + "; }\n"
                  "button.inputButton { height: 2.5em; padding: 0; position: relative; font-size: 2em; background-color: #" + _BUTTONCOLOR + "; border-color: #" + _BUTTONCOLOR + "; color: #" + _TEXTCOLOR + "; font-family: " + _FONT + "; }\n"
                  "input.textInput { height: 2em; padding: 0 0.5em 0 0.5em; position: relative; text-align: center; font-size: 2em; background-color: #" + _BUTTONCOLOR + "; border-color: #" + _BUTTONCOLOR + "; border-width: 0; color: #" + _TEXTCOLOR + "; font-family: " + _FONT + "; }\n"
                  "input[type=range] { outline: 0; -webkit-appearance: none; width: 100%; height: 2.5em; margin: 0; background: linear-gradient(to right, #f00 0%, #ff8000 8.3%, #ff0 16.6%, #80ff00 25%, #0f0 33.3%, #00ff80 41.6%, #0ff 50%, #007fff 58.3%, #00f 66.6%, #7f00ff 75%, #f0f 83.3%, #ff0080 91.6%, #f00 100%); }\n"
                  "input[type=checkbox] { transform: scale(2); margin: 1em; }\n"
                  "input[type=time] { font-family: Helvetica; }\n"
                  ".container { display: flex; align-items: center; justify-content: center; height: 100%; border: 0px; }\n"
                  ".centered-element { margin: 0; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); width: 95%; }\n"
                  ".tabs { position: relative; min-height: 200px; clear: both; margin: 25px 0; }\n"
                  ".tab { float: left; }\n"
                  ".tab label { background-color: #" + _BUTTONCOLOR + "; padding: 10px; border: 3px solid #" + _BGCOLOR + "; margin-left: -1px; position: relative; left: 1px; font-size: 1.25em; }\n"
                  ".tab [type=radio] { display: none; }\n"
                  ".content { position: absolute; top: 33px; left: 0; right: 0; padding: 20px; border: 0px; background-color: #" + _TABBGCOLOR + "; height: " + _TABHEIGHTEM + "em; }\n"
                  "[type=radio]:checked ~ label { background: #" + _TABBGCOLOR + "; border-bottom: 0px; z-index: 2; }\n"
                  "[type=radio]:checked ~ label ~ .content { z-index: 1; }\n"
                  "div.table { display: table; width: 100%; table-layout: fixed; }\n"
                  "div.table span { display: table-cell; text-align: center; }\n"
                "</style>\n"
              "</head>\n"
              "<body>\n";
}

// Add footer to webpage
void espHTTPServer::addFooter(String &webpage) {
  webpage += F(  "</body>\n"
                 "</html>\n");
}

// Send a 404 page with info
void espHTTPServer::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Sends the client back to the root page
void espHTTPServer::redirect() {
  server.sendHeader("Location", "/");
  server.send(303);
}
