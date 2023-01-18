// ---------------------------------------------------------------------- Knižnice ---------------------------------------------------------------------  
#include <WiFi.h>  //Kniznica pre pripojenie sa ESP k Wi-fi
#include <PubSubClient.h> //Klientska knižnica pre zasielanie správ MQTT
// ---------------------------------------------------------------------- Definovanie -------------------------------------------------------------------- 
#define schranka_senzor_GPIO_PIN  4   // RTC-GPIO pin.Počas hlbokého spánku môže koprocesor ULP použiť niektoré piny ESP32, konkrétne piny RTC_GPIO: 0,2,4,12-15,25-27,32-39.

#define bitova_maska_GPIO_Pinu 0xA  // 2^4 = 16 decimalne. (Použitý GPIO 4) v hex cisle je to A. 
 
int status_schranky;       // datovy typ
   
// ------------------------------------------------------------- Nastavenie Wi-fi -----------------------------------------------------------------------

const char* meno_wifi = "nazov wifi";         //Použili sme teraz domácu adresu zadame nazov wifi
const char* heslo_wifi =  "heslo wifi";       // Heslo zadame heslo wifi


// ---------------------------------------------------------- Nastavenie MQTT Servera -------------------------------------------------------------------
const char* mqttServer = "192.168.x.xxx";     //Adresa MQTT servera (v nasom pripade lokalna adresa RPI) 
const int mqttPort = 1883;                    // Základny port MQTT

const char* mqttPouzivatel = "meno pouzivatela mqtt";      //Meno používatela MQTT    
const char* mqttheslo = "heslo pouzivatela mqtt";          //Heslo pouzivatela MQTT

const char* topic_esp32 = "nazov topicu";    //Topic MQTT

WiFiClient espClient;
PubSubClient client(espClient);
// ------------------------------------------------------- Pripojenie k MQTT serveru   -------------------------------------------------------------------
void mqtt_pripojenie(){
  client.setServer(mqttServer, mqttPort);
  
  while (!client.connected()) {                               //Slučka while ktorá overuje či je pripojeny client (ESP32) k MQTT serveru (RPI)
    Serial.println("Pripajam sa k MQTT Serveru ...");
 
    if (client.connect("ESP32Client", mqttPouzivatel, mqttheslo )) {   //Overenie používateľa pripojenie sa k MQTT serveru (meno a heslo)
 
      Serial.println("Pripojeny k MQTT");
      
    } else {
 
      Serial.print("Nepodarilo sa pripojit k MQTT serveru");
      Serial.print(client.state());                               // Vypíše dôvod nepripojenia sa 
      delay(2000);                                                // Ak sa nepripojí k MQTT zopakuje to o 2S
 
    }
  }
}
// ---------------------------------------------------------- Pripojenie Wi-fi  ----------------------------------------------------------------------------
void pripojenie_wifi() {
  
  Serial.println();                                //Prázdny riadok
  Serial.print("Pripajanie k ");
  Serial.println(meno_wifi);                       //Vypíše meno wifi

  WiFi.begin(meno_wifi, heslo_wifi);              //Začiatok pripajania sa k Wifi

  while (WiFi.status() != WL_CONNECTED) {         //Pokým nie je pripojeny k wifi každú pol sekundu načitava bodku.
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi pripojene");
  Serial.println("IP adresa zariadenia ESP32: ");
  Serial.println(WiFi.localIP());                   //Vypíše lokalnu IP adresu ESP32
}
// ---------------------------------------------------------------------- Zobudenie zariadenia --------------------------------------------------------------- 
void dovod_zobudenia_zariadenia(){
  esp_sleep_wakeup_cause_t zobudenie_dovod;

  zobudenie_dovod = esp_sleep_get_wakeup_cause();

  switch(zobudenie_dovod)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Schranka sa otvorila"); break;

    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Uplynul cas hlbokeho spanku"); break;   // max 71 min
    
    default : Serial.printf("Dovod zobudenia: %d\n",zobudenie_dovod); break;
  }
}



// ---------------------------------------------------------------------- Nastavenie --------------------------------------------------------------- 

void setup() {
  
  Serial.begin(115200);                    // Nastavenie rýchlosti seriálovej komunikácie
  
  pripojenie_wifi();                       //Privolanie funkcie pripojenia sa wifi
  
  mqtt_pripojenie();                        // Privolanie funkcie pripojenia sa k MQTT serveru
  

  //Print the wakeup reason for ESP32
  dovod_zobudenia_zariadenia();               //Vypíše dôvod zobudenia
                                    
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0); //1 = High, 0 = Low   
  
  
  pinMode(schranka_senzor_GPIO_PIN, INPUT_PULLUP); // Nastavenie ESP32 pin vstupu na pull-up mode
  status_schranky = digitalRead(schranka_senzor_GPIO_PIN); // Vyčíta stav GPIO pinu 4
  
  if (status_schranky == LOW) 
  {
    
    Serial.println("Prisla posta");
    client.publish(topic_esp32, "1");   //Odošle na MQTT server správu o hodnote 1 na topic ESP32/Schranka
    delay(5000);                        //Čaká 5 s na to aby sa zavrela schránka a neprišla duplicitná správa
 
   }


// ---------------------------------------------------------------------- SLEEP --------------------------------------------------------------- 

  Serial.println("Ide do hlbokeho spanku");
  
  esp_deep_sleep_start();                         //Začiatok hlbokého spánku

}
void loop(){     // Nekonečná sučka, nekonečnú slučku nevyužívame pretože sa zariadenie vždy reštartuje,program sa spustí vždy od začiatu
}
