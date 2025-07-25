#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "ALC_Project.h"

// #define USE_LED_7SEG

#include <ESPAsyncWebServer.h>
// #include "LoRaConfig.h"
// AsyncWebServer server(80);

AsyncWebSocket ALC_webSocketServer("/ws");

// Web: Monitor page
const char monitor_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Monitor</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body{font-family:sans-serif;}
    .card{border:1px solid #ccc;padding:16px;margin:8px;border-radius:8px;}
    .card-title{font-weight:bold;}
  </style>
</head>
<body>
  <h2>Pin State Monitor</h2>
  <div id="pinCards"></div>
  <h2>Counter Monitor</h2>
  <div id="counterCards"></div>
  <script>
    function update() {
      fetch('/status').then(r=>r.json()).then(data=>{
        let pinHtml = '';
        data.pins.forEach(p=>{
          pinHtml += `<div class="card"><div class="card-title">Pin ${p.pin}</div>State: ${p.state}</div>`;
        });
        document.getElementById('pinCards').innerHTML = pinHtml;
        let counterHtml = '';
        data.counters.forEach(c=>{
          counterHtml += `<div class="card"><div class="card-title">Counter ${c.idx}</div>Value: ${c.value} / Plan: ${c.plan}</div>`;
        });
        document.getElementById('counterCards').innerHTML = counterHtml;
      });
    }
    setInterval(update, 1000); update();
  </script>
  <a href="/config">Config</a>
</body>
</html>
)rawliteral";

// Web: Config page
const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Config</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body{font-family:sans-serif;}
    .card{border:1px solid #ccc;padding:16px;margin:8px;border-radius:8px;}
    label{display:block;margin-top:8px;}
  </style>
</head>
<body>
  <h2>ESP32 Counter Config</h2>
  <form id="cfgForm">
    <div class="card">
      <label>Function Pin: <input name="funcPin" type="number" min="0" max="39" required></label>
    </div>
    <div id="counterCfg"></div>
    <button type="submit">Save</button>
  </form>
  <script>
    let NUM_COUNTERS = 4;
    function loadConfig() {
      fetch('/getcfg').then(r=>r.json()).then(cfg=>{
        document.querySelector('[name=funcPin]').value = cfg.funcPin;
        let html = '';
        for(let i=0;i<NUM_COUNTERS;i++){
          html += `<div class="card">
            <div class="card-title">Counter ${i}</div>
            <label>Pin: <input name="pin${i}" type="number" min="0" max="39" value="${cfg.counters[i].pin}" required></label>
            <label>Filter(ms): <input name="filter${i}" type="number" min="0" max="1000" value="${cfg.counters[i].filter}" required></label>
            <label>Multiple: <input name="multiple${i}" type="number" min="1" max="100" value="${cfg.counters[i].multiple}" required></label>
            <label>Plan: <input name="plan${i}" type="number" min="0" max="9999" value="${cfg.plan[i]}" required></label>
            <label>Result: <input name="result${i}" type="number" min="0" max="9999" value="${cfg.result[i]}" disabled></label> 
          </div>
          <div class="card">
            <div class="card-title">Main Config</div>
            <input type="hidden" name="id" value="${cfg.id}">
            <input type="hidden" name="netID" value="${cfg.netID}">
           </div>`;
        }
        document.getElementById('counterCfg').innerHTML = html;
      });
    }
    document.getElementById('cfgForm').onsubmit = function(e){
      e.preventDefault();
      let fd = new FormData(this);
      fetch('/setcfg', {method:'POST', body:fd}).then(r=>r.text()).then(t=>{
        alert(t);
        loadConfig();
      });
    };
    loadConfig();
  </script>
  <a href="/">Monitor</a>
</body>
</html>
)rawliteral";
#ifdef USE_LED_7SEG
#include "7Seg.h"
#endif// USE_LED_7SEG

