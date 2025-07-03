#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
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

// ==== Panel config ====
#define PANEL_RES_X 104//104//64      // Số pixel ngang của panel
#define PANEL_RES_Y 52//52//32      // Số pixel dọc của panel

// ==== Scan type mapping ====
#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH // hoặc FOUR_SCAN_64PX_HIGH tùy panel thực tế
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

// ==== Khai báo đối tượng DMA và VirtualPanel ====
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;

// ==== Màu sắc mẫu ====
uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== VirtualMatrixPanel Example 3: Single 1/4 Scan Panel ===");

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

  // In 4 dòng chữ
  virtualDisp->setTextColor(myWHITE);
  virtualDisp->setCursor(0, 0);
  virtualDisp->print("dma display");

  virtualDisp->setCursor(0, 8);
  virtualDisp->print("*********");

  virtualDisp->setCursor(0, 16);
  virtualDisp->print("*       *");

  virtualDisp->setCursor(0, 24);
  virtualDisp->print("hello world");
}

void loop() {
  // Không làm gì trong loop
