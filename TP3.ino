/*******LIBRERIAS**********/
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "UniversalTelegramBot.h"
#include "ArduinoJson.h"

#include "U8g2lib.h"
#include "string"
#include "DHT_U.h"
#include "Adafruit_Sensor.h"
#include "Wire.h"

/*******WIFI*******/
const char* ssid = "UF 52";//"China-Tati";//"ORT-IoT";
const char* password = "uf52lilas";//"mardelaspampas";//"OrtIOTnew22$2";

/*******BOT TELEGRAM********/
#define BOTtoken "5821206439:AAEOmG9yeNDj_CMVDXMw1UFPFoeHz86-Vbw"
#define CHAT_ID "5314020354"//"-930008419" /// 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

/*********CONSTRUCTORES Y VARIABLES GLOBALES**********/
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int maquina = 0;
int VALOR_UMBRAL = 10;
#define BTN_SUMA 35
#define BTN_RESTA 34
float t;
char temp[5];

//const int ledPin = 25;
//bool ledState = LOW;

#define pantalla1 0
#define pantalla2 1
#define limpiar1 2
#define limpiar2 3

int botones;
#define sw1 0
#define espera1 1
#define sw2 2
#define espera2 3
#define sw3 4
#define espera3 5

unsigned long tiempoAhora, tiempoCambio, tiempoAviso;

#define TIEMPO_CODIGO 5000

// funcion que se comunica con telegram
void handleNewMessages(int numNewMessages) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {  ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/temp") {
      bot.sendMessage(chat_id, temp , "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  ///bloquea el programa si no se puede conectar a internet
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot Hola mundo", "");

  Serial.println(F("OLED test"));
  u8g2.begin();
  dht.begin();

  pinMode(BTN_SUMA, INPUT_PULLUP);
  pinMode(BTN_RESTA, INPUT_PULLUP);
}

void loop() {
  /*****TELEGRAM*******/
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Veo los msj nuevos");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  /********TEMPERATURA********/
  t = dht.readTemperature();
  //Serial.println(t);
  sprintf(temp, "%2.1f", t);
  Serial.println(temp);

  if (t >= VALOR_UMBRAL && (tiempoAhora - tiempoAviso <= 10000))
  {
    bot.sendMessage(CHAT_ID, "Supero el valor umbral" , "");
  }

  char umbral[2];
  sprintf(umbral, "%i", VALOR_UMBRAL);
  Serial.println(umbral);

  tiempoAhora = millis();

  Serial.print("Tiempo ahora: ");
  Serial.println(tiempoAhora);
  Serial.print("Tiempo cambio: ");
  Serial.println(tiempoCambio);

  /*********PANTALLA********/
  switch (maquina) {
    case pantalla1:
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(0, 12, "Temperatura: ");
      u8g2.drawStr(5, 27, temp);
      u8g2.drawStr(50, 27, "C");
      u8g2.drawStr(0, 47, "Valor umbral:");
      u8g2.drawStr(0, 62, umbral);
      u8g2.sendBuffer();
      switch (botones) {
        case sw1:
          if (digitalRead(BTN_RESTA) == LOW) {
            //tiempoCambio = millis();
            botones = espera1;
          }
          Serial.println("En estado sw 1");
          break;

        case espera1:
          Serial.println("En estado espera 1");
          if (digitalRead(BTN_RESTA) == HIGH) {
            tiempoCambio = millis();
            botones = sw2;
          }
          break;

        case sw2:
          if (digitalRead(BTN_SUMA) == LOW) {
            if (tiempoAhora - tiempoCambio <= TIEMPO_CODIGO)
            {
              //tiempoCambio = millis();
              botones = espera2;
            }
            else
            {
              botones = sw1;
            }
          }

          Serial.println("En estado sw 2");
          break;

        case espera2:
          Serial.println("En estado espera 2");
          if (digitalRead(BTN_SUMA) == HIGH) {
            tiempoCambio = millis();
            botones = sw3;
          }
          break;

        case sw3: //en realidad no hay sw3, se tiene que apretar de nuevo el sw1
          Serial.println("En estado sw 3");
          if (tiempoAhora - tiempoCambio <= TIEMPO_CODIGO)
          {
            if (digitalRead(BTN_RESTA) == LOW) {
              botones = espera3;
            }
          }
          else
          {
            botones = sw1;
          }
          break;

        case espera3:
          Serial.println("En estado espera 3");
          if (digitalRead(BTN_RESTA) == HIGH) {
            maquina = limpiar1;
            botones = sw1;
          }
          break;
      }
      Serial.println("En estado pantalla 1");
      break;

    case limpiar1:
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      Serial.println("En estado limpiar 1");
      if (digitalRead(BTN_SUMA) == HIGH && digitalRead(BTN_RESTA) == HIGH) {
        maquina = pantalla2;
      }
      break;

    case pantalla2:
      Serial.println("En estado pantalla 2");
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(0, 30, "Valor umbral: ");
      u8g2.drawStr(0, 50, umbral);
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == LOW && digitalRead(BTN_RESTA) == LOW) {
        maquina = limpiar2;
      }
      while (digitalRead(BTN_SUMA) == LOW) {
        if (digitalRead(BTN_SUMA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          VALOR_UMBRAL++;
        }
      }
      while (digitalRead(BTN_RESTA) == LOW) {
        if (digitalRead(BTN_RESTA) == HIGH) {
          u8g2.clearBuffer();
          u8g2.sendBuffer();
          VALOR_UMBRAL--;
        }
      }
      break;

    case limpiar2:
      Serial.println("En estado limpiar 2");
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      if (digitalRead(BTN_SUMA) == HIGH && digitalRead(BTN_RESTA) == HIGH) {
        maquina = pantalla1;
      }
      break;
  }
}
