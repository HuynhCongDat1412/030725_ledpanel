#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
// ==== Pin mapping (giữ nguyên như bạn đang dùng) ====
#define R1_PIN 3
#define G1_PIN 10
#define B1_PIN 11
#define R2_PIN 12
#define G2_PIN 13
#define B2_PIN 14

#define CLK_PIN 42
#define LAT_PIN 41
#define OE_PIN 47

#define A_PIN 36
#define B_PIN 35
#define C_PIN 45
#define D_PIN 46
#define E_PIN -1

#define FORMAT_LITTLEFS_IF_FAILED true

// const char* ssid = "I-Soft";
// const char* password = "i-soft@2023";
const char* ssid = "DAT PHUONG";
const char* password = "19201974";

// ==== Panel config ====
#define PANEL_RES_X 64//104//64      // Số pixel ngang của panel
#define PANEL_RES_Y 32//52//32      // Số pixel dọc của panel

// Định nghĩa struct cho từng dòng chữ
struct TextLine {
    int x;
    int y;
    int id;
    uint16_t color; 
};



// Tối đa 10 nhóm, mỗi nhóm tối đa 10 dòng (bạn có thể tăng nếu cần)
#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10

TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP];// tao mang 2 chieu de luu toa do va id cua tung dong chu trong tung nhom
int rowsTextInEachShapeFilter[MAX_GROUPS]; // Số dòng thực tế trong mỗi nhóm

#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10
#define MAX_TEXT_LENGTH 10

char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]; // [nhóm][dòng][ký tự]




// ==== Scan type mapping ====
#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;

// ==== Màu sắc mẫu ====
uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;

WebServer server(80);
WebSocketsServer webSocketServer = WebSocketsServer(81);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void setTextContentAndCoord(int group, int row, int x, int y, int id, const char* content);
void get_CoordsAndID(JsonArray textInfoArray, TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP]);
void showAllTextLines(TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP], char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
void drawShapeFromType(DynamicJsonDocument dataIn);
void draw_MSG(DynamicJsonDocument dataIn);
void setup_web();
void setup_WS();
void saveConfig(const DynamicJsonDocument& doc, String filePath = "/CONFIG.json");
DynamicJsonDocument loadConfig(String filePath = "/CONFIG.json");
void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void DrawRound(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void drawTextInShape(DynamicJsonDocument doc);
void clearMSG(DynamicJsonDocument doc);
void setup_littleFS();
void setup_ledPanel();

void mapDataToTextContents( DynamicJsonDocument doc, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
void mapDocToTextContents(const DynamicJsonDocument doc, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
DynamicJsonDocument createSampleJson() ;
void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color = myWHITE) {
  virtualDisp->drawLine(x0, y0, x1, y1, color);
  Serial.print("Đã vẽ");
}
void DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t color = myWHITE) {
  virtualDisp->drawRect(x, y, w, h, color);
}
void DrawCircle(uint16_t x, uint16_t y, uint16_t r,uint16_t color = myWHITE) {
  virtualDisp->drawCircle(x, y, r, color);
}
void DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color = myWHITE) {
  virtualDisp->drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

void drawShapeFromType(DynamicJsonDocument dataIn) {

    if (!dataIn.is<JsonArray>() || dataIn.size() < 1) {
        Serial.println("Dữ liệu JSON không đúng định dạng mảng.");
        return;
    }

    JsonArray shapes = dataIn[0];
    for (JsonVariant shapeObj : shapes) {
        if (!shapeObj.containsKey("type") || !shapeObj.containsKey("data")) {
            Serial.println("Bỏ qua hình không hợp lệ!");
            continue;
        }

        int Type = shapeObj["type"];
        JsonArray dataArray = shapeObj["data"];

        uint16_t shapeColor = myWHITE;
        if (shapeObj.containsKey("color")) {
            String hex = shapeObj["color"].as<String>(); // VD: "#FF0000"
            int r = strtol(hex.substring(1,3).c_str(), nullptr, 16);
            int g = strtol(hex.substring(3,5).c_str(), nullptr, 16);
            int b = strtol(hex.substring(5,7).c_str(), nullptr, 16);
            shapeColor = virtualDisp->color565(r, g, b);
        }

        uint16_t v1 = dataArray.size() > 0 ? dataArray[0].as<int>() : 0;
        uint16_t v2 = dataArray.size() > 1 ? dataArray[1].as<int>() : 0;
        uint16_t v3 = dataArray.size() > 2 ? dataArray[2].as<int>() : 0;
        uint16_t v4 = dataArray.size() > 3 ? dataArray[3].as<int>() : 0;
        uint16_t v5 = dataArray.size() > 4 ? dataArray[4].as<int>() : 0;
        uint16_t v6 = dataArray.size() > 5 ? dataArray[5].as<int>() : 0;
        switch(Type) {
            case 0: DrawLine(v1, v2, v3, v4,shapeColor); break;
            case 1: DrawRect(v1, v2, v3, v4,shapeColor); break;
            case 2: DrawCircle(v1, v2, v3, shapeColor); break;
            case 3: DrawTriangle(v1, v2, v3, v4, v5, v6, shapeColor); break;
            default:
                Serial.println("Loại hình không xác định: " + String(Type));
                break;
        }
    }

    Serial.println("Drawing complete.");
}

DynamicJsonDocument parseStringToJSON(String jsonStr) {
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) {
        Serial.print("Lỗi parse JSON: ");
        Serial.println(err.c_str());
        return DynamicJsonDocument(0); // Trả về doc rỗng nếu lỗi
    }
    return doc;
}

