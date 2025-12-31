// ESP8266 Server with OTA Upload Mode
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP_EEPROM.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#define EEPROM_SIZE 100
#define ADDR_MODE        0
#define ADDR_PASSWORD    50
#define ADDR_SSID        1

//////////////////////////
char newSSID[32];
char newPASS[32];

bool ssidReceived = false;
bool passReceived = false;
bool waitingWiFiInfo = true;
////////////////////////////

char ssid[20]     = "JAM_PANEL";
char password[20] = "00000000";

const char* otaSsid = "KELUARGA02";
const char* otaPass = "khusnul23";
const char* otaHost = "SERVER_5";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

IPAddress local_IP(192, 168, 2, 1);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

bool clientReady[5] = { false, false, false, false, false };
bool modeOTA = false;

unsigned long lastTimeSend = 0;
const unsigned long intervalSendTime = 60000; // 1 menit

/////////////////////////////
///////struck Display////////
/////////////////////////////
struct SettingDisplay {
  uint8_t Br;
  uint8_t Sptx1;
  uint8_t Sptx2;
  uint8_t Spdt;
  uint8_t Spnm;
  bool Bzr;
  uint8_t Da;
  int CoHi;
  bool mode=0;
  char newPassword[9]="00000000";
} cacheDisplay;

///////////////////////////
// struck lokasi //////////
//////////////////////////
struct SettingLokasi {
  float Lt;
  float Lo;
  uint8_t Tz;
  uint8_t Al;
} cacheLokasi;

//////////////////////////
////  struck pesan ////
/////////////////////////
struct SettingPesan {
  char message1[100];
  char message2[100];
  char textName[100];
  uint8_t index=1;
} cachePesan;

/////////////////////////
////  struck koreksi  ///
/////////////////////////
struct SettingKoreksi {
  uint8_t SUBUH_KOREKSI;
  uint8_t DHUHUR_KOREKSI;
  uint8_t ASHAR_KOREKSI;
  uint8_t MAGHRIB_KOREKSI;
  uint8_t ISYA_KOREKSI;
} cacheKoreksi;

/////////////////////////
////  struck iqomah  ///
/////////////////////////
struct SettingIqomah {
  uint8_t SUBUH_IQOMAH;
  uint8_t DHUHUR_IQOMAH;
  uint8_t ASHAR_IQOMAH;
  uint8_t MAGHRIB_IQOMAH;
  uint8_t ISYA_IQOMAH;

  uint8_t SUBUH_BLINK;
  uint8_t DHUHUR_BLINK;
  uint8_t ASHAR_BLINK;
  uint8_t MAGHRIB_BLINK;
  uint8_t ISYA_BLINK;
} cacheIqomah;

String DEVICE_ID;

void getData(String input) {
  Serial.println(input);
  // Di sini bisa tambahkan pengolahan data lebih lanjut
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      clientReady[num] = false;
      break;
    case WStype_DISCONNECTED:
      clientReady[num] = false;
      break;
    case WStype_TEXT: {
      String msg = String((char*)payload);
      if (msg == "CLIENT_READY") {
        clientReady[num] = true;
      } else if (msg == "restart") {
        getData(msg + "=1");
        delay(500);
        ESP.restart();
      } else if (msg == "jadwal") {
        getData(msg + "=1");
      }else {
        getData(msg);
      }
      break;
    }
  }
}

void handleGetID() {
  String json = "device_id = " + DEVICE_ID;
  server.send(200, "application/json", json);
}

///// handle display ///////
void handleGetDisplay() {
  cekInput("REQ:DISPLAY");
  String json = "{";

  json += "\"device_id\":\"" + DEVICE_ID + "\",";

  json += "\"Br\":" + String(cacheDisplay.Br) + ",";
  json += "\"Sptx1\":" + String(cacheDisplay.Sptx1) + ",";
  json += "\"Sptx2\":" + String(cacheDisplay.Sptx2) + ",";
  json += "\"Spdt\":" + String(cacheDisplay.Spdt) + ",";
  json += "\"Spnm\":" + String(cacheDisplay.Spnm) + ",";
  json += "\"Bzr\":" + String(cacheDisplay.Bzr) + ",";
  json += "\"Da\":" + String(cacheDisplay.Da) + ",";
  json += "\"CoHi\":" + String(cacheDisplay.CoHi) + ",";
  json += "\"newPassword\":\"" + String(cacheDisplay.newPassword) + "\",";
  json += "\"mode\":" + String(cacheDisplay.mode);

  json += "}";

  server.send(200, "application/json", json);
  Serial.println("display ready");
}

