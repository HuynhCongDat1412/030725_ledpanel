#include <Arduino.h>
#include "ModuleLoRaE32.h"
M_LoRa_E32 loraE32;
#include "LoRa_E32.h"
// #include <Arduino_JSON.h>
#include <ArduinoJson.h>

// LoRa E32 Configuration Pins
#define E32_M0_PIN    15
#define E32_M1_PIN    16
#define E32_TX_PIN    17
#define E32_RX_PIN    18
#define E32_AUX_PIN   -1

M_LoRa_E32::M_LoRa_E32() {}

void sendDebugMessage(const String& message) {
    Serial.println(message); // Gửi thông điệp debug qua Serial
}
#define LOG(s) sendDebugMessage(s)

// LoRa E32 instance
LoRa_E32 e32ttl100(&Serial1, E32_AUX_PIN, E32_M0_PIN, E32_M1_PIN);

void printParameters(struct Configuration configuration);

void M_LoRa_E32::CMND(const String& cmd) {

    DynamicJsonDocument doc(1000);
    DeserializationError error = deserializeJson(doc, cmd);
    if (error) {
      Serial.println("Invalid JSON received");
      sendDebugMessage("Invalid JSON received");
      sendDebugMessage("Error: " + String(error.c_str()));
      sendDebugMessage("Command: " + cmd);
      return;
    }
    String action = doc["action"] | "";
    action.trim();

    if (action == "set_lora_e32_config") {
      if (!doc.containsKey("addh") || !doc.containsKey("addl") || !doc.containsKey("chan") ||
          !doc.containsKey("uartParity") || !doc.containsKey("uartBaudRate") ||
          !doc.containsKey("airDataRate") || !doc.containsKey("fixedTransmission") ||
          !doc.containsKey("ioDriveMode") || !doc.containsKey("wirelessWakeupTime") ||
          !doc.containsKey("fec") || !doc.containsKey("transmissionPower")) {
        sendDebugMessage("Invalid LoRa E32 configuration data");
        return;
      }
      LoRaE32Config *config = new LoRaE32Config();
      config->addh = doc["addh"];
      config->addl = doc["addl"];
      config->chan = doc["chan"];
      config->uartParity = doc["uartParity"];
      config->uartBaudRate = doc["uartBaudRate"];
      config->airDataRate = doc["airDataRate"];
      config->fixedTransmission = doc["fixedTransmission"];
      config->ioDriveMode = doc["ioDriveMode"];
      config->wirelessWakeupTime = doc["wirelessWakeupTime"];
      config->fec = doc["fec"];
      config->transmissionPower = doc["transmissionPower"];
      // config->operatingMode = systemStatus.loraE32.operatingMode;
      // You may want to call setLoRaConfig() here if needed
    }
    else if (action == "set_lora_operating_mode") {
      uint8_t mode = doc["mode"];
      setLoRaOperatingMode(mode);
      // sendSystemStatus();
    }
}

