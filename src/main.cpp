

#define USE_LMD_V1
// #define USE_LMD_Master
// #define USE_LMD_V2
// #define USE_Test_P5_RGB_64x32
// #define USE_OpenWeatherMap

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
#include <Fonts/micross10pt7b.h>
#include <Fonts/micross9pt7b.h>
#include <Fonts/micross6pt7b.h>
#include <Fonts/micross5pt7b.h>
#include <Fonts/micross7pt7b.h>
#include <Fonts/micross8pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/SansSerifCollection9pt7b.h>
#include <Fonts/ONYX6pt7b.h>
#include <Fonts/BOD_PSTC6pt7b.h>
#include <Fonts/dutcheb6pt7b.h>
#include <Fonts/dutcheb7pt7b.h>
#include <Fonts/dutcheb8pt7b.h>

// Font đặc biệt (pixel, mini, custom)
#include <Fonts/B_5px.h>
#include <Fonts/hud5pt7b.h>
#include <Fonts/mythic_5pixels.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/TomThumb.h>
#include <Fonts/ENGR6pt7b.h>
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
// #define E32_M0_PIN    15
// #define E32_M1_PIN    16
// #define E32_TX_PIN    17
// #define E32_RX_PIN    18
// #define E32_AUX_PIN   -1
//   HardwareSerial MySerial1(1); // UART1
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


#include "ALC_Project.h"
#include "WifiPostal.h"

// void consolePrintln(const String& line) {
//   Serial.println(line);
// //   addConsoleLine(line + '\n');
// }
#include "ModuleLoRaE32.h"
M_LoRa_E32 LORAE32;


// ==== Scan type mapping ====
#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;
// ==== Màu sắc mẫu ====
uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE, myYELLOW, myORANGE;

