//NodeMCU RGB-Controller for Homebridge & HomeKit (Siri)
#include <FastLED.h> 
#include <ESP8266WiFi.h> 
#include <math.h>

    #define DATA_PIN 4
//#define CLK_PIN   4
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 144
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 10
#define FRAMES_PER_SECOND 120

WiFiServer server(80); //Set server port

String hexString = "080100";
unsigned long hexHex = 0x080100; //Define initial color here (hex value), 080100 would be a calm warmtone i.e.
String readString; //String to hold incoming request

int state;

int r;
int g;
int b;

int x;
int V;

///// WiFi SETTINGS - Replace with your values /////////////////
const char * ssid = "PrettyFiForAWiFly";
const char * password = "hate-cheat-fi";
IPAddress ip(192, 168, 0, 110); // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 0, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    WiFi.config(ip, gateway, subnet); //Set a fixed IP. You can comment this out and set it in your router instead.
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print("_");
    }
    Serial.println();
    Serial.println("Done");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

    server.begin();
}


void setup() {
    delay(3000);
    Serial.begin(9600);

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    WiFi.mode(WIFI_STA);
    WiFiStart();
    //showValues(); //Uncomment for serial output
}

void allOff() {
    state = 0;
      for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
    FastLED.show();
    //FastLED.delay(1000/FRAMES_PER_SECOND);
}

//Write requested hex-color to the pins (10bit pwm)
void setHex() {
    state = 1;
      for( int i = 0; i < NUM_LEDS; i++) {
         leds[i] = strtoul(&hexString[0],NULL,16);
        //leds[i] = CRGB::HotPink;
      }
    FastLED.show();
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    EVERY_N_MILLISECONDS(20) {
        gHue++;
    } // slowly cycle the "base color" through the rainbow
}

void loop() {
    //Reconnect on lost WiFi connection (superfluous - will reconnect anyway)
    /*if (WiFi.status() != WL_CONNECTED) {
      WiFiStart();
    }
    */

    WiFiClient client = server.available();

    if (!client) {
        return;
    }

    while (client.connected() && !client.available()) {
        delay(1);
    }

    //Respond on certain Homebridge HTTP requests
    if (client) {
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (readString.length() < 100) {
                    readString += c;
                }
                if (c == '\n') {
                    //Serial.print("Request: "); //Uncomment for serial output 
                    //Serial.println(readString); //Uncomment for serial output

                    //Send reponse
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println();

                    //On
                    if (readString.indexOf("on") > 0) {
                        rainbow();
                        FastLED.show();
                        FastLED.delay(1000 / FRAMES_PER_SECOND);
                        EVERY_N_MILLISECONDS(20) {
                          gHue++;
                        }
                        //showValues();
                    }

                    //Off
                    if (readString.indexOf("off") > 0) {
                        allOff();
                        //showValues();
                    }

                    //Set color
                    if (readString.indexOf("set") > 0) {
                        hexString = "";
                        hexString = readString.substring(9, 15);
                        //hexHex = strtoul(hexString,NULL,16);
                        setHex();
                        //showValues();
                    }

                    //Status on/off
                    if (readString.indexOf("status") > 0) {
                        client.println(state);
                    }

                    //Status color (hex)
                    if (readString.indexOf("color") > 0) {
                        client.println(hexString);
                    }

                    //Status brightness (%)
                    if(readString.indexOf("bright") >0) {
                    client.println(BRIGHTNESS);
                    }


                    delay(1);
                    client.stop();
                    readString = "";
                }
            }
        }
    }
}