// Initialize LoRa E32
void M_LoRa_E32::initLoRaE32() {
  Serial.println("=== Initializing LoRa E32 ===");
  
  // Initialize chân M0, M1 là OUTPUT
  pinMode(E32_M0_PIN, OUTPUT);
  pinMode(E32_M1_PIN, OUTPUT);
  
  // Khởi tạo Serial1 với các chân đã định nghĩa
  Serial1.begin(9600, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);
  
  // Khởi tạo LoRa E32
  e32ttl100.begin();
  
  // Đặt chế độ mặc định là Normal Mode
  setLoRaOperatingMode(0); // Normal mode
  
  vTaskDelay(pdMS_TO_TICKS(1000)); // Tăng delay để module ổn định
  
  Serial.println("Reading module information...");
  
  // Đọc thông tin module với error handling
  ResponseStructContainer c;
  c = e32ttl100.getModuleInformation();
  
  if (c.status.code == 1 && c.data != nullptr) {
    ModuleInformation mi = *(ModuleInformation*)c.data;
    // printModuleInformation(mi);
    
    // Lưu thông tin module vào system status
    // systemStatus.loraE32.moduleInfo = "HEAD: " + String(mi.HEAD, HEX) + 
    //                                   ", Freq: " + String(mi.frequency, HEX) + 
    //                                   ", Ver: " + String(mi.version, HEX) + 
    //                                   ", Features: " + String(mi.features, HEX);
    // systemStatus.loraE32.initialized = true;
    
    sendDebugMessage("LoRa E32 module information read successfully");
  } else {
    Serial.print("  ❌ Error reading module information: ");
    Serial.println(c.status.getResponseDescription());
    sendDebugMessage("  ❌ Error reading LoRa E32 module information: " + String(c.status.getResponseDescription()));
    // systemStatus.loraE32.initialized = false;
  }
  
  if (c.data != nullptr) {
    c.close();
  }
  
  vTaskDelay(pdMS_TO_TICKS(1000));
  
    
  //   // Đọc cấu hình hiện tại
    ResponseStructContainer configContainer;
    configContainer = e32ttl100.getConfiguration();
    
    if (configContainer.status.code == 1 && configContainer.data != nullptr) {
      Configuration configuration = *(Configuration*)configContainer.data;
      Serial.println("Configuration read successfully!");
      
      printParameters(configuration);
      
  //     // // Lưu cấu hình vào system status
  //     // systemStatus.loraE32.addh = configuration.ADDH;
  //     // systemStatus.loraE32.addl = configuration.ADDL;
  //     // systemStatus.loraE32.chan = configuration.CHAN;
  //     // systemStatus.loraE32.uartParity = configuration.SPED.uartParity;
  //     // systemStatus.loraE32.uartBaudRate = configuration.SPED.uartBaudRate;
  //     // systemStatus.loraE32.airDataRate = configuration.SPED.airDataRate;
  //     // systemStatus.loraE32.fixedTransmission = configuration.OPTION.fixedTransmission;
  //     // systemStatus.loraE32.ioDriveMode = configuration.OPTION.ioDriveMode;
  //     // systemStatus.loraE32.wirelessWakeupTime = configuration.OPTION.wirelessWakeupTime;
  //     // systemStatus.loraE32.fec = configuration.OPTION.fec;
  //     // systemStatus.loraE32.transmissionPower = configuration.OPTION.transmissionPower;
  //     // systemStatus.loraE32.frequency = configuration.getChannelDescription();
  //     // systemStatus.loraE32.parityBit = configuration.SPED.getUARTParityDescription();
  //     // systemStatus.loraE32.airDataRateStr = configuration.SPED.getAirDataRate();
  //     // systemStatus.loraE32.uartBaudRateStr = configuration.SPED.getUARTBaudRate();
  //     // systemStatus.loraE32.fixedTransmissionStr = configuration.OPTION.getFixedTransmissionDescription();
  //     // systemStatus.loraE32.ioDriveModeStr = configuration.OPTION.getIODroveModeDescription();
  //     // systemStatus.loraE32.wirelessWakeupTimeStr = configuration.OPTION.getWirelessWakeUPTimeDescription();
  //     // systemStatus.loraE32.fecStr = configuration.OPTION.getFECDescription();
  //     // systemStatus.loraE32.transmissionPowerStr = configuration.OPTION.getTransmissionPowerDescription();
      
      sendDebugMessage("LoRa E32 configuration read successfully");
    } else {
      Serial.print("  ❌ Error reading configuration: ");
      Serial.println(configContainer.status.getResponseDescription());
      sendDebugMessage("  ❌ Error reading LoRa E32 configuration: " + String(configContainer.status.getResponseDescription()));
    }
    
    if (configContainer.data != nullptr) {
      configContainer.close();
    }
  // }
  
  Serial.println("=== LoRa E32 Initialization Complete ===");
}


