#ifndef ALC_PROJECT_H
#define ALC_PROJECT_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>


// ==== Configurable constants ====
#define NUM_COUNTERS 4
#define CONFIG_FILE "/project_config.json"

#define M0_PIN    15
#define M1_PIN    16
#define TX_PIN    17
#define RX_PIN    18
#define BAUD_RATE 9600
#define TTL_1W

//hardcode 
// Pin definitions for 74HC595 shift registers
#define DATA1_PIN 3   // Data pin for first 7-seg array
#define DATA2_PIN 12  // Data pin for second 7-seg array
#define CLK_PIN   42  // Clock pin (shared)
#define LAT_PIN   41  // Latch pin (shared)

// ==== Data Structures ====
struct CounterConfig {
    uint8_t pin;         // ESP32-S3 pin number
    uint16_t filter;     // debounce/filter time (ms)
    uint16_t multiple;   // counting multiple
};

struct ProjectConfig {
    char lineName[32]; // Project name
    uint8_t role;        // 0 = master, 1 = node
    uint8_t Device;          // LoRa ID (0-255)
    uint8_t channel;    // LoRa channel (0-255)
    uint16_t id;          // LoRa ID (0-255)
    uint8_t netID;       // LoRa NetID (0-255)
    uint16_t plan[NUM_COUNTERS];    // 0-9999
    uint16_t result[NUM_COUNTERS];  // 0-9999
    CounterConfig counters[NUM_COUNTERS];
    char name[NUM_COUNTERS][16]; // Counter names
    uint8_t funcPin;                // function button pin
    uint8_t LoRaPin[5]; // LoRa pins: M0, M1, AUX, RX, TX
};




// Counter state
struct CounterState {
    uint32_t value[NUM_COUNTERS];
    uint32_t lastTriggerTime[NUM_COUNTERS];
};

// Global counter state
extern CounterState g_counterState;

void ALC_setup(AsyncWebServer &server);
void ALC_loop();
// Function to save project configuration
bool saveProjectConfig(const ProjectConfig &cfg);
// Function to load project configuration
bool loadProjectConfig(ProjectConfig &cfg);
// Function to setup web server and routes

// Function to handle counter input (call in loop or interrupt)
void handleCounters();

// Function to handle func button (call in loop)
void handleFuncButton();

// Function to monitor system (RAM/CPU/Temp)
String getSystemStatus();

// Function to set plan/result from web
void setPlan(uint8_t idx, uint16_t val);
void setResult(uint8_t idx, uint16_t val);

// Function to configure pins (call after loading config)
void configureCounterPins();

 uint16_t getPlan(uint8_t id);
 uint16_t getResult(uint8_t id);
 uint8_t getConnectQuality(uint8_t id);

 #endif // ALC_PROJECT_H