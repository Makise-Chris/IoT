#include <WiFi.h>
#include <PubSubClient.h>
#define PUB "esp32/on"
// WiFi
const char *ssid = "Saber"; // Enter your WiFi name
const char *password = "itco5568";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/132667/healthcheck";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

bool requestData = false;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length);

void setup() {
 // Set software serial baud to 115200;
 Serial.begin(115200);
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.publish(topic, "{\"Id\": \"E123458\",\"heartRate\": \"999\",\"bodyTemperature\": \"999\",\"bloodPressure\": \"102\"}");
 client.subscribe("esp32/on");
}

void callback(char *topic, byte *payload, unsigned int length) {
    char status[20];
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        status[i] = payload[i];
    }
    Serial.println(status);
    Serial.println();
    if(String(topic) == PUB){
        if(String(status) == "send"){
            requestData = true;
        }
    }
    Serial.println("-----------------------");
}

void loop() {
    //client.subscribe("esp32/on");
    client.loop();
    // if(requestData){
    //     client.publish(topic, "{\"Id\": \"E123458\",\"heartRate\": \"999\",\"bodyTemperature\": \"101\",\"bloodPressure\": \"102\"}");
    //     requestData = false;
    // }
}