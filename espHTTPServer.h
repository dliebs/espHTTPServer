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

#ifndef espHTTPServer_h
#define espHTTPServer_h

/*----------  Libraries  ----------*/

#include "Arduino.h"

#ifdef ESP8266
  #include <ESP8266WebServer.h>
#endif
#ifdef ESP32
  #include <WebServer.h>
#endif


/*----------  espHTTPServer Class  ----------*/

class espHTTPServer {
  public:
    // Create HTTP Server - PAGETITLE, BGCOLOR, TABBGCOLOR, BUTTONCOLOR, TEXTCOLOR, FONT, TABHEIGHTEM, REFRESHPAGE, PORT
    espHTTPServer(String, String, String, String, String, String, String, bool, int);

    // Returns full webpage HTML with header and footer
    String assembleHTML(String &);
    // Send a 404 page with info
    void handleNotFound();
    // Sends the client back to the root page
    void redirect();

    // Server Object, public because we'll need to call it in our project file for serverSetup, handleRoot, and main loop
    #ifdef ESP8266
      ESP8266WebServer server;
    #endif
    #ifdef ESP32
      WebServer server;
    #endif
  private:
    // Add header and footer, used only by assembleHTML
    void addHead(String &);
    void addFooter(String &);

    // Variables to store style settings
    String  _PAGETITLE, _BGCOLOR, _TABBGCOLOR, _BUTTONCOLOR, _TEXTCOLOR, _FONT, _TABHEIGHTEM;
    bool _REFRESHPAGE;
};

#endif