bool SocketIsConnected = false;
bool onece = false;
void ALC_webSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  // Xử lý sự kiện WebSocket
  if(type == WS_EVT_CONNECT){
    SocketIsConnected = true;
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    SocketIsConnected = false;onece = false;
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}
      
// #include "ModuleLoRaE32.h"
// M_LoRa_E32 loraE32s;

// #include "LMD.h"

// ==== Global Variables ====
ProjectConfig gConfig;

// ==== Example Counter Handler ====
volatile uint32_t counterValue[NUM_COUNTERS] = {0};
unsigned long lastTrigger[NUM_COUNTERS] = {0};

// Helper: Get JSON status for all pins/counters
String getStatusJson() {
    StaticJsonDocument<512> doc;
    JsonArray pins = doc.createNestedArray("pins");
    for (int i = 0; i < NUM_COUNTERS; i++) {
        JsonObject p = pins.createNestedObject();
        p["pin"] = gConfig.counters[i].pin;
        p["state"] = digitalRead(gConfig.counters[i].pin);
    }
    JsonArray counters = doc.createNestedArray("counters");
    for (int i = 0; i < NUM_COUNTERS; i++) {
        JsonObject c = counters.createNestedObject();
        c["idx"] = i;
        c["value"] = gConfig.result[i];
        c["plan"] = gConfig.plan[i];
    }
    String out;
    serializeJson(doc, out);
    return out;
}
// Web: API handlers
void setupWebServer(AsyncWebServer &server) {
  // Dashboard HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", monitor_html);
  });

  // API: Get all counter status (for dashboard)
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<512> doc;
    JsonArray counters = doc.createNestedArray("counters");
    for (int i = 0; i < NUM_COUNTERS; i++) {
      JsonObject c = counters.createNestedObject();
      c["id"] = i + 1;
      c["name"] = String(gConfig.name[i]);
      c["plan"] = gConfig.plan[i];
      c["result"] = gConfig.result[i];
      c["inc"] = gConfig.counters[i].multiple;
      c["filter"] = gConfig.counters[i].filter;
      c["pin"] = gConfig.counters[i].pin;
    }
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // API: Get config (for modal)
  server.on("/api/counter", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->hasParam("id")) {
      req->send(400, "application/json", "{\"error\":\"Missing id\"}");
      return;
    }
    int idx = req->getParam("id")->value().toInt() - 1;
    if (idx < 0 || idx >= NUM_COUNTERS) {
      req->send(400, "application/json", "{\"error\":\"Invalid id\"}");
      return;
    }
    StaticJsonDocument<256> doc;
    doc["id"] = idx + 1;
    doc["name"] = String(gConfig.name[idx]);
    doc["plan"] = gConfig.plan[idx];
    doc["result"] = gConfig.result[idx];
    doc["inc"] = gConfig.counters[idx].multiple;
    doc["filter"] = gConfig.counters[idx].filter;
    doc["pin"] = gConfig.counters[idx].pin;
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // API: Update plan only
  server.on("/api/plan", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Accept x-www-form-urlencoded
    if (req->hasParam("id", true) && req->hasParam("plan", true)) {
      int idx = req->getParam("id", true)->value().toInt() - 1;
      int plan = req->getParam("plan", true)->value().toInt();
      if (idx >= 0 && idx < NUM_COUNTERS) {
        gConfig.plan[idx] = plan;
        saveProjectConfig(gConfig);
        req->send(200, "application/json", "{\"ok\":true}");
        return;
      }
      req->send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
    } else {
      req->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    }
  });

  // API: Update result only
  server.on("/api/result", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Accept x-www-form-urlencoded
    if (req->hasParam("id", true) && req->hasParam("result", true)) {
      int idx = req->getParam("id", true)->value().toInt() - 1;
      int result = req->getParam("result", true)->value().toInt();
      if (idx >= 0 && idx < NUM_COUNTERS) {
        gConfig.result[idx] = result;
        saveProjectConfig(gConfig);
        req->send(200, "application/json", "{\"ok\":true}");
        return;
      }
      req->send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
    } else {
      req->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    }
  });

  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Accept x-www-form-urlencoded
    if (req->hasParam("cmnd", true) && req->hasParam("id", true)) {
      String cmnd = req->getParam("cmnd", true)->value();
      int idx = req->getParam("id", true)->value().toInt() - 1;
      if (cmnd == "resetCounter" && idx >= 0 && idx < NUM_COUNTERS) {
        gConfig.result[idx] = 0;
        counterValue[idx] = 0;
        saveProjectConfig(gConfig);
        req->send(200, "application/json", "{\"ok\":true}");
        return;
      }
      req->send(400, "application/json", "{\"error\":\"Invalid command\"}");
    } else {
      req->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    }
  });

  // API: Save counter settings (from modal)
  server.on("/api/setting", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Accept x-www-form-urlencoded
    if (req->hasParam("id", true) &&
      req->hasParam("name", true) &&
      req->hasParam("filter", true) &&
      req->hasParam("inc", true) &&
      req->hasParam("pin", true) &&
      req->hasParam("address", true)) {
      int idx = req->getParam("id", true)->value().toInt() - 1;
      if (idx < 0 || idx >= NUM_COUNTERS) {
        req->send(400, "text/plain", "Invalid id");
        return;
      }
      String name = req->getParam("name", true)->value();
      int filter = req->getParam("filter", true)->value().toInt();
      int inc = req->getParam("inc", true)->value().toInt();
      int pin = req->getParam("pin", true)->value().toInt();
      int address = req->getParam("address", true)->value().toInt();

      if (name.length() > 15) {
        req->send(400, "text/plain", "Name too long");
        return;
      }
      strncpy(gConfig.name[idx], name.c_str(), sizeof(gConfig.name[idx]) - 1);
      gConfig.name[idx][sizeof(gConfig.name[idx]) - 1] = '\0';
      gConfig.counters[idx].filter = filter;
      gConfig.counters[idx].multiple = inc;
      gConfig.counters[idx].pin = pin;
      gConfig.id = address;
      saveProjectConfig(gConfig);
      req->send(200, "text/plain", "Settings updated.");
    } else {
      req->send(400, "text/plain", "Missing parameters");
    }
  });

  // API: Load config file
  server.on("/api/loadcfg", HTTP_GET, [](AsyncWebServerRequest *req) {
    StaticJsonDocument<1024> doc;
    for (int i = 0; i < NUM_COUNTERS; i++) {
      doc["plan"][i] = gConfig.plan[i];
      doc["result"][i] = gConfig.result[i];
      doc["counters"][i]["pin"] = gConfig.counters[i].pin;
      doc["counters"][i]["filter"] = gConfig.counters[i].filter;
      doc["counters"][i]["multiple"] = gConfig.counters[i].multiple;
      doc["name"][i] = String(gConfig.name[i]);
    }
    doc["funcPin"] = gConfig.funcPin;
    doc["id"] = gConfig.id;
    doc["netID"] = gConfig.netID;
    doc["channel"] = gConfig.channel;
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // API: Save config file
  server.on("/api/savecfg", HTTP_POST, [](AsyncWebServerRequest *req) {
    // Accept JSON body
    String body;
    if (req->hasParam("plain", true)) {
      body = req->getParam("plain", true)->value();
    } else {
      req->send(400, "application/json", "{\"error\":\"Missing body\"}");
      return;
    }
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      req->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    for (int i = 0; i < NUM_COUNTERS; i++) {
      gConfig.plan[i] = doc["plan"][i] | gConfig.plan[i];
      gConfig.result[i] = doc["result"][i] | gConfig.result[i];
      gConfig.counters[i].pin = doc["counters"][i]["pin"] | gConfig.counters[i].pin;
      gConfig.counters[i].filter = doc["counters"][i]["filter"] | gConfig.counters[i].filter;
      gConfig.counters[i].multiple = doc["counters"][i]["multiple"] | gConfig.counters[i].multiple;
      String name = doc["name"][i] | String(gConfig.name[i]);
      strncpy(gConfig.name[i], name.c_str(), sizeof(gConfig.name[i]) - 1);
      gConfig.name[i][sizeof(gConfig.name[i]) - 1] = '\0';
    }
    gConfig.funcPin = doc["funcPin"] | gConfig.funcPin;
    gConfig.id = doc["id"] | gConfig.id;
    gConfig.netID = doc["netID"] | gConfig.netID;
    gConfig.channel = doc["channel"] | gConfig.channel;
    saveProjectConfig(gConfig);
    req->send(200, "application/json", "{\"ok\":true}");
  });

}

// ==== Save/Load Functions ====
bool saveProjectConfig(const ProjectConfig &cfg) {
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) return false;

  StaticJsonDocument<1024> doc;
  for (int i = 0; i < NUM_COUNTERS; i++) {
    doc["plan"][i] = cfg.plan[i];
    doc["result"][i] = cfg.result[i];
    doc["counters"][i]["pin"] = cfg.counters[i].pin;
    doc["counters"][i]["filter"] = cfg.counters[i].filter;
    doc["counters"][i]["multiple"] = cfg.counters[i].multiple;
    doc["name"][i] = String(cfg.name[i]);
  }
  doc["funcPin"] = cfg.funcPin;
  doc["id"] = cfg.id;
  doc["netID"] = cfg.netID;
  doc["channel"] = cfg.channel;
  doc["LoRaPin"][0] = cfg.LoRaPin[0];
  doc["LoRaPin"][1] = cfg.LoRaPin[1];
  doc["LoRaPin"][2] = cfg.LoRaPin[2];
  doc["LoRaPin"][3] = cfg.LoRaPin[3];
  doc["LoRaPin"][4] = cfg.LoRaPin[4];

  bool ok = serializeJson(doc, file) > 0;
  file.close();
  return ok;
}