//// handle lokasi ////
void handleGetLokasi() {
  cekInput("REQ:LOKASI");
  String json = "{";
  json += "\"device_id\":\"" + DEVICE_ID + "\",";
  json += "\"Lt\":" + String(cacheLokasi.Lt, 6) + ",";
  json += "\"Lo\":" + String(cacheLokasi.Lo, 6) + ",";
  json += "\"Tz\":" + String(cacheLokasi.Tz) + ",";
  json += "\"Al\":" + String(cacheLokasi.Al);

  json += "}";

  server.send(200, "application/json", json);
}

//// handle pesan ////
String jsonEscape(const char* src) {
  String out;
  while (*src) {
    char c = *src++;
    switch (c) {
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n";  break;
      case '\r': out += "\\r";  break;
      case '\t': out += "\\t";  break;
      default:
        if ((uint8_t)c < 0x20) {
          // karakter kontrol lain diabaikan
        } else {
          out += c;
        }
    }
  }
  return out;
}

void handleGetPesan() {
  cekInput("REQ:PESAN");
  String json = "{";
  json += "\"device_id\":\"" + DEVICE_ID + "\",";
  json += "\"message1\":\"";
  json += jsonEscape(cachePesan.message1);
  json += "\",";

  json += "\"message2\":\"";
  json += jsonEscape(cachePesan.message2);
  json += "\",";

  json += "\"textName\":\"";
  json += jsonEscape(cachePesan.textName);
  json += "\",";

  json += "\"index\":" + String(cachePesan.index);

  json += "}";

  server.send(200, "application/json", json);
}


void handleGetKoreksi() { 
  cekInput("REQ:KOREKSI");
  String json = "{"; 
  json += "\"device_id\":\"" + DEVICE_ID + "\","; 
  json += "\"SUBUH_KOREKSI\":" + String(cacheKoreksi.SUBUH_KOREKSI) + ","; 
  json += "\"DHUHUR_KOREKSI\":" + String(cacheKoreksi.DHUHUR_KOREKSI) + ","; 
  json += "\"ASHAR_KOREKSI\":" + String(cacheKoreksi.ASHAR_KOREKSI) + ","; 
  json += "\"MAGHRIB_KOREKSI\":" + String(cacheKoreksi.MAGHRIB_KOREKSI) + ","; 
  json += "\"ISYA_KOREKSI\":" + String(cacheKoreksi.ISYA_KOREKSI); 
  json += "}"; server.send(200, "application/json", 
  json); }

////  handle iqomah  ////
void handleGetIqomah() {
  cekInput("REQ:IQOMAH");
  String json = "{";
  json += "\"device_id\":\"" + DEVICE_ID + "\",";
  // === IQOMAH ===
  json += "\"SUBUH_IQOMAH\":"   + String(cacheIqomah.SUBUH_IQOMAH)   + ",";
  json += "\"DHUHUR_IQOMAH\":"  + String(cacheIqomah.DHUHUR_IQOMAH)  + ",";
  json += "\"ASHAR_IQOMAH\":"   + String(cacheIqomah.ASHAR_IQOMAH)   + ",";
  json += "\"MAGHRIB_IQOMAH\":" + String(cacheIqomah.MAGHRIB_IQOMAH) + ",";
  json += "\"ISYA_IQOMAH\":"    + String(cacheIqomah.ISYA_IQOMAH)    + ",";

  // === BLINK ===
  json += "\"SUBUH_BLINK\":"    + String(cacheIqomah.SUBUH_BLINK)    + ",";
  json += "\"DHUHUR_BLINK\":"   + String(cacheIqomah.DHUHUR_BLINK)   + ",";
  json += "\"ASHAR_BLINK\":"    + String(cacheIqomah.ASHAR_BLINK)    + ",";
  json += "\"MAGHRIB_BLINK\":"  + String(cacheIqomah.MAGHRIB_BLINK)  + ",";
  json += "\"ISYA_BLINK\":"     + String(cacheIqomah.ISYA_BLINK);

  json += "}";

  server.send(200, "application/json", json);
}


