
#include <WiFi.h>

const char* ssid = "tothelobbyyougo";
const char* password = "fuzzyocean086";



void connect_to_WiFi(){
  unsigned long run_time = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.print("Connecting...");
  while(WiFi.status() != WL_CONNECTED && millis() - run_time < 60000){
    Serial.print('.');
    delay(1000);
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nconnection code: "); // 3|ok 4|all atempts failed 5|connection lost  6|disconnected
    Serial.print(WiFi.status());
    Serial.println(WiFi.SSID());
    Serial.print("IP address : ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.println(WiFi.RSSI());
  }else {
    try_to_reconnect(ssid,password);
    delay(10000); 
  }
  
}

void try_to_reconnect(const char* ssid, const char* password) {
  Serial.println("attempting to reconnect...");
  WiFi.begin(ssid, password);

  unsigned long reconnectStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - reconnectStart < 30000) {
    Serial.print('.');
    delay(1000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nreconnected successfully!");
    Serial.print("New IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nreconnection failed! Will retry after 10 sec!");
  }
}

void scan_WiFi(){
  Serial.print("Scanning...");
  int n = WiFi.scanNetworks();
  Serial.println("Done");
  if (n == 0){
    Serial.println("there is no available WiFi");
  }else {
    Serial.print("available networks: ");
    Serial.print(n);
    for(int i = 0 ; i < n ; ++i) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
      delay(1000);
    }
  }
  Serial.println("");

  delay(8000);
}


void setup() {
  Serial.begin(115200);
  connect_to_WiFi();
  scan_WiFi();


}

void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("connection has been lost! reconnecting...");
    try_to_reconnect(ssid, password);
  }
  delay(10000); //constant check 
}