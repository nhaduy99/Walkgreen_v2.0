#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

  
//ESP8266WebServer server(80);
//define your default values here, if there are different values in config.json, they are overwritten.

#define TRIGGER_PIN  13  // a button for reseting Wifi SSID - PASS WEMOS: D7 = GPIO13
#define RED_LED   4  //Connect LED with D2 pin and resistor 100 Ohm
#define GREEN_LED 5 //Connect LED with D1 pin and resistor 100 Ohm

const char* host_read;
const char* port_read;
const char* token_read;
const char* path_read;
const char* sleep_read;

char token[80];
char host[120];
char port[8]; 
char path[60] ;
char sleep[10] ;

String x = "";

float value;

char flag =0;


unsigned int count = 0;
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Time to sleep (in seconds):
const int sleepTimeS = 5;


void setup() {

  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);                 //Moisture sensor
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(14, INPUT_PULLUP); 
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  
  digitalWrite(LED_BUILTIN,HIGH);
  digitalWrite(RED_LED,LOW);
  digitalWrite(GREEN_LED,LOW);
  

   //------------------Reset when pushing the button at first-------------------------
       while(digitalRead(TRIGGER_PIN) ==0)
        {
          Serial.println("CLICKED>>>");       //when reseting, the button is pushed, it will reset all SSID & password, restaurant_ID and table_ID
          for(int i=0;i<10;i++)               //GREEN LED blinks 10 times fast.
          {
            digitalWrite(LED_BUILTIN,1);          
            digitalWrite(GREEN_LED,1);
            delay(100);
            digitalWrite(LED_BUILTIN,0);
            digitalWrite(GREEN_LED,0);
            delay(100);
          }
         
            WiFiManager wifiManager;
          //reset saved settings
             wifiManager.resetSettings();   // Clear only SSID & pass
            // SPIFFS.format();             // Clear only restaurant_ID & table ID
            // delay(500);  
        }

 //-----------------read configuration from FS json------------------------------
      Serial.println("mounting FS...");
     
      if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        //SPIFFS.format();
        
        if (SPIFFS.exists("/config.json")) {
          //file exists, reading and loading
          Serial.println("reading config file");
          File configFile = SPIFFS.open("/config.json", "r");
          if (configFile) {
            Serial.println("opened config file");
            size_t size = configFile.size();
            // Allocate a buffer to store contents of the file.
            std::unique_ptr<char[]> buf(new char[size]);
    
            configFile.readBytes(buf.get(), size);
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.parseObject(buf.get());
            json.printTo(Serial);
            if (json.success()) {
              Serial.println("\nparsed json");
              strcpy(token, json["token"]);
              strcpy(host, json["host"]);
              strcpy(port, json["port"]);
              strcpy(sleep, json["sleep"]);
              
            } else {
              Serial.println("failed to load json config");
            }
          }
        }
      } else {
        Serial.println("failed to mount FS");
      }
      //end read
      Serial.println("Unclicked .............");
      delay (300);
      
      digitalWrite(GREEN_LED,1);      // green LED emits to wait for setting the Wifi SSID/Password
 
  // -------The extra parameters to be configured (can be either global or just in the setup)-----------
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  
      WiFiManagerParameter  custom_token("token","token",token, 80);
      WiFiManagerParameter  custom_host("host","host",host, 120);
      WiFiManagerParameter  custom_port("port","port",port, 8);
      WiFiManagerParameter  custom_sleep("sleep","sleep",sleep, 10);

  //--------------------------WiFiManager---------------------------
    WiFiManager wifiManager;

//  wifiManager.autoConnect("AutoConnectAP222");
     //reset saved settings
//    wifiManager.resetSettings();

//----------------------set custom ip for portal--------------------------
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255,255,255,0));
    //set config save notify callback
   wifiManager.setSaveConfigCallback(saveConfigCallback);

   
    wifiManager.addParameter(&custom_sleep);
    wifiManager.addParameter(&custom_host);
    wifiManager.addParameter(&custom_port);
    wifiManager.addParameter(&custom_token);
    
    

 
  //set minimu quality of signal so it ignores AP's under that quality
  wifiManager.setMinimumSignalQuality(30);      // Show Wifi signal at least 30%
  
 

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Walkgreen_v2.0")) {
    Serial.println("failed to connect and hit timeout");
    delay(5000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);

}
    
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  digitalWrite(LED_BUILTIN,0);
   for(int i=0;i<5;i++)               // top LED blinks 10 times fast.
          {
            digitalWrite(GREEN_LED,1);
            delay(200);
            digitalWrite(GREEN_LED,0);
            delay(200);
          }
          
   strcpy(host, custom_host.getValue());
   strcpy(port, custom_port.getValue());
   strcpy(token, custom_token.getValue());
   strcpy(sleep, custom_sleep.getValue());
    
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["host"] = host;
    json["port"] = port;
    json["token"] = token;
    json["sleep"] = sleep;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  FS_data_read();
  Serial.println("begin to loop");
}