void drawTextInShape(DynamicJsonDocument doc) {
    if (!doc.is<JsonArray>() || doc.size() < 2) {
        return;
    }
    get_CoordsAndID(doc[1].as<JsonArray>(), textCoorID);
    for (int shapeIndex = 0; shapeIndex < MAX_GROUPS; shapeIndex++) {
        for (int rowIndex = 0; rowIndex < rowsTextInEachShapeFilter[shapeIndex]; rowIndex++) {
            int x = textCoorID[shapeIndex][rowIndex].x;
            int y = textCoorID[shapeIndex][rowIndex].y;
            virtualDisp->setCursor(x, y);
            virtualDisp->setTextColor(myWHITE);
            virtualDisp->setTextSize(1); // hoặc 2 nếu muốn chữ to hơn
            virtualDisp->print(textContents[shapeIndex][rowIndex]);
        }
    }
}

void clearMSG(DynamicJsonDocument doc)
{
    // Nếu không còn shape và text, reset textContents và rowsTextInEachShapeFilter
        for (int g = 0; g < MAX_GROUPS; g++) {
            rowsTextInEachShapeFilter[g] = 0;
            for (int l = 0; l < MAX_LINES_PER_GROUP; l++) {
                textContents[g][l][0] = '\0';
                textCoorID[g][l].x = 0;
                textCoorID[g][l].y = 0;
                textCoorID[g][l].id = 0;
            }
        }
    }


void draw_MSG(DynamicJsonDocument doc)
{
    virtualDisp->fillScreen(myBLACK);

    drawShapeFromType(doc);

    // Lấy tọa độ và id cho text
    if (doc.size() > 1 && doc[1].is<JsonArray>()) {
        get_CoordsAndID(doc[1].as<JsonArray>(), textCoorID);
    }

    // Map nội dung text vào đúng vị trí
    mapDataToTextContents(doc, textContents);

    // Hiển thị tất cả các dòng chữ
    showAllTextLines(textCoorID, textContents);

    // Gửi JSON ra WebSocket
    String jsonOut;
    serializeJson(doc, jsonOut);
    webSocketServer.broadcastTXT(jsonOut);
}

void showAllTextLines(TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP], char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]) {
  for (int shapeIndex = 0; shapeIndex < MAX_GROUPS; shapeIndex++) {
    // Nếu nhóm này không có dòng nào thì bỏ qua
    if (rowsTextInEachShapeFilter[shapeIndex] == 0) continue;

    for (int rowIndex = 0; rowIndex < rowsTextInEachShapeFilter[shapeIndex]; rowIndex++) {
      int x = textCoorID[shapeIndex][rowIndex].x;
      int y = textCoorID[shapeIndex][rowIndex].y;

      virtualDisp->setCursor(x, y);
      virtualDisp->setTextColor(textCoorID[shapeIndex][rowIndex].color); // hoặc myWHITE
      virtualDisp->setTextSize(1);
      virtualDisp->print(textContents[shapeIndex][rowIndex]);
    }
  }

  virtualDisp->flipDMABuffer(); // cập nhật nội dung vẽ
}

