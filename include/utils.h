#include <WebServer.h>
#include <AutoConnect.h>
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "SPIFFS.h"
#include "WiFi.h"
#include "Button2.h"
#include "icons/logo.h"
#include "icons/battery_01.h"
#include "icons/battery_02.h"
#include "icons/battery_03.h"
#include "icons/battery_04.h"
#include "icons/disconnected.h"
#include "icons/candleIcon.h"
#include "icons/tableIcon.h"
#include "icons/pairIcon.h"
#include "icons/hourglassIcon.h"
#include "icons/errorIcon.h"
#include "icons/wifiOff.h"
#include <Pangodream_18650_CL.h>
/* Use 'processing' to generate the fonts */
#include "fonts/NotoSansBold15.h"
#include "fonts/NotoSans15.h"
#include "fonts/UbuntuBold24.h"
#include "fonts/ArialBold15.h"
#include "fonts/ArialBold24.h"
#include "fonts/Arial15.h"
#include "fonts/ArialBold46.h"

void buttonTask(void * params);
void showWifiDisconnected();
String initialize2(AutoConnectAux& aux, PageArgument& args);
bool startCP(IPAddress ip);
void connectClient();
void rootPage();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void deleteAllCredentials(void);
void setDefaultValues();
String formatStringToFloat(String s, short precision);
String formatStringPercChange(String s);
String httpGETRequest(const char* serverName);
bool checkCoin(String testSymbol, short c);
void saveSettings(void);
boolean isValidNumber(String str);
void showInvalidParams();
void writeStringToEEPROM(int addrOffset, const String &strToWrite);
String readStringFromEEPROM(int addrOffset);
void readCoinConfig();
void button_init();
int32_t rgbToInt(short r, short g, short b);
void espDelay(int ms);
void showCandlesLoading();
void buildCandles();
void showCandleFooter();
void showLoadingCandleFooter();
void showTicker();
void showCmcLoading();
void showCmc();
void showLoadingCmcHeader();
void showSymbol();
bool detectAP(void);
void readBatteryLevel();

#define EEPROM_SIZE 512
#define NOTO_SANS_BOLD_15 NotoSansBold15
#define NOTO_SANS_15 NotoSans15
#define ARIAL_BOLD_24 ArialBold24
#define ARIAL_BOLD_15 ArialBold15
#define ARIAL_15 Arial15
#define TICKER_FONT ArialBold46
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0
#define TFT_BL              4  // Display backlight control pin
#define MIN_USB_VOL 4.75
#define ADC_PIN 34
#define CONV_FACTOR 1.8
#define READS 20

Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);