bool loadProjectConfig(ProjectConfig &cfg) {
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) return false;

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return false;

  for (int i = 0; i < NUM_COUNTERS; i++) {
    cfg.plan[i] = doc["plan"][i] | 0;
    cfg.result[i] = doc["result"][i] | 0;
    cfg.counters[i].pin = doc["counters"][i]["pin"] | 0;
    cfg.counters[i].filter = doc["counters"][i]["filter"] | 0;
    cfg.counters[i].multiple = doc["counters"][i]["multiple"] | 1;
    String name = doc["name"][i] | "";
    strncpy(cfg.name[i], name.c_str(), sizeof(cfg.name[i]) - 1);
    cfg.name[i][sizeof(cfg.name[i]) - 1] = '\0';
  }
  cfg.funcPin = doc["funcPin"] | 0;
  cfg.id = doc["id"] | 0;
  cfg.netID = doc["netID"] | 0;
  cfg.channel = doc["channel"] | 0;
  cfg.LoRaPin[0] = doc["LoRaPin"][0] | 0;
  cfg.LoRaPin[1] = doc["LoRaPin"][1] | 0;
  cfg.LoRaPin[2] = doc["LoRaPin"][2] | 0;
  cfg.LoRaPin[3] = doc["LoRaPin"][3] | 0;
  cfg.LoRaPin[4] = doc["LoRaPin"][4] | 0;
  return true;
}