void handleSetTime() {
  String data = "";
   if (server.hasArg("Tm")) {
    data = server.arg("Tm");
    data = "Tm=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Settingan jam berhasil diupdate");
  }
  if (server.hasArg("text")) {
    data = server.arg("text");
    data = "text=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Settingan text berhasil diupdate");
  }
  if (server.hasArg("name")) {
    data = server.arg("name");
    data = "name=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Settingan nama berhasil diupdate");
  }
  if (server.hasArg("Br")) {
    data  = server.arg("Br");
    data = "Br=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Kecerahan berhasil diupdate");
  }
  if (server.hasArg("Spdt")) {
    data = server.arg("Spdt"); // Atur kecepatan date
    data = "Spdt=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Kecepatan kalender berhasil diupdate");
  }
  if (server.hasArg("Sptx1")) {
    data = server.arg("Sptx1"); // Atur kecepatan text
    data = "Sptx1=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Kecepatan info 1 berhasil diupdate");
  }
  if (server.hasArg("Sptx2")) {
    data = server.arg("Sptx2"); // Atur kecepatan text
    data = "Sptx2=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Kecepatan info 2 berhasil diupdate");
  }
  if (server.hasArg("Spnm")) {
    data = server.arg("Spnm"); // Atur kecepatan text
    data = "Spnm=" + data;
   // Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Kecepatan nama berhasil diupdate");
  }
  if (server.hasArg("Iq")) {
    data = server.arg("Iq"); // Atur koreksi iqomah
    data = "Iq=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"iqomah diupdate");
  }
  if (server.hasArg("Dy")) {
    data = server.arg("Dy"); // Atur durasi adzan
    data = "Dy=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"displayBlink diupdate");
  }
  if (server.hasArg("Kr")) {
    data = server.arg("Kr"); // Atur koreksi waktu jadwal sholat
    data = "Kr=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"Selisih jadwal sholat diupdate");
  }
  if (server.hasArg("Lt")) {
    data = server.arg("Lt"); // Atur latitude
    data = "Lt=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"latitude diupdate");
  }
  if (server.hasArg("Lo")) {
    data = server.arg("Lo"); // Atur latitude
    data = "Lo=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"longitude diupdate");
  }
  if (server.hasArg("Tz")) {
    data = server.arg("Tz"); // Atur latitude
    data = "Tz=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"timezone diupdate");
  }
  if (server.hasArg("Al")) {
    data = server.arg("Al"); // Atur latitude
    data = "Al=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"altitude diupdate");
  }
  if (server.hasArg("Da")) { 
    data = server.arg("Da"); 
    data = "Da=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");// "durasi adzan diupdate");
  }
  if (server.hasArg("CoHi")) {
    data = server.arg("CoHi"); // Atur latitude    data = "CoHi=" + data;

    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain", "OK");//"coreksi hijriah diupdate");
  }

  if (server.hasArg("Bzr")) {
    data = server.arg("Bzr"); // Atur status buzzer
    data = "Bzr=" + data;
    //Serial.println(data);
    getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("mode")) {
    data = server.arg("mode"); // Atur status mode
    EEPROM.write(ADDR_MODE, data.toInt());
    EEPROM.commit();
    data = "mode=" + data;
    kirimDataKeClient(data);
    getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
    delay(500);
    ESP.restart();
  }
   if (server.hasArg("PLAY")) {//
    data = server.arg("PLAY"); // Atur status play
    //Serial.println("data mentah: " + data);
    int idx = 0;
    byte folder = getIntPart(data,idx);
    byte file   = getIntPart(data,idx);
    data = "PLAY:" + String(folder) + "," + String(file);
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("PLAD")) {//
    data = server.arg("PLAD"); // Atur status play
    //Serial.println("data mentah: " + data);
    int idx = 0;
    byte file   = getIntPart(data,idx);
    data = "PLAD:" + String(file);
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
   if (server.hasArg("STOP")) {
    data = "STOP";;
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("VOL")) {
    data = server.arg("VOL"); // Atur status mode
    data = "VOL:" + data;
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("HR")) {
    data = server.arg("HR"); // Ambil argumen HR
    kirimDataKeClient("HR:" + data); // (Opsional) Kirim juga ke semua client via WebSocket
    server.send(200, "text/plain", "OK");
  }
  if (server.hasArg("NAMAFILE")) {//
    data = server.arg("NAMAFILE"); // Atur status play
    //Serial.println("data mentah: " + data);
    int idx = 0;
    byte folder = getIntPart(data,idx);
    byte file   = getIntPart(data,idx);
    int durasi = getIntPart(data,idx);
    data = "NAMAFILE:" + String(folder) + "," + String(file)+ "," + String(durasi);
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("ADZAN")) {//
    data = server.arg("ADZAN"); // Atur status play
    int idx = 0;
    byte file = getIntPart(data,idx);
    int durasi   = getIntPart(data,idx);
    data = "ADZAN:" + String(file) + "," + String(durasi);
    kirimDataKeClient(data);
    //getData(data);
    server.send(200, "text/plain","OK");// (stateBuzzer) ? "Suara Diaktifkan" : "Suara Dimatikan");
  }
  if (server.hasArg("status")) {
    server.send(200, "text/plain", "CONNECTED");
  }
 
  if (server.hasArg("newPassword")) {
      data = server.arg("newPassword");
      data = "newPassword=" + data;
      //Serial.println(data);
      getData(data);
      server.send(200, "text/plain","OK");// "Password WiFi diupdate");
    } 
  data="";
  //EEPROM.commit();
}


