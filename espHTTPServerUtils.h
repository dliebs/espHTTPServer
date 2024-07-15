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

/*----------  Libraries  ----------*/

#include "Arduino.h"

#ifdef ESP8266
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif

#ifdef ESP32
  #include <WebServer.h>
  WebServer server(80);
#endif