HardwareSerial LoRaSerial(1); // Use LoRaSerial for LoRa E32 communication

#define START_MARKER 0xAA
#define CMD_REQUEST  0x01
#define CMD_RESPONSE 0x02
#define ACK_MARKER   0x55

#define MAX_NODES 4
#define PAYLOAD_SIZE 16

struct LoraPacket {
  uint8_t start;
  uint8_t cmd;
  uint8_t netID;
  uint8_t deviceID;
  uint8_t sequence;
  uint8_t payload[PAYLOAD_SIZE];
};

uint8_t gNetID = 1;   
uint8_t gDeviceID = 0;      
uint8_t sequenceCounter = 0;

#define RESPONSE_TIMEOUT 300

unsigned long lastRequestTime = 0;
uint8_t currentTarget = 0;
uint8_t connectQuality[MAX_NODES] = {4, 4, 4, 4};
unsigned long lastDataTime[MAX_NODES] = {0, 0, 0, 0};

void printConnectState();

void sendRequestTo(uint8_t targetID) {
  LoraPacket packet;
  packet.start = START_MARKER;
  packet.cmd = CMD_REQUEST;
  packet.netID = gNetID;
  packet.deviceID = targetID;
  packet.sequence = sequenceCounter++;
  memset(packet.payload, 0, PAYLOAD_SIZE); 

      digitalWrite(M0_PIN, LOW);
      digitalWrite(M1_PIN, LOW);
  LoRaSerial.write((uint8_t*)&packet, sizeof(packet));
  // Serial.printf("Master > Request to node %d [seq=%d]\n", targetID, packet.sequence);
}

