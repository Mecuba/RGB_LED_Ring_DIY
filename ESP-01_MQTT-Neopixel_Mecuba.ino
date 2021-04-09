#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>


#define NEO_PIN 3

#define NUM_LEDS 60


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Ajustar de acuerdo a tu red WiFi
const char* ssid = "TuRedWiFiAqui";
const char* password = "Contrasena_superDificil123";

const char* mqtt_server = "test.mosquitto.org"; //Éste es un servidor gratuito pero público, para cosas sencillas está bien.

//Variables a utilizar
float red = 0.0;
float green = 0.0;
float blue = 0.0;
float white = 0.0;
bool fading = 0;
int targetN = 0;
int targetRed, targetBlue, targetGreen, targetWhite;
String topicString = "change/this/topic"; //Aquí poner el tópico de MQTT que tú usarás

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  //Serial.begin(115200); //Desactivamos todo el puerto serial para usar ese pin del ESP-01. Para otros ESP sí pueden funcionar.
  //Serial.println("Hola a todos");
  delay(1000);
  pinMode(1, OUTPUT);
  
  digitalWrite(1, LOW);   //Secuencia para verificar que encendió bien el ESP-01
  delay(1000);                      
  digitalWrite(1, HIGH);  
  delay(1000);       

  digitalWrite(1, LOW);   
  delay(1000);                      
  digitalWrite(1, HIGH);  
  delay(1000); 

  digitalWrite(1, LOW);   
  delay(1000);                      
  digitalWrite(1, HIGH);  
  delay(1000); 
  
  strip.begin();  //Se inicializa la tira LED
  strip.setPixelColor(0, strip.Color(100, 0, 0, 0)); //Verificar que esté bien conectado, luz en rojo

  strip.show(); // Initialize all pixels to 'off'
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());

  
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void callback(char* topic, byte* payload, unsigned int length) { //Recibe los mensajes un caracter a la vez, los une
  String message = "";
  String r;
  String g;
  String b;

  //Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  //Serial.println();

  String thistopic = String(topic);

  // Los mensajes deberan llegar de la forma "rgb(rrr, ggg, bbb)"
  if (thistopic == topicString) {

    r = getValue(message, ',', 0); //Separa lo primero hasta la primer coma
    targetRed = r.substring(4).toInt(); //Reconoce a partir del caracter 4

    g = getValue(message, ',', 1); //Separa lo restante hasta la segunda coma
    targetGreen = g.toInt();

    b = getValue(message, ',', 2); //Separa lo restante hasta el paréntesis final
    b = getValue(b, ')', 0);
    targetBlue = b.toInt();

    targetN = 0; //Parche temporal que resultó permanente

    if (targetN > 0) {
      fading = true;
    } else {
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, targetRed, targetGreen, targetBlue, targetWhite);
      }
      strip.show();
      red = targetRed;
      green = targetGreen;
      blue = targetBlue;
      white = targetWhite;
    }
    
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("aroluz", topicString, 0, false, "disconnected")) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topicString, "connected");
      // ... and resubscribe
      client.subscribe(topicString);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) { //Asegurar la conexión MQTT
    reconnect();
  }
  
  client.loop();
  yield();

}