// Set LoRa operating mode
void M_LoRa_E32::setLoRaOperatingMode(uint8_t mode) {
  switch (mode) {
    case 0: // Normal Mode (M0=0, M1=0)
      digitalWrite(E32_M0_PIN, LOW);
      digitalWrite(E32_M1_PIN, LOW);
      e32ttl100.setMode(MODE_0_NORMAL);
      // systemStatus.loraE32.operatingMode = 0;
      sendDebugMessage("LoRa E32 set to Normal Mode");
      break;
    case 1: // Wake-Up Mode (M0=1, M1=0)
      digitalWrite(E32_M0_PIN, HIGH);
      digitalWrite(E32_M1_PIN, LOW);
      e32ttl100.setMode(MODE_1_WAKE_UP);
      // systemStatus.loraE32.operatingMode = 1;
      sendDebugMessage("LoRa E32 set to Wake-Up Mode");
      break;
    case 2: // Power-Saving Mode (M0=0, M1=1)
      digitalWrite(E32_M0_PIN, LOW);
      digitalWrite(E32_M1_PIN, HIGH);
      e32ttl100.setMode(MODE_2_POWER_SAVING);
      // systemStatus.loraE32.operatingMode = 2;
      sendDebugMessage("LoRa E32 set to Power-Saving Mode");
      break;
    case 3: // Sleep Mode (M0=1, M1=1)
      digitalWrite(E32_M0_PIN, HIGH);
      digitalWrite(E32_M1_PIN, HIGH);
      e32ttl100.setMode(MODE_3_PROGRAM);
      // systemStatus.loraE32.operatingMode = 3;
      sendDebugMessage("LoRa E32 set to Sleep Mode");
      break;
    default:
      sendDebugMessage("Invalid LoRa E32 operating mode");
      return;
  }
  // saveLoRaConfig(); // Lưu chế độ vào LittleFS
  vTaskDelay(pdMS_TO_TICKS(100));// Đợi module ổn định
}



// Set LoRa E32 configuration
void M_LoRa_E32::setLoRaConfig() {
    loraE32.setLoRaOperatingMode(3); // MODE_3_PROGRAM
    LoRaE32Config config;
    Configuration loraConfig;
    loraConfig.HEAD = 0xC0;
    loraConfig.ADDH = config.addh;
    loraConfig.ADDL = config.addl;
    loraConfig.CHAN = config.chan;
    loraConfig.SPED.uartParity = config.uartParity;
    loraConfig.SPED.uartBaudRate = config.uartBaudRate;
    loraConfig.SPED.airDataRate = config.airDataRate;
    loraConfig.OPTION.fixedTransmission = config.fixedTransmission;
    loraConfig.OPTION.ioDriveMode = config.ioDriveMode;
    loraConfig.OPTION.wirelessWakeupTime = config.wirelessWakeupTime;
    loraConfig.OPTION.fec = config.fec;
    loraConfig.OPTION.transmissionPower = config.transmissionPower;

    ResponseStatus rs = e32ttl100.setConfiguration(loraConfig, WRITE_CFG_PWR_DWN_SAVE);
    if (rs.code == SUCCESS) {
      Serial.println("LoRa E32 configuration set successfully");
      sendDebugMessage("LoRa E32 configuration set successfully");
    }else {
      Serial.print("  ❌ Error setting LoRa E32 configuration: ");
      Serial.println(rs.getResponseDescription());
      sendDebugMessage("  ❌ Error setting LoRa E32 configuration: " + rs.getResponseDescription());
    }
    if (rs.code == SUCCESS) {
        // systemStatus.loraE32.initialized = true;
        // systemStatus.loraE32.operatingMode = 0; // Normal mode
      sendDebugMessage("LoRa E32 configuration saved successfully");
        ResponseStructContainer configContainer;
        configContainer = e32ttl100.getConfiguration();
    
      if (configContainer.status.code == 1 && configContainer.data != nullptr) {
        Configuration configuration = *(Configuration*)configContainer.data;
        Serial.println("Configuration read successfully!");
        
        printParameters(configuration);
      }
    } else {
        Serial.print("  ❌ Error saving LoRa E32 configuration: ");
        Serial.println(rs.getResponseDescription());
        sendDebugMessage("  ❌ Error saving LoRa E32 configuration: " + String(rs.getResponseDescription()));
    }
    loraE32.setLoRaOperatingMode(0); // Quay lại chế độ Normal
}