void checkReceive() {
  if (LoRaSerial.available() >= sizeof(LoraPacket)) {
    LoraPacket pkt;
    LoRaSerial.readBytes((uint8_t*)&pkt, sizeof(pkt));
    if (pkt.start == START_MARKER && pkt.cmd == CMD_RESPONSE && pkt.netID == gNetID) {
      uint8_t idx = pkt.deviceID - 1;
      if (idx < MAX_NODES) {
        connectQuality[idx] = min(4, connectQuality[idx] + 1);
        lastDataTime[idx] = millis();

        Serial.println();
        Serial.printf("Node %d replied (seq=%d): ", pkt.deviceID, pkt.sequence);
        Serial.println();
        for (int i = 0; i < 8; i++) {
          Serial.printf("%02X ", pkt.payload[i]);
        }
        // for (int i = 0; i < NUM_COUNTERS; i++) {
        //   gConfig.plan[i]   = (pkt.payload[i * 4] << 8) | pkt.payload[i * 4 + 1];
        //   gConfig.result[i] = (pkt.payload[i * 4 + 2] << 8) | pkt.payload[i * 4 + 3];
        //   counterValue[i] = gConfig.result[i];
        // }
          gConfig.plan[idx]   = (pkt.payload[0 * 4] << 8) | pkt.payload[0 * 4 + 1];
          gConfig.result[idx] = (pkt.payload[0 * 4 + 2] << 8) | pkt.payload[0 * 4 + 3];
        
        Serial.println();
        Serial.println("Plan1: " + String(gConfig.plan[0]) +
            " | Plan2: " + String(gConfig.plan[1]) +
            " | Plan3: " + String(gConfig.plan[2]) +
            " | Plan4: " + String(gConfig.plan[3]));
        Serial.println("Result1: " + String(gConfig.result[0]) +
            " | Result2: " + String(gConfig.result[1]) +
            " | Result3: " + String(gConfig.result[2]) +
            " | Result4: " + String(gConfig.result[3]));
        Serial.println();
        Serial.println();
      }
    }
  }
}

uint16_t getResult(uint8_t id) {
  if (id < NUM_COUNTERS) {
    return gConfig.result[id];
  }
  return 0;
}

uint16_t getPlan(uint8_t id) {
  if (id < NUM_COUNTERS) {
    return gConfig.plan[id];
  }
  return 0;
}

uint8_t getConnectQuality(uint8_t id) {
  if (id < MAX_NODES) {
    return connectQuality[id];
  }
  return 0;
}

