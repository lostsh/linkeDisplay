#include "arduino_secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <LiquidCrystal.h>

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;

int status = WL_IDLE_STATUS;

WiFiClient client;

const int rs=12, en=11, d4=0, d5=1, d6=2, d7=3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// server address:
char server[] = "ztHost.home";

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;

void setup() {
  lcd.begin(16,2);
  Serial.begin(9600);
  delay(5000);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected print out the status:
  printWifiStatus();
  displayWifiStatus();
}


String text = "";
void loop() {
  
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
    text.concat(c);
  }
  
  if (millis() - lastConnectionTime > postingInterval) {
    displayData(text);
    text = "";
    httpRequest();
  }
}

void httpRequest() {
  client.stop();
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print("connecting...");
    // send the HTTP PUT request:
    client.println("GET /me.txt HTTP/1.1");
    client.println("Host: ztHost.home");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    Serial.println("connection failed");
    lcd.println("connection failed");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void displayWifiStatus(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SSID: ");
  lcd.setCursor(0,1);
  lcd.print(WiFi.SSID());
  delay(3000);

  lcd.clear();
  lcd.setCursor(0,0);
  IPAddress ip = WiFi.localIP();
  lcd.print("IP Address: ");
  lcd.setCursor(0,1);
  lcd.print(ip);
  delay(3000);
}

//afficher les donnes recues sur la ligne du bas
void displayData(String texte){
  lcd.clear();
  lcd.setCursor(0,0);
  Serial.println("Date : \n---["+extractDate(texte)+"]---");
  lcd.print(extractDate(texte));
  lcd.setCursor(0,1);
  Serial.println("---["+extractData(texte)+"]---");
  lcd.print(extractData(texte));
  delay(100);
}

/*
 *Recupere les informations a partir de la chaine retourne par le client
 *Et renvoi ce qui se trouve apres l'entete
 *il faut encore l'amelirorer !!
 */
String extractData(String brutIncomingData){
  int size = brutIncomingData.length();
  char text[size];
  brutIncomingData.toCharArray(text, size);
  int indexEndReqInfo = 0;
  for(int i=0;i<=size;i++){
    if(text[i]=='\n'){
      indexEndReqInfo = i;
    }
  }
  return brutIncomingData.substring(indexEndReqInfo, size);
}

/*
 * Get the actual date from the result of client.read()
*/
String extractDate(String brutIncomingData){
  int begDateIndex = 0;
  int i=0;
  while(begDateIndex==0 && i<brutIncomingData.length()){
    if(brutIncomingData.substring(i,i+5)=="Date:"){
      begDateIndex = i+6;
    }
    i++;
  }
  int endDateIndex = brutIncomingData.indexOf('\n',begDateIndex);
  return brutIncomingData.substring(begDateIndex, endDateIndex);
}