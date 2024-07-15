//
// espHTTP v1.0.3
// 2024.01.10
//

String assembleHTML(String &body) {
  String webpage;
  addHead(webpage);
  webpage += body;
  addFooter(webpage);
  return webpage;
}


void addHead(String &webpage) {
  webpage =  F( "<!DOCTYPE html>\n"
                "<html>\n"
                "<head>\n"
                  "<title>" WiFiHostname "</title>\n"
                  "<meta name=\"mobile-web-app-capable\" content=\"yes\" />\n"
                  "<meta name=\"viewport\" content=\"width=device-width\" />\n"
      //          "<meta http-equiv=\"refresh\" content=\"10\" />\n"
                  "<style>\n"
                    "body {background-color: #" BGCOLOR "; color: #" TEXTCOLOR "; font-family: " FONT "; }\n"
                    "p { font-size: 1.25em; }\n"
                    "input.simpleButton { height: 2.5em; padding: 0; font-size: 2em; background-color: #" BUTTONCOLOR "; border-color: #" BUTTONCOLOR "; color: #" TEXTCOLOR "; font-family: " FONT "; }\n"
                    "input.textInput { height: 2em; padding: 0 0.5em 0 0.5em; position: relative; text-align: center; font-size: 2em; background-color: #" BUTTONCOLOR "; border-color: #" BUTTONCOLOR "; border-width: 0; color: #" TEXTCOLOR "; font-family: " FONT "; }\n"
                    "button.inputButton { height: 2.5em; padding: 0; position: relative; font-size: 2em; background-color: #" BUTTONCOLOR "; border-color: #" BUTTONCOLOR "; color: #" TEXTCOLOR "; font-family: " FONT "; }\n"
                    "input[type=range] { outline: 0; -webkit-appearance: none; width: 100%; height: 2.5em; margin: 0; background: linear-gradient(to right, #f00 0%, #ff8000 8.3%, #ff0 16.6%, #80ff00 25%, #0f0 33.3%, #00ff80 41.6%, #0ff 50%, #007fff 58.3%, #00f 66.6%, #7f00ff 75%, #f0f 83.3%, #ff0080 91.6%, #f00 100%); }\n"
                    ".container { display: flex; align-items: center; justify-content: center; height: 100%; border: 0px; }\n"
                    ".centered-element { margin: 0; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); width: 95%; }\n"
#ifdef TABBEDPAGE
                    ".tabs { position: relative; min-height: 200px; clear: both; margin: 25px 0; }\n"
                    ".tab { float: left; }\n"
                    ".tab label { background-color: #" BUTTONCOLOR "; padding: 10px; border: 3px solid #" BGCOLOR "; margin-left: -1px; position: relative; left: 1px; font-size: 1.25em; }\n"
                    ".tab [type=radio] { display: none; }\n"
                    ".content { position: absolute; top: 33px; left: 0; right: 0; padding: 20px; border: 0px; background-color: #" TABBGCOLOR "; height: " TABHEIGHTEM "em; }\n"
                    "[type=radio]:checked ~ label { background: #" TABBGCOLOR "; border-bottom: 0px; z-index: 2; }\n"
                    "[type=radio]:checked ~ label ~ .content { z-index: 1; }\n"
                    "div.table { display: table; width: 100%; table-layout: fixed; }\n"
                    "div.table span { display: table-cell; text-align: center; }\n"
                    "input[type=checkbox] { transform: scale(2); margin: 1em; }\n"
                    "input[type=time] { font-family: Helvetica; }\n"
#endif
                  "</style>\n"
                "</head>\n"
                "<body>\n");
}

void addFooter(String &webpage) {
  webpage += F(  "</body>\n"
                 "</html>\n");
}

void handleNotFound() {
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

void redirect() {
  server.sendHeader("Location", "/");
  server.send(303);
}
