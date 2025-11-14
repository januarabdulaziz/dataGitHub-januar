#include <WiFi.h>
#include <PubSubClient.h>
#include <DFRobot_SHT40.h>

#define mq2Pin 32
#define ledHPin 33
#define ledMPin 25
#define buzzerPin 26

DFRobot_SHT40 SHT40(SHT40_AD1B_IIC_ADDR);

uint32_t id = 0;
float temperature;
float humidity;
float mq2;

// setup wifi
const char* ssid = "Elitech";
const char* password = "wifis1nko";

// setup mqtt server dan topik
const char* mqtt_server = "192.168.13.17";
const int mqtt_port = 1883;
const char* topic_status = "espS1nko/suhu4/S";
const char* topic_msg = "espS1nko/suhu4/M";
const char* topic_temp = "espS1nko/suhu4/temp";
const char* topic_hum = "espS1nko/suhu4/hum";
const char* topic_smoke = "espS1nko/suhu4/asap";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long value = 0;
String pesan = "";
bool alarmActive = false;
bool buzzerState = LOW;
unsigned long prevMillis = 0;
const long interval = 500;

// Tentukan IP statis yang diinginkan
IPAddress local_IP(192, 168, 14, 108);    // IP yang ingin dipasang pada ESP32
IPAddress gateway(192, 168, 13, 1);       // Gateway (biasanya router)
IPAddress subnet(255, 255, 252, 0);       // Subnet mask
IPAddress DNS(8, 8, 8, 8);                // DNS primer (opsional)

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
    digitalWrite(ledHPin, HIGH); delay(250);
    digitalWrite(ledHPin, LOW); delay(250);
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
  Serial.begin(115200);
  pinMode(mq2Pin, INPUT);
  pinMode(ledHPin, OUTPUT);
  pinMode(ledMPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  SHT40.begin();

  while((id = SHT40.getDeviceID()) == 0){
    Serial.println("ID retrieval error, please check whether the device is connected correctly!!!");
    delay(1000);
  }

  Serial.println("Setup");
  digitalWrite(ledMPin, HIGH);
  digitalWrite(ledHPin, LOW);
  delay(20000);
  digitalWrite(ledMPin, LOW);
  digitalWrite(ledHPin, HIGH);
  Serial.println("Done");
  Serial.print("id :0x"); Serial.println(id, HEX);
}

void loop() {
  // put your main code here, to run repeatedly:
  temperature = SHT40.getTemperature(PRECISION_HIGH);
  humidity = SHT40.getHumidity(PRECISION_HIGH);
  mq2 = analogRead(mq2Pin);

  if(temperature == MODE_ERR){
    Serial.println("Incorrect mode configuration to get temperature");
  } else{
    Serial.print("Temperature :"); Serial.print(temperature); Serial.println(" C");
  }
  if(humidity == MODE_ERR){
    Serial.println("The mode for getting humidity was misconfigured");
  } else{
    Serial.print("Humidity :"); Serial.print(humidity); Serial.println(" %RH");
  }
  if(humidity > 80){
    SHT40.enHeater(POWER_CONSUMPTION_H_HEATER_1S);
  }
  Serial.print("Asap : ");
  Serial.println(mq2, 2);
  Serial.println("===END===");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // kirim ke MQTT
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    client.publish(topic_status, String(value).c_str());
    client.publish(topic_temp, String(temperature).c_str());
    client.publish(topic_hum, String(humidity).c_str());
    client.publish(topic_smoke, String(mq2).c_str());
  }

  // led & buzzer
  if(pesan == "ON") {
    alarmActive = true;
  }
  else {
    alarmActive = false;
    digitalWrite(ledMPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
  if(alarmActive) {
    if(now - prevMillis > interval) {
      prevMillis = now;
      // toggle state
      buzzerState = !buzzerState;
      digitalWrite(buzzerPin, buzzerState);
      digitalWrite(ledMPin, buzzerState);
    }
  }

  delay(250);
}