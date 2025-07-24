

#define USE_LMD_V1
// #define USE_LMD_Master
// #define USE_LMD_V2
// #define USE_Test_P5_RGB_64x32
// #define USE_OpenWeatherMap
#ifdef USE_LMD_Master
#include <Arduino.h>
#include "ALC_Project.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>

AsyncWebServer server(80);
#include "SocketConsole.h"

#include "WifiPostal.h"
#include "ModuleLoRaE32.h"
M_LoRa_E32 loraE32Master;

#define USE_ALC

void setup(){
  Serial.begin(115200);
  Serial.println("Starting LMD Master...");
  WiFi_AP_setup(server); // Khởi tạo WiFi AP
#ifdef USE_ALC
  ALC_setup(server); // Khởi tạo ALC Project
  // Initialize LoRa
  loraE32Master.initLoRaE32();
  loraE32Master.API_Setup(server); // Khởi tạo API cho LoRa E32
#endif // USE_ALC

  server.begin(); // Bắt đầu máy chủ web

}

void loop(){
  WiFi_AP_loop(); // Xử lý các sự kiện WiFi AP 
  // Xử lý các sự kiện khác nếu cần
  delay(10); // Giảm tải CPU
  #ifdef USE_ALC
  ALC_loop(); // Xử lý các sự kiện ALC Project
  #endif // USE_ALC
  static long lastMillis = 0;
  if (millis() - lastMillis > 10000) { // Mỗi giây giây
    lastMillis = millis();
    // Gửi thông điệp debug qua Serial
    Serial.println("CPU Temperature: " + String(temperatureRead()) + "°C  | Uptime: " + String(millis() / 1000) + "s  | Free Heap/min: " + String(ESP.getFreeHeap()/1024) + "/" + String(ESP.getMinFreeHeap()/1024) + " Kb" );
  }
}


#endif// USE_LMD_Master

#ifdef USE_LMD_V1
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
// #include <WebServer.h>
// #include <WebSocketsServer.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
// #include <Fonts/FreeSerifBoldOblique9pt7b.h>
// #include <Fonts/FreeSerifOblique9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

// Font đặc biệt (pixel, mini, custom)
#include <Fonts/B_5px.h>
#include <Fonts/hud5pt7b.h>
#include <Fonts/mythic_5pixels.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/TomThumb.h>

#include <ESPAsyncWebServer.h>



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

const char* ssid = "I-Soft";
const char* password = "i-soft@2023";
// const char* ssid = "DAT PHUONG";
// const char* password = "19201974";

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

#define USE_LORA

#ifdef USE_LORA

// LoRa E32 Configuration Pins
#define E32_M0_PIN    15
#define E32_M1_PIN    16
#define E32_TX_PIN    17
#define E32_RX_PIN    18
#define E32_AUX_PIN   -1
  HardwareSerial MySerial1(1); // UART1
// M_LoRa_E32::M_LoRa_E32() {}

// void sendDebugMessage(const String& message) {
//     Serial.println(message); // Gửi thông điệp debug qua Serial
// }
// #define LOG(s) sendDebugMessage(s)

// LoRa E32 instance
// LoRa_E32 e32ttl100(&Serial1, E32_AUX_PIN, E32_M0_PIN, E32_M1_PIN);

#endif // USE_LORA

// Tối đa 10 nhóm, mỗi nhóm tối đa 10 dòng (bạn có thể tăng nếu cần)
#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10

TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP];// tao mang 2 chieu de luu toa do va id cua tung dong chu trong tung nhom
int rowsTextInEachShapeFilter[MAX_GROUPS]; // Số dòng thực tế trong mỗi nhóm

#define MAX_GROUPS 10
#define MAX_LINES_PER_GROUP 10
#define MAX_TEXT_LENGTH 10

char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]; // [nhóm][dòng][ký tự]

int numberContents[MAX_TEXT_LENGTH]; // Dùng để lưu số đếm, nếu có


// #include "ALC_Project.h"
// Thay Serial.print/println bằng hàm này
// void consolePrintln(const String& line) {
//   Serial.println(line);
// //   addConsoleLine(line + '\n');
// }


// ==== Scan type mapping ====
#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;

// ==== Màu sắc mẫu ====
uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;




AsyncWebServer server(80);
AsyncWebSocket webSocketServer("/ws");
// void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void setTextContentAndCoord(int group, int row, int x, int y, int id, const char* content);
void get_CoordsAndID(JsonArray textInfoArray, TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP]);
const GFXfont* getFontByIndex(int index);
const GFXfont* getFontByName(const String& name);
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
void printLedPanel( int16_t number, int x, int y, int8_t fontSize, const GFXfont* font, uint16_t color) ;
void mapDataToTextContents( JsonArray arr, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
void mapDocToTextContents(const DynamicJsonDocument doc, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]);
DynamicJsonDocument loadConfig(String filePath);
int getTotalPages();
void showCurrentPage(int currentPage);
DynamicJsonDocument createSampleJson() ;
void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color = myWHITE) {
  virtualDisp->drawLine(x0, y0, x1, y1, color);
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

int CurrentPage = 0;
int TotalPages = 0;
DynamicJsonDocument configDoc = loadConfig("/CONFIG.json");


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

    // Serial.println("Drawing complete.");
}

DynamicJsonDocument parseStringToJSON(String jsonStr) {
    DynamicJsonDocument doc(8024);
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

void GenNumber(int number,int x, int y, int font, uint16_t color = myWHITE) {
            // 5x5 7-segment digits
            // Segment map:    0b0GFEDCBA
            const uint8_t segMap[10] = {
                0b00111111, // 0: A B C D E F
                0b00000110, // 1: B C
                0b01011011, // 2: A B D E G
                0b01001111, // 3: A B C D G
                0b01100110, // 4: B C F G
                0b01101101, // 5: A C D F G
                0b01111101, // 6: A C D E F G
                0b00000111, // 7: A B C
                0b01111111, // 8: A B C D E F G
                0b01101111  // 9: A B C D F G
            };
            if (font < 2){virtualDisp->fillRect(x, y, 5, 6, myBLACK);} // Xóa vùng số cũ trước khi vẽ số mới
            if (font == 2){virtualDisp->fillRect(x+1, y, 3, 5, myBLACK);} // Xóa vùng số cũ trước khi vẽ số mới
            // Adjust x, y to top-left of 5x5 box
            x = x ;
            if (number == 1) x-= 1;
            y = y ; // Bắt đầu vẽ từ (x+1, y+1) để tạo khoảng cách
            // Segment positions for 5x5 grid
            // A: (x+1,y) to (x+3,y)
            // B: (x+4,y+1) to (x+4,y+2)
            // C: (x+4,y+3) to (x+4,y+4)
            // D: (x+1,y+4) to (x+3,y+4)
            // E: (x,  y+3) to (x,  y+4)
            // F: (x,  y+1) to (x,  y+2)
            // G: (x+1,y+2) to (x+3,y+2)
            if (number < 0 || number > 9) {
                Serial.println("Số không hợp lệ");
                return;
            }
            uint8_t segs = segMap[number];
            // A
            if (segs & 0x01)
                if ( font == 0) {virtualDisp->drawLine(x+1, y, x+4, y, color);}
                else if (font == 1) {virtualDisp->drawLine(x+1, y+1, x+3, y+1, color);}
                else if (font == 2) { virtualDisp->drawLine(x+1, y, x+3, y, color); }
                // else if (font == 2) {virtualDisp->drawLine(x+1, y+2, x+4, y+2, color);}
                // else if (font == 3) {virtualDisp->drawLine(x+1, y+3, x+4, y+3, color);}
            // B
            if (segs & 0x02)
                if ( font == 0) {virtualDisp->drawLine(x+4, y, x+4, y+2, color);}
                else if (font == 1) {virtualDisp->drawLine(x+4, y+2, x+4, y+3, color);}
                else if (font == 2) { virtualDisp->drawLine(x+3, y, x+3, y+2, color); }

                // else if (font == 2) {virtualDisp->drawLine(x+4, y+2, x+4, y+4, color);}
                // else if (font == 3) {virtualDisp->drawLine(x+4, y+3, x+4, y+5, color);}
            // C
            if (segs & 0x04)
                if ( font == 0) {virtualDisp->drawLine(x+4, y+2, x+4, y+4, color);}
                else if (font == 1) {virtualDisp->drawLine(x+4, y+3, x+4, y+4, color);}
                else if (font == 2) { virtualDisp->drawLine(x+3, y+2, x+3, y+4, color); }

                // else if (font == 2) {virtualDisp->drawLine(x+4, y+4, x+4, y+6, color);}
                // else if (font == 3) {virtualDisp->drawLine(x+4, y+5, x+4, y+7, color);}
            // D
            if (segs & 0x08)
                if ( font == 0) {virtualDisp->drawLine(x, y+4, x+3, y+4, color);}
                else if (font == 1) {virtualDisp->drawLine(x+1, y+5, x+3, y+5, color);}
                else if (font == 2) { virtualDisp->drawLine(x+1, y+4, x+3, y+4, color); }

                // else if (font == 2) {virtualDisp->drawLine(x+1, y+6, x+3, y+6, color);}
                // else if (font == 3) {virtualDisp->drawLine(x+1, y+7, x+3, y+7, color);}
            // E
            if (segs & 0x10)
                if ( font == 0) {virtualDisp->drawLine(x, y+2, x, y+4, color);}
                else if (font == 1) {virtualDisp->drawLine(x, y+3, x, y+4, color);}
                else if (font == 2) { virtualDisp->drawLine(x+1, y+2, x+1, y+4, color); }

                // else if (font == 2) {virtualDisp->drawLine(x, y+4, x, y+6, color);}
                // else if (font == 3) {virtualDisp->drawLine(x, y+5, x, y+7, color);}
            // F
            if (segs & 0x20)
                if ( font == 0) {virtualDisp->drawLine(x, y, x, y+2, color);}
                else if (font == 1) {virtualDisp->drawLine(x, y+2, x, y+3, color);}
                else if (font == 2) { virtualDisp->drawLine(x+1, y, x+1, y+2, color); }

                // else if (font == 2) {virtualDisp->drawLine(x, y+2, x, y+4, color);}
                // else if (font == 3) {virtualDisp->drawLine(x, y+3, x, y+5, color);}
            // G
            if (segs & 0x40)
                if ( font == 0) {virtualDisp->drawLine(x+1, y+2, x+3, y+2, color);}
                else if (font == 1) {virtualDisp->drawLine(x+1, y+3, x+3, y+3, color);}
                else if (font == 2) { virtualDisp->drawLine(x+1, y+2, x+3, y+2, color); }

                // else if (font == 2) {virtualDisp->drawLine(x+1, y+4, x+3, y+4, color);}
                // else if (font == 3) {virtualDisp->drawLine(x+1, y+5, x+3, y+5, color);}
}

void showBlockNumber(int number, int x, int y, int font, uint16_t color = myWHITE) {
    uint8_t digits[4] = {0}; // Tối đa 5 chữ số
    digits[3] = number % 10; // Đơn vị
    digits[2] = (number / 10) % 10; // Chục
    digits[1] = (number / 100) % 10; // Trăm
    digits[0] = (number / 1000) % 10; // Ngh
    for (int i = 0; i < 4; i++) {
        GenNumber(digits[i], x + i * 6, y, font, color);
    }
}



void draw_MSG(DynamicJsonDocument doc)
{
    virtualDisp->fillScreen(myBLACK);
    drawShapeFromType(doc);
    // Lấy tọa độ và id cho text
    if (doc.size() > 1 && doc[1].is<JsonArray>()) {
        get_CoordsAndID(doc[1].as<JsonArray>(), textCoorID);
        mapDataToTextContents(doc[1].as<JsonArray>(), textContents); // map text từ doc[1]
    }
    showAllTextLines(textCoorID, textContents);

    // Gửi JSON ra WebSocket
    String jsonOut;
    serializeJson(doc, jsonOut);
    webSocketServer.textAll(jsonOut);
    Serial.println("Đã vẽ MSG và gửi JSON ra WebSocket: " + jsonOut);
}
void showAllTextLines(TextLine textCoorID[MAX_GROUPS][MAX_LINES_PER_GROUP], char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]) {
    // Giả sử bạn có mảng allTextMessages chứa thông tin font cho từng dòng
    DynamicJsonDocument dataDoc = loadConfig("/DATA.json");

    JsonArray allTextMessages = dataDoc.as<JsonArray>();
    Serial.println("Tất cả dòng chữ:");
    for (int g = 0; g < MAX_GROUPS; g++) {
        for (int r = 0; r < MAX_LINES_PER_GROUP; r++) {
            if (textContents[g][r][0] != '\0') {
                Serial.printf("textContents[%d][%d]: %s\n", g, r, textContents[g][r]);
            }
        }
    }

for (int shapeIndex = 0; shapeIndex < MAX_GROUPS; shapeIndex++) {
    if (rowsTextInEachShapeFilter[shapeIndex] == 0) continue;

    for (int rowIndex = 0; rowIndex < rowsTextInEachShapeFilter[shapeIndex]; rowIndex++) {
        int x = textCoorID[shapeIndex][rowIndex].x;
        int y = textCoorID[shapeIndex][rowIndex].y;
        uint16_t color = textCoorID[shapeIndex][rowIndex].color;
        int id = textCoorID[shapeIndex][rowIndex].id;

        // Tìm font cho dòng này từ allTextMessages
        String fontName = "mythic_5pixels";
        for (JsonObject msg : allTextMessages) {
            if (msg["info"].is<JsonArray>() &&
                msg["info"][0] == shapeIndex &&
                msg["info"][1] == id) { // so sánh id
                fontName = msg["font"] | "mythic_5pixels";
                Serial.printf("Font cho shape %d, row %d: %s\n", shapeIndex, rowIndex, fontName.c_str());
                break;
            }
        }
        const GFXfont* fontPtr = getFontByName(fontName);
        if (fontPtr) {
            virtualDisp->setFont(fontPtr);
        } else {
            virtualDisp->setFont(&mythic_pixels5pt7b);
        }

        virtualDisp->setCursor(x, y);
        virtualDisp->setTextColor(color);
        virtualDisp->setTextSize(1);
        virtualDisp->print(textContents[shapeIndex][rowIndex]);
    }
}
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
  
const GFXfont* font1 = nullptr;
const GFXfont* font2 = nullptr;
const GFXfont* font3 = nullptr;
const GFXfont* font4 = nullptr;

int conStatus1 = 0;
int conStatus2 = 0;
int conStatus3 = 0;
int conStatus4 = 0;

        static bool needSendConfigToClient = false;
        static AsyncWebSocketClient* pendingClient = nullptr;
void webSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *payload, size_t length) {
    if (type == WS_EVT_DATA) {
      Serial.println("Websocket MESSAGE!!!");
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == length && info->opcode == WS_TEXT) {
            String jsonStr = "";
            for (size_t i = 0; i < length; i++) {
                jsonStr += (char)payload[i];
            }
            Serial.println("Nhận JSON từ web:");
            Serial.println(jsonStr);
            DynamicJsonDocument doc(8024);
            doc = parseStringToJSON(jsonStr);
            // DeserializationError err = deserializeJson(doc, jsonStr);
            if (!doc.isNull()) {
                // Nếu là masterArray (shapes + texts)
                if (doc.is<JsonArray>() && doc.size() == 2 && doc[0].is<JsonArray>() && doc[1].is<JsonArray>()) {
                    saveConfig(doc, "/CONFIG.json");
                    draw_MSG(doc); // chỉ gọi draw_MSG cho masterArray
                }
                else if (doc.is<JsonObject>() && doc.containsKey("pages") && doc["pages"].is<JsonArray>()) {
                    JsonArray pages = doc["pages"].as<JsonArray>();
                    if (pages.size() > 0) {
                        saveConfig(doc, "/CONFIG.json");
                        configDoc = loadConfig("/CONFIG.json");
                        TotalPages = getTotalPages();
                        
                        
                        // Serial.printf("font1: %p, font2: %p, font3: %p, font4: %p\n", font1, font2, font3, font4);
                        showCurrentPage(CurrentPage); 
                    
                    }
                }
                
                // Nếu là text mapping (mảng các object có "data" và "info")
                else if (doc.is<JsonArray>() && doc[0].is<JsonObject>() && doc[0].containsKey("data") && doc[0].containsKey("info")) {
                    Serial.println("Tôi dang map data");
                    mapDataToTextContents(doc.as<JsonArray>(), textContents);
                    showAllTextLines(textCoorID, textContents);   
                    saveConfig(doc, "/DATA.json");
                    Serial.println("✅ Đã lưu DATA.json");          
                }
                else {
                    Serial.println("❌ Không nhận diện được loại JSON!");
                }
            } else {
                Serial.println("❌ Dữ liệu JSON không hợp lệ.");
            }
        }
    }
    if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket client %u disconnected\n", client->id());
    }
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client %u connected\n", client->id());
        // Nếu xử lý gửi file lâu, cho vào hàng đợi để xử lý sau
        // Tạo một task hoặc dùng hàng đợi đơn giản (ví dụ với std::queue nếu dùng ESP-IDF, hoặc tạo biến flag)
        // Ở đây, đơn giản chỉ tạo một flag để gửi sau trong loop
        needSendConfigToClient = true;
        pendingClient = client;
        
    }
}

