#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>

// ==== Pin mapping ====
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
#define PANEL_RES_X 64
#define PANEL_RES_Y 64

#define PANEL_SCAN_TYPE FOUR_SCAN_32PX_HIGH//FOUR_SCAN_32PX_HIGH // Change this to your desired scan type
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;

MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<CHAIN_NONE, MyScanTypeMapping>* virtualDisp = nullptr;
uint16_t myBLACK, myWHITE;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== VirtualMatrixPanel: STANDARD_TWO_SCAN, 128x32 ===");

  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X*2,
    PANEL_RES_Y/2,
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
  virtualDisp->clearScreen();
    virtualDisp->drawLine(0,0,3, 0, virtualDisp->color565(0, 255, 255));
    virtualDisp->drawLine(4,0,7, 0, virtualDisp->color565(0, 255, 0));
    virtualDisp->drawLine(12,0,15, 0, virtualDisp->color565(255, 0, 0));
    virtualDisp->drawLine(16,0,19, 0, virtualDisp->color565(255, 0, 0));
    virtualDisp->drawLine(24,0,27, 0, virtualDisp->color565(255, 255, 0));
    // virtualDisp->drawLine(6,0,PANEL_RES_X, 0, virtualDisp->color565(255, 255, 0));
    virtualDisp->flipDMABuffer();
}

void handle_serial () {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int row = input.toInt();
    virtualDisp->clearScreen();
    virtualDisp->drawLine(5, row, 5, PANEL_RES_Y - 1, virtualDisp->color565(255, 255, 255));
    virtualDisp->flipDMABuffer();    
  }
}
void loop() {
  
  
}