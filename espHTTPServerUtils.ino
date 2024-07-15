//
// espHTTP v1.0.1
// 2023.12.20
//

String assembleHTML(String &body) {
  String webpage;
  addHead(webpage);
  webpage += body;
  addFooter(webpage);
  return webpage;
}

#ifdef BASICPAGE
  void addHead(String &webpage) {
    webpage =  F( "<!DOCTYPE html>\n"
                  "<html>\n"
                  "<head>\n"
                    "<title>" WiFiHostname "</title>\n"
                    "<meta name=\"mobile-web-app-capable\" content=\"yes\" />\n"
                    "<meta name=\"viewport\" content=\"width=device-width\" />\n"
    //              "<meta http-equiv=\"refresh\" content=\"10\" />\n"
                    "<style>\n"
                      "body {background-color: #" BGCOLOR "; color: #a40; font-family: Helvetica;}\n"
                      "p { font-size: 1.25em; }\n"
                      "input.customButton { width: 100%; height: 2.5em; padding: 0; font-size: 2em; background-color: #222; border-color: #222; color: #a40; font-family: Helvetica;}\n"
                      ".container { display: flex; align-items: center; justify-content: center; height: 100%; border: 0px; }\n"
                      ".centered-element { margin: 0; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); width: 95%; }\n"
                    "</style>\n"
                  "</head>\n"
                  "<body>\n");
  }
  
  void addFooter(String &webpage) {
    webpage += F( "</body>\n"
                  "</html>\n");
  }
#endif

#ifdef TABBEDPAGE
  void addHead(String &webpage) {
    webpage = F(  "<!DOCTYPE html>\n"
                  "<html>\n"
                  "<head>\n"
                    "<title>" WiFiHostname "</title>\n"
                    "<meta name=\"mobile-web-app-capable\" content=\"yes\" />\n"
                    "<meta name=\"viewport\" content=\"width=device-width\" />\n"
    //              "<meta http-equiv=\"refresh\" content=\"10\" />\n"
                    "<style>\n"
                      "body {background-color: #000; color: #a40; font-family: Helvetica;}\n"
                      "p { font-size: 1.25em; }\n"
                      "input.customButton { width: 100%; height: 2.5em; padding: 0; font-size: 2em; background-color: #222; border-color: #222; color: #a40; font-family: Helvetica;}\n"
                      "button.myButton { width: 33.33%; height: 2.5em; padding: 0; position: relative; font-size: 2em; background-color: #222; border-color: #222; color: #a40; font-family: Helvetica; }\n"
                      ".tabs { position: relative; min-height: 200px; clear: both; margin: 25px 0; }\n"
                      ".tab { float: left; }\n"
                      ".tab label { background: #eee; padding: 10px; border: 3px solid #000; margin-left: -1px; position: relative; left: 1px; font-size: 1.25em; background-color: #222; }\n"
                      ".tab [type=radio] { display: none; }\n"
                      ".content { position: absolute; top: 33px; left: 0; background: white; right: 0; padding: 20px; border: 0px solid #222; background-color: #111; height: 47em; }\n"
                      "[type=radio]:checked ~ label { background: #111; border-bottom: 0px solid #333; z-index: 2; }\n"
                      "[type=radio]:checked ~ label ~ .content { z-index: 1; }\n"
                     "</style>\n"
                   "</head>\n"
                   "<body>\n");
  }
  
  void addFooter(String &webpage) {
    webpage += F(  "</body>\n"
                   "</html>\n");
  }
#endif

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