void setup_WS() {
    Serial.println("=== Khởi tạo WebSocket ===");
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: "); Serial.println(ssid);
    Serial.print("Password: "); Serial.println(password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

    webSocketServer.onEvent(webSocketEvent);      // Đăng ký sự kiện trước
    server.addHandler(&webSocketServer);          // Thêm WebSocket vào server
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index_aP.html");
    server.begin();                               // Bắt đầu server
    Serial.println("✅ HTTP & WebSocket server started!");
}

//tác dụng: lưu nội dung trong doc vào file config.json
// nếu file đã tồn tại thì ghi đè, nếu không thì tạo mới
void saveConfig(const DynamicJsonDocument& doc, String filePath ) {
  File file = LittleFS.open(filePath, "w");
  if (!file) {
    Serial.println("❌ Lỗi: Không thể mở file config để ghi.");
    return;
  }
  if (serializeJson(doc, file) == 0) {
    Serial.println("❌ Lỗi: Ghi file config thất bại (JSON rỗng?).");
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
  DynamicJsonDocument doc(8024);
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
    Serial.println("Load Config bị lỗi ");
    String jsonOut;
    serializeJson(doc, jsonOut);
    Serial.print ("Lỗi: ");
    Serial.println(jsonOut);
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


void setup_littleFS () {
    Serial.println("=== VirtualMatrixPanel Single 1/4 Scan Panel ===");
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
    Serial.println("=== Khởi tạo LED Panel ===");
    // Giải phóng nếu đã khởi tạo trước đó
    if (dma_display) {
        delete dma_display;
        dma_display = nullptr;
    }
    if (virtualDisp) {
        delete virtualDisp;
        virtualDisp = nullptr;
    }
    bool initialized = false;
    int retryCount = 0;
    const int maxRetries = 10;
    while (!initialized && retryCount < maxRetries) {
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
        if (dma_display != nullptr) {
            dma_display->begin();
            dma_display->setBrightness8(90);
            dma_display->clearScreen();

            virtualDisp = new VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>(1, 1, PANEL_RES_X, PANEL_RES_Y);
            if (virtualDisp != nullptr) {
                virtualDisp->setDisplay(*dma_display);

                myBLACK = virtualDisp->color565(0, 0, 0);
                myWHITE = virtualDisp->color565(255, 255, 255);

                virtualDisp->fillScreen(myBLACK);
                initialized = true;
            }
        }
        if (!initialized) {
            Serial.println("❌ Khởi tạo panel thất bại, thử lại sau 500ms...");
            delay(500);
            retryCount++;
            if (retryCount >= maxRetries) {
                Serial.println("❌ Đã vượt quá số lần thử khởi tạo panel. Thoát khỏi vòng lặp.");
                break;
            }
        }
    }
    if (initialized) {
        Serial.println("✅ Khởi tạo panel thành công!");
        Serial.printf("Kích thước panel: %dx%d\n", PANEL_RES_X, PANEL_RES_Y);
        Serial.printf("Số chuỗi: %d, Số scan: %d\n", 1, PANEL_SCAN_TYPE);
    } else {
        Serial.println("❌ Không thể khởi tạo panel sau nhiều lần thử.");
    }
    Serial.println(">> RAM:" + String(ESP.getFreeHeap()/1024) + " KB");
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
void mapDataToTextContents(JsonArray arr, char textContents[MAX_GROUPS][MAX_LINES_PER_GROUP][MAX_TEXT_LENGTH]) {
    //xóa hết trước khi map
    // for (int g = 0; g < MAX_GROUPS; g++) {
    //     for (int r = 0; r < MAX_LINES_PER_GROUP; r++) {
    //         textContents[g][r][0] = '\0';
    //     }
    // }

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


void handleSerialConfig() {
    static String inputString = "";
    static bool stringComplete = false;

    // Đọc dữ liệu từ Serial
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            stringComplete = true;
            break;
        } else {
            inputString += inChar;
        }
    }

    // Nếu đã nhận đủ 1 dòng JSON
    if (stringComplete) {
        DynamicJsonDocument doc(8000);
        DeserializationError err = deserializeJson(doc, inputString);
        if (err) {
            Serial.print("Lỗi parse JSON: ");
            Serial.println(err.c_str());
        } else {
            // Cấu hình WiFi
            if (doc.containsKey("ssid") && doc.containsKey("password")) {
                const char* ssid = doc["ssid"];
                const char* password = doc["password"];
                Serial.printf("Kết nối WiFi: %s ...\n", ssid);
                WiFi.begin(ssid, password);
                int t = 0;
                while (WiFi.status() != WL_CONNECTED && t < 20) {
                    delay(500);
                    Serial.print(".");
                    t++;
                }
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\n✅ Đã kết nối WiFi!");
                    Serial.print("IP: ");
                    Serial.println(WiFi.localIP());
                } else {
                    Serial.println("\n❌ Kết nối WiFi thất bại!");
                }
            }
            // Đổi font
            if (doc.containsKey("font")) {
                String fontName = doc["font"].as<String>();
                const GFXfont* fontPtr = getFontByName(fontName);
                if (fontPtr != nullptr) {
                    virtualDisp->setFont(fontPtr);
                    Serial.println("✅ Đã set font: " + fontName);
                } else {
                    Serial.println("❌ Font không hỗ trợ: " + fontName);
                }
                showAllTextLines(textCoorID, textContents); // Cập nhật hiển thị chữ với font mới
            }
        }
        inputString = "";
        stringComplete = false;
    }
}

#define MODE_BUTTON_PIN 7 //5 6 7
#define COUNT0_BUTTON_PIN 37
#define COUNT1_BUTTON_PIN 38
#define COUNT2_BUTTON_PIN 39
#define COUNT3_BUTTON_PIN 40
#define BUTTON_HOLD_TIME 3000 // ms

bool configMode = false;
unsigned long modeButtonPressTime = 0;
bool modeButtonLastState = HIGH;

volatile int count0 = 0, count1 = 0, count2 = 0, count3 = 0;


int getTotalPages() {
    if (configDoc.isNull()) {
        Serial.println("❌ Không thể đọc file CONFIG.json hoặc file rỗng.");
        return 0;
    }
    if (configDoc.is<JsonObject>() && configDoc.containsKey("pages") && configDoc["pages"].is<JsonArray>()) {
        JsonArray pages = configDoc["pages"].as<JsonArray>();
        return pages.size() > 0 ? pages.size() - 1 : 0;
    }
    if (configDoc.is<JsonArray>()) {
        JsonArray arr = configDoc.as<JsonArray>();
        return arr.size() > 0 ? arr.size() - 1 : 0;
    }
    return 0;
}

int countDigits(int number) {
  if (number == 0) return 1;
  int count = 0;
  int n = abs(number);
  while (n > 0) {
    n /= 10;
    count++;
  }
  return count;
}

uint8_t autoMiddle(uint8_t px, int number) {
    // Tính toán vị trí vẽ số sao cho căn giữa

    uint8_t totalWidth = (px+1)*countDigits(number);

    return totalWidth/2;
}

void showCurrentPage(int currentPage) {
    JsonArray pages = JsonArray();
    // Serial.println("Page size: " + String(TotalPages));
    if (configDoc.is<JsonObject>() && configDoc.containsKey("pages") && configDoc["pages"].is<JsonArray>()) {
        pages = configDoc["pages"].as<JsonArray>();
    } else if (configDoc.is<JsonArray>()) {
        pages = configDoc.as<JsonArray>();
    }
    if (!pages.isNull() && currentPage < pages.size()) {
        DynamicJsonDocument pageDoc(8024);
        pageDoc.set(pages[currentPage]);
        
        
        numberContents[0] = count0;
        numberContents[1] = count1;
        numberContents[2] = count2;
        numberContents[3] = count3;
        numberContents[4] = count0;
        numberContents[5] = count1;
        numberContents[6] = count2;
        numberContents[7] = count3;
        
        // Lấy và vẽ các counter tương ứng cho từng vị trí
        if (pages[currentPage].is<JsonArray>() && pages[currentPage].size() > 1) {
            JsonArray textInfoArray = pages[currentPage][1].as<JsonArray>();
            for (int i = 0; i < 4; i++) {
                  int rectWidth = 30;
                  int rectHeight = 12;
                  // Lấy kích thước và tọa độ của shape (hình chữ nhật) chứa số
                  int rectX = 0, rectY = 0;
                  if (pages[currentPage][0].is<JsonArray>()) {
                      JsonArray shapes = pages[currentPage][0].as<JsonArray>();
                      if (shapes.size() > i && shapes[i].is<JsonObject>()) {
                          JsonObject shape = shapes[i];
                          if (shape.containsKey("data")) {
                              JsonArray dataArr = shape["data"].as<JsonArray>();
                              if (dataArr.size() >= 4) {
                                  rectX = dataArr[0].as<int>()+1;
                                  rectY = dataArr[1].as<int>()+1;
                                  rectWidth = dataArr[2].as<int>()-2;
                                  rectHeight = dataArr[3].as<int>()-2;
                              }
                          }
                      }
                  }
                  virtualDisp->fillRect(rectX, rectY, rectWidth, rectHeight, myBLACK);
                }
            JsonArray fontArr = pages[currentPage][2].as<JsonArray>();
            if (fontArr.size() >= 5) {
                int brightness = fontArr[4].as<int>();
                dma_display->setBrightness8(brightness);
                Serial.printf("Đã set brightness: %d\n", brightness);
            }
            JsonArray connectionStatus = pages[currentPage][3].as<JsonArray>();
            if (connectionStatus.size() >= 3) {
                conStatus1 = connectionStatus[0].as<int>();
                conStatus2 = connectionStatus[1].as<int>();
                conStatus3 = connectionStatus[2].as<int>();
                conStatus4 = connectionStatus[3].as<int>();
                Serial.printf("Connection Status: %d, %d, %d, %d\n", conStatus1, conStatus2, conStatus3, conStatus4);
                }

            if(currentPage == 0){
              
              for (int i = 0; i < 4; i++) {
                  int rectWidth = 30;
                  int rectHeight = 12;
                  // Lấy kích thước và tọa độ của shape (hình chữ nhật) chứa số
                  int rectX = 0, rectY = 0;
                  if (pages[currentPage][0].is<JsonArray>()) {
                      JsonArray shapes = pages[currentPage][0].as<JsonArray>();
                      if (shapes.size() > i && shapes[i].is<JsonObject>()) {
                          JsonObject shape = shapes[i];
                          if (shape.containsKey("data")) {
                              JsonArray dataArr = shape["data"].as<JsonArray>();
                              if (dataArr.size() >= 4) {
                                  rectX = dataArr[0].as<int>()+1;
                                  rectY = dataArr[1].as<int>()+1;
                                  rectWidth = dataArr[2].as<int>()-2;
                                  rectHeight = dataArr[3].as<int>()-2;
                              }
                          }
                      }
                  }
                  virtualDisp->fillRect(rectX, rectY, rectWidth, rectHeight, myBLACK);
                }
                drawShapeFromType(pageDoc);
                for (int i = 0; i < 4; i++) {
                    int rectWidth = 30;
                    int rectHeight = 12;
                    // Lấy kích thước và tọa độ của shape (hình chữ nhật) chứa số
                    int rectX = 0, rectY = 0;
                    if (pages[currentPage][0].is<JsonArray>()) {
                        JsonArray shapes = pages[currentPage][0].as<JsonArray>();
                        if (shapes.size() > i && shapes[i].is<JsonObject>()) {
                            JsonObject shape = shapes[i];
                            if (shape.containsKey("data")) {
                                JsonArray dataArr = shape["data"].as<JsonArray>();
                                if (dataArr.size() >= 4) {
                                    rectX = dataArr[0].as<int>()+1;
                                    rectY = dataArr[1].as<int>()+1;
                                    rectWidth = dataArr[2].as<int>()-2;
                                    rectHeight = dataArr[3].as<int>()-2;
                                }
                            }
                        }
                    }
                    int x = 0, y = 0;
                    uint16_t color = myWHITE;
                    JsonArray groupTextInEachShape = textInfoArray[0].as<JsonArray>();
                    if (groupTextInEachShape.size() > 0) {
                        // Serial.printf("groupTextInEachShape[%d] size: %d\n", i, groupTextInEachShape.size());
                        JsonObject rowInfor = groupTextInEachShape[i];
                        if (rowInfor.containsKey("xy")) {
                            JsonArray rowInfor_xy = rowInfor["xy"];
                            if (rowInfor_xy.size() >= 2) {
                                x = rowInfor_xy[0].as<int>();
                                y = rowInfor_xy[1].as<int>() ;//
                                
                            }

                        }
                        if (rowInfor.containsKey("color")) {
                            String hex = rowInfor["color"].as<String>(); // VD: "#FF0000"
                            int r = strtol(hex.substring(1,3).c_str(), nullptr, 16);
                            int g = strtol(hex.substring(3,5).c_str(), nullptr, 16);
                            int b = strtol(hex.substring(5,7).c_str(), nullptr, 16);
                            color = virtualDisp->color565(r, g, b);
                        } else {
                            color = myWHITE;
                        }
                    }
                    // }
                    textCoorID[i][0].x = x;
                    textCoorID[i][0].y = y;
                    textCoorID[i][0].color = color;
                    

                    // virtualDisp->setCursor(x, y- 7);
                    // virtualDisp->setTextColor(color);
                    // virtualDisp->setTextSize(1);
                    // virtualDisp->print(numberContents[i]);
                    
                    
                    font1 = getFontByIndex(fontArr[0].as<int>());
                    font2 = getFontByIndex(fontArr[1].as<int>());
                    font3 = getFontByIndex(fontArr[2].as<int>());
                    font4 = getFontByIndex(fontArr[3].as<int>()); 
                    int8_t digits = countDigits(numberContents[i]);                    
                    if (digits == 1) printLedPanel(numberContents[i], x, y+1, 1, font1, color);
                    if (digits == 2) printLedPanel(numberContents[i], x+1, y, 1, font2, color);
                    if (digits == 3) printLedPanel(numberContents[i], x, y, 1, font3, color); //3 này ổn đấy
                    if (digits == 4) printLedPanel(numberContents[i], x, y, 1, font4, color);
               }
            }
        }
        // 7-8,7-14,7-24,7-30,38-8,38-14,38-24,38-30
        //  4   0    5    1    6   2    7    3
        textCoorID[3][0].x = 7;textCoorID[7][0].y = 8;//1
        textCoorID[7][0].x = 7;textCoorID[3][0].y = 14;//2
        textCoorID[1][0].x = 7;textCoorID[5][0].y = 24;//3
        textCoorID[5][0].x = 7;textCoorID[1][0].y = 30;//4
        textCoorID[2][0].x = 38;textCoorID[6][0].y = 8;//5
        textCoorID[6][0].x = 38;textCoorID[2][0].y = 14;//6
        textCoorID[0][0].x = 38;textCoorID[4][0].y = 24;//7
        textCoorID[4][0].x = 38;textCoorID[0][0].y = 30; //8

        textCoorID[4][0].color = myWHITE;
        textCoorID[5][0].color = myWHITE;
        textCoorID[6][0].color = myWHITE;
        textCoorID[7][0].color = myWHITE;
        
        if(currentPage == 1){
            // Hiển thị đếm ở trang 1
            drawShapeFromType(pageDoc);
            for(int i = 0; i < 8; i++) {
                int x = textCoorID[i][0].x;
                int y = textCoorID[i][0].y - 5;
                uint16_t color = textCoorID[i][0].color;
                showBlockNumber(numberContents[i], x, y, 0, color);
            }
            // Hiển thị đếm ở trang
        } 
        if ( currentPage == 2) {
          drawShapeFromType(pageDoc);
            for (int i = 0; i < 8; i++) {
                int x = textCoorID[i][0].x;
                int y = textCoorID[i][0].y - 5;
                uint16_t color = textCoorID[i][0].color;
                showBlockNumber(numberContents[i], x, y-1, 1, color);
            }
        }
        if(currentPage == 3){
            // Hiển thị đếm ở trang 1
          drawShapeFromType(pageDoc);

            for(int i = 0; i < 8; i++) {
                int x = textCoorID[i][0].x;
                int y = textCoorID[i][0].y - 5;
                uint16_t color = textCoorID[i][0].color;
                showBlockNumber(numberContents[i], x, y, 2, color);
            }
            // Hiển thị đếm ở trang
        } 
        GenNumber(1, 0, 1, 2, virtualDisp->color565(93, 71, 255));
        GenNumber(2, 32, 1, 2, virtualDisp->color565(93, 71, 255));
        GenNumber(3, 0, 17, 2,  virtualDisp->color565(93, 71, 255));
        GenNumber(4, 32, 17, 2,  virtualDisp->color565(93, 71, 255));


        virtualDisp->drawLine(2, 7, 2, 7+conStatus1-1, virtualDisp->color565(128, 0, 255));
        virtualDisp->drawLine(34, 7, 34, 7+conStatus2-1, virtualDisp->color565(255, 255, 0));
        virtualDisp->drawLine(2, 24, 2, 24+conStatus3-1, virtualDisp->color565(0, 255, 0));
        virtualDisp->drawLine(34, 24, 34, 24+conStatus4-1, virtualDisp->color565(0, 128, 255));


        // else {
        //     Serial.println("Không ở trang 1, không hiển thị đếm");
        // }
        // showAllTextLines(textCoorID, textContents);
    } else {
        Serial.println("❌ Trang hiện tại không hợp lệ hoặc không có dữ liệu.");
    }
}

const GFXfont* getFontByIndex(int index) {
    switch (index) {
        case 0:  return nullptr; // Default
        case 1:  return &FreeMono9pt7b;
        case 2:  return &FreeMonoBold9pt7b;
        case 3:  return &FreeMonoOblique9pt7b;
        case 4:  return &FreeMonoBoldOblique9pt7b;
        case 5:  return &FreeSans9pt7b;
        case 6:  return &FreeSansBold9pt7b;
        case 7:  return &FreeSansOblique9pt7b;
        case 8:  return &FreeSansBoldOblique9pt7b;
        case 9:  return &FreeSerif9pt7b;
        case 10: return &FreeSerifBold9pt7b;
        case 11: return &FreeSerifItalic9pt7b;
        case 12: return &FreeSerifBoldItalic9pt7b;

        case 15: return &B_085pt7b; // 04B_5px (nếu tên biến đúng là B_085pt7b)

        case 17: return &hud5pt7b;
        case 18: return &mythic_pixels5pt7b;
        case 19: return &Org_01;
        case 20: return &Picopixel;
        case 21: return &Tiny3x3a2pt7b;
        case 22: return &TomThumb;
        default: return nullptr;
    }
}
const GFXfont* getFontByName(const String& name) {
    // FreeMono
    if (name == "FreeMono9pt7b") return &FreeMono9pt7b;
    else if (name == "FreeMonoBold9pt7b") return &FreeMonoBold9pt7b;
    else if (name == "FreeMonoOblique9pt7b") return &FreeMonoOblique9pt7b;
    else if (name == "FreeMonoBoldOblique9pt7b") return &FreeMonoBoldOblique9pt7b;

    // FreeSans
    else if (name == "FreeSans9pt7b") return &FreeSans9pt7b;
    else if (name == "FreeSansBold9pt7b") return &FreeSansBold9pt7b;
    else if (name == "FreeSansOblique9pt7b") return &FreeSansOblique9pt7b;
    else if (name == "FreeSansBoldOblique9pt7b") return &FreeSansBoldOblique9pt7b;

    // FreeSerif
    else if (name == "FreeSerif9pt7b") return &FreeSerif9pt7b;
    else if (name == "FreeSerifBold9pt7b") return &FreeSerifBold9pt7b;
    else if (name == "FreeSerifItalic9pt7b") return &FreeSerifItalic9pt7b;
    else if (name == "FreeSerifBoldItalic9pt7b") return &FreeSerifBoldItalic9pt7b;


    // Font đặc biệt (pixel, mini, custom)
    else if (name == "04B_5px") return &B_085pt7b;
    else if (name == "hud5pt7b") return &hud5pt7b;
    else if (name == "mythic_5pixels") return &mythic_pixels5pt7b;
    else if (name == "Org_01") return &Org_01;
    else if (name == "Picopixel") return &Picopixel;
    else if (name == "Tiny3x3a2pt7b") return &Tiny3x3a2pt7b;
    else if (name == "TomThumb") return &TomThumb;

    // Nếu không khớp, trả về nullptr
    else return nullptr;
}
void printLedPanel( int16_t number, int x, int y, int8_t fontSize, const GFXfont* font, uint16_t color = myWHITE) {
    // In chữ lên LED Panel
    virtualDisp->setTextColor(color);
      if (font) {
          virtualDisp->setFont(font);
      } else {
          virtualDisp->setFont(); // Mặc định nếu không tìm thấy font
      }
    int16_t x1, y1;
    uint16_t w, h;
    virtualDisp->getTextBounds("0", 0, 0, &x1, &y1, &w, &h);
    uint8_t fontColPx = w*fontSize;
    Serial.println ("Font col px: " + String(fontColPx));

    int8_t digitOfNumber = countDigits(number);
    if (font == nullptr) x = x - autoMiddle(5, number);    
    else x = x - autoMiddle(fontColPx, number);
    if (font == nullptr) virtualDisp->setCursor(x, y-7);
    else virtualDisp->setCursor(x, y);
    virtualDisp->print(number);
}

void enterConfigMode() {
    configMode = true;
    Serial.println(">> Đã vào chế độ config (socket mở)");
    // if (dma_display) {
    //     dma_display->stopDMAoutput();
    //     delete dma_display;
    //     dma_display = nullptr;
    // }
    // if (virtualDisp) {
    //     delete virtualDisp;
    //     virtualDisp = nullptr;
    // }
    // setup_WS(); // Mở socket
    // if (dma_display) {
    //     dma_display->stopDMAoutput(); // Tạm dừng DMA để tránh xung đột với WebSocket
    // }
    // setup_WS(); // Khởi tạo WebSocket server khi vào chế độ config
    // Đặt biến configMode vào LittleFS để lưu trạng thái
    File file = LittleFS.open("/MODE.txt", "w");
    if (file) {
        file.print("CONFIG");
        file.close();
        Serial.println("Đã lưu trạng thái CONFIG vào MODE.txt");
    } else {
        Serial.println("Không thể ghi MODE.txt");
    }
    // Khởi động lại ESP để vào chế độ cấu hình
    ESP.restart();
}

void exitConfigMode() {
    // configMode = false;
    // Serial.println(">> Thoát chế độ config (socket đóng, hiển thị panel)");
    // setup_ledPanel(); // Khởi tạo lại LED Panel
    // server.end(); // Đóng socket
    // draw_MSG(loadConfig("/CONFIG.json")); // Hiển thị lại panel
    configMode = false;
    Serial.println(">> Thoát chế độ config (vào chế độ NORMAL)");
    File file = LittleFS.open("/MODE.txt", "w");
    if (file) {
        file.print("NORMAL");
        file.close();
        Serial.println("Đã lưu trạng thái NORMAL vào MODE.txt");
    } else {
        Serial.println("Không thể ghi MODE.txt");
    }
    ESP.restart();
}

void setup() {

    Serial.begin(115200);
    // WiFi_AP_setup(server) ; // Khởi tạo WiFi AP
    // delay(5000);
    setup_littleFS();
    setup_WS(); // Khởi tạo WebSocket sau khi khởi tạo panel

    Serial.println("✅ Đã khởi tạo LittleFS và LED Panel thành công!");
    setup_ledPanel();
    if (!virtualDisp || !dma_display) {
        Serial.println("❌ Khởi tạo panel thất bại. Không tiếp tục.");
        while (1) delay(1000);
    }
    // Kiểm tra file MODE.txt để chọn chế độ chạy (CONFIG hoặc NORMAL)
    String mode = "NORMAL";
    // if (LittleFS.exists("/MODE.txt")) {
        File modeFile;
    //     if (!LittleFS.exists("/MODE.txt")) {
    //         // Nếu file chưa tồn tại, tạo mới với nội dung NORMAL
            modeFile = LittleFS.open("/MODE.txt", "w");
            if (modeFile) {
            modeFile.print("NORMAL");
            modeFile.close();
            }
    //         // Mở lại file để đọc
    //         modeFile = LittleFS.open("/MODE.txt", "r");
    //     } else {
    //         modeFile = LittleFS.open("/MODE.txt", "r");
    //     }
    //     if (modeFile) {
    //         mode = modeFile.readString();
    //         mode.trim();
    //         modeFile.close();
    //     }
    // }
    if (mode == "CONFIG") {
        configMode = true;
        // ALC_setup(server);
        Serial.println(">> Đang ở chế độ CONFIG (WebSocket mở)");
    } else {
        configMode = false;
        Serial.println(">> Đang ở chế độ NORMAL (hiển thị panel)");
    }
    // Khởi tạo các nút nhấn trước khi khởi tạo LED Panel để tránh xung đột pin
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT0_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT3_BUTTON_PIN, INPUT_PULLUP);
    if (mode == "NORMAL") {
        Serial.println(">> Đang ở chế độ NORMAL, sẽ hiển thị panel");
        // Hiển thị chữ
        // Khởi tạo LED Panel trước để có virtualDisp, myWHITE, myBLACK...

        // MySerial1.begin(9600, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN); // Khởi tạo Serial1 cho LoRa E32
        // Serial1.begin(9600); // Khởi tạo LoRa E32
        // Tạo file CONFIG.json nếu chưa có
        if (!LittleFS.exists("/CONFIG.json")) {
            Serial.println("⚠️ File CONFIG.json không tồn tại, tạo mới...");
            DynamicJsonDocument defaultConfig = createSampleJson();
            saveConfig(defaultConfig, "/CONFIG.json");
        } else {
            Serial.println("✅ Đã tìm thấy CONFIG.json, sẽ load nội dung.");
        }

        configDoc = loadConfig("/CONFIG.json");
        TotalPages = getTotalPages();
        // Khởi tạo WebSocket server (đã gọi trong setup_ledPanel)
        // Hiển thị nội dung từ DATA.json nếu có
        DynamicJsonDocument dataDoc = loadConfig("/DATA.json");
        if (dataDoc.is<JsonArray>() && dataDoc.size() > 0) {
            mapDataToTextContents(dataDoc.as<JsonArray>(), textContents);
            showCurrentPage(0); // Hiển thị trang đầu tiên
            Serial.println("✅ Đã load DATA.json và hiển thị text!");
        } else {
            Serial.println("⚠️ DATA.json rỗng hoặc không đúng định dạng!");
        }
        showCurrentPage(0);
    }
    // Serial.println(">> Serial1 Initialized for LoRa E32");
    // pinMode(E32_M0_PIN, OUTPUT);
    // pinMode(E32_M1_PIN, OUTPUT);
    // Serial1.begin(9600, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);
}
    // Hiển thị chữ lên màn hình
    // showAllTextLines(textCoorID, textContents);

void loop() {
    static bool lastModeBtn = HIGH;
    bool modeBtn = digitalRead(MODE_BUTTON_PIN);
    static unsigned long pressStart = 0;
    // Xử lý nhấn giữ để vào/thoát config
    if (modeBtn == LOW && lastModeBtn == HIGH) {
        pressStart = millis();
    }
    if (modeBtn == LOW && (millis() - pressStart > BUTTON_HOLD_TIME)) {
        if (!configMode) enterConfigMode();
        else exitConfigMode();
        while (digitalRead(MODE_BUTTON_PIN) == LOW) delay(50);
    }
    // Xử lý nhấn ngắn để chuyển page
    if (modeBtn == HIGH && lastModeBtn == LOW && (millis() - pressStart < BUTTON_HOLD_TIME)) {
        int totalPages = getTotalPages();
        static int currentPage = 0;
        // Serial.printf("Chuyển sang trang %d/%d\n", currentPage, totalPages);
        currentPage++;
        if(currentPage > totalPages) currentPage = 0; // Quay lại trang đầu nếu quá cuối
        // currentPage = (currentPage + 1) % (totalPages + 1); // Quay lại trang đầu nếu quá cuối
        Serial.printf("Trang hiện tại: %d\n", currentPage);
        if (currentPage < 0) currentPage = 0; // Đảm bảo currentPage không âm
        if (currentPage > totalPages) currentPage = totalPages;
        CurrentPage = currentPage; // Đảm bảo currentPage không vượt quá tổng số trang
        // Hiển thị trang hiện tại
        virtualDisp->fillScreen(myBLACK);
        showCurrentPage(currentPage );
        
    }
    lastModeBtn = modeBtn;
    static uint8_t debounceBtn = 10;
    // Xử lý nút đếm
    if (digitalRead(COUNT0_BUTTON_PIN) == LOW) {
        count0++;
       // showCountsOnPanel();
        showCurrentPage(CurrentPage);
        delay(debounceBtn); // chống bounce
    }
    // Xử lý nút đếm
    if (digitalRead(COUNT1_BUTTON_PIN) == LOW) {
        count1++;
       // showCountsOnPanel();
        showCurrentPage(CurrentPage);
        delay(debounceBtn); // chống bounce
    }
    if (digitalRead(COUNT2_BUTTON_PIN) == LOW) {
        count2++;
        // showCountsOnPanel();
        showCurrentPage(CurrentPage );
        delay(debounceBtn);
    }
    if (digitalRead(COUNT3_BUTTON_PIN) == LOW) {
        count3++;
        // showCountsOnPanel();
        showCurrentPage(CurrentPage );
        delay(debounceBtn);
    }

    // Nhận dữ liệu từ LoRa E32 qua Serial1
    if (MySerial1.available() >= 12) { // id(1) + netid(1) + data(8) + cmnd(1) + padding(1)
        uint8_t buf[12];
        MySerial1.readBytes(buf, 12);
        uint8_t id = buf[0];
        uint8_t netid = buf[1];
        uint8_t cmnd = buf[10];
        if(id != 0xFF && netid != 0xFF) {
            Serial.printf("Nhận dữ liệu từ LoRa E32: id=%d, netid=%d, cmnd=%d\n", id, netid, cmnd);
        } else {
            Serial.println("❌ Nhận dữ liệu không hợp lệ từ LoRa E32!");
            return;
        }
        // uint8_t padding = buf[11]; // nếu có
        if (cmnd == 1) { // data
            count1 = buf[2];
            count2 = buf[3];
            count3 = buf[4];
            showCurrentPage(CurrentPage);
        } else if (cmnd == 2) { // config
            // Xử lý cấu hình nếu cần
            Serial.println("Nhận lệnh cấu hình LoRa E32");
        }
    }

    // Nhận chuỗi JSON qua Serial để cập nhật led-panel
    static String serialJson = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            DynamicJsonDocument doc(8024);
            DeserializationError err = deserializeJson(doc, serialJson);
            if (!doc.isNull()) {
              // serializeJson(doc, Serial);
                Serial.println();
                // Nếu là object có trường "pages"
                if (doc.is<JsonObject>() && doc.containsKey("pages") && doc["pages"].is<JsonArray>()) {
                    JsonArray pages = doc["pages"].as<JsonArray>();
                    if (pages.size() > 0) {
                        saveConfig(doc, "/CONFIG.json");
                        configDoc = loadConfig("/CONFIG.json");
                        TotalPages = getTotalPages();
                        showCurrentPage(CurrentPage); // Hiển thị trang hiện tại
                        // DynamicJsonDocument pageDoc(2048);
                        // pageDoc.set(pages[0]);
                        // draw_MSG(pageDoc); // Hiển thị page đầu tiên, hoặc tuỳ ý
                    }
                }
                //{"cmnd":"update","data":[1,2,3,4,5,6,7,8]}
                else if (doc.is<JsonObject>() && doc.containsKey("cmnd") && doc["cmnd"] == "update" && doc.containsKey("data") && doc["data"].is<JsonArray>()) {
                  JsonArray arr = doc["data"].as<JsonArray>();
                  int n = arr.size();
                  for (int i = 0; i < n && i < MAX_TEXT_LENGTH; i++) {
                    numberContents[i] = arr[i].as<int>();
                  }
                  showCurrentPage(CurrentPage);
                } 
                // Nếu là text mapping (mảng các object có "data" và "info")
                else if (doc.is<JsonArray>() && doc[0].is<JsonObject>() && doc[0].containsKey("data") && doc[0].containsKey("info")) {
                    Serial.println("Tôi dang map data");
                    mapDataToTextContents(doc.as<JsonArray>(), textContents);
                    showAllTextLines(textCoorID, textContents);   
                    saveConfig(doc, "/DATA.json");
                    Serial.println("✅ Đã lưu DATA.json");          
                }
                else {
                    Serial.println("❌ Không nhận diện được loại JSON!");
                }
            } else {
                Serial.println("❌ Dữ liệu JSON không hợp lệ.");
            }
            
            serialJson = "";
            doc.clear(); // Xoá nội dung của doc để tránh lỗi khi đọc tiếp
        } else {
            serialJson += c;
        }
    }