void updateConnectionQuality() {
  for (int i = 0; i < MAX_NODES; i++) {
    if (millis() - lastDataTime[i] > 5000) {
      if (connectQuality[i] > 1) {
        connectQuality[i]--;
        lastDataTime[i] = millis(); 
        Serial.printf("Quality[%d] ↓ %d\n", i + 1, connectQuality[i]);
      }
      
    }
  }
}

void printConnectState() {
  for (int i = 0; i < MAX_NODES; i++) {
    Serial.printf("Node[%d] quality: %d\n", i + 1, connectQuality[i]);
  }
}

void masterLoop() {
  if (millis() - lastRequestTime > 900) {
    currentTarget = (currentTarget % MAX_NODES) + 1;
    sendRequestTo(currentTarget);
    lastRequestTime = millis();
      // printConnectState();
  }

  checkReceive();
  updateConnectionQuality();
}

void checkReceiveAndRespond() {
  
      digitalWrite(M0_PIN, LOW);
      digitalWrite(M1_PIN, LOW);
  if (LoRaSerial.available() >= sizeof(LoraPacket)) {
    LoraPacket pkt;
    LoRaSerial.readBytes((uint8_t*)&pkt, sizeof(pkt));

    if (pkt.start == START_MARKER && pkt.cmd == CMD_REQUEST && pkt.netID == gNetID && pkt.deviceID == gDeviceID) {
      Serial.printf("Request from master [seq=%d]\n", pkt.sequence);
      LoraPacket reply;
      reply.start = START_MARKER;
      reply.cmd = CMD_RESPONSE;
      reply.netID = gNetID;
      reply.deviceID = gDeviceID;
      reply.sequence = pkt.sequence;
      for (int i = 0; i < 4; i++) {
        reply.payload[i * 2] = (gConfig.result[i] >> 8) & 0xFF;
        reply.payload[i * 2 + 1] = gConfig.result[i] & 0xFF;
      }
      LoRaSerial.write((uint8_t*)&reply, sizeof(reply));
      Serial.println("Reply sent");
    }
  }
}

void nodeLoop() {
  checkReceiveAndRespond();
}



void counterPollingLoop() {
  static uint8_t lastState[NUM_COUNTERS] = {0};

  for (int i = 1; i < 4; i++) {
    uint8_t state = digitalRead(gConfig.counters[i].pin);
    if (lastState[i] == HIGH && state == LOW) { // Detect falling edge
      unsigned long now = millis();
      if (now - lastTrigger[i] >= gConfig.counters[i].filter) {
        lastTrigger[i] = now;
        counterValue[i] += gConfig.counters[i].multiple;
        gConfig.result[i] = min((uint16_t)counterValue[i], (uint16_t)9999);
        Serial.printf("Counter %d: %d\n", i+1, gConfig.result[i]);
        
        // sendLoRaData(); // Gửi dữ liệu qua LoRa
        if (SocketIsConnected) {
          String Name = gConfig.name[i];
          ALC_webSocketServer.textAll(
            String("{\"id\":") + String(i + 1) + 
            String(",\"result\":") + String(gConfig.result[i]) + 
            String(",\"plan\":") + String(gConfig.plan[i]) + 
            String(",\"name\":\"") + Name + 
            String("\"}")
          );
        }
      }
    }
    lastState[i] = state;
  }
}

void setupCounters() {
    Serial.println("Setup Counters...");
    for (int i = 0; i < NUM_COUNTERS; i++) {
        // pinMode(gConfig.counters[i].pin, INPUT_PULLUP);
        counterValue[i] = 0;
        lastTrigger[i] = 0;
    }
    Serial.println("  ✅ Counters initialized.");
}


