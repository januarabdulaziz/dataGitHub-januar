#include <ESP8266WiFi.h>
// #include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTTYPE DHT22
// #define dhtPin 33 // esp32
// #define buzzerPin 23 // esp32
#define dhtPin 5 // D1 esp8266
#define buzzerPin 13 // D7 esp8266

DHT dht(dhtPin, DHTTYPE);

float temperature;
float humidity;

// setup wifi
// const char* ssid = "ESP8266-AccessPoint";
// const char* password = "12345678";
// const char* ssid = "Pixel_5729";
// const char* password = "57295729";
const char* ssid = "PAK BUDI";
const char* password = "tenggerraya";

// setup mqtt server dan topik
// const char* mqtt_server = "192.168.4.2";
const char* mqtt_server = "10.68.60.24";
const int mqtt_port = 1883;
const char* topic_status = "espCoba/Home2/S";
const char* topic_msg = "espCoba/Home2/M";
const char* topic_temp = "espCoba/Home2/temp";
const char* topic_hum = "espCoba/Home2/hum";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long value = 0;
String pesan = "";
bool alarmActive = false;
bool buzzerState = HIGH;
unsigned long prevMillis = 0;
const long interval = 500;

// Tentukan IP statis yang diinginkan
// IPAddress local_IP(192, 168, 4, 11);  // IP yang ingin dipasang pada ESP32
// IPAddress gateway(192, 168, 4, 1);    // Gateway (biasanya router)
// IPAddress subnet(255, 255, 255, 0);   // Subnet mask
// IPAddress DNS(192, 168, 4, 1);        // DNS primer (opsional)

IPAddress local_IP(10, 68, 60, 102);
IPAddress gateway(10, 68, 60, 198);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(10, 68, 60, 198);

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
    digitalWrite(buzzerPin, LOW); delay(250);
    digitalWrite(buzzerPin, HIGH); delay(250);
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
      // Once connected, publish an announcement...
      client.publish(topic_status, "hello world of MQTT");
      // ... and resubscribe
      client.subscribe(topic_msg);
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
  Serial.println("Hello ESP8266");
  pinMode(dhtPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();

  Serial.println("Ready!");
  digitalWrite(buzzerPin, LOW);
  delay(1000);
  digitalWrite(buzzerPin, HIGH);
  pesan = "OFF";
}

void loop() {
  // put your main code here, to run repeatedly:
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if(!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if(now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    client.publish(topic_status, String(value).c_str());
    client.publish(topic_temp, String(temperature).c_str());
    client.publish(topic_hum, String(humidity).c_str());
  }

  if(pesan == "ON") {
    alarmActive = true;
  }
  else {
    alarmActive = false;
    digitalWrite(buzzerPin, HIGH);
  }

  if(alarmActive) {
    if(now - prevMillis > interval) {
      prevMillis = now;
      // toggle state
      buzzerState = !buzzerState;
      digitalWrite(buzzerPin, buzzerState);
    }
  }

  Serial.print("Status MQTT : "); Serial.println(value);
  Serial.print("Temperature : "); Serial.println(temperature);
  Serial.print("Humidity    : "); Serial.println(humidity);
  Serial.println("======END======");
  delay(100);

}