static unsigned long lastStatusTime = 0;
unsigned long currentTime = millis();
if (currentTime - lastStatusTime >= 3000) {
    lastStatusTime = currentTime;

    // RAM
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();

    // CPU
    uint8_t cpuFreq = ESP.getCpuFreqMHz();

    // ESP32-S3: Không có API chính thức để đọc nhiệt độ chip
    Serial.printf("RAM: %u/%u | %u/%u KB (min: %u | %u KB) | CPU: %u MHz\n", freeHeap, totalHeap, freeHeap/1024 , totalHeap/1024, minFreeHeap, minFreeHeap/1024, cpuFreq);
    // Serial.println("Nhiệt độ chip: Không hỗ trợ trực tiếp trên ESP32-S3.");

  }

// Trong loop() thêm đoạn sau để xử lý gửi khi rảnh:
        if (needSendConfigToClient && pendingClient != nullptr) {
            DynamicJsonDocument configDoc = loadConfig("/CONFIG.json");
            String jsonOut;
            serializeJson(configDoc, jsonOut);
            pendingClient->text(jsonOut);
            configDoc = loadConfig("/DATA.json");
            serializeJson(configDoc, jsonOut);
            pendingClient->text(jsonOut);
            Serial.println("✅ Đã gửi lại nội dung CONFIG.json và DATA.json cho client.");
            needSendConfigToClient = false;
            pendingClient = nullptr;
        }
  }