void AP_init() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  server.on("/setPanel", handleSetTime);
  server.on("/id", handleGetID);
  server.on("/display", handleGetDisplay);
  server.on("/lokasi", handleGetLokasi);
  server.on("/pesan", handleGetPesan);
  server.on("/koreksi", handleGetKoreksi);
  server.on("/iqomah", handleGetIqomah);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

   // (Opsional) baca data tersimpan
  char savedSSID[30];
  char savedPASS[30];
  int data = EEPROM.read(ADDR_MODE);
  EEPROM.get(ADDR_SSID, savedSSID);
  EEPROM.get(ADDR_PASSWORD, savedPASS);
  
  Serial.println("data:" + String(data));
  Serial.println("SSID:" + String(savedSSID));
  Serial.println("PASS:" + String(savedPASS));
}

/*void ONLINE() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(otaSsid, otaPass);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("OTA WiFi gagal. Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname(otaHost);
 
  ArduinoOTA.onEnd([]() {
    Serial.println("restart=1");
    EEPROM.write(ADDR_MODE, 0);
    EEPROM.commit();
    delay(1000);
    ESP.restart();
  });
  
  ArduinoOTA.begin();
  //Serial.println("OTA Ready");
}*/

// ======== ONLINE MODE (OTA / WIFI NORMAL) ========
void ONLINE(const char* wifiSSID, const char* wifiPASS) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPASS);

  Serial.print("Connecting to ");
  Serial.println(wifiSSID);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    ArduinoOTA.begin();
    ArduinoOTA.setHostname(otaHost);
 
    ArduinoOTA.onEnd([]() {
    
    Serial.println("restart=1");
//    EEPROM.write(ADDR_MODE, 1);
//    EEPROM.commit();
    delay(1000);
    ESP.restart();
  });
  
  
  waitingWiFiInfo=false;
  } else {
    Serial.println("\nFailed to connect. Restarting...");
    delay(2000);
    ESP.restart();
  }
}

void kirimDataKeClient(String data) {

  for (uint8_t i = 0; i < 5; i++) {
    if (clientReady[i] && webSocket.clientIsConnected(i)) {
      webSocket.sendTXT(i, data);
    } else {
      clientReady[i] = false;
    }
  }
}

void cekSerialMonitor() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  /* ========= WIFI SETUP ========= */
  if (input.startsWith("WIFI_SSID:")) {
    input.remove(0, 10);
    input.toCharArray(newSSID, sizeof(newSSID));
    ssidReceived = true;
    return;
  }

  if (input.startsWith("WIFI_PASS:")) {
    input.remove(0, 10);
    input.toCharArray(newPASS, sizeof(newPASS));
    passReceived = true;
    return;
  }

  if (ssidReceived && passReceived) {
    waitingWiFiInfo = false;
    ONLINE(newSSID, newPASS);
    return;
  }

  /* ========= RESPONSE DARI JAM ========= */
  if (input.startsWith("DISPLAY:")) {
    input.remove(0, 8);
    parseDisplay(input);
    return;
  }

  if (input.startsWith("LOKASI:")) {
    input.remove(0,7);
    parseLokasi(input);
    return;
  }

  if (input.startsWith("PESAN:")) {
    input.remove(0,6);
    parsePesan(input);
    return;
  }

  if (input.startsWith("KOREKSI:")) {
    input.remove(0,8);
    parseKoreksi(input);
    return;
  }

  if (input.startsWith("IQOMAH:")) {
    input.remove(0,7);
    parseIqomah(input);
    return;
  }


  /* ========= LAINNYA ========= */
  kirimDataKeClient(input);
}