String M_LoRa_E32::GetJsonConfig(){
    loraE32.setLoRaOperatingMode(3); // MODE_3_PROGRAM

    ResponseStructContainer configContainer = e32ttl100.getConfiguration();
    String result = "";
    if (configContainer.status.code == 1 && configContainer.data != nullptr) {
        Configuration newConfig = *(Configuration*)configContainer.data;
        LoRaE32Config config;
        DynamicJsonDocument jsonConfig(256);
        jsonConfig["role"] = config.loraRole; 
        jsonConfig["addh"] = newConfig.ADDH;
        jsonConfig["addl"] = newConfig.ADDL;
        jsonConfig["chan"] = newConfig.CHAN;
        jsonConfig["uartParity"] = newConfig.SPED.uartParity;
        jsonConfig["uartBaudRate"] = newConfig.SPED.uartBaudRate;
        jsonConfig["airDataRate"] = newConfig.SPED.airDataRate;
        jsonConfig["fixedTransmission"] = newConfig.OPTION.fixedTransmission;
        jsonConfig["ioDriveMode"] = newConfig.OPTION.ioDriveMode;
        jsonConfig["wirelessWakeupTime"] = newConfig.OPTION.wirelessWakeupTime;
        jsonConfig["fec"] = newConfig.OPTION.fec;
        jsonConfig["transmissionPower"] = newConfig.OPTION.transmissionPower;

        String jsonStr;
        serializeJson(jsonConfig, jsonStr);
        result = jsonStr;
    } else {
        Serial.print("  ❌ Error reading configuration: ");
        Serial.println(configContainer.status.getResponseDescription());
        sendDebugMessage("  ❌ Error reading LoRa E32 configuration: " + String(configContainer.status.getResponseDescription()));
    }

    if (configContainer.data != nullptr) {
        configContainer.close();
    }
    loraE32.setLoRaOperatingMode(0); // Quay lại chế độ Normal
    return result;
}

void M_LoRa_E32::SetConfigFromJson(String jsonString) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.println("Invalid JSON received: " + String(error.c_str()));
    sendDebugMessage("Invalid JSON received");
    sendDebugMessage("  ❌ Error: " + String(error.c_str()));
    sendDebugMessage("Command: " + jsonString);
    return;
  }

  // Validate all required keys are present and are integers
  const char* keys[] = {"addh", "addl", "chan", "uartParity", "uartBaudRate", "airDataRate", "fixedTransmission", "ioDriveMode", "wirelessWakeupTime", "fec", "transmissionPower"};
  for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i) {
    if (!doc.containsKey(keys[i]) || !doc[keys[i]].is<int>()) {
      sendDebugMessage(String("  ❌ Invalid or missing key in LoRa E32 configuration data: ") + keys[i]);
      return;
    }
  }

  LoRaE32Config config;
  config.addh = doc["addh"].as<int>();
  config.addl = doc["addl"].as<int>();
  config.chan = doc["chan"].as<int>();
  config.uartParity = doc["uartParity"].as<int>();
  config.uartBaudRate = doc["uartBaudRate"].as<int>();
  config.airDataRate = doc["airDataRate"].as<int>();
  config.fixedTransmission = doc["fixedTransmission"].as<int>();
  config.ioDriveMode = doc["ioDriveMode"].as<int>();
  config.wirelessWakeupTime = doc["wirelessWakeupTime"].as<int>();
  config.fec = doc["fec"].as<int>();
  config.transmissionPower = doc["transmissionPower"].as<int>();

  // Gọi hàm để thiết lập cấu hình LoRa E32
  setLoRaConfig();
}
// Print configuration parameters
void printParameters(struct Configuration configuration) {
  Serial.println("======== LORA E32 CONFIGURATION ========");
  
  Serial.print("HEAD: ");
  Serial.print(configuration.HEAD, BIN);
  Serial.print(" (");
  Serial.print(configuration.HEAD, DEC);
  Serial.print(") 0x");
  Serial.println(configuration.HEAD, HEX);
  
  Serial.println();
  
  Serial.print("High Address (ADDH): ");
  Serial.print(configuration.ADDH, DEC);
  Serial.print(" (0x");
  Serial.print(configuration.ADDH, HEX);
  Serial.print(", ");
  Serial.print(configuration.ADDH, BIN);
  Serial.println(")");
  
  Serial.print("Low Address (ADDL): ");
  Serial.print(configuration.ADDL, DEC);
  Serial.print(" (0x");
  Serial.print(configuration.ADDL, HEX);
  Serial.print(", ");
  Serial.print(configuration.ADDL, BIN);
  Serial.println(")");
  
  Serial.print("Channel (CHAN): ");
  Serial.print(configuration.CHAN, DEC);
  Serial.print(" → ");
  Serial.println(configuration.getChannelDescription());
  
  Serial.println();
  Serial.println("=== SPEED CONFIGURATION (SPED) ===");
  
  Serial.print("UART Parity: ");
  Serial.print(configuration.SPED.uartParity, BIN);
  Serial.print(" → ");
  Serial.println(configuration.SPED.getUARTParityDescription());
  
  Serial.print("UART Baud Rate: ");
  Serial.print(configuration.SPED.uartBaudRate, BIN);
  Serial.print(" → ");
  Serial.println(configuration.SPED.getUARTBaudRate());
  
  Serial.print("Air Data Rate: ");
  Serial.print(configuration.SPED.airDataRate, BIN);
  Serial.print(" → ");
  Serial.println(configuration.SPED.getAirDataRate());
  
  Serial.println();
  Serial.println("=== OPTIONS ===");
  
  Serial.print("Fixed Transmission: ");
  Serial.print(configuration.OPTION.fixedTransmission, BIN);
  Serial.print(" → ");
  Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  
  Serial.print("IO Drive Mode: ");
  Serial.print(configuration.OPTION.ioDriveMode, BIN);
  Serial.print(" → ");
  Serial.println(configuration.OPTION.getIODroveModeDescription());
  
  Serial.print("Wireless Wakeup Time: ");
  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);
  Serial.print(" → ");
  Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  
  Serial.print("FEC (Forward Error Correction): ");
  Serial.print(configuration.OPTION.fec, BIN);
  Serial.print(" → ");
  Serial.println(configuration.OPTION.getFECDescription());
  
  Serial.print("Transmission Power: ");
  Serial.print(configuration.OPTION.transmissionPower, BIN);
  Serial.print(" → ");
  Serial.println(configuration.OPTION.getTransmissionPowerDescription());
  
  Serial.println("===================================");
}