#endif//USE_LMD_V1

#ifdef USE_LMD_V2
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 
// ##################################################################################
// # I SUGGEST YOU WATCH THE VIDEO UNTIL THE END, SO YOU KNOW HOW TO USE THIS CODE. #
// ##################################################################################
//
// Information about the LED Matrix P5 RGB 64x32 panel that I am using :
// - Information from the seller :
//    > Panel P5 RGB Full Color Indoor.
//    > Dimensions 32x64 cm.
//    > 64x32 Pixels.
//    > HUB75 Scan 1/16S.
// - Information written on the P5 RGB PCB board :
//   > P5(2121)-3264-16S-M5.
//   > L-M2121MY-P5RGB.
//   > IC : SM16208/SM5166/221221
// - HUB75 (Scan 1/16) : 
//    -----------
//   | DR1 | DG1 |
//   | DB1 | GND |
//   | DR2 | DG2 |
//   | DB2 | GND |
//   |  A  |  B  |
//   |  C  |  D  |
//   | CLK | LAT |
//   | OE  | GND |
//    -----------
// 
// Software :
// - Arduino IDE 1.8.19
// 
// Arduino core for the ESP32 :
// - Arduino core for the ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6 and ESP32-H2 (V2.0.16).
// 
// Arduino Libraries :
// - Arduino_JSON by Arduino (V0.2.0).
// - Adafruit BusIO by Adafruit (V1.16.1).
// - RTClib by Adafruit (V2.1.4).
// - Adafruit GFX Library by Adafruit (V1.11.9).
// - HUB75 RGB LED matrix panel library utilizing ESP32 DMA by mrfaptastic (V3.0.10).
// 
// Hardware :
// - ESP32 DEVKIT V1.
// - LED Matrix P5 RGB 64x32.
// - DS3231 RTC Module.
// - 5V Power Supply.
// - For more details, see the installation picture.
//
// API used :
// - OpenWeatherMap API.
// 
// Troubleshooting (Common Issues) for P5 RGB : https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA/issues/134
//
// Reference :
// - OpenWeatherMap API (Current weather data) : https://openweathermap.org/current
// - HUB75 RGB LED matrix panel library utilizing ESP32 DMA by mrfaptastic : https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA
// - Power, Power and Power! : https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA?tab=readme-ov-file#power-power-and-power
// - Adafruit_GFX getTextBounds() : https://forums.adafruit.com/viewtopic.php?p=486141#p486141
// - Adafruit GFX font size points to pixels for Epapers : https://forum.arduino.cc/t/adafruit-gfx-font-size-points-to-pixels-for-epapers/556091/11
// - ESP32 HTTP GET with Arduino IDE (OpenWeatherMap.org and ThingSpeak) : https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
// - image2cpp : https://javl.github.io/image2cpp/
// - Adafruit GFX Pixel font customiser : https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// - ASCII Table : https://www.rapidtables.com/code/text/ascii-table.html
// - PIXILART draw : https://www.pixilart.com/draw?ref=home-page
// - Weather Conditions : https://openweathermap.org/weather-conditions
// - Convert JSONVar to string Temperature from Open Weather : https://forum.arduino.cc/t/convert-jsonvar-to-string-temperature-from-open-weather/1004020
// - String replace Function : https://docs.arduino.cc/built-in-examples/strings/StringReplace/
// - String startsWith and endsWith Functions : https://docs.arduino.cc/built-in-examples/strings/StringStartsWithEndsWith/
// - fixed monospaced font 5x7 (Author Rob Jennings) : https://github.com/robjen/GFX_fonts/blob/master/GFX_fonts/Font5x7FixedMono.h
// - And from other sources.
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




