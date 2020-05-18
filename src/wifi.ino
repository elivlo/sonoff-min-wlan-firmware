#include <ESP8266WiFi.h>

#define version 0.1 

#ifndef STASSID
#define STASSID "SSID" // Needs to be edited
#define STAPSK  "password" // Needs to be edited
#endif

#define S_CONN 2
#define S_RCONN 3
#define S_ON 1
#define S_OFF 0


#define LED 13
#define RELAY 12
#define BUTTON 0

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiServer wifiServer(80);

unsigned long l_on = 0;
unsigned long l_off = 0;
unsigned long but_hit = 0;
unsigned long l_check = 0;

unsigned int freq = 25;

byte led = S_OFF;
byte state = S_OFF;
byte relay = S_OFF;
byte push = 0;

void led_f(long t){

  if(state==S_CONN){
    if(led == S_OFF && (t - l_off) > 500){
      led = S_ON;
      l_on = t;
    }
    if(led == S_ON && (t - l_on) > 500){
      led = S_OFF;
      l_off = t;
    }
  }else if(relay==S_ON){
    led = S_ON;
  }else if(relay==S_OFF){
    if(led == S_OFF && (t - l_off) > 5000){
      led = S_ON;
      l_on = t;
    }
    if(led == S_ON && (t - l_on) > 100){
      led = S_OFF;
      l_off = t;
    }
  }
}

void button_f(long t){
  int b = digitalRead(BUTTON);

  if (b == LOW && (t - but_hit) > 175) {    
    push++;
    but_hit = t;
  }
  

  if (push > 0 && b == HIGH && (t - but_hit) > 700 ) {

    if(push == 1){
      if (relay == S_OFF){
        relay = S_ON;
      }
      else if (relay == S_ON){
        relay = S_OFF;
      }
    } else if(push == 2){
      //TODO: neue Eingabe!
    }
    push = 0;
  }
}

void outs_f(){

  //RELAY control
  if(relay == S_ON){
    digitalWrite(RELAY, HIGH);
  } else {
    digitalWrite(RELAY, LOW);
  }

  //LED control
  if(led == S_ON){
    digitalWrite(LED, LOW);
  } else {
    digitalWrite(LED, HIGH);
  }
  
}

void server_f(){
  WiFiClient client = wifiServer.available();
    
  if (client) {
 
    if (client.connected()) {
 
      if (client.available()>0) {
        byte c = client.read();
        if(c == 1){
          client.write(1);
          relay = S_ON;
        } else if(c == 2){
          client.write(0);
          relay = S_OFF;
        } else if(c == 3){
          if(relay == S_ON){
            client.write(1);
          }else{
            client.write(0);
          }
        }
      }
    } else
        client.stop();
  }
}

void setup_f(){
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(RELAY, LOW);


  //WIFI
  IPAddress staticIP(192,168,0,202);
  IPAddress gateway(192,168,0,1);
  IPAddress subnet(255,255,0,0);

  //WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, IPAddress(8, 8, 8, 8), gateway, subnet);
  WiFi.hostname("aussen-lampe-HL");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
  }

  //Network
  wifiServer.begin();
  
  l_off = millis();
  l_check = l_off;
  state = S_ON;
  relay = S_OFF;
  led = S_OFF;
}

//-----Setup
void setup() {
  setup_f();
}

//reset Function
void(* resetFunc) (void) = 0;

//-----Loop
void loop() {
  unsigned long t = millis();
  if((t-l_check)>5000)if(WiFi.status() != WL_CONNECTED){resetFunc();}else {state = S_ON; l_check=t;}
  
  
  button_f(t);
  server_f();
  led_f(t);
  outs_f();
  
  
  long a = (1000/freq) - (millis()-t);
  if(a>0)delay(a);
}
