/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}
const char HTTP_LOGO[] PROGMEM            = "<section class='logo' ></section>";
const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}  h1 {text-align: center; background-color: green; color: white;} h3 {text-align: center; font-size:1.5rem;padding-top:0; margin-top: 0;} h5 {text-align: right; font-size:0.8rem; padding-top:0; margin-top: 0;} h4 {width 60%; padding: 0; margin: 0;text-align: left; font-size:1.2rem;} label{padding: 5px; padding-bottom:15px;font-size:1.1em; font-weight: bold;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;} .logo {width: 80%; margin-left: auto; margin-right: auto; background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAXAAAAA/CAYAAAD5YJMyAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAA3ZpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuNi1jMTM4IDc5LjE1OTgyNCwgMjAxNi8wOS8xNC0wMTowOTowMSAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iIHhtbG5zOnN0UmVmPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvc1R5cGUvUmVzb3VyY2VSZWYjIiB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtcE1NOk9yaWdpbmFsRG9jdW1lbnRJRD0ieG1wLmRpZDpmZTJjNzlkYS00YmU1LTQ2ZGUtYjg3Yy04N2UzYjc0MDdlODgiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6MkU4Q0Y3NzA2ODY3MTFFN0E4OENGNDI4NkZCQzc3RjkiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6MkU4Q0Y3NkY2ODY3MTFFN0E4OENGNDI4NkZCQzc3RjkiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENDIDIwMTcgTWFjaW50b3NoIj4gPHhtcE1NOkRlcml2ZWRGcm9tIHN0UmVmOmluc3RhbmNlSUQ9InhtcC5paWQ6ZmUyYzc5ZGEtNGJlNS00NmRlLWI4N2MtODdlM2I3NDA3ZTg4IiBzdFJlZjpkb2N1bWVudElEPSJ4bXAuZGlkOmZlMmM3OWRhLTRiZTUtNDZkZS1iODdjLTg3ZTNiNzQwN2U4OCIv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\") no-repeat center; background-size: contain; height: 64px} </style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='padding:5px 30px; text-align:left;display:inline-block;min-width:260px; border-style: solid; border-color: green; border-width: medium;'>";
// const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}><br/>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div style = 'font-size: 1.2em;'><span style = 'font-weight: bold;'>Credentials is saving</span><br />Trying to connect device to network.<br />If it fails the <span style ='color: RED;'> RED LED</span> will blink several times. reconnect to the device's wifi to try again<br />If it connects successfully <span style ='color: green;'>GREEN LED</span> will blink several times</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";


#define WIFI_MANAGER_MAX_PARAMS 10
#define WFM_LABEL_BEFORE 1
#define WFM_LABEL_AFTER 2
#define WFM_NO_LABEL 0
class WiFiManagerParameter {
  public:
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;
    char          temp[1500];

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    void          handleSubmit();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