#ifdef USE_Test_OpenWeatherMap_API

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 01_Test_OpenWeatherMap_API
//----------------------------------------Including the libraries.
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
//----------------------------------------

//----------------------------------------Variable declaration for your network credentials.
const char* ssid = "I-Soft";  //--> Your wifi name.
const char* password = "i-soft@2023"; //--> Your wifi password.
//----------------------------------------

// Your OpenWeatherMap API.
// Example:
// String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";
String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";

// Replace with your country code and city.
// Find city and country code here : https://openweathermap.org/
String city = "REPLACE_WITH_YOUR_CITY_NAME";
String countryCode = "REPLACE_WITH_YOUR_COUNTRY_ID_CODE";

String jsonBuffer;




//________________________________________________________________________________ connecting_To_WiFi()
void connecting_To_WiFi() {
  //----------------------------------------Set Wifi to STA mode.
  Serial.println();
  Serial.println("-------------WIFI mode");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  delay(1000);
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("-------------Connection");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      Serial.println();
      Serial.println("Failed to connect to WiFi. The ESP32 will be restarted.");
      Serial.println("-------------");
      delay(1000);
      ESP.restart();
    }
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.println("-------------");
  //:::::::::::::::::: 
  delay(1000);
  //---------------------------------------- 
}
//________________________________________________________________________________ 




//________________________________________________________________________________ get_Data_from_OpenWeatherMap()
void get_Data_from_OpenWeatherMap() {
  Serial.println();
  Serial.println("-------------");
  Serial.println("Getting Weather Data from OpenWeatherMap.");
  Serial.println("Please wait...");
  
  // Check WiFi connection status.
  if(WiFi.status()== WL_CONNECTED){
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&units=metric&APPID=" + openWeatherMapApiKey;
    
    jsonBuffer = httpGETRequest(serverPath.c_str());
    Serial.println();
    Serial.println("Weather Data in JSON form :");
    Serial.println(jsonBuffer);
    JSONVar myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var.
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    Serial.println();
    Serial.println("Weather Data taken :");

    String str_Weather = JSON.stringify(myObject["weather"][0]["main"]);
    str_Weather.replace("\"", "");
    String str_Temp = JSON.stringify(myObject["main"]["temp"]);
    String str_Humd = JSON.stringify(myObject["main"]["humidity"]);
    String str_Wind_Speed = JSON.stringify(myObject["wind"]["speed"]);
  
    Serial.print("Weather : ");     Serial.println(str_Weather);   
    Serial.print("Temperature : "); Serial.print(str_Temp);       Serial.println(" °C");
    Serial.print("Humidity : ");    Serial.print(str_Humd);       Serial.println(" %");
    Serial.print("Wind Speed : ");  Serial.print(str_Wind_Speed); Serial.println(" m/s");

    Serial.println("-------------");
    Serial.println();
  }
  else {
    Serial.println("WiFi Disconnected");
    Serial.println("-------------");
    Serial.println();
  }
}
//________________________________________________________________________________




//________________________________________________________________________________ httpGETRequest()
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path.
  http.begin(client, serverName);
  
  // Send HTTP POST request.
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  // Free resources.
  http.end();

  return payload;
}
//________________________________________________________________________________ 




//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  delay(1000);
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  connecting_To_WiFi();
  delay(500);

  get_Data_from_OpenWeatherMap();
  delay(500);
}
//________________________________________________________________________________




//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);
}
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif//USE_Test_OpenWeatherMap_API



#ifdef USE_Test_RTC
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 02_Test_and_Setup_DS3231_RTC_Module
//----------------------------------------Including Libraries.
#include "RTClib.h"
//----------------------------------------

char daysOfTheWeek[8][10] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "ERROR"};

String inputString = "";  // a String to hold incoming data.
bool stringComplete = false;  // whether the string is complete.

int d_year;
byte d_month, d_day, daysOfTheWeek_Val;
byte t_hour, t_minute, t_second;

unsigned long prevMill_Update_DateTime = 0;
const long interval_Update_DateTime = 1000;

RTC_DS3231 rtc;




//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  delay(2000);
  Serial.begin(115200);

  // reserve 200 bytes for the inputString.
  inputString.reserve(200);

  //----------------------------------------Starting and setting up the DS3231 RTC module.
  Serial.println();
  Serial.println("------------");
  Serial.println("Starting the DS3231 RTC module.");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  Serial.println("Successfully started the DS3231 RTC module.");
  Serial.println("------------");
  Serial.println();
  //----------------------------------------

  //----------------------------------------
  Serial.println();
  Serial.println("------------");
  Serial.println("Serial monitor settings :");
  Serial.println("- End Char  : Newline");
  Serial.println("- Baud Rate : 115200");
  Serial.println("------------");
  Serial.println();
  //----------------------------------------

  Serial.println();
  Serial.println("------------");
  Serial.println("Example command to set the time and date on the RTC module : ");
  Serial.println("SET,2024,7,9,12,3,0");
  Serial.println();
  Serial.println("SET = command to set.");
  Serial.println("2024 = Year.");
  Serial.println("7 = Month.");
  Serial.println("9=Day.");
  Serial.println("12 = Hour.");
  Serial.println("3 = Minute.");
  Serial.println("0 = Second.");
  Serial.println("------------");
  Serial.println();

  delay(3000);
}
//________________________________________________________________________________ 