#include <ArduinoJson.h>

const char lora_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LoRa E32 Config</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body{font-family:sans-serif;}
    .card{border:1px solid #ccc;padding:16px;margin:8px;border-radius:8px;}
    label{display:block;margin-top:8px;}
  </style>
</head>
<body>
  <h2>LoRa E32 Config</h2>
  <form id="loraForm">
    <div class="card">
      <label>Role:
        <select name="role" required>
          <option value="master">Master</option>
          <option value="slave">Slave</option>
        </select>
      </label>
      <label>Address (ADDH:ADDL): 
        <input name="address" type="number" min="0" max="65535" required>
      </label>
      <label>Channel: <input name="chan" type="number" min="0" max="83" required></label>

      <label>UART Parity:
        <select name="uartParity" required>
          <option value="0">8N1 (None)</option>
          <option value="1">8O1 (Odd)</option>
          <option value="2">8E1 (Even)</option>
        </select>
      </label>
      <label>UART BaudRate:
        <select name="uartBaudRate" required>
          <option value="0">1200</option>
          <option value="1">2400</option>
          <option value="2">4800</option>
          <option value="3">9600</option>
          <option value="4">19200</option>
          <option value="5">38400</option>
          <option value="6">57600</option>
          <option value="7">115200</option>
        </select>
      </label>
      <label>Air Data Rate:
        <select name="airDataRate" required>
          <option value="0">0.3 kbps</option>
          <option value="1">1.2 kbps</option>
          <option value="2">2.4 kbps</option>
          <option value="3">4.8 kbps</option>
          <option value="4">9.6 kbps</option>
          <option value="5">19.2 kbps</option>
          <option value="6">38.4 kbps</option>
          <option value="7">62.5 kbps</option>
        </select>
      </label>
      <label>Fixed Transmission:
        <select name="fixedTransmission" required>
          <option value="0">Transparent</option>
          <option value="1">Fixed</option>
        </select>
      </label>
      <label>IO Drive Mode:
        <select name="ioDriveMode" required>
          <option value="0">Push-pull</option>
          <option value="1">Open-drain</option>
        </select>
      </label>
      <label>Wireless Wakeup Time:
        <select name="wirelessWakeupTime" required>
          <option value="0">250ms</option>
          <option value="1">500ms</option>
          <option value="2">750ms</option>
          <option value="3">1000ms</option>
          <option value="4">1250ms</option>
          <option value="5">1500ms</option>
          <option value="6">1750ms</option>
          <option value="7">2000ms</option>
        </select>
      </label>
      <label>FEC:
        <select name="fec" required>
          <option value="0">Off</option>
          <option value="1">On</option>
        </select>
      </label>
      <label>Transmission Power:
        <select name="transmissionPower" required>
          <option value="0">30dBm (Max)</option>
          <option value="1">27dBm</option>
          <option value="2">24dBm</option>
          <option value="3">21dBm (Min)</option>
        </select>
      </label>
    </div>
    <button type="submit">Save</button>
  </form>
  <script>
    function loadLoraCfg() {
      fetch('/api/get_lora_config')
        .then(res => res.json())
        .then(cfg => {
          if(cfg.addh !== undefined && cfg.addl !== undefined) {
            let addr = (parseInt(cfg.addh) << 8) | parseInt(cfg.addl);
            let el = document.querySelector('[name=address]');
            if(el) el.value = addr;
          }
          if(cfg.role !== undefined) {
            let el = document.querySelector('[name=role]');
            if(el) el.value = cfg.role;
          }
          for(let k in cfg) {
            if(k === "addh" || k === "addl") continue;
            let el = document.querySelector('[name='+k+']');
            if(el) el.value = cfg[k];
          }
        })
        .catch(err => console.error(err));
    }

    document.getElementById('loraForm').onsubmit = function(e){
      e.preventDefault();
      let fd = new FormData(this);

      // Split address into addh/addl
      let addr = parseInt(fd.get('address')) || 0;
      fd.set('addh', (addr >> 8) & 0xFF);
      fd.set('addl', addr & 0xFF);

      // Remove combined address field before sending
      fd.delete('address');

      fetch('/lora/setcfg', {method:'POST', body:fd}).then(r=>r.text()).then(t=>{
        alert(t);
        loadLoraCfg();
      });
    };
    loadLoraCfg();
  </script>
  <a href="/">Monitor</a>
</body>
</html>
)rawliteral";