// ==== Setup/Loop Example ====
void ALC_setup(AsyncWebServer &server) {
    Serial.begin(115200);
    LoRaSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // LoRa E32 Serial
    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    // pinMode(37, INPUT_PULLUP);
    pinMode(38, INPUT_PULLUP);
    pinMode(39, INPUT_PULLUP);
    pinMode(40, INPUT_PULLUP);
    Serial.println("  ✅ Pins initialized.");

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
        return;
    }
    if (!loadProjectConfig(gConfig)) {
        Serial.println("  ❌ Load config failed, using defaults.");
        memset(&gConfig, 0, sizeof(gConfig));
        for (int i = 0; i < NUM_COUNTERS; i++) {
            gConfig.counters[i].pin = 37 + i;
            gConfig.counters[i].filter = 10;
            gConfig.counters[i].multiple = 1;
        }
        gConfig.funcPin = 7;
        saveProjectConfig(gConfig);
    }else {
      Serial.println("  ✅ Load config successful.");
      Serial.println("Project Config Loaded:");
      Serial.println("ID: " + String(gConfig.id));
      Serial.println("NetID: " + String(gConfig.netID));
      Serial.println("Channel: " + String(gConfig.channel));
      Serial.println("Function Pin: " + String(gConfig.funcPin));
    for (int i = 0; i < NUM_COUNTERS; i++) {
        Serial.printf("Counter %d: Pin=%d, Filter=%d, Multiple=%d\n", 
            i, gConfig.counters[i].pin, gConfig.counters[i].filter, gConfig.counters[i].multiple);
    }
    } 
    setupCounters();
    Serial.println("Ram :" + String(ESP.getFreeHeap()/1024 )+ " KB");
    Serial.println("Setting up Web Server...");
    
    Serial.println("Ram :" + String(ESP.getFreeHeap()/1024 )+ " KB");
    
    setupWebServer(server);
    Serial.println("  ✅ Web Server Setup Complete");
    ALC_webSocketServer.onEvent(ALC_webSocketEvent);
    server.addHandler(&ALC_webSocketServer);
    server.begin();
    Serial.println("  ✅ WebSocket Server Started");
    #ifdef USE_LED_7SEG
    Serial.println("  ✅ Using 7-Segment Display");
    
    pinMode(DATA1_PIN, OUTPUT);
    pinMode(DATA2_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(LAT_PIN, OUTPUT);
    // SetPin7Seg(DATA1_PIN ,DATA2_PIN , -1, CLK_PIN, LAT_PIN);
    #endif// USE_LED_7SEG
    gConfig.role = 1;
}

void ALC_loop() {
  #ifdef USE_LED_7SEG
  // PrintSeg(counterValue[1], counterValue[2], counterValue[3]);latch();
  displays(counterValue[0], counterValue[1], DATA1_PIN, DATA2_PIN, CLK_PIN, LAT_PIN, 0);
  #endif// USE_LED_7SEG
  // receiveLoRaData(); // Nhận dữ liệu từ LoRa
    // Handle WebSocket events
    ALC_webSocketServer.cleanupClients();
    // Your main logic here (web config, admin monitor, etc.)
    // delay(1000);
    counterPollingLoop();
    if (!onece) {
      onece = true;
      if (SocketIsConnected) {
        for (int i = 0; i < NUM_COUNTERS; i++) {
          String Name = gConfig.name[i];
          ALC_webSocketServer.textAll(
            String("{\"id\":") + String(i + 1) + 
            String(",\"result\":") + String(gConfig.result[i]) + 
            String(",\"plan\":") + String(gConfig.plan[i]) + 
            String(",\"name\":\"") + Name + 
            String("\"}")
          );
        }
      }
    }
    masterLoop();
    // nodeLoop();
    // if(gConfig.role == 0){masterLoop();}
    // if(gConfig.role == 1){nodeLoop();}
    // static unsigned long lastSend = 0;
    // static unsigned long nextInterval = 0;
    // if (nextInterval == 0) {
    //   nextInterval = random(5000, 10001); // 5-10s
    //   lastSend = millis();
    // }
    // if (millis() - lastSend >= nextInterval) {
    //   // sendLoRaData();
    //   lastSend = millis();
    //   nextInterval = random(5000, 10001);
    // }
}