//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  serialEvent();

  unsigned long currentMillis_Update_DateTime = millis();
  if (currentMillis_Update_DateTime - prevMill_Update_DateTime >= interval_Update_DateTime) {
    prevMill_Update_DateTime = currentMillis_Update_DateTime;

    get_DateTime();
  }

  // print the string when a newline arrives.
  if (stringComplete) {
    Serial.print("Input String : ");
    Serial.println(inputString);

    String command = "";
    command = getValue(inputString, ',', 0);

    if (command == "SET") {
      Serial.println();
      Serial.println("------------");
      Serial.println("Set the Time and Date of the DS3231 RTC Module.");
      Serial.println("Incoming settings data : ");
      
      d_year = getValue(inputString, ',', 1).toInt();
      d_month = getValue(inputString, ',', 2).toInt();
      d_day = getValue(inputString, ',', 3).toInt();
      t_hour = getValue(inputString, ',', 4).toInt();
      t_minute = getValue(inputString, ',',5).toInt();
      t_second = getValue(inputString, ',', 6).toInt();

      Serial.print("- Year : ");Serial.println(d_year);
      Serial.print("- Month : ");Serial.println(d_month);
      Serial.print("- Day : ");Serial.println(d_day);
      Serial.print("- Hour : ");Serial.println(t_hour);
      Serial.print("- Minute : ");Serial.println(t_minute);
      Serial.print("- Second : ");Serial.println(t_second);
      
      Serial.println("Set Time and Date...");
      rtc.adjust(DateTime(d_year, d_month, d_day, t_hour, t_minute, t_second));

      Serial.println("Setting the Time and Date has been completed.");
      Serial.println("------------");
      Serial.println();
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}
//________________________________________________________________________________ 




//________________________________________________________________________________ serialEvent()
void serialEvent() {
  while (Serial.available()) {
    // get the new byte.
    char inChar = (char)Serial.read();

    // if the incoming character is a newline, set a flag so the main loop can do something about it.
    if (inChar == '\n') {
      stringComplete = true;
      return;
    }
    
    // add it to the inputString.
    inputString += inChar;
  }
}
//________________________________________________________________________________ 




//________________________________________________________________________________ getValue()
// String function to process the data received
// I got this from : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//________________________________________________________________________________ 




//________________________________________________________________________________ get_DateTime()
void get_DateTime() {
  DateTime now = rtc.now();

  d_year = now.year();
  d_month = now.month();
  d_day = now.day();
  daysOfTheWeek_Val = now.dayOfTheWeek();
  if (daysOfTheWeek_Val > 7 || daysOfTheWeek_Val < 0) daysOfTheWeek_Val = 7;
  t_hour = now.hour();
  t_minute = now.minute();
  t_second = now.second();

  char full_DateTime[60];
  sprintf(full_DateTime, "%s | %02d-%02d-%d | Time : %02d:%02d:%02d", daysOfTheWeek[daysOfTheWeek_Val], d_day, d_month, d_year, t_hour, t_minute, t_second);

  Serial.print("Date : ");
  Serial.println(full_DateTime);
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif //USE_Test_RTC


#ifdef USE_Test_P5_RGB_64x32


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 03_Test_P5_RGB_64x32
//----------------------------------------Including the libraries.
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
//----------------------------------------

#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>

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
// //----------------------------------------Defines the connected PIN between P5 and ESP32.
// #define R1_PIN 19
// #define G1_PIN 13
// #define B1_PIN 18
// #define R2_PIN 5
// #define G2_PIN 12
// #define B2_PIN 17

// #define A_PIN 16
// #define B_PIN 14
// #define C_PIN 4
// #define D_PIN 27
// #define E_PIN -1  //--> required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32.

// #define LAT_PIN 26
// #define OE_PIN 15
// #define CLK_PIN 2
//----------------------------------------

//----------------------------------------Defines the P5 Panel configuration.
#define PANEL_RES_X 64  //--> Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32  //--> Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1   //--> Total number of panels chained one to another
//----------------------------------------

#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>

// ==== Scan type mapping ====
#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;

// // Initialize MatrixPanel_I2S_DMA as "dma_display".
// MatrixPanel_I2S_DMA *dma_display = nullptr;

//----------------------------------------Variable for color.
// color565(0, 0, 0); --> RGB color code. Use the "color picker" to use or find another color code.
uint16_t myRED      = dma_display->color565(255, 0, 0);
uint16_t myGREEN    = dma_display->color565(0, 255, 0);
uint16_t myBLUE     = dma_display->color565(0, 0, 255);
uint16_t myWHITE    = dma_display->color565(255, 255, 255);
uint16_t myYELLOW   = dma_display->color565(255, 255, 0);
uint16_t myCYAN     = dma_display->color565(0, 255, 255);
uint16_t myMAGENTA  = dma_display->color565(255, 0, 255);
uint16_t myVIOLET   = dma_display->color565(127, 0, 255);
uint16_t myBLACK    = dma_display->color565(0, 0, 0);
//----------------------------------------

//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // Initialize the connected PIN between Panel P5 and ESP32.
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //----------------------------------------Module configuration.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   //--> module width.
    PANEL_RES_Y,   //--> module height.
    PANEL_CHAIN,   //--> Chain length.
    _pins          //--> pin mapping.
  );
  delay(10);
  //----------------------------------------

  // Set I2S clock speed.
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;  // I2S clock speed, better leave as-is unless you want to experiment.
  delay(10);

  mxconfig.clkphase = false;
  delay(10);

  //----------------------------------------Display Setup.
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(128); //--> 0-255.
  //----------------------------------------
  
  dma_display->clearScreen();
  delay(1000);
  
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);
  dma_display->fillScreen(myWHITE);
  delay(1000);
  
  dma_display->clearScreen();
  delay(1000);

  dma_display->setTextSize(1);    
  dma_display->setTextWrap(false);
  delay(10);
}
//________________________________________________________________________________




//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  dma_display->clearScreen();
  delay(1000);
  
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myRED);
  dma_display->print("0123");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myGREEN);
  dma_display->print("ABCD");

  dma_display->setCursor(0, 17);
  dma_display->setTextColor(myBLUE);
  dma_display->print("0123");

  dma_display->setCursor(0, 25);
  dma_display->setTextColor(myWHITE);
  dma_display->print("ABCD");
  delay(1500);

  dma_display->clearScreen();
  delay(1000);

  dma_display->setCursor(21, 0);
  dma_display->setTextColor(myRED);
  dma_display->print("0123");

  dma_display->setCursor(21, 8);
  dma_display->setTextColor(myGREEN);
  dma_display->print("ABCD");

  dma_display->setCursor(21, 17);
  dma_display->setTextColor(myBLUE);
  dma_display->print("0123");

  dma_display->setCursor(21, 25);
  dma_display->setTextColor(myWHITE);
  dma_display->print("ABCD");
  delay(1500);

  dma_display->clearScreen();
  delay(1000);

  dma_display->setCursor(41, 0);
  dma_display->setTextColor(myRED);
  dma_display->print("0123");

  dma_display->setCursor(41, 8);
  dma_display->setTextColor(myGREEN);
  dma_display->print("ABCD");

  dma_display->setCursor(41, 17);
  dma_display->setTextColor(myBLUE);
  dma_display->print("0123");

  dma_display->setCursor(41, 25);
  dma_display->setTextColor(myWHITE);
  dma_display->print("ABCD");
  delay(1500);
}
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif//USE_Test_P5_RGB_64x32


#ifdef USE_OpenWeatherMap

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 04_ESP32_P5_RGB_OpenWeatherMap_Weather_Station
//----------------------------------------Including the libraries.
#include <WiFi.h>
#include <HTTPClient.h>
// #include <Arduino_JSON.h>
#include "time.h"
#include "RTClib.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Font5x7Uts.h"
//----------------------------------------
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
//----------------------------------------Defines the connected PIN between P5 and ESP32.
// #define R1_PIN 19
// #define G1_PIN 13
// #define B1_PIN 18
// #define R2_PIN 5
// #define G2_PIN 12
// #define B2_PIN 17

// #define A_PIN 16
// #define B_PIN 14
// #define C_PIN 4
// #define D_PIN 27
// #define E_PIN -1  //--> required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32.

// #define LAT_PIN 26
// #define OE_PIN  15
// #define CLK_PIN 2
//----------------------------------------

//----------------------------------------Defines the P5 Panel configuration.
#define PANEL_RES_X 64  //--> Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32  //--> Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1   //--> Total number of panels chained one to another
//----------------------------------------

#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>


// ==== Scan type mapping ====
// #define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
// using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
// VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;
// // Initialize MatrixPanel_I2S_DMA as "dma_display".
// MatrixPanel_I2S_DMA *dma_display = nullptr;

//----------------------------------------Variable for color.
// color565(0, 0, 0); --> RGB color code. Use the "color picker" to use or find another color code.
// uint16_t myRED      = dma_display->color565(255, 0, 0);
// uint16_t myGREEN    = dma_display->color565(0, 255, 0);
// uint16_t myBLUE     = dma_display->color565(0, 0, 255);
// uint16_t myWHITE    = dma_display->color565(255, 255, 255);
// uint16_t myYELLOW   = dma_display->color565(255, 255, 0);
// uint16_t myCYAN     = dma_display->color565(0, 255, 255);
// uint16_t myMAGENTA  = dma_display->color565(255, 0, 255);
// uint16_t myVIOLET   = dma_display->color565(127, 0, 255);
// uint16_t myBLACK    = dma_display->color565(0, 0, 0);

uint16_t day_Weather_Colors   = dma_display->color565(255, 170, 51);
uint16_t night_Weather_Colors = dma_display->color565(255, 255, 255);
//----------------------------------------

//----------------------------------------Variable declaration for your network credentials.
const char* ssid = "I-Soft";  //--> Your wifi name.
const char* password = "i-soft@2023"; //--> Your wifi password.
//----------------------------------------

//----------------------------------------Your OpenWeatherMap API.
// Example:
// String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";
String openWeatherMapApiKey = "REPLACE_WITH_YOUR_OPEN_WEATHER_MAP_API_KEY";

// Replace with your country code and city.
// Find city and country code here : https://openweathermap.org/
String city = "REPLACE_WITH_YOUR_CITY_NAME";
String countryCode = "REPLACE_WITH_YOUR_COUNTRY_ID_CODE";

String jsonBuffer;
//----------------------------------------

// Khai báo biến màu ở ngoài, KHÔNG gán giá trị ở đây
uint16_t myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myVIOLET, myBLACK;


//----------------------------------------Time and date variables displayed on the P5 RGB panel.
byte t_second = 0;
byte t_minute = 0;
byte t_hour   = 0;
byte d_daysOfTheWeek = 0;
byte d_day    = 0;
byte d_month  = 0;
int  d_year   = 0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char monthName[12][4]     = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

char chr_t_second[3];
char chr_t_minute[3];
char chr_t_hour  [3];
char chr_d_day   [3];
char chr_d_year  [5];

byte last_t_second = 0;
byte last_t_minute = 0;
byte last_t_hour   = 0;
byte last_d_day    = 0;
byte last_d_month  = 0;
int  last_d_year   = 0;

bool reset_Time_and_Date_Display = false;
String curr_daysOfTheWeek;
//----------------------------------------

// Timer/millis variable to display or update the time and date display.
unsigned long prevMillis_ShowTimeDate = 0;
const long interval_ShowTimeDate = 1000;

// Timer/millis variables for scrolling text.
unsigned long prevMill_Scroll_Text = 0;
byte scrolling_Speed = 0;

// Variables used to scroll text.
int scrolling_Y_Pos = 0;
long scrolling_X_Pos;
long scrolling_X_Pos_CT;
uint16_t scrolling_Text_Color;
uint16_t text_Color;
String scrolling_Text;
uint16_t text_Length_In_Pixel;
bool set_up_Scrolling_Text_Length = true;
bool start_Scroll_Text = false;

//----------------------------------------NTP Server and Time Settings.
// The DS3231 RTC module is not always accurate, so every time the ESP32 is turned on or rebooted,
// the time and date on the DS3231 RTC module will be set based on the time from the NTP Server.
// Please adjust the settings below to suit your area.

// Source : https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/

const char* ntpServer = "pool.ntp.org";

// Example setting for "gmtOffset_sec".
// - For UTC -5.00 : -5 * 60 * 60 = -18000
// - For UTC +1.00 : 1 * 60 * 60 = 3600
// - For UTC +0.00 : 0 * 60 * 60 = 0
// Check the UTC list here: https://en.wikipedia.org/wiki/List_of_UTC_offsets
// Where I live uses UTC+07:00, so : 7 * 60 * 60 = 25200
// or 3600 * 7 = 25200
const long  gmtOffset_sec = 3600 * 7;

// Set it to 3600 if your country observes Daylight saving time. Otherwise, set it to 0.
// https://en.wikipedia.org/wiki/Daylight_saving_time
const int   daylightOffset_sec = 0;
//----------------------------------------

//----------------------------------------Icon or Bitmap for weather conditions during the day.
// Icon code : "01d".
// Weather : Clear :
//           - clear sky.
const unsigned char weather_icon_code_01d [] PROGMEM = {
  0x01, 0x80, 0x21, 0x84, 0x71, 0x8e, 0x38, 0x1c, 0x13, 0xc8, 0x07, 0xe0, 0x0f, 0xf0, 0xef, 0xf7, 
  0xef, 0xf7, 0x0f, 0xf0, 0x07, 0xe0, 0x13, 0xc8, 0x38, 0x1c, 0x71, 0x8e, 0x21, 0x84, 0x01, 0x80
};

// Icon code : "02d".
// Weather : Clouds :
//           - few clouds: 11-25%.
const unsigned char weather_icon_code_02d [] PROGMEM = {
  0x00, 0x20, 0x02, 0x22, 0x01, 0x04, 0x00, 0x70, 0x00, 0xf8, 0x06, 0xfb, 0x00, 0xf8, 0x06, 0x70, 
  0x0f, 0x04, 0x1f, 0xa2, 0x1f, 0x80, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe
};

// Icon code : "03d" (I combined the icon images for the icon codes "03d", "03n", "04d" and "04n").
// Weather : Clouds :
//           - scattered clouds: 25-50%.
//           - broken clouds: 51-84%.
//           - overcast clouds: 85-100%.
const unsigned char weather_icon_code_03d [] PROGMEM = {
  0x0e, 0x00, 0x1f, 0x18, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x00, 0x00, 0x06, 0x00, 
  0x0f, 0x00, 0x1f, 0x98, 0x1f, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe
};

