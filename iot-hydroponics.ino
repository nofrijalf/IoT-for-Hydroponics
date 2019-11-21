#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <Wire.h>
#include <analogWrite.h>

#define addr 0x23;
BH1750 lightMeter;

const char* ssid = "Nantenezumi";
const char* password = "pornhub123";
const char* mqtt_server = "192.168.1.1";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int resval = 0;
int resPin = 34;
float panjangSensor = 4.0;

// output Pin
int ledPin = 27;
int lampu = 32;
int pompa2 = 33;

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  pinMode(lampu, OUTPUT);
  pinMode(pompa2, OUTPUT);
  pinMode(lampu, OUTPUT);
  Wire.begin();

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));
}

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "Lamp") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(lampu, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(lampu, LOW);
    }
  }

  if (String(topic) == "Pump") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(pompa2, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(pompa2, LOW);
    }
  }

  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("Lamp");
      client.subscribe("Pump");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    resval = analogRead(resPin);
    float tinggiAir = resval * panjangSensor / 4095;
    if (tinggiAir <= 0.5) {
      Serial.println("Volume air : Empty ");
    }
    else if (tinggiAir > 0.5 && tinggiAir <= 2) {
      Serial.println("Volume air : Low ");
    }
    else if (tinggiAir > 2 && tinggiAir <= 3) {
      Serial.println("Volume air : Medium ");
    }
    else if (tinggiAir > 3) {
      Serial.println("Volume air : High ");
    }
    char watString[8];
    dtostrf(tinggiAir, 1, 2, watString);
    Serial.print("Water: ");
    Serial.println(watString);
    client.publish("Water", watString);

    float lux = lightMeter.readLightLevel();
    float sinyalux = (255. / 4095.) * lux;
    char luxString[8];
    dtostrf(sinyalux, 1, 2, luxString);
    Serial.print("Intensitas: ");
    Serial.println(luxString);
    client.publish("Intensitas", luxString);

    if (sinyalux <= 1 ) {
      digitalWrite(lampu, HIGH);
    }
    else if (sinyalux >= 15) {
      digitalWrite(lampu, LOW);
    }



    if (tinggiAir <= 1.5) {
      digitalWrite(pompa2, HIGH);
    }
    else if (tinggiAir >= 3.6) {
      digitalWrite(pompa2, LOW);
    }
  }
}