uint16_t connectionStatusColor[4];// = {myRED, myYELLOW, myORANGE, myGREEN};


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
void defaultLedPanel();

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
            if (number == 1 && font == 2) {
              x-= 1;
              virtualDisp->drawLine(x+2, y+4, x+4, y+4, color);
              virtualDisp->drawLine(x+2, y+1, x+2, y+1, color);

            }
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
    defaultLedPanel();
    Serial.println("=== Khởi tạo WebSocket ===");
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: "); Serial.println(ssid);
    Serial.print("Password: "); Serial.println(password);
    WiFi.begin(ssid, password);

    // defaultLedPanel();

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

                myBLACK  = virtualDisp->color565(0, 0, 0);
                myWHITE  = virtualDisp->color565(255, 255, 255);
                myRED    = virtualDisp->color565(255, 0, 0);
                myYELLOW = virtualDisp->color565(255, 255, 0);
                myORANGE = virtualDisp->color565(255, 128, 0);
                myGREEN  = virtualDisp->color565(0, 255, 0);
                myBLUE   = virtualDisp->color565(0, 0, 255);
                connectionStatusColor[3] = myRED;
                connectionStatusColor[2] = myORANGE;
                connectionStatusColor[1] = myYELLOW;
                connectionStatusColor[0] = myGREEN;

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
volatile int plan0 = 0, plan1 = 0, plan2 = 0, plan3 = 0;
uint16_t counters[4] = {0}; // 4 số đếm, 4 số hiển thị
uint16_t plans[4] = {0}; // 4 số đếm, 4 số hiển thị
int swapConnectQuality(int val) {
    if (val == 4) return 1;
    if (val == 3) return 2;
    if (val == 2) return 3;
    if (val == 1) return 4;
    return val;
}

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
        
        
        numberContents[0] = count3;
        numberContents[1] = count2;
        numberContents[2] = count1;
        numberContents[3] = count0;
        numberContents[4] = plan3;
        numberContents[5] = plan2;
        numberContents[6] = plan1;
        numberContents[7] = plan0;
        
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
                // Serial.printf("Đã set brightness: %d\n", brightness);
            }
            JsonArray connectionStatus = pages[currentPage][3].as<JsonArray>();
            if (connectionStatus.size() >= 3) {
                // conStatus1 = connectionStatus[0].as<int>();
                // conStatus2 = connectionStatus[1].as<int>();
                // conStatus3 = connectionStatus[2].as<int>();
                // conStatus4 = connectionStatus[3].as<int>();
                
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
                    // printLedPanel(8888,x+1,y,1,font4,color);
                    

                    if (digits == 1) printLedPanel(numberContents[i], x, y+1, 1, font1, color);
                    if (digits == 2) printLedPanel(numberContents[i], x+1, y+1, 1, font2, color);
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
        GenNumber(1, 0, 1, 2, virtualDisp->color565(255, 115, 255));
        GenNumber(2, 32, 1, 2, virtualDisp->color565(255, 115, 255));
        GenNumber(3, 0, 17, 2,  virtualDisp->color565(255, 115, 255));
        GenNumber(4, 32, 17, 2,  virtualDisp->color565(255, 115, 255));

        virtualDisp->drawLine(1, 7+conStatus1-1, 1, 7+3 , connectionStatusColor[conStatus1-1]);
        virtualDisp->drawLine(33, 7+conStatus2-1, 33, 7+3 , connectionStatusColor[conStatus2-1]);
        virtualDisp->drawLine(1, 24+conStatus3-1, 1, 24+3 ,connectionStatusColor[conStatus3-1]);
        virtualDisp->drawLine(33, 24+conStatus4-1, 33, 24+3 , connectionStatusColor[conStatus4-1]);
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
        case 13: return &ONYX6pt7b;
        case 14: return &ENGR6pt7b;
        case 15: return &BOD_PSTC6pt7b; // 04B_5px (nếu tên biến đúng là B_085pt7b)
        case 16: return &dutcheb6pt7b;
        case 17: return &hud5pt7b;
        case 18: return &mythic_pixels5pt7b;
        case 19: return &Org_01;
        case 20: return &Picopixel;
        case 21: return &Tiny3x3a2pt7b;
        case 22: return &TomThumb;
        case 23: return &dutcheb7pt7b;
        case 24: return &dutcheb8pt7b;
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
    // Serial.println ("Font col px: " + String(fontColPx));

    int8_t digitOfNumber = countDigits(number);
    if (font == nullptr) x = x - autoMiddle(5, number);    
    else x = x - autoMiddle(fontColPx, number);
    if (font == nullptr) virtualDisp->setCursor(x, y-7);
    // if(font == &dutcheb7pt7b) y+1;
    // if(font== &FreeSerifBold9pt7b) y+1;

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

void defaultLedPanel() {
    // Thiết lập mặc định cho LED Panel
  if (virtualDisp) {
    Serial.println(">> Thiết lập LED Panel về mặc định");
    virtualDisp->fillScreen(myBLACK);
    virtualDisp->setTextColor(myWHITE);
    virtualDisp->setTextSize(1);
    virtualDisp->setFont(&FreeMono9pt7b);
    virtualDisp->setCursor(4, 16);
    virtualDisp->print("I Soft");
    dma_display->flipDMABuffer();
  }
  else {
      Serial.println("❌ Không thể thiết lập LED Panel, virtualDisp là nullptr.");
  }
}
void setup() {

    Serial.begin(115200);
    WiFi_AP_setup(server) ; // Khởi tạo WiFi AP
    
    webSocketServer.onEvent(webSocketEvent);      // Đăng ký sự kiện trước
    server.addHandler(&webSocketServer);          // Thêm WebSocket vào server
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index_aP.html");

    delay(5000);
    setup_ledPanel();

    setup_littleFS();
    // setup_WS(); // Khởi tạo WebSocket sau khi khởi tạo panel
if (!virtualDisp || !dma_display) {
        Serial.println("❌ Khởi tạo panel thất bại. Không tiếp tục.");
        while (1) delay(1000);
    }
    ALC_setup(server);
    LORAE32.initLoRaE32(); // Khởi tạo LoRa E32
    LORAE32.API_Setup(server); // Thiết lập API cho LoRa E32  

    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT0_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COUNT3_BUTTON_PIN, INPUT_PULLUP);
    Serial.println(">> Đang ở chế độ NORMAL, sẽ hiển thị panel");
    // Tạo file CONFIG.json nếu chưa có
    if (!LittleFS.exists("/CONFIG.json")) {
        Serial.println("⚠️ File CONFIG.json không tồn tại, tạo mới...");
    } else {
        Serial.println("✅ Đã tìm thấy CONFIG.json, sẽ load nội dung.");
    }
    configDoc = loadConfig("/CONFIG.json");
    TotalPages = getTotalPages();
    // Khởi tạo WebSocket server (đã gọi trong setup_ledPanel)
    // Hiển thị nội dung từ DATA.json nếu có
    DynamicJsonDocument dataDoc = loadConfig("/DATA.json");
    virtualDisp->fillScreen(myBLACK);
    showCurrentPage(0);
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
        Serial.printf("Trang hiện tại: %d\n", currentPage);
        if (currentPage < 0) currentPage = 0; // Đảm bảo currentPage không âm
        if (currentPage > totalPages) currentPage = totalPages;
        CurrentPage = currentPage; // Đảm bảo currentPage không vượt quá tổng số trang
        // Hiển thị trang hiện tại
        virtualDisp->fillScreen(myBLACK);
        showCurrentPage(currentPage );
        
    }
    lastModeBtn = modeBtn;
    ALC_loop(); 
    WiFi_AP_loop();
    for(uint8_t i = 0; i < 4; i++) {
      if(getResult(i) != counters[i]) {
        counters[i] = getResult(i);
        Serial.printf("Counter %d: %d\n", i, counters[i]);
        if (i == 0) count0 = counters[i];
        else if (i == 1) count1 = counters[i];
        else if (i == 2) count2 = counters[i];
        else if (i == 3) count3 = counters[i];
        // Hiển thị lại trang hiện tại
        showCurrentPage(CurrentPage); delay(100); 
      }
    }
    
    for(uint8_t i = 0; i < 4; i++) {
      if(getPlan(i) != plans[i]) {
        plans[i] = getPlan(i);
        Serial.printf("Plan %d: %d\n", i, plans[i]);
        if (i == 0) plan0 = plans[i];
        else if (i == 1) plan1 = plans[i];
        else if (i == 2) plan2 = plans[i];
        else if (i == 3) plan3 = plans[i];
        showCurrentPage(CurrentPage); delay(100);
      }
    }
          conStatus1 = swapConnectQuality(getConnectQuality(0));
          conStatus2 = swapConnectQuality(getConnectQuality(1));
          conStatus3 = swapConnectQuality(getConnectQuality(2));
          conStatus4 = swapConnectQuality(getConnectQuality(3));
        if (
            conStatus1 != swapConnectQuality(getConnectQuality(0)) ||
            conStatus2 != swapConnectQuality(getConnectQuality(1)) ||
            conStatus3 != swapConnectQuality(getConnectQuality(2)) ||
            conStatus4 != swapConnectQuality(getConnectQuality(3))) {
            showCurrentPage(CurrentPage);delay(100);
          }
          static long timeLastUpdate = 0;

          
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
  Serial.println("Get RF Status: " + String(getConnectQuality(0)) + " " + String(getConnectQuality(1)) + " " + String(getConnectQuality(2)) + " " + String(getConnectQuality(3)));
    lastStatusTime = currentTime;
    // if (getMsgRF == true){
    //     conStatus1 = swapConnectQuality(getConnectQuality(0));
    //     conStatus2 = swapConnectQuality(getConnectQuality(1));
    //     conStatus3 = swapConnectQuality(getConnectQuality(2));
    //     conStatus4 = swapConnectQuality(getConnectQuality(3));
    //     showCurrentPage(CurrentPage);
    // } 
    //     getMsgRF = false;
    // }
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