// textInfoArray là mảng JSON chứa thông tin về các dòng chữ trong từng shape
void get_CoordsAndID(JsonArray textInfoArray, TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP]) {
  for (int shapeIndex = 0; shapeIndex <  textInfoArray.size() && shapeIndex < MAX_GROUPS; shapeIndex++) {
    JsonArray groupTextInEachShape =  textInfoArray[shapeIndex].as<JsonArray>();
    int rowsTextInEachShape = groupTextInEachShape.size();
    rowsTextInEachShapeFilter[shapeIndex] = rowsTextInEachShape > MAX_LINES_PER_GROUP ? MAX_LINES_PER_GROUP : rowsTextInEachShape;
    for (int row = 0; row < rowsTextInEachShapeFilter[shapeIndex]; row++) {
      JsonObject rowInfor = groupTextInEachShape[row];
      JsonArray rowInfor_xy = rowInfor["xy"];
      textCoorID[shapeIndex][row].x = rowInfor_xy[0].as<int>();
      textCoorID[shapeIndex][row].y = rowInfor_xy[1].as<int>();
      textCoorID[shapeIndex][row].id = rowInfor["id"].as<int>();
      if (rowInfor.containsKey("color")) {
        String hex = rowInfor["color"].as<String>(); // VD: "#FF0000"
        int r = strtol(hex.substring(1,3).c_str(), nullptr, 16);
        int g = strtol(hex.substring(3,5).c_str(), nullptr, 16);
        int b = strtol(hex.substring(5,7).c_str(), nullptr, 16);
        textCoorID[shapeIndex][row].color = virtualDisp->color565(r, g, b);
    } else {
        textCoorID[shapeIndex][row].color = myWHITE;
    }
    }
  }
}

// void setup_web() {
//         // Start HTTP server and webSocketServer
//     server.on("/", []() {
//       server.send_P(200, "text/html", MAINWEBPAGE);
    
//     });
// }
void setup_WS() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());


    server.begin();
    Serial.println("✅ HTTP server started!");
    webSocketServer.begin();
    webSocketServer.onEvent(webSocketEvent);
    Serial.println("✅ WebSocket server started!");
}

//tác dụng: lưu nội dung trong doc vào file config.json
// nếu file đã tồn tại thì ghi đè, nếu không thì tạo mới
void saveConfig(const DynamicJsonDocument& doc, String filePath ) {
  File file = LittleFS.open(filePath, "w");
  if (!file) {
    // Serial.println("❌ Lỗi: Không thể mở file config để ghi.");
    return;
  }
  if (serializeJson(doc, file) == 0) {
    // Serial.println("❌ Lỗi: Ghi file config thất bại (JSON rỗng?).");
  } else {
    Serial.print("✅ Đã lưu config thành công vào /config.json: ");
    String jsonOut;
    serializeJson(doc, jsonOut);
    // Serial.println(jsonOut);

  }
  file.close();
}

// Tác dụng: đọc nội dung từ file config.json rồi return biến struct Config
DynamicJsonDocument loadConfig(String filePath) {
  DynamicJsonDocument doc(2048);
  if (!LittleFS.exists(filePath)) {
    Serial.println("⚠️ File config chưa tồn tại. Sẽ tạo mặc định...");
    saveConfig(doc, filePath); // Lưu file mặc định
    return doc;
  }
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    Serial.println("❌ Lỗi mở file để đọc.");
    return doc;
  }
  DeserializationError err = deserializeJson(doc, file);
  file.close();
if (err) {
    // Serial.println("❌ Lỗi parse JSON. Tạo lại file mặc định.");
    // doc.to<JsonArray>(); // Đảm bảo doc là mảng
    // doc.add(JsonArray()); // shapes rỗng
    // doc.add(JsonArray()); // texts rỗng
    // saveConfig(doc, filePath);
    Serial.println("Đã bị lỗi ");
    String jsonOut;
    serializeJson(doc, jsonOut);
    Serial.print ("Lỗi: ");
    // Serial.println(jsonOut);
    return doc;
}

  Serial.println("✅ Đã đọc config từ " + filePath);
    String jsonOut;
    serializeJson(doc, jsonOut);
    Serial.print ("Nội dung JSON load: ");
    // Serial.println(jsonOut);
    // webSocketServer.broadcastTXT(jsonOut);
  return doc;
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
        Serial.printf("Client %u connected\n", num);
        String jsonOut;
        serializeJson(loadConfig("/CONFIG.json"), jsonOut);
        webSocketServer.sendTXT(num, jsonOut); // Gửi riêng cho client vừa kết nối
    }
  if (type == WStype_DISCONNECTED) {
    Serial.printf("Client %u disconnected\n", num);
  }
  if (type == WStype_TEXT) {
    String jsonStr = (const char*)payload;
    Serial.println("Nhận JSON từ web:");
    Serial.println(jsonStr);
    DynamicJsonDocument doc = parseStringToJSON(jsonStr);
    
    saveConfig(doc, "/CONFIG.json"); // Lưu lại file config.json

    if (!doc.isNull()) {
      draw_MSG(doc); // Gọi hàm vẽ với dữ liệu JSON
    } else {
      Serial.println("❌ Dữ liệu JSON không hợp lệ.");
    }
   
  }
}

