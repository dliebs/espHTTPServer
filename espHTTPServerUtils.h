//
// espHTTP v1.0.1
// 2023.12.20
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
