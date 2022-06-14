#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>

#define wifi "Saber"
#define password "itco5568"
#define token "BBFF-YJA7zO68bjYHVgFsDpSCeEkcIDMrdq"
#define mqttClient "mymqttclient"
#define variableLabel "Sensor_Data1"
#define deviceLabel "Health_Monitor"
#define sensorPin A0

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[10000];
char topic[150];
// Space to store values to send
char str_sensor[10];
char str_millis[20];
double epochseconds = 0;
double epochmilliseconds = 0;
double current_millis = 0;
double current_millis_at_sensordata = 0;
double timestampp = 0;
int j = 0;


int UpperThreshold = 2400;
int LowerThreshold = 2350;
int reading = 0;
float BPM = 0.0;
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;

/****************************************
   Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attemp to connect
    if (client.connect(mqttClient, token, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(wifi, password);
  // Assign the pin as INPUT
  pinMode(sensorPin, INPUT);
  Serial.println();
  Serial.print("Waiting for WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  timeClient.update();
  epochseconds = timeClient.getEpochTime();
  epochmilliseconds = epochseconds * 1000;
  Serial.print("epochmilliseconds=");
  Serial.println(epochmilliseconds);
  current_millis = millis();
  Serial.print("current_millis=");
  Serial.println(current_millis);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
    j = 0;
  }
  //sprintf(payload, "%s", "{\"ECG_Sensor_data\": [{\"value\":1234, \"timestamp\": 1595972075},{\"value\":1111, \"timestamp\": 1595971075},{\"value\":2222, \"timestamp\": 1595970075}]}");
  j = j + 1;
  Serial.print("j=");
  Serial.println(j);
  sprintf(topic, "%s%s", "/v1.6/devices/", deviceLabel);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\": [", variableLabel); // Adds the variable label
  /*for (int i = 1; i <= 3; i++)
  {
    float sensor = analogRead(sensorPin);
    dtostrf(sensor, 4, 2, str_sensor);
    sprintf(payload, "%s{\"value\":", payload); // Adds the value
    sprintf(payload, "%s %s,", payload, str_sensor); // Adds the value
    current_millis_at_sensordata = millis();
    timestampp = epochmilliseconds + (current_millis_at_sensordata - current_millis);
    dtostrf(timestampp, 10, 0, str_millis);
    sprintf(payload, "%s \"timestamp\": %s},", payload, str_millis); // Adds the value
    delay(150);
  }*/
  float sensor = analogRead(sensorPin);
  dtostrf(sensor, 4, 2, str_sensor);
  current_millis_at_sensordata = millis();
  timestampp = epochmilliseconds + (current_millis_at_sensordata - current_millis);
  dtostrf(timestampp, 10, 0, str_millis);
  //sprintf(payload, "%s{\"value\":%s, \"timestamp\": %s}]}", payload, str_sensor, str_millis);


  if(sensor > UpperThreshold && IgnoreReading == false){
    if(FirstPulseDetected == false){
      FirstPulseTime = millis();
      FirstPulseDetected = true;
    }
    else{
      SecondPulseTime = millis();
      PulseInterval = SecondPulseTime - FirstPulseTime;
      FirstPulseTime = SecondPulseTime;
    }
    IgnoreReading = true;
  }

  if(sensor < LowerThreshold){
    IgnoreReading = false;
  }  

  BPM = (1.0/PulseInterval) * 60.0 * 1000;

  Serial.print(sensor);
  Serial.print("\t");
  Serial.print(PulseInterval);
  Serial.print("\t");
  Serial.print(BPM);
  Serial.println(" BPM");
  Serial.flush();


  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  Serial.println(payload);
  // client.loop();
}