// Icon code : "09d" (Also for "09n").
// Weather : Drizzle :
//           - All Drizzle weather conditions.
//           Rain :
//           - light intensity shower rain.
//           - shower rain.
//           - heavy intensity shower rain.
//           - ragged shower rain.
const unsigned char weather_icon_code_09d [] PROGMEM = {
  0x06, 0x00, 0x0f, 0x00, 0x1f, 0x98, 0x1f, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0x7f, 0xfe, 0x00, 0x00, 0x24, 0x92, 0x49, 0x24, 0x00, 0x00, 0x24, 0x92, 0x49, 0x24
};

// Icon code : "10d" (Also for "10n").
// Weather : Rain :
//           - light rain.
//           - moderate rain.
//           - heavy intensity rain.
//           - very heavy rain.
//           - extreme rain.
const unsigned char weather_icon_code_10d [] PROGMEM = {
  0x00, 0x20, 0x02, 0x22, 0x01, 0x04, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xfb, 0x1c, 0x00, 0x3e, 0x70, 
  0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x00, 0x00, 0x24, 0x92, 0x49, 0x24, 0x92, 0x48
};

// Icon code : "11d" (Also for "11n").
// Weather : Thunderstorm :
//           - All Thunderstorm weather conditions.
const unsigned char weather_icon_code_11d [] PROGMEM = {
  0x0e, 0x00, 0x1f, 0x00, 0x7f, 0x98, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 
  0x00, 0x00, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x7e, 0x7e, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10
};

// Icon code : "13d" (Also for "13n").
// Weather : Rain :
//           - freezing rain.
//           Snow :
//           - All Snow weather conditions.
const unsigned char weather_icon_code_13d [] PROGMEM = {
  0x06, 0x00, 0x0f, 0x00, 0x1f, 0x98, 0x1f, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x7f, 0xfe, 0x00, 0x00, 0xa0, 0x50, 0x40, 0x20, 0xa0, 0x50, 0x0a, 0x05, 0x04, 0x02, 0x0a, 0x05
};

// Icon code : "50d" (Also for "50n").
// Weather : Mist :
//           - All Mist weather conditions.
const unsigned char weather_icon_code_50d [] PROGMEM = {
  0x06, 0x00, 0x0f, 0x00, 0x1f, 0x98, 0x1f, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0x33, 0x33, 0x00, 0x00, 0xcc, 0xcc, 0x33, 0x33
};
//----------------------------------------

//----------------------------------------Icon or Bitmap for weather conditions at night.
// Icon code : "01n".
// Weather : Clear :
//           - clear sky.
const unsigned char weather_icon_code_01n [] PROGMEM = {
  0x00, 0x10, 0x18, 0x38, 0x38, 0x10, 0x78, 0x00, 0x78, 0x00, 0xf8, 0x00, 0xf8, 0x02, 0xfc, 0x07, 
  0xfc, 0x02, 0xfe, 0x00, 0xff, 0x80, 0x7f, 0xfe, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x07, 0xe0
};

// Icon code : "02n".
// Weather : Clouds :
//           - few clouds: 11-25%.
const unsigned char weather_icon_code_02n [] PROGMEM = {
  0x02, 0x20, 0x06, 0x70, 0x0e, 0x20, 0x1e, 0x00, 0x1e, 0x02, 0x1f, 0x00, 0x1f, 0x80, 0x1f, 0xfc, 
  0x0f, 0xf8, 0x07, 0xf0, 0x03, 0xe0, 0x18, 0x0c, 0x7f, 0x9e, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe
};
//----------------------------------------

String str_Day_Icon[10] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d"};
String str_Night_Icon[10] = {"01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

const unsigned char* day_Weather_Icon_List[10] = {
  weather_icon_code_01d,
  weather_icon_code_02d,
  weather_icon_code_03d,
  weather_icon_code_03d,
  weather_icon_code_09d,
  weather_icon_code_10d,
  weather_icon_code_11d,
  weather_icon_code_13d,
  weather_icon_code_50d
};

const unsigned char* night_Weather_Icon_List[10] = {
  weather_icon_code_01n,
  weather_icon_code_02n,
  weather_icon_code_03d,
  weather_icon_code_03d,
  weather_icon_code_09d,
  weather_icon_code_10d,
  weather_icon_code_11d,
  weather_icon_code_13d,
  weather_icon_code_50d
};

String str_Temp, str_Humd, str_Weather_Icon;
String str_Weather_Conditions, str_Weather_Conditions_Des;
String str_Feels_like, str_Pressure, str_Wind_Speed;
    
// Declare the “RTC_DS3231” object as “rtc”.
RTC_DS3231 rtc;




//________________________________________________________________________________ connecting_To_WiFi()
void connecting_To_WiFi() {
  dma_display->clearScreen();
  delay(500);

  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Connecting");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myBLUE);
  dma_display->print("to WiFi...");

  //----------------------------------------Set Wifi to STA mode.
  Serial.println();
  Serial.println("-------------WIFI mode");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  delay(1000);
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("-------------Connection");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      dma_display->clearScreen();
      delay(500);

      dma_display->setCursor(0, 0);
      dma_display->setTextColor(myRED);
      dma_display->print("Failed to");
    
      dma_display->setCursor(0, 8);
      dma_display->setTextColor(myRED);
      dma_display->print("Connect to");
    
      dma_display->setCursor(0, 16);
      dma_display->setTextColor(myRED);
      dma_display->print("WiFi !");
  
      Serial.println();
      Serial.println("Failed to connect to WiFi. The ESP32 will be restarted.");
      Serial.println("-------------");
      delay(1000);
      ESP.restart();
    }
  }

  dma_display->clearScreen();
  delay(500);

  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myGREEN);
  dma_display->print("Successfully");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myGREEN);
  dma_display->print("Connected");

  dma_display->setCursor(0, 16);
  dma_display->setTextColor(myGREEN);
  dma_display->print("to WiFi.");
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.println("-------------");
  //:::::::::::::::::: 
  delay(1000);
  //---------------------------------------- 

  dma_display->clearScreen();
  delay(500);
}
//________________________________________________________________________________ 




//________________________________________________________________________________ get_TimeDate_from_NTP_server()
void get_TimeDate_from_NTP_server() {
  dma_display->clearScreen();
  delay(500);
  
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Set RTC");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myBLUE);
  dma_display->print("From");

  dma_display->setCursor(0, 16);
  dma_display->setTextColor(myBLUE);
  dma_display->print("NTP Server");
  delay(2000);

  dma_display->clearScreen();
  delay(500);

  Serial.println();
  Serial.println("-------------");
  Serial.println("Get Time and Date from NTP server.");
  Serial.println("Please wait...");

  if(WiFi.status()== WL_CONNECTED){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
    struct tm timeinfo;
  
    if (!getLocalTime(&timeinfo)) {
      dma_display->clearScreen();
      delay(500);
  
      dma_display->setCursor(0, 0);
      dma_display->setTextColor(myRED);
      dma_display->print("Failed to");
    
      dma_display->setCursor(0, 8);
      dma_display->setTextColor(myRED);
      dma_display->print("obtain time.");
    
      Serial.println();
      Serial.println("Failed to obtain time");
      Serial.println("-------------");
      delay(1500);

      dma_display->clearScreen();
      delay(500);
      return;
    }
  
    t_hour   = timeinfo.tm_hour;
    t_minute = timeinfo.tm_min;
    t_second = timeinfo.tm_sec;
  
    d_day   = timeinfo.tm_mday;
    d_month = timeinfo.tm_mon+1;
    d_year  = timeinfo.tm_year+1900;
  
    rtc.adjust(DateTime(d_year, d_month, d_day, t_hour, t_minute, t_second));
  
    char TimeDate[40];
    sprintf(TimeDate, "Date : %02d-%02d-%d | Time : %02d:%02d:%02d", d_day, d_month, d_year, t_hour, t_minute, t_second);
  
    Serial.println();
    Serial.println(TimeDate);
    Serial.println("-------------");
    Serial.println();
  } else {
    dma_display->clearScreen();
    delay(500);

    dma_display->setCursor(11, 3);
    dma_display->setTextColor(myRED);
    dma_display->print("FAILED !");
  
    dma_display->setCursor(24, 13);
    dma_display->setTextColor(myRED);
    dma_display->print("WiFi");
  
    dma_display->setCursor(3, 21);
    dma_display->setTextColor(myRED);
    dma_display->print("Disconnected");
    
    Serial.println("WiFi Disconnected");
    Serial.println("-------------");
    Serial.println();

    delay(1500);
  }

  dma_display->clearScreen();
  delay(500);
}
//________________________________________________________________________________ 




//________________________________________________________________________________ get_TimeDate()
void get_TimeDate() {
  DateTime now = rtc.now();

  t_second        = now.second();
  t_minute        = now.minute();
  t_hour          = now.hour();
  d_daysOfTheWeek = now.dayOfTheWeek();
  d_day           = now.day();
  d_month         = now.month();
  d_year          = now.year();
  
  sprintf(chr_t_second, "%02d", t_second);
  sprintf(chr_t_minute, "%02d", t_minute);
  sprintf(chr_t_hour,   "%02d", t_hour);
  sprintf(chr_d_day,    "%02d", d_day);
  sprintf(chr_d_year,   "%d", d_year);
}
//________________________________________________________________________________ 




//________________________________________________________________________________ displays_Time_and_Date()
void displays_Time_and_Date() {
  if (t_hour != last_t_hour || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(19, 9, 11, 7, myBLACK);

    dma_display->setCursor(19, 9);
    dma_display->setTextColor(myRED);
    dma_display->print(chr_t_hour);

    dma_display->setCursor(32, 9);
    dma_display->setTextColor(dma_display->color565(223, 255, 0));
    dma_display->print(":");
  }

  if (t_minute != last_t_minute || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(36, 9, 11, 7, myBLACK);
    
    dma_display->setCursor(36, 9);
    dma_display->setTextColor(myRED);
    dma_display->print(chr_t_minute);

    dma_display->setCursor(49, 9);
    dma_display->setTextColor(dma_display->color565(223, 255, 0));
    dma_display->print(":");
  }

  if (t_second != last_t_second || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(53, 9, 11, 7, myBLACK);
    
    dma_display->setCursor(53, 9);
    dma_display->setTextColor(myRED);
    dma_display->print(chr_t_second);
  }

  if (d_day != last_d_day || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(0, 17, 11, 7, myBLACK);
    
    dma_display->setCursor(0, 17);
    dma_display->setTextColor(dma_display->color565(255, 0, 217));
    dma_display->print(chr_d_day);

    for (int i = 12; i < 17; i++) {
      dma_display->drawPixel(i, 20, dma_display->color565(255, 0, 64));
    }
  }

  if (d_month != last_d_month || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(18, 17, 17, 7, myBLACK);
    
    dma_display->setCursor(18, 17);
    dma_display->setTextColor(dma_display->color565(160, 0, 255));
    dma_display->print(monthName[d_month-1]);

    for (int i = 36; i < 40; i++) {
      dma_display->drawPixel(i, 20, dma_display->color565(255, 0, 64));
    }
  }

  if (d_year != last_d_year || reset_Time_and_Date_Display == true) {
    dma_display->fillRect(41, 17, 23, 7, myBLACK);
    
    dma_display->setCursor(41, 17);
    dma_display->setTextColor(myBLUE);
    dma_display->print(chr_d_year);
  }

  last_t_second = t_second;
  last_t_minute = t_minute;
  last_t_hour   = t_hour;
  last_d_day    = d_day;
  last_d_month  = d_month;
  last_d_year   = d_year;

  reset_Time_and_Date_Display = false;
}
//________________________________________________________________________________ 




//________________________________________________________________________________ displays_Weather_Data()
void displays_Weather_Data() {
  if (str_Weather_Icon.endsWith("d")) {
    int str_Day_Icon_Length = sizeof(str_Day_Icon) / sizeof(str_Day_Icon[0]);
    for (int i = 0; i < str_Day_Icon_Length; i++) {
      if (str_Day_Icon[i] == str_Weather_Icon) {
        dma_display->fillRect(0, 0, 16, 16, myBLACK);
        dma_display->drawBitmap(0,0, day_Weather_Icon_List[i], 16,16, day_Weather_Colors);
        break;
      }
    }
  }
  
  if (str_Weather_Icon.endsWith("n")) {
    int str_Night_Icon_Length = sizeof(str_Night_Icon) / sizeof(str_Night_Icon[0]);
    for (int i = 0; i < str_Night_Icon_Length; i++) {
      if (str_Night_Icon[i] == str_Weather_Icon) {
        dma_display->fillRect(0, 0, 16, 16, myBLACK);
        dma_display->drawBitmap(0,0, night_Weather_Icon_List[i], 16,16, night_Weather_Colors);
        break;
      }
    }
  }
  
  dma_display->fillRect(19, 0, 28, 7, myBLACK);
  dma_display->setCursor(19, 0);
  dma_display->setTextColor(dma_display->color565(255, 117, 24));
  int curr_Temp = str_Temp.toInt();
  dma_display->print(curr_Temp);
  dma_display->print("°C");

  int curr_Humd = str_Humd.toInt();
  if (curr_Humd < 100) {
    dma_display->fillRect(47, 0, 17, 7, myBLACK);
    dma_display->setCursor(47, 0);
    dma_display->setTextColor(myCYAN);
    dma_display->print(curr_Humd);
    dma_display->print("%");
  } else {
    dma_display->fillRect(41, 0, 17, 7, myBLACK);
    dma_display->setCursor(41, 0);
    dma_display->setTextColor(myCYAN);
    dma_display->print(curr_Humd);
    dma_display->print("%");
  }
  
}
//________________________________________________________________________________ 