/*
void cekSerialMonitor() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    //Serial.print("[Serial] Kirim ke semua client: ");
    //Serial.println(input);
    if (input.startsWith("WIFI_SSID:")) {
      input.remove(0, 10);
      input.toCharArray(newSSID, sizeof(newSSID));
      ssidReceived = true;
      Serial.println("[Receiver] SSID diterima: " + String(newSSID));
    }
    else if (input.startsWith("WIFI_PASS:")) {
      input.remove(0, 10);
      input.toCharArray(newPASS, sizeof(newPASS));
      passReceived = true;
      Serial.println("[Receiver] Password diterima: " + String(newPASS));
    }

    // Jika sudah lengkap dua-duanya
    if (ssidReceived && passReceived) {
      Serial.println("[Receiver] Menyimpan SSID & Password...");
//      EEPROM.put(ADDR_SSID, newSSID);
//      EEPROM.put(ADDR_PASSWORD, newPASS);
//      EEPROM.write(ADDR_MODE, 0);
//      EEPROM.commit();
      waitingWiFiInfo=false;
      Serial.println("[Receiver] Reconnecting ke WiFi baru...");
      ONLINE(newSSID, newPASS);
    }
    
    for (uint8_t i = 0; i < 5; i++) {
      if (clientReady[i] && webSocket.clientIsConnected(i)) {
        webSocket.sendTXT(i, input);
      }
    }
  }
}*/

void cekInput(String input){
  Serial.println(input);
}

void parseDisplay(String payload) {
  int pos = 0;

  while (pos < payload.length()) {
    int eq = payload.indexOf('=', pos);
    int cm = payload.indexOf(',', pos);
    if (eq == -1) break;
    if (cm == -1) cm = payload.length();

    String key   = payload.substring(pos, eq);
    String value = payload.substring(eq + 1, cm);

    int v = value.toInt();

    if (key == "Br")        cacheDisplay.Br = v;
    else if (key == "Sptx1") cacheDisplay.Sptx1 = v;
    else if (key == "Sptx2") cacheDisplay.Sptx2 = v;
    else if (key == "Spnm")  cacheDisplay.Spnm  = v;
    else if (key == "Spdt")  cacheDisplay.Spdt  = v;
    else if (key == "Bzr")   cacheDisplay.Bzr   = v;
    else if (key == "Da")    cacheDisplay.Da    = v;
    else if (key == "CoHi")  cacheDisplay.CoHi  = v;
    else if (key == "newPassword") key.toCharArray(cacheDisplay.newPassword, sizeof(cacheDisplay.newPassword));
    else if (key == "mode")  cacheDisplay.mode  = v;

    pos = cm + 1;
  }

  // DEBUG
//  Serial.println("[DISPLAY UPDATED]");
//  Serial.println("Br=" + String(Br));
//  Serial.println("Sptx1=" + String(Sptx1));
//  Serial.println("Sptx2=" + String(Sptx2));
//  Serial.println("Spnm=" + String(Spnm));
}


void parseLokasi(String payload) {
  int pos = 0;
  while (pos < payload.length()) {
    int eq = payload.indexOf('=', pos);
    int cm = payload.indexOf(',', pos);
    if (eq == -1) break;
    if (cm == -1) cm = payload.length();

    String key = payload.substring(pos, eq);
    String val = payload.substring(eq + 1, cm);

    if (key == "Lt") cacheLokasi.Lt = val.toFloat();
    else if (key == "Lo") cacheLokasi.Lo = val.toFloat();
    else if (key == "Tz") cacheLokasi.Tz = val.toInt();
    else if (key == "Al") cacheLokasi.Al = val.toInt();

    pos = cm + 1;
  }
}

void parsePesan(String payload) {
  int pos = 0;
  while (pos < payload.length()) {
    int eq = payload.indexOf('=', pos);
    int cm = payload.indexOf(',', pos);
    if (eq == -1) break;
    if (cm == -1) cm = payload.length();

    String key = payload.substring(pos, eq);
    String val = payload.substring(eq + 1, cm);

    if (key == "M1") val.toCharArray(cachePesan.message1, sizeof(cachePesan.message1));
    else if (key == "M2") val.toCharArray(cachePesan.message2, sizeof(cachePesan.message2));
    else if (key == "NM") val.toCharArray(cachePesan.textName, sizeof(cachePesan.textName));
    else if (key == "IDX") cachePesan.index = val.toInt();

    pos = cm + 1;
  }
}