void setup_littleFS () {
    delay(2000);
    Serial.println("=== VirtualMatrixPanel Example 3: Single 1/4 Scan Panel ===");
    if (!LittleFS.begin()) {
        Serial.println("❌ Không thể mount LittleFS! Đang format lại...");
        LittleFS.format(); // Thêm dòng này để format lại
    if (!LittleFS.begin()) {
        Serial.println("❌ Vẫn không mount được LittleFS sau khi format!");
        return;
    }
    Serial.println("✅ Đã format và mount lại LittleFS thành công!");
    }
}
void setup_ledPanel(){
    HUB75_I2S_CFG mxconfig(
    PANEL_RES_X * 2,
    PANEL_RES_Y / 2,
    1,
    HUB75_I2S_CFG::i2s_pins{
      R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN,
      A_PIN, B_PIN, C_PIN, D_PIN, E_PIN,
      LAT_PIN, OE_PIN, CLK_PIN
    }
  );
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);
  dma_display->clearScreen();


  virtualDisp = new VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>(1, 1, PANEL_RES_X, PANEL_RES_Y);
  virtualDisp->setDisplay(*dma_display);

  myBLACK = virtualDisp->color565(0, 0, 0);
  myWHITE = virtualDisp->color565(255, 255, 255);

  virtualDisp->fillScreen(myBLACK);
}

DynamicJsonDocument createSampleJson() {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();

    for (int group = 0; group < MAX_GROUPS; group++) {
        for (int row = 0; row < 4; row++) {
            JsonObject obj = arr.createNestedObject();
            obj["data"] = String(group) + "-" + String(row);
            JsonArray info = obj.createNestedArray("info");
            info.add(group); // shapeIndex
            info.add(row);   // row
        }
    }
    return doc;
}
void mapDataToTextContents( DynamicJsonDocument doc, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]) {
    JsonArray arr = doc.as<JsonArray>();
    for (JsonVariant v : arr) {
        if (!v.is<JsonObject>()) continue;
        String content = v["data"] | "";
        if (!v["info"].is<JsonArray>()) continue;
        JsonArray info = v["info"].as<JsonArray>();
        if (info.size() < 2) continue;

        int group = info[0].as<int>();
        int row = info[1].as<int>();
        if (group >= 0 && group < MAX_GROUPS && row >= 0 && row < MAX_LINES_PER_GROUP) {
            strncpy(textContents[group][row], content.c_str(), MAX_TEXT_LENGTH - 1);
            textContents[group][row][MAX_TEXT_LENGTH - 1] = '\0';
        }
    }
    Serial.println("Nội dung textContents sau khi map:");
    for (int g = 0; g < MAX_GROUPS; g++) {
        for (int r = 0; r < MAX_LINES_PER_GROUP; r++) {
            if (textContents[g][r][0] != '\0') {
                Serial.printf("textContents[%d][%d]: %s\n", g, r, textContents[g][r]);
            }
        }
    }
} 

// void mapTextDataToCoorByID(JsonArray textDataArray, TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP], char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]) {
//     for (JsonVariant v : textDataArray) {
//     if (!v.is<JsonObject>()) continue;

//     String content = v["data"] | "";
//     auto infoVar = v["info"];
//     if (!infoVar.is<JsonArray>()) continue;
//     JsonArray info = infoVar.as<JsonArray>();
//     if (info.size() < 2) continue;

//     int group = info[0].as<int>();
//     int row = info[1].as<int>();

//     if (group < 0 || group >= MAX_GROUPS || row < 0 || row >= MAX_LINES_PER_GROUP) continue;

//     // Gán nội dung vào đúng tọa độ nếu có sẵn
//     if (textCoorID[group][row].id != 0) {
//         strncpy(textContents[group][row], content.c_str(), MAX_TEXT_LENGTH - 1);
//         textContents[group][row][MAX_TEXT_LENGTH - 1] = '\0';
//         Serial.printf("✅ Gán '%s' vào textContents[%d][%d] (id=%d)\n", content.c_str(), group, row, textCoorID[group][row].id);
//     }
// }

// }
void setup() {

    Serial.begin(115200);
    setup_littleFS(); 
    setup_WS();
    // setup_web();
    setup_ledPanel();
    // listLittleFSFiles();

    draw_MSG(loadConfig("/CONFIG.json"));
    DynamicJsonDocument doc = createSampleJson();
    String jsonOut;
    serializeJson(doc, jsonOut);
    Serial.println("Nội dung doc:");
    Serial.println(jsonOut);    
    mapDataToTextContents(doc, textContents);

    // Hiển thị chữ lên màn hình
    // showAllTextLines(textCoorID, textContents);
}

void loop() {
  // Không làm gì trong loo
  webSocketServer.loop();
  server.handleClient();
  }

