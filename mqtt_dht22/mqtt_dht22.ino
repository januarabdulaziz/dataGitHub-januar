// #include <WiFi.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTTYPE DHT11
// #define DHTTYPE DHT22

// #define ledPin 13
// #define buzzerPin 12

#define dhtPin 13

DHT dht(dhtPin, DHTTYPE);

float suhu;
float kelembapan;

const char* ssid = "PAK BUDI";
const char* password = "tenggerraya";
// const char* ssid = "Pixel_5729";
// const char* password = "57295729";
// const char* ssid = "narzo 50i 123492";
// const char* password = "11333399";
// const char* mqtt_server = "test.mosquitto.org";
// const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_server = "192.168.0.110";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
char temp[MSG_BUFFER_SIZE];
char hum[MSG_BUFFER_SIZE];
unsigned short value = 0;
String pesan = "";

IPAddress local_IP(192, 168, 0, 106);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(8, 8, 8, 8);

void setup_wifi() {

  delay(10);
  // PENTING: panggil WiFi.config sebelum WiFi.begin
  if (!WiFi.config(local_IP, gateway, subnet, DNS)) {
    Serial.println("Konfigurasi IP statis GAGAL. Mungkin driver tidak mendukung.");
  } else {
    Serial.println("Konfigurasi IP statis diterapkan (WiFi.config returned true).");
  }
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.println("DNS: ");
  Serial.println(WiFi.dnsIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP-Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.println(clientId.c_str());
      // Once connected, publish an announcement...
      // client.publish("espCoba/DHT/S", "hello world");
      // ... and resubscribe
      client.subscribe("espCoba/DHT1/M");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  pesan = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    pesan += (char)payload[i];
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  // pinMode(ledPin, OUTPUT);
  // pinMode(buzzerPin, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();

  Serial.println("Sistem siap...");
  Serial.println("========================");
  // digitalWrite(ledPin, HIGH);
  // digitalWrite(buzzerPin, HIGH);
  delay(3000);
  // digitalWrite(ledPin, LOW);
  // digitalWrite(buzzerPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  // nilaiGetaran = analogRead(inPotensio);

  suhu = dht.readTemperature();
  kelembapan = dht.readHumidity();

  // if (pesan == "ON") {
  //   digitalWrite(ledPin, HIGH);
  //   digitalWrite(buzzerPin, HIGH);
  //   delay(500);
  //   digitalWrite(ledPin, LOW);
  //   digitalWrite(buzzerPin, LOW);
  //   delay(500);
  // }
  // else if (pesan == "OFF") {
  //   digitalWrite(ledPin, LOW);
  // }
  // else {
  //   digitalWrite(ledPin, LOW);
  //   digitalWrite(buzzerPin, LOW);
  // }

  Serial.print("Suhu: ");
  Serial.println(suhu);
  Serial.print("Kelembapan: ");
  Serial.println(kelembapan);
  // Serial.println("======END======");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    char payload[128];

    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    snprintf(temp, MSG_BUFFER_SIZE, "%.2f", suhu);
    snprintf(hum, MSG_BUFFER_SIZE, "%.2f", kelembapan);
    snprintf(payload, sizeof(payload), "{\"status\":%d,\"suhu\":%.2f,\"kelembapan\":%.2f}", value, suhu, kelembapan);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("espCoba/DHT1/S", msg);

    Serial.print("Pub_Temp: ");
    Serial.println(temp);
    client.publish("espCoba/DHT1/temp", temp);

    Serial.print("Pub_Hum: ");
    Serial.println(hum);
    client.publish("espCoba/DHT1/hum", hum);

    Serial.print("Pub_Payload: ");
    Serial.println(payload);
    client.publish("espCoba/DHT1/payload", payload);
  }
  Serial.println("======END======");
  delay(500);
}