String httpGETRequest(const char* serverName) ;
uint16_t getTextWidth(const char* text);
//________________________________________________________________________________ get_Data_from_OpenWeatherMap()
void get_Data_from_OpenWeatherMap() {
  Serial.println();
  Serial.println("-------------");
  Serial.println("Getting Weather Data from OpenWeatherMap.");
  Serial.println("Please wait...");
  
  // // Check WiFi connection status.
  // if(WiFi.status()== WL_CONNECTED){
  //   String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&units=metric&APPID=" + openWeatherMapApiKey;
    
  //   jsonBuffer = httpGETRequest(serverPath.c_str());
  //   Serial.println();
  //   Serial.println("Weather Data in JSON form :");
  //   Serial.println(jsonBuffer);
  //   JSONVar myObject = JSON.parse(jsonBuffer);

  //   // JSON.typeof(jsonVar) can be used to get the type of the var
  //   if (JSON.typeof(myObject) == "undefined") {
  //     Serial.println("Parsing input failed!");
  //     return;
  //   }

  //   str_Weather_Icon = JSON.stringify(myObject["weather"][0]["icon"]);
  //   str_Weather_Icon.replace("\"", "");
  //   str_Temp = JSON.stringify(myObject["main"]["temp"]);
  //   str_Humd = JSON.stringify(myObject["main"]["humidity"]);

  //   str_Weather_Conditions = JSON.stringify(myObject["weather"][0]["main"]);
  //   str_Weather_Conditions_Des = JSON.stringify(myObject["weather"][0]["description"]);
  //   str_Weather_Conditions.replace("\"", "");
  //   str_Weather_Conditions_Des.replace("\"", "");

  //   str_Feels_like = JSON.stringify(myObject["main"]["feels_like"]);
  //   str_Pressure = JSON.stringify(myObject["main"]["pressure"]);
  //   str_Wind_Speed = JSON.stringify(myObject["wind"]["speed"]);

  //   Serial.println();
  //   Serial.println("Weather Data taken :");
  //   Serial.print("Temperature : ");Serial.print(str_Temp);Serial.print(" °C | ");
  //   Serial.print("Humidity : ");Serial.print(str_Humd);Serial.println(" %");
  //   Serial.println();
    
  //   Serial.print("City : ");Serial.print(city);Serial.print(" ¦ ");
  //   Serial.print("Weather : ");Serial.print(str_Weather_Conditions);Serial.print(" (");Serial.print(str_Weather_Conditions_Des);Serial.print(") ¦ ");
  //   Serial.print("Feels like : ");Serial.print(str_Feels_like);Serial.print(" °C ¦ ");
  //   Serial.print("Pressure : ");Serial.print(str_Pressure);Serial.print(" hPa ¦ ");
  //   Serial.print("Wind Speed : ");Serial.print(str_Wind_Speed);Serial.println(" m/s");
    
  //   Serial.println("-------------");
  //   Serial.println();
  // }
  // else {
  //   dma_display->clearScreen();
  //   delay(500);

  //   dma_display->setCursor(11, 3);
  //   dma_display->setTextColor(myRED);
  //   dma_display->print("FAILED !");
  
  //   dma_display->setCursor(24, 13);
  //   dma_display->setTextColor(myRED);
  //   dma_display->print("WiFi");
  
  //   dma_display->setCursor(3, 21);
  //   dma_display->setTextColor(myRED);
  //   dma_display->print("Disconnected");
  
  //   Serial.println("WiFi Disconnected");
  //   Serial.println("-------------");
  //   Serial.println();

  //   delay(1500);
  // }

  // dma_display->clearScreen();
  // delay(500);
}
//________________________________________________________________________________




//________________________________________________________________________________ httpGETRequest()
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path.
  http.begin(client, serverName);
  
  // Send HTTP POST request.
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  // Free resources.
  http.end();

  return payload;
}
//________________________________________________________________________________ 




//________________________________________________________________________________ run_Scrolling_Text()
// Subroutine for scrolling text.
void run_Scrolling_Text(uint8_t st_Y_Pos, byte st_Speed, const char * st_Text, uint16_t st_Color) {
  if (start_Scroll_Text == true && set_up_Scrolling_Text_Length == true) {
    if (strlen(st_Text) > 0) {
      text_Length_In_Pixel = getTextWidth(st_Text);
      scrolling_X_Pos = PANEL_RES_X;
      
      set_up_Scrolling_Text_Length = false;
    } else {
      start_Scroll_Text = false;
      return;
    }
  }

  unsigned long currentMillis_Scroll_Text = millis();
  if (currentMillis_Scroll_Text - prevMill_Scroll_Text >= st_Speed) {
    prevMill_Scroll_Text = currentMillis_Scroll_Text;

    scrolling_X_Pos--;
    if (scrolling_X_Pos < -(PANEL_RES_X + text_Length_In_Pixel)) {
      set_up_Scrolling_Text_Length = true;
      start_Scroll_Text = false;
      
      return;
    }

    dma_display->fillRect(0, st_Y_Pos, 64, 7, myBLACK);
    
    dma_display->setTextColor(st_Color);
    dma_display->setCursor(scrolling_X_Pos, st_Y_Pos);
    dma_display->print(st_Text);
  }
}
//________________________________________________________________________________ 




//________________________________________________________________________________ getTextWidth()
// Subroutine to get the length of text in pixels.
uint16_t getTextWidth(const char* text) {
  int16_t x1, y1;
  uint16_t w, h;
  dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}
//_______________________________________________________________________________
//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  delay(1000);
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  // Initialize the connected PIN between Panel P5 and ESP32.
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //----------------------------------------Module configuration.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   //--> module width.
    PANEL_RES_Y,   //--> module height.
    PANEL_CHAIN,   //--> Chain length.
    _pins          //--> pin mapping.
  );
  delay(10);
  //----------------------------------------

  // Set I2S clock speed.
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_15M;  // I2S clock speed, better leave as-is unless you want to experiment.
  delay(10);

  mxconfig.min_refresh_rate = 120;
  delay(10);

  mxconfig.clkphase = false;
  delay(10);

  //----------------------------------------Display Setup.
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(128); //--> 0-255.
  //----------------------------------------
  
myRED      = dma_display->color565(255, 0, 0);
myGREEN    = dma_display->color565(0, 255, 0);
myBLUE     = dma_display->color565(0, 0, 255);
myWHITE    = dma_display->color565(255, 255, 255);
myYELLOW   = dma_display->color565(255, 255, 0);
myCYAN     = dma_display->color565(0, 255, 255);
myMAGENTA  = dma_display->color565(255, 0, 255);
myVIOLET   = dma_display->color565(127, 0, 255);
myBLACK    = dma_display->color565(0, 0, 0);

  dma_display->clearScreen();
  
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);
  dma_display->fillScreen(myWHITE);
  delay(1000);
  
  dma_display->clearScreen();
  delay(1000);

  dma_display->setTextSize(1);    
  dma_display->setTextWrap(false);
//   dma_display->setFont(&Font5x7Uts);
  delay(10);

  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myRED);
  dma_display->print("ESP32");

  dma_display->setCursor(35, 0);
  dma_display->setTextColor(myGREEN);
  dma_display->print("P5RGB");

  dma_display->setCursor(11, 8);
  dma_display->setTextColor(myBLUE);
  dma_display->print("WEATHER");

  dma_display->setCursor(12, 16);
  dma_display->setTextColor(myBLUE);
  dma_display->print("STATION");

  dma_display->setCursor(8, 25);
  dma_display->setTextColor(myWHITE);
  dma_display->print("UTEH STR");
  delay(1500);

  //----------------------------------------Starting and setting up the DS3231 RTC module.
  dma_display->clearScreen();
  delay(500);
  
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Starting");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myBLUE);
  dma_display->print("RTC Module");
  delay(1500);

  Serial.println();
  Serial.println("------------");
  Serial.println("Starting the DS3231 RTC module.");
  if (!rtc.begin()) {
    dma_display->clearScreen();
    delay(500);
  
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(myRED);
    dma_display->print("Failed to");
  
    dma_display->setCursor(0, 8);
    dma_display->setTextColor(myRED);
    dma_display->print("Start");
  
    dma_display->setCursor(0, 16);
    dma_display->setTextColor(myRED);
    dma_display->print("RTC Module !");

    Serial.println("Couldn't find RTC");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("Successfully started the DS3231 RTC module.");
  Serial.println("------------");
  //----------------------------------------

  connecting_To_WiFi();
  delay(500);

  get_TimeDate_from_NTP_server();
  delay(500);

  dma_display->clearScreen();
  delay(500);
  
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Get");

  dma_display->setCursor(0, 8);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Weather");

  dma_display->setCursor(0, 16);
  dma_display->setTextColor(myBLUE);
  dma_display->print("Data...");
  delay(1000);
  
  get_Data_from_OpenWeatherMap();
  delay(1000);

  get_TimeDate();

  reset_Time_and_Date_Display = true;
  displays_Time_and_Date();
  displays_Weather_Data();
  delay(1000);
}
//________________________________________________________________________________




//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currMillis_ShowTimeDate = millis();
  if (currMillis_ShowTimeDate - prevMillis_ShowTimeDate >= interval_ShowTimeDate) {
    prevMillis_ShowTimeDate = currMillis_ShowTimeDate;

    get_TimeDate();
    displays_Time_and_Date();

    // Retrieves weather data from OpenWeatherMap every 10 minutes and 10 seconds.
    // This means that weather data will be retrieved or updated from OpenWeatherMap if:
    // - minutes = 10 and seconds = 10
    // - minutes = 20 and seconds = 10
    // - minutes = 30 and seconds = 10
    // - and so on.
    if ((t_minute % 10) == 0 && t_second == 10) {
      dma_display->clearScreen();
      delay(500);
    
      dma_display->setCursor(9, 3);
      dma_display->setTextColor(myBLUE);
      dma_display->print("UPDATING");
    
      dma_display->setCursor(11, 12);
      dma_display->setTextColor(myBLUE);
      dma_display->print("WEATHER");
    
      dma_display->setCursor(20, 21);
      dma_display->setTextColor(myBLUE);
      dma_display->print("DATA");
      delay(1000);

      get_Data_from_OpenWeatherMap();
      delay(500);

      reset_Time_and_Date_Display = true;
      displays_Time_and_Date();
      displays_Weather_Data();

      start_Scroll_Text == false;
      set_up_Scrolling_Text_Length = true;
    }
  }

  if (start_Scroll_Text == false) {
    scrolling_Y_Pos = 25;   //--> Y position settings for scrolling text.
    scrolling_Speed = 35;   //--> Speed ​​settings for scrolling text.
    scrolling_Text_Color = myGREEN;

    get_TimeDate();
    curr_daysOfTheWeek = String(daysOfTheWeek[d_daysOfTheWeek]);

    scrolling_Text  = "";
    scrolling_Text  = "City : " + city + " ¦ ";
    scrolling_Text += "Day : " + curr_daysOfTheWeek + " ¦ ";
    scrolling_Text += "Weather : " + str_Weather_Conditions + " ( ";
    scrolling_Text += str_Weather_Conditions_Des + " ) ¦ ";
    scrolling_Text += "Feels like : " + str_Feels_like + " °C ¦ ";
    scrolling_Text += "Pressure : " + str_Pressure + " hPa ¦ ";
    scrolling_Text += "Wind Speed : " + str_Wind_Speed + " m/s";
    
    start_Scroll_Text = true;
  }

  if (start_Scroll_Text == true) {
    run_Scrolling_Text(scrolling_Y_Pos, scrolling_Speed, scrolling_Text.c_str(), scrolling_Text_Color);
  }
}
#endif//USE_Weather_Station
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif //USE_LMD_V2