const char lora_simple_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>LoRa E32 Simple Config</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #f5f7fa;
      margin: 0;
      padding: 0;
    }
    .container {
      max-width: 400px;
      margin: 40px auto;
      background: #fff;
      border-radius: 12px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.08);
      padding: 32px 24px 24px 24px;
    }
    h2 {
      text-align: center;
      color: #2d3a4b;
      margin-bottom: 24px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #34495e;
      font-weight: 500;
    }
    input, select {
      width: 100%;
      padding: 10px 8px;
      margin-bottom: 18px;
      border: 1px solid #d1d5db;
      border-radius: 6px;
      font-size: 1rem;
      background: #f9fafb;
      transition: border 0.2s;
    }
    input:focus, select:focus {
      border-color: #4f8cff;
      outline: none;
      background: #fff;
    }
    button {
      width: 100%;
      padding: 12px;
      background: linear-gradient(90deg,#4f8cff,#38b6ff);
      color: #fff;
      border: none;
      border-radius: 6px;
      font-size: 1.1rem;
      font-weight: bold;
      cursor: pointer;
      transition: background 0.2s;
      margin-top: 8px;
    }
    button:hover {
      background: linear-gradient(90deg,#38b6ff,#4f8cff);
    }
    .note {
      font-size: 0.95em;
      color: #888;
      margin-bottom: 16px;
      text-align: center;
    }
    .footer {
      text-align: center;
      margin-top: 18px;
    }
    .footer a {
      color: #4f8cff;
      text-decoration: none;
      font-size: 1em;
    }
    .footer a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Wireless Config</h2>
    <div class="note">Config Address, Network ID and Power.</div>
    <form id="loraSimpleForm">
      <label>Device ID (1-4)
        <input name="deviceId" type="number" min="1" max="4" required>
      </label>
      <label>Network ID (0-255)
        <input name="networkId" type="number" min="0" max="255" required>
      </label>
      <label>Address (0-65535)
        <input name="address" type="number" min="0" max="65535" required>
      </label>
      <label>Channel (0-25)
        <input name="chan" type="number" min="0" max="25" required>
      </label>
      <label>Transmission Power
        <select name="transmissionPower" required>
          <option value="0">30dBm (Max)</option>
          <option value="1">27dBm</option>
          <option value="2">24dBm</option>
          <option value="3">21dBm (Min)</option>
        </select>
      </label>
      <button type="submit">Save</button>
    </form>
    <div class="footer">
      <a href="/">Back to Monitor</a>
    </div>
  </div>
  <script>
    // Default values for other parameters
    const defaultParams = {
      uartParity: 0,
      uartBaudRate: 3,
      airDataRate: 2,
      fixedTransmission: 0,
      ioDriveMode: 0,
      wirelessWakeupTime: 0,
      fec: 1
    };

    function loadSimpleLoraCfg() {
      fetch('/api/get_lora_config')
        .then(res => res.json())
        .then(cfg => {
          if (cfg.deviceId !== undefined) {
            let el = document.querySelector('[name=deviceId]');
            if (el) el.value = cfg.deviceId;
          }
          if (cfg.networkId !== undefined) {
            let el = document.querySelector('[name=networkId]');
            if (el) el.value = cfg.networkId;
          }
          if (cfg.addh !== undefined && cfg.addl !== undefined) {
            let addr = (parseInt(cfg.addh) << 8) | parseInt(cfg.addl);
            let el = document.querySelector('[name=address]');
            if (el) el.value = addr;
          }
          if (cfg.chan !== undefined) {
            let el = document.querySelector('[name=chan]');
            if (el) el.value = cfg.chan;
          }
          if (cfg.transmissionPower !== undefined) {
            let el = document.querySelector('[name=transmissionPower]');
            if (el) el.value = cfg.transmissionPower;
          }
        })
        .catch(err => console.error(err));
    }
    document.getElementById('loraSimpleForm').onsubmit = function(e) {
      e.preventDefault();
      let fd = new FormData(this);

      // Split address into addh/addl
      let addr = parseInt(fd.get('address')) || 0;
      fd.set('addh', (addr >> 8) & 0xFF);
      fd.set('addl', addr & 0xFF);
      fd.delete('address');

      // Add default parameters
      for (let k in defaultParams) {
        fd.set(k, defaultParams[k]);
      }

      // Network ID and Device ID are kept as-is
      // fd includes: deviceId, networkId, chan, transmissionPower, addh, addl, ...

      fetch('/lora/setcfg', {
        method: 'POST',
        body: fd
      })
        .then(r => r.text())
        .then(t => {
          alert(t);
          loadSimpleLoraCfg();
        });
    };

  </script>
</body>
</html>
)rawliteral";


        LoRaE32Config loraE32Cf;
        
void M_LoRa_E32::API_Setup(AsyncWebServer &server) {

    // Web: Lora E32 config page
    server.on("/lora", HTTP_GET, [](AsyncWebServerRequest *req) {
      req->send_P(200, "text/html", lora_simple_html);
    });
    server.on("/lora-advance", HTTP_GET, [](AsyncWebServerRequest *req) {
      req->send_P(200, "text/html", lora_html);
    });
    // API: Save LoRa E32 config
    server.on("/lora/setcfg", HTTP_POST, [](AsyncWebServerRequest *req) {
      if (
        req->hasParam("addh", true) && req->hasParam("addl", true) &&
        req->hasParam("chan", true) && req->hasParam("uartParity", true) &&
        req->hasParam("uartBaudRate", true) && req->hasParam("airDataRate", true) &&
        req->hasParam("fixedTransmission", true) && req->hasParam("ioDriveMode", true) &&
        req->hasParam("wirelessWakeupTime", true) && req->hasParam("fec", true) &&
        req->hasParam("transmissionPower", true)
      ) {
        if (req->hasParam("role", true)) {
            loraE32Cf.loraRole = req->getParam("role", true)->value();
        }
            // StaticJsonDocument<256> json;
            // json["addh"] = req->getParam("addh", true)->value().toInt();
            // json["addl"] = req->getParam("addl", true)->value().toInt();
            // json["chan"] = req->getParam("chan", true)->value().toInt();
            // json["uartParity"] = req->getParam("uartParity", true)->value().toInt();
            // json["uartBaudRate"] = req->getParam("uartBaudRate", true)->value().toInt();
            // json["airDataRate"] = req->getParam("airDataRate", true)->value().toInt();
            // json["fixedTransmission"] = req->getParam("fixedTransmission", true)->value().toInt();
            // json["ioDriveMode"] = req->getParam("ioDriveMode", true)->value().toInt();
            // json["wirelessWakeupTime"] = req->getParam("wirelessWakeupTime", true)->value().toInt();
            // json["fec"] = req->getParam("fec", true)->value().toInt();
            // json["transmissionPower"] = req->getParam("transmissionPower", true)->value().toInt();

            // String jsonString;
            // serializeJson(json, jsonString);

            loraE32.setLoRaOperatingMode(3); // MODE_3_PROGRAM
            ResponseStructContainer c;
            c = e32ttl100.getConfiguration();
            // It's important get configuration pointer before all other operation
            Configuration configuration = *(Configuration*) c.data;
            Serial.println(c.status.getResponseDescription());
            Serial.println(c.status.code);

            printParameters(configuration);
            configuration.ADDL = req->getParam("addl", true)->value().toInt();
            configuration.ADDH = req->getParam("addh", true)->value().toInt();
            configuration.CHAN = req->getParam("chan", true)->value().toInt();
            configuration.SPED.uartParity = (E32_UART_PARITY)req->getParam("uartParity", true)->value().toInt();
            configuration.SPED.uartBaudRate = (UART_BPS_TYPE)req->getParam("uartBaudRate", true)->value().toInt();
            configuration.SPED.airDataRate = (AIR_DATA_RATE)req->getParam("airDataRate", true)->value().toInt();
            configuration.OPTION.fixedTransmission = (FIDEX_TRANSMISSION)req->getParam("fixedTransmission", true)->value().toInt();
            configuration.OPTION.ioDriveMode = (IO_DRIVE_MODE)req->getParam("ioDriveMode", true)->value().toInt();
            configuration.OPTION.wirelessWakeupTime = (WIRELESS_WAKE_UP_TIME)req->getParam("wirelessWakeupTime", true)->value().toInt();
            configuration.OPTION.fec = (FORWARD_ERROR_CORRECTION_SWITCH)req->getParam("fec", true)->value().toInt();
            configuration.OPTION.transmissionPower = (TRANSMISSION_POWER)req->getParam("transmissionPower", true)->value().toInt();
            

            // Set configuration changed and set to not hold the configuration
            ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
            if (rs.code == SUCCESS) {
              Serial.println("LoRa E32 configuration set successfully");
              sendDebugMessage("LoRa E32 configuration set successfully");
            }else {
              Serial.print("Error setting LoRa E32 configuration: ");
              Serial.println(rs.getResponseDescription());
              sendDebugMessage("Error setting LoRa E32 configuration: " + rs.getResponseDescription());
            }
            if (rs.code == SUCCESS) {
                // systemStatus.loraE32.initialized = true;
                // systemStatus.loraE32.operatingMode = 0; // Normal mode
              sendDebugMessage("LoRa E32 configuration saved successfully");
                ResponseStructContainer configContainer;
                configContainer = e32ttl100.getConfiguration();
            
              if (configContainer.status.code == 1 && configContainer.data != nullptr) {
                Configuration configuration = *(Configuration*)configContainer.data;
                Serial.println("Configuration read successfully!");
                
                printParameters(configuration);
              }
            } else {
                Serial.print("Error saving LoRa E32 configuration: ");
                Serial.println(rs.getResponseDescription());
                sendDebugMessage("Error saving LoRa E32 configuration: " + String(rs.getResponseDescription()));
            }
            loraE32.setLoRaOperatingMode(0); // Quay lại chế độ Normal

        // Save config to LoRa module
        // loraE32.SetConfigFromJson(jsonString);
            c.close();

        req->send(200, "text/plain", "LoRa config saved.");
      } else {
        req->send(400, "text/plain", "Invalid LoRa config");
      }
    });
    // API: Load LoRa E32 config page
    server.on("/api/get_lora_config", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = loraE32.GetJsonConfig();
        request->send(200, "application/json", json);
    });

    // LoRa command endpoint
    server.on("/lora/cmnd", HTTP_GET, [](AsyncWebServerRequest *req) {
        if (req->hasParam("cmd")) {
            String cmd = req->getParam("cmd")->value();
            loraE32.CMND(cmd);
            req->send(200, "text/plain", "Command sent: " + cmd);
        } else {
            req->send(400, "text/plain", "Missing command parameter");
        }
    });
}