void loop() {
  Data_sending();
  deep_sleep();
}
//-----------------Function----------------------------------------
void FS_data_read()
{
         
 File configFile = SPIFFS.open("/config.json", "r");   
          
        WiFiManagerParameter  custom_token("token","token",token, 80);
        WiFiManagerParameter  custom_host("host","host",host, 120);
        WiFiManagerParameter  custom_port("port","port",port, 8);
        WiFiManagerParameter  custom_sleep("sleep","sleep",sleep, 10);
        
         Serial.println("INSIDE THE FS DATA READ FUNCTION");
           if (!configFile) {
            Serial.println("failed to open config file for reading");
            
          }
          else{
            
            Serial.println("reading config file");
            String s=configFile.readStringUntil('\n');
            Serial.print("CONTENT");
            Serial.print(":");
            Serial.println(s);

            token_read = custom_token.getValue();
            Serial.print("Token: ");
            Serial.println(token_read);

            
            Serial.print("Chip_ID: ");
            Serial.println(ESP.getChipId());
            Serial.println("");
            
            host_read = custom_host.getValue();
            Serial.print("host: ");
            Serial.println(host_read);
            
            port_read = custom_port.getValue();
            Serial.print("port: ");
            Serial.println(port_read);

             
//            path_read = custom_path.getValue();
//            Serial.print("path: ");
//            Serial.println(path_read);

            sleep_read = custom_sleep.getValue();
            Serial.print("sleep: ");
            Serial.println(sleep_read);
         
            
          }      
}
//------------FUNCTIONS FOR SENSOR VALUES----------------
String wet_dry(){
   if(digitalRead(14)) { return "DRY"; }
    else{ return "WET";}
}
int wet_dry_2(){
   if(digitalRead(14)) { return 1; }
    else{ return 0; }
}
float percent()
{
  value = analogRead(A0);
  value = 100-((value - 350)/630)*100;
  if(value>100){value = 100;}
  if(value<0){value = 0;}
  return value;
}

float humid()
{
  value = analogRead(A0);
  Serial.print("ADC value= ");
  Serial.println(value);
  value = 1-((value - 350)/630);
  if(value>1){value = 1;}
  if(value<0){value = 0;}
  Serial.print("humid= ");
  Serial.println(value);
  return value;
}



void Data_sending(){
  flag=1;
  Serial.print("connecting to ");
  Serial.println(host_read);
 
  const char* host_new = host_read;
   
  if(strstr(host_new,"https"))
  {
    Serial.println("there is : 'https'");
    for(int i =8 ; i<strlen(host_new);i++) 
    {
      x = x+ host_new[i];
     }
  
   Serial.print("host after cutting: ");
   Serial.println(x);
   sending_frame_2(x.c_str(),atoi(port_read),path_read, token_read,ESP.getChipId());
   x = "";
  }
  else
  {
    
      for(int i =7 ; i<strlen(host_new);i++) 
      {
        x = x+ host_new[i];
       }
       Serial.print("host after cutting: ");
      Serial.println(x);
      sending_frame_1(x.c_str(),atoi(port_read),path_read, token_read,ESP.getChipId());
      x = "";
  }

  

}
//-------------SENDING SENSOR DATA TO SERVER--------------
void sending_frame_1(const char* host,int port, const char* path_read, const char* token, int ID)
{
  WiFiClient client;
  if (!client.connect(host, port)) {   
    Serial.println("connection failed");
    flag = 0;
    return;
  }
  
  
   //   String url = "/notify?device_id=test&humidity=0.7";
  String url = "/notify?device_id=";
  url += String(ESP.getChipId());
  url += "&humidity=";
  url += String(humid());
  
  Serial.println("Frame send to server: GET " + url + " HTTP/1.1");
 //--------FRAME OF SENDING DATA TO THE SERVER------------------
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

 //--------CHECKING THE RESPONSE FROM THE SERVER---------------
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 //---------EXPORT SERVER'S RESPONSE TO EXAMINE------------------ 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
 //   String line = client.readString();
    Serial.print(line);
    if(line=="HTTP/1.1 200 OK")
    {
      Serial.println("OKKKK");
      flag=1;
      break;
    }
    else
    {
      
      flag = 0;
      break;
    }
  
  }
  Serial.println();
  Serial.println("<------------Ending of receiving frame from server------------->");
  Serial.println("closing connection");
  delay(1000);
}
void sending_frame_2(const char* host,int port, const char* path_read, const char* token, int ID)
{
   WiFiClientSecure client;
  if (!client.connect(host, port)) {   
    Serial.println("connection failed");
      flag = 0;
    return;
  }
  

   //   String url = "/notify?device_id=test&humidity=0.7";
  String url = "/notify?device_id=";
  url += String(ESP.getChipId());
  url += "&humidity=";
  url += String(humid());
  
  Serial.println("Frame send to server: GET " + url + " HTTP/1.1");  
 //--------FRAME OF SENDING DATA TO THE SERVER------------------
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

 //--------CHECKING THE RESPONSE FROM THE SERVER---------------
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

 //---------EXPORT SERVER'S RESPONSE TO EXAMINE------------------ 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
 //   String line = client.readString();
   Serial.print(line);
  if(line=="HTTP/1.1 200 OK")
    {
      Serial.println("OKKKK");
      flag=1;
      break;
    }
    else
    {
      flag = 0;
      break;
    }
  }
  Serial.println();
  Serial.println("<------------Ending of receiving frame from server------------->");
  Serial.println("closing connection");
  delay(1000);
  
}

void deep_sleep(){
  Serial.println("ESP8266 in sleep mode");
  ESP.deepSleep(atoi(sleep_read) * 1000000);
//ESP.deepSleep(1000000);
  delay(500);
}
