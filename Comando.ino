#include <ESPFlashCounter.h>
#include <ESPFlash.h>
#include <ESPFlashString.h>

#include <sdios.h>
#include <MinimumSerial.h>
#include <BufferedPrint.h>
#include <FreeStack.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
  Example based on the Universal Arduino Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP8266/FlashLED/FlashLED.ino
*/

#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Deco-Gabi";
const char* password = "poliana90";

String StrLocal;
String StrAcender;
String StrApagar;
String StrEstado;

// Initialize Telegram BOT
#define BOTtoken "1979733101:AAEz583XmK_jXO5vozCRcYVcbOoy1Wlm7VE"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID_DANILO "1337388095"
#define CHAT_ID_GABRIELE "1163829049"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";
const char* PARAM_INPUT_3 = "bottoken";
const char* PARAM_INPUT_4 = "chatid1";
const char* PARAM_INPUT_5 = "chatid2";
const char* PARAM_INPUT_6 = "local";
const char* PARAM_INPUT_7 = "command";

//<meta name="viewport" content="width=device-width, initial-scale=1">
// HTML web page to handle 3 input fields (input1, input2, input3)
/*const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
  <html>
  <head>
  <title>TELEGRAM WebServer</title>  
  </head>
  <body>
  <h1>TELEGRAM WebServer Configuration</h1>
<form action="/action_page.php">
  <label for="ssid">SSID: </label>
  <input type="text" id="ssid" name="ssid"><br><br>
  <label for="password">Password: </label>
  <input type="text" id="password" name="password"><br><br>
  <label for="bottoken">BotToken: </label>
  <input type="text" id="bottoken" name="bottoken"><br><br>
  <label for="chatid1">CHAT ID1: </label>
  <input type="text" id="chatid1" name="chatid1"><br><br>
  <label for="chatid2">CHAT ID2: </label>
  <input type="text" id="chatid2" name="chatid2"><br><br> 
  <label for="local">LOCAL:</label>
  <input type="text" id="local" name="local"><br><br>   
  <br><br>
  <input type="submit" value="Submit">
</form>  
</body></html>)rawliteral";*/

WiFiClientSecure client;
UniversalTelegramBot *bot;
AsyncWebServer server(80);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan = 0;

const int ledPin = 2;
bool ledState = LOW;

const int relayPin = 5; /* PIN D1 - ESP8266 MINI D1*/
bool relayState = LOW;

const int reedSwitchPin = 4; /* PIN D2 - ESP8266 MINI D1*/


//String strLightState;
String strSSID;
String strPASSW;
String strBOTTOKEN;
String strCHATID1;
String strCHATID2;
int last_message_received;
#define CONFIG_SSID                  "Deco-Gabi"
#define CONFIG_SSID_FILEPATH         "/config_ssid"
#define CONFIG_PASSW                 "poliana90"
#define CONFIG_PASSW_FILEPATH        "config_passw"
#define CONFIG_BOTTOKEN              "1979733101:AAEz583XmK_jXO5vozCRcYVcbOoy1Wlm7VE"
#define CONFIG_BOTTOKEN_FILEPATH     "/config_bottoken"
#define CONFIG_CHATID1              "1337388095"
#define CONFIG_CHATID1_FILEPATH     "/config_chatid1"
#define CONFIG_CHATID2              "1163829049"
#define CONFIG_CHATID2_FILEPATH     "/config_chatid2"

#define CONFIG_ACENDER_FILEPATH     "/acender"
#define CONFIG_APAGAR_FILEPATH      "/apagar"
#define CONFIG_ESTADO_FILEPATH      "/estado"
#define CONFIG_LOCAL_FILEPATH       "/local"

ESPFlashString ConfigSSID(CONFIG_SSID_FILEPATH, CONFIG_SSID);
ESPFlashString ConfigPASSW(CONFIG_PASSW_FILEPATH, CONFIG_PASSW);
ESPFlashString ConfigBOTTOKEN(CONFIG_BOTTOKEN_FILEPATH, CONFIG_BOTTOKEN);
ESPFlashString ConfigCHATID1(CONFIG_CHATID1_FILEPATH, CONFIG_CHATID1);
ESPFlashString ConfigCHATID2(CONFIG_CHATID2_FILEPATH, CONFIG_CHATID2);

ESPFlashString ConfigACENDER(CONFIG_ACENDER_FILEPATH, "");
ESPFlashString ConfigAPAGAR(CONFIG_APAGAR_FILEPATH, "");
ESPFlashString ConfigESTADO(CONFIG_ESTADO_FILEPATH, "");
ESPFlashString ConfigLOCAL(CONFIG_LOCAL_FILEPATH, "");

bool request_serial_output = false;

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
/*  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot->messages[i].chat_id);
    if ((chat_id != strCHATID1) && (chat_id != strCHATID2)){
      bot->sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot->messages[i].text;
    Serial.print("Text:");
    Serial.println(text);
  
    String from_name = bot->messages[i].from_name;

    if (text == StrAcender) {
      String welcome = "Ola, " + from_name + ".\n";
      welcome += "A luz da "+ StrLocal +" será acesa!\n\n";
      bot->sendMessage(chat_id, welcome, "");
      relayState = HIGH;
      digitalWrite(relayPin, relayState);     
      ConfigESTADO.set(StrAcender);   

      int numNewMessages = bot->getUpdates(bot->last_message_received+1);            
    }    
    if (text == StrApagar) {
      String welcome = "Ola, " + from_name + ".\n";
      welcome += "A luz da "+StrLocal+" será apagada!\n\n";
      bot->sendMessage(chat_id, welcome, "");
      relayState = LOW;
      digitalWrite(relayPin, relayState);      
      ConfigESTADO.set(StrApagar);
      int numNewMessages = bot->getUpdates(bot->last_message_received+1);                  
    }

    StrEstado = ConfigESTADO.get();
    Serial.print("StrEstado:");
    Serial.println(StrEstado);    
  }
*/  
}


char cWebpage[1024];
char cCommand[1024];
char cHtml[256];
char cArray[64];

int readingState;
int lastReadingState = HIGH;  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 150;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);


  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);

  pinMode(reedSwitchPin, INPUT_PULLUP);
  
  
}


void loop() {

  // read the state of the switch into a local variable:
  int reading = digitalRead(reedSwitchPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastReadingState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != readingState) {
      readingState = reading;

      // only toggle the relay if the new button state is HIGH
      if (readingState == LOW) {
        relayState = HIGH;
      }
      else
      {
        relayState = LOW;
      }
    }
  }

  // set the relay:
  digitalWrite(relayPin, relayState);          

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastReadingState = reading;


}