void parseKoreksi(String payload) {
  int pos = 0;
  while (pos < payload.length()) {
    int eq = payload.indexOf('=', pos);
    int cm = payload.indexOf(',', pos);
    if (eq == -1) break;
    if (cm == -1) cm = payload.length();

    uint8_t idx = payload.substring(pos, eq).toInt();
    int val = payload.substring(eq + 1, cm).toInt();

    //if (idx >= 1 && idx <= 5) koreksi[idx] = val;
    if(idx == 1) cacheKoreksi.SUBUH_KOREKSI = val;
    else if(idx == 2) cacheKoreksi.DHUHUR_KOREKSI = val;
    else if(idx == 3) cacheKoreksi.ASHAR_KOREKSI = val;
    else if(idx == 4) cacheKoreksi.MAGHRIB_KOREKSI = val;
    else if(idx == 5) cacheKoreksi.ISYA_KOREKSI = val;

    pos = cm + 1;
  }
}

void parseIqomah(String payload) {
  int pos = 0;
  while (pos < payload.length()) {
    int eq = payload.indexOf('=', pos);
    int cm = payload.indexOf(',', pos);
    if (eq == -1) break;
    if (cm == -1) cm = payload.length();

    String key = payload.substring(pos, eq);
    int val = payload.substring(eq + 1, cm).toInt();

    if (key.startsWith("I")) {
      uint8_t idx = key.substring(1).toInt();
      //if (idx >= 1 && idx <= 5) iqomah[idx] = val;
      if(idx == 1) cacheIqomah.SUBUH_IQOMAH = val;
      else if(idx == 2) cacheIqomah.DHUHUR_IQOMAH = val;
      else if(idx == 3) cacheIqomah.ASHAR_IQOMAH = val;
      else if(idx == 4) cacheIqomah.MAGHRIB_IQOMAH = val;
      else if(idx == 5) cacheIqomah.ISYA_IQOMAH = val;
    }
    else if (key.startsWith("B")) {
      uint8_t idx = key.substring(1).toInt();
      //if (idx >= 1 && idx <= 5) displayBlink[idx] = val;
      if(idx == 1) cacheIqomah.SUBUH_BLINK = val;
      else if(idx == 2) cacheIqomah.DHUHUR_BLINK = val;
      else if(idx == 3) cacheIqomah.ASHAR_BLINK = val;
      else if(idx == 4) cacheIqomah.MAGHRIB_BLINK = val;
      else if(idx == 5) cacheIqomah.ISYA_BLINK = val;
    }

    pos = cm + 1;
  }
}


int getIntPart(String &s, int &pos) {
  int comma = s.indexOf(',', pos);
  if (comma == -1) comma = s.length();
  int val = s.substring(pos, comma).toInt();
  pos = comma + 1;
  return val;
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);
  //waitingWiFiInfo = EEPROM.read(ADDR_MODE);
  //Serial.println("waitingWiFiInfo:"+String(waitingWiFiInfo));
  
  DEVICE_ID = "JWS-" + String(ESP.getChipId());

  AP_init();
  waitingWiFiInfo = true;
  
//  EEPROM.write(ADDR_MODE, 1);
//  EEPROM.commit();
  //if (waitingWiFiInfo) {
    //AP_init();
//  } else {
//    ONLINE(newSSID, newPASS);
//  }
//  AP_init();
//  waitingWiFiInfo = true;
}

void loop() {
  /*if (modeOTA) {
    ArduinoOTA.handle();
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.equalsIgnoreCase("restart")) {
        delay(1000);
        ESP.restart();
      }
    }
  } else {
    server.handleClient();
    webSocket.loop();
    cekSerialMonitor();
  }*/
 if (waitingWiFiInfo==1) {
    server.handleClient();
    webSocket.loop();
    cekSerialMonitor();  // pantau data dari sender
  } else if(waitingWiFiInfo==0) {
    ArduinoOTA.handle();
    Serial.println("waitingWiFiInfo:"+String(waitingWiFiInfo));
    //Serial.println("WIFI:" + WiFi.status());
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.equalsIgnoreCase("restart")) {
//        EEPROM.write(ADDR_MODE, 1);
//        EEPROM.commit();
        delay(1000);
        ESP.restart();
      }
    }
  }
  //

  
}
