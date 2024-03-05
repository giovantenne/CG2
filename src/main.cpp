// 240x135
#include "utils.h"

static const String zticker_version = "v2.34";
static const String binanceStreamDomain = "data-stream.binance.com";
static const String binanceApiBaseUrl = "https://data-api.binance.vision";
static const String coingekoApiBaseUrl = "https://api.coingecko.com";

String initialize2(AutoConnectAux&, PageArgument&);
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite class needs to be invoked
TaskHandle_t Task1;
TaskHandle_t Task2;
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
int btn1Click = false;
int btn2Click = false;
WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;
WebSocketsClient client;
String value;
String oldValue;
String percChange;
String valChange;
String symbol;
String pairs[3];
short precisions[3];
short selectedSymbol;
String dailyHigh;
String dailyLow;
String receivedSymbol;
short separatorPosition;
short coinPrecision;
short brightness;
bool updateDisplay;
bool hideWsDisconnected = false;
short screen = 0;
String candleInterval[] = { "1m", "5m", "30m", "1h", "6h", "12h", "1d", "1w", "1M" };
short selectedCandleInterval = 3;
short candlesHeight=119;
short cmcPage=0;
bool looped;
bool cmcLoading = false;
int batteryLevel;
bool charging = false;
// Setting PWM properties, do not change this!
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
bool comingFromCP;
unsigned long tmDetection;
const unsigned long scanInterval = 120 * 1000;
unsigned long timedTaskmDetection;
const unsigned long timedTaskInterval = 30 * 1000;
unsigned long wsTimedTaskmDetection;
const unsigned long wsTimedTaskInterval = 100;
ACText(caption1, "Do you want to perform a factory reset?");
ACSubmit(save1, "Yes, reset the device", "/delconnexecute");
AutoConnectAux aux1("/delconn", "Reset", true, {caption1, save1});
AutoConnectAux aux1Execute("/delconnexecute", "Wifi reset", false);
ACText(captionPair0, "<strong>Display</strong>");
ACText(captionPair1, "<hr><strong>Pair n°1</strong>");
ACText(captionPair2, "<hr><strong>Pair n°2</strong>");
ACText(captionPair3, "<hr><strong>Pair n°3</strong>");
ACText(captionPair4, "<hr>");
ACInput(inputBrightness, "", "Brightness (0-100)", "", "100");
ACInput(pair1, "", "Binance symbol (eg. BTCUSDT)", "", "BTCUSDT");
ACInput(pair2, "", "Binance symbol (eg. ETHBTC)", "", "ETHBTC");
ACInput(pair3, "", "Binance symbol (eg. XRPUSDT)", "", "XRPUSDT");
ACText(credits, "<hr>Follow me on Twitter: <a href='https://twitter.com/CryptoGadgetsIT'>@CryptoGadgetsIT</a>");
ACSubmit(save2, "Save", "/setupexecute");
AutoConnectAux aux2("/setup", "Settings", true, {captionPair0, inputBrightness, captionPair1, pair1, captionPair2, pair2, captionPair3, pair3, captionPair4, save2, credits});
AutoConnectAux aux3("/setupexecute", "", false);

void setup()
{
  Serial.begin(114600);
  Serial.println("Start");
  EEPROM.begin(EEPROM_SIZE);
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);
  tft.begin();
  tft.setRotation(1);
  spr.setSwapBytes(true);
  spr.setColorDepth(16);
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 120);
  readCoinConfig();
  Serial.println(esp_random());
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.fillScreen(TFT_BLACK);
  spr.pushImage(46, 0,  135, 135, logo);
  spr.setTextDatum(MR_DATUM);
  spr.drawString(zticker_version, 220, 125);
  spr.pushSprite(0,0);
  spr.unloadFont();
  espDelay(2000);
  button_init();
  Server.on("/", rootPage);
  Server.on("/delconnexecute", deleteAllCredentials);
  Server.on("/setupexecute", saveSettings);
  Config.autoReconnect = true;
  Config.reconnectInterval = 1;
  Config.ota = AC_OTA_BUILTIN;
  Config.title = "CryptoGadgets " + zticker_version;
  Config.apid = "ToTheMoon";
  Config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_UPDATE;
  Config.boundaryOffset = EEPROM_SIZE;
  Portal.config(Config);
  Portal.onDetect(startCP);
  Portal.whileCaptivePortal(detectAP);
  aux2.on(initialize2, AC_EXIT_AHEAD);
  Portal.join({aux2, aux1, aux1Execute, aux3});
  value = "";
  oldValue = "";

  if(digitalRead(BUTTON_1) == 0){
    deleteAllCredentials();
  }

  if (Portal.begin()) {
    Serial.println("Connected to WiFi");
    if (WiFi.localIP().toString() != "0.0.0.0") {
      spr.createSprite(240, 135);
      spr.loadFont(ARIAL_BOLD_24);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
      spr.drawString("WiFi connected", 120, 40);
      spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
      spr.drawString(WiFi.localIP().toString(), 120, 80);
      spr.pushSprite(0,0);
      spr.unloadFont();
      Serial.println("Connection Opened");
      delay(3000);
      readBatteryLevel();
      short convertedBrightness = brightness*255/100;
      Serial.println(convertedBrightness);
      ledcWrite(pwmLedChannelTFT, convertedBrightness);
      if (WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
        WiFi.enableAP(false);
      }
    }
    if(!checkCoin(symbol, 0)){
      if(symbol != "btcusdt"){
        setDefaultValues();
      }
    }
    if(screen == 1) {
      showCandlesLoading();
      buildCandles();
    } else if(screen == 2) {
      cmcPage = 0;
      showCmcLoading();
      showCmc();
    }
    looped = false;
    connectClient();
  }

  xTaskCreatePinnedToCore(
      buttonTask, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
}

void loop()
{
  while(true){
    Portal.handleClient();
    if (WiFi.status() != WL_CONNECTED) {
      showWifiDisconnected();
      delay(2000);
    } else {
      if (btn1Click){
        btn1Click = false;
        Serial.println("btn1Click");
        if(screen == 0){
          selectedSymbol = (selectedSymbol + 1) % 3;
          EEPROM.write(sizeof(brightness), selectedSymbol);
          EEPROM.commit();
          symbol = pairs[selectedSymbol];
          coinPrecision = precisions[selectedSymbol];
          showSymbol();
          hideWsDisconnected = true;
          connectClient();
          hideWsDisconnected = false;
        }else if(screen == 1){
          selectedCandleInterval = (selectedCandleInterval + 1) % 9;
          EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0])  + sizeof(pairs[1])  + sizeof(pairs[2]) + (sizeof(coinPrecision)*3), selectedCandleInterval);
          EEPROM.commit();
          showLoadingCandleFooter();
          buildCandles();
        } else if (screen == 2){
          cmcPage = (cmcPage + 1) % 4;
          showLoadingCmcHeader();
          showCmc();
        }
      }
      if (btn2Click){
        btn2Click = false;
        Serial.println("btn2Click; screen:" + screen);
        if(screen == 0) {
          screen = 1;
          showCandlesLoading();
          buildCandles();
        } else if(screen == 1) {
          screen = 2;
          cmcPage = 0;
          showCmcLoading();
          cmcLoading = true;
          showCmc();
          cmcLoading = false;
        } else {
          screen = 0;
          showSymbol();
          hideWsDisconnected = true;
          connectClient();
          hideWsDisconnected = false;
        }
        EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0])  + sizeof(pairs[1])  + sizeof(pairs[2]) + (sizeof(coinPrecision)*3) + sizeof(selectedCandleInterval), screen);
        EEPROM.commit();
      }
      if (millis() - timedTaskmDetection > timedTaskInterval) {
        readBatteryLevel();
        timedTaskmDetection = millis();
        Serial.println("Execute timed task...");
        Serial.println(millis());
        if (screen == 1) {
          Serial.println("Build candles...");
          buildCandles();
        } else if (screen == 2 && !cmcLoading) {
          Serial.println("Refresh CMC");
          showCmc();
        }
      }
      looped = true;
      client.loop();
      looped = false;
    }
    /* Serial.println(ESP.getFreeHeap()); */
  }
}

void buttonTask(void * params)
{
  while(true) {
    if (WiFi.status() == WL_CONNECTED) {
      btn1.loop();
      btn2.loop();
    }
  }
}

void showWifiDisconnected(){
  Serial.println("Reconnect wifi...");
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, wifiOff);
  spr.loadFont(ARIAL_BOLD_24);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  if(comingFromCP)
    spr.drawString("Connecting to WiFi", 120, 115);
  else
    spr.drawString("WiFi disconnected", 120, 115);
  spr.pushSprite(0,0);
  spr.unloadFont();
}

String initialize2(AutoConnectAux& aux, PageArgument& args) {
  String tmpSymbol;
  tmpSymbol=pairs[0];
  tmpSymbol.toUpperCase();
  pair1.value = tmpSymbol;
  tmpSymbol=pairs[1];
  tmpSymbol.toUpperCase();
  pair2.value = tmpSymbol;
  tmpSymbol=pairs[2];
  tmpSymbol.toUpperCase();
  pair3.value = tmpSymbol;
  inputBrightness.value=brightness;
  return String();
}

bool startCP(IPAddress ip){
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Please connect to WiFi", 120, 16);
  spr.setTextDatum(MR_DATUM);
  spr.drawString("SSID:", 115, 16 + 25);
  spr.drawString("Password:", 115, 16 + 50);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.drawString("ToTheMoon", 125, 16 + 25);
  spr.drawString("12345678", 125, 16 + 50);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("and browse to", 120, 16 + 75);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.drawString("http://172.217.28.1", 120, 16 + 100);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  return true;
}

void connectClient(){
  client.disconnect();
  client.beginSSL(binanceStreamDomain, 9443, "/ws/" + symbol + "@ticker/" + symbol + "@aggTrade");
  client.onEvent(webSocketEvent);
}

void rootPage() {
  Server.sendHeader("Location", "/_ac");
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      if(!hideWsDisconnected){
        spr.createSprite(30, 30);
        spr.pushImage(0, 0,  30, 24, disconnectedIcon);
        spr.pushSprite(0,0);
        Serial.println("[WSc] Disconnected!\n");
        delay(2000);
      }
      connectClient();
      break;
    case WStype_CONNECTED:
      {
        Serial.println("[WSc] Connected");
      }
      break;
    case WStype_TEXT:
      if (millis() - wsTimedTaskmDetection > wsTimedTaskInterval) {
        wsTimedTaskmDetection = millis();
        if (looped && !btn1Click && !btn2Click) {
          looped = false;
          String event;
          updateDisplay = false;
          StaticJsonDocument<1024> myObject;
          deserializeJson(myObject, payload);
          event = myObject["e"].as<String>();
          if(event == "24hrTicker"){
            percChange = myObject["P"].as<String>();
            valChange = myObject["p"].as<String>();
            dailyHigh = myObject["h"].as<String>();
            dailyLow = myObject["l"].as<String>();
            updateDisplay = true;
          } else if (event == "aggTrade") {
            oldValue = value;
            value = myObject["p"].as<String>();
            receivedSymbol = myObject["s"].as<String>();
            updateDisplay = true;
          }
          if(updateDisplay && value != ""){
            if(screen == 0){
              showTicker();
            } else if(screen == 1){
              showCandleFooter();
            }
          }
        }
      }
      break;
    case WStype_BIN:
      Serial.println("[WSc] get binary length: " + length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void deleteAllCredentials(void) {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();
  Serial.println("Delete all credentials");
  while (ent--) {
    credential.load((int8_t)0, &config);
    credential.del((const char*)&config.ssid[0]);
  }
  setDefaultValues();
  char content[] = "Factory reset; Device is restarting...";
  Server.send(200, "text/plain", content);
  delay(3000);
  WiFi.disconnect(false, true);
  delay(3000);
  ESP.restart();
}

void setDefaultValues() {
  Serial.println("Setting up default values!");
  spr.createSprite(240, 135);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Setting up", 120, 50);
  spr.drawString("default values", 120, 80);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  symbol = "btcusdt";
  coinPrecision = 2;
  selectedSymbol = 0;
  brightness = 75;
  EEPROM.write(0, brightness);
  EEPROM.write(sizeof(brightness), selectedSymbol);
  pairs[0] = "btcusdt";
  precisions[0] = 2;
  pairs[1] = "ethusdt";
  precisions[1] = 2;
  pairs[2] = "xrpusdt";
  precisions[2] = 4;
  writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol), pairs[0]);
  EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]), precisions[0]);
  writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(coinPrecision), pairs[1]);
  EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(coinPrecision) + sizeof(pairs[1]), precisions[1]);
  writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(coinPrecision) + sizeof(pairs[1]) + sizeof(coinPrecision), pairs[2]);
  EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(coinPrecision) + sizeof(pairs[1]) + sizeof(coinPrecision) + sizeof(pairs[2]), precisions[2]);
  selectedCandleInterval = 3;
  EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(coinPrecision) + sizeof(pairs[1]) + sizeof(coinPrecision) + sizeof(pairs[2]) + sizeof(coinPrecision), selectedCandleInterval);
  screen = 0;
  EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0])  + sizeof(pairs[1])  + sizeof(pairs[2]) + (sizeof(coinPrecision)*3) + sizeof(selectedCandleInterval), screen);
  EEPROM.commit();
  delay(3000);
}

String formatStringToFloat(String s, short precision){
  separatorPosition = s.indexOf('.');
  if(precision == 0) {
    return s.substring(0, separatorPosition);
  } else {
    return s.substring(0, separatorPosition) + "." + s.substring(separatorPosition + 1, separatorPosition + 1 + precision);
  }
}

String formatStringPercChange(String s){
  String tmp;
  tmp = formatStringToFloat(s, 2);
  if(tmp.indexOf("-") == -1)
    tmp = "+" + tmp;
  return tmp;
}

String httpGETRequest(const char* serverName) {
  Serial.println(serverName);
  HTTPClient http;
  http.setReuse(false);
  http.begin(serverName);
  short httpResponseCode = http.GET();
  String payload;
  if (httpResponseCode > 0) {
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println(http.getString());
  }
  http.end();
  return payload;
}

bool checkCoin(String testSymbol, short c) {
  testSymbol.toUpperCase();
  String serverPath = binanceApiBaseUrl + "/api/v3/exchangeInfo?symbol=" + testSymbol;
  String jsonBuffer = httpGETRequest(serverPath.c_str());
  Serial.println(jsonBuffer);
  DynamicJsonDocument myResponseObject(2048);
  deserializeJson(myResponseObject, jsonBuffer);
  if(myResponseObject["code"].is<int>() || jsonBuffer == ""){
    return false;
  }
  String minPrice = myResponseObject["symbols"][0]["filters"][0]["minPrice"];
  precisions[c] = (minPrice.substring(minPrice.indexOf('.') + 1)).indexOf('1') + 1;
  Serial.println(precisions[c]);
  return true;
}

void saveSettings(void){
  updateDisplay = false;
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, hourglassIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("Checking values...", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  short newBrightness = Server.arg("inputBrightness").toInt();
  String c0 = Server.arg("pair1");
  String c1 = Server.arg("pair2");
  String c2 = Server.arg("pair3");
  c0.toLowerCase();
  c1.toLowerCase();
  c2.toLowerCase();
  if(isValidNumber(Server.arg("inputBrightness")) && newBrightness >= 0 && newBrightness <= 100){
    if (checkCoin(c0, 0) && checkCoin(c1, 1) && checkCoin(c2, 2)){
      pairs[0]=c0;
      pairs[1]=c1;
      pairs[2]=c2;
      selectedSymbol = 0;
      symbol=pairs[selectedSymbol];
      coinPrecision=precisions[selectedSymbol];
      brightness=newBrightness;
      Serial.println("WRITE TO EEPROM");
      EEPROM.write(0, brightness);
      EEPROM.write(sizeof(brightness), selectedSymbol);
      writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol), pairs[0]);
      EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]), precisions[0]);
      writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]), pairs[1]);
      EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]), precisions[1]);
      writeStringToEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]) + sizeof(precisions[1]), pairs[2]);
      EEPROM.write(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]) + sizeof(precisions[1]) + sizeof(pairs[2]), precisions[2]);
      EEPROM.commit();
      short convertedBrightness = brightness*255/100;
      Serial.println(convertedBrightness);
      ledcWrite(pwmLedChannelTFT, convertedBrightness);
      value = "";
      hideWsDisconnected = true;
      connectClient();
      hideWsDisconnected = false;
      if(screen == 0){
        showSymbol();
      } else if(screen == 1) {
        buildCandles();
      } else if (screen == 2) {
        showCmc();
      }
      Server.sendHeader("Location", "/setup?valid=true");
    }else{
      showInvalidParams();
    }
  }else{
    showInvalidParams();
  }
  Server.sendHeader("Cache-Control", "no-cache");
  Server.send(301);
  updateDisplay = true;
}

boolean isValidNumber(String str){
  for(byte i=0;i<str.length();i++)
  {
    if(isDigit(str.charAt(i))) return true;
  }
  return false;
}

void showInvalidParams(){
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_15);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(20, 50, 30, 30, errorIcon);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.drawString("Invalid parameters", 65, 67);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  Server.sendHeader("Location", "/setup?valid=false&msg=invalidParameters");
  delay(2000);
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

void readCoinConfig(){
  Serial.println("--- READ FROM EEPROM ---");
  brightness = EEPROM.read(0);
  selectedSymbol = EEPROM.read(sizeof(brightness));
  pairs[0]=readStringFromEEPROM(sizeof(brightness) + sizeof(selectedSymbol));
  precisions[0]=EEPROM.read(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]));
  pairs[1]=readStringFromEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]));
  precisions[1]=EEPROM.read(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]));
  pairs[2]=readStringFromEEPROM(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]) + sizeof(precisions[1]));
  precisions[2]=EEPROM.read(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(precisions[0]) + sizeof(pairs[1]) + sizeof(precisions[1])+ sizeof(pairs[2]));
  screen = EEPROM.read(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0])  + sizeof(pairs[1])  + sizeof(pairs[2]) + (sizeof(coinPrecision)*3) + sizeof(selectedCandleInterval));
  if(precisions[0] < 0 || precisions[0] > 8 || precisions[1] < 0 || precisions[1] > 8 || precisions[2] < 0 || precisions[2] > 8 || selectedSymbol < 0 || selectedSymbol >= 3){
    setDefaultValues();
  }
  symbol = pairs[selectedSymbol];
  coinPrecision = precisions[selectedSymbol];
  selectedCandleInterval = EEPROM.read(sizeof(brightness) + sizeof(selectedSymbol) + sizeof(pairs[0]) + sizeof(pairs[1]) + sizeof(pairs[2]) + (sizeof(coinPrecision) * 3 ));
  if(selectedCandleInterval<0 || selectedCandleInterval >=9)
    selectedCandleInterval = 3;
}

void button_init()
{
  btn2.setLongClickTime(2000);
  btn2.setLongClickDetectedHandler([](Button2 & b) {
      Serial.println("Going to sleep...");
      int r = digitalRead(TFT_BL);
      digitalWrite(TFT_BL, !r);
      tft.writecommand(TFT_DISPOFF);
      tft.writecommand(TFT_SLPIN);
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
      esp_deep_sleep_start();
      });

  btn1.setPressedHandler([](Button2 & b) {
      btn1Click = true;
      btn2Click = false;
      Serial.println("Button 1 pressed..");
      });

  btn2.setClickHandler([](Button2 & b) {
      Serial.println("Button 2 pressed..");
      btn1Click = false;
      btn2Click = true;
      });
}

int32_t rgbToInt(short r, short g, short b) {
  return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void showCandlesLoading(){
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, candleIcon);
  spr.loadFont(ARIAL_BOLD_24);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("GRAPH", 120, 115);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  delay(1000);
}

void buildCandles(){
  float cOpen, cHigh, cLow, cClose;
  short yOpen, yHigh, yLow, yClose, height;
  uint16_t color;
  Serial.println("Getting candles...");
  String s = symbol;
  s.toUpperCase();
  String serverPath = binanceApiBaseUrl + "/api/v3/klines?symbol=" + s + "&interval=" + candleInterval[selectedCandleInterval] + "&limit=40";
  String jsonBuffer = httpGETRequest(serverPath.c_str());
  if(jsonBuffer == "{}"){
    Serial.println("Error retrieving candles!");
  }else{
    /* Serial.println(jsonBuffer); */
    DynamicJsonDocument myJson(16384);
    deserializeJson(myJson, jsonBuffer);
    JsonArray candlesArray = myJson.as<JsonArray>();
    Serial.println(ESP.getFreeHeap());
    if(candlesArray.size() > 0){
      float rangeMin = atof(candlesArray[0][3]);
      float rangeMax = atof(candlesArray[0][2]);
      for(JsonVariant v : candlesArray) {
        if(atof(v[3]) < rangeMin)
          rangeMin = atof(v[3]);
        if(atof(v[2]) > rangeMax)
          rangeMax = atof(v[2]);
      }
      spr.createSprite(240, candlesHeight + 1);
      spr.fillSprite(TFT_BLACK);
      int i = 0;
      for(JsonVariant v : candlesArray) {
        short x = (i*6) + 2;
        cOpen = atof(v[1]);
        cHigh = atof(v[2]);
        cLow = atof(v[3]);
        cClose = atof(v[4]);
        if(cOpen > cClose){
          color = TFT_RED;
          yHigh = candlesHeight - ((cOpen - rangeMin) * candlesHeight / (rangeMax-rangeMin));
          yLow = candlesHeight - ((cClose - rangeMin) * candlesHeight / (rangeMax-rangeMin));
        } else {
          color = TFT_GREEN;
          yHigh = candlesHeight - ((cClose - rangeMin) * candlesHeight / (rangeMax-rangeMin));
          yLow = candlesHeight - ((cOpen - rangeMin) * candlesHeight / (rangeMax-rangeMin));
        }
        spr.drawRect(x-2, yHigh, 5, yLow - yHigh, color);
        spr.drawRect(x-1, yHigh, 3, yLow - yHigh, color);
        yLow = candlesHeight - ((cLow - rangeMin) * candlesHeight / (rangeMax-rangeMin));
        yHigh = candlesHeight - ((cHigh - rangeMin) * candlesHeight / (rangeMax-rangeMin));
        spr.drawLine(x, yHigh, x, yLow, color);
        i++;
      }
      spr.loadFont(NOTO_SANS_BOLD_15);
      spr.setTextColor(TFT_WHITE, TFT_BLACK);
      bool maxPrinted = false;
      bool minPrinted = false;
      i = 0;
      for(JsonVariant v : candlesArray) {
        short x = (i*6) + 2;
        cHigh = atof(v[2]);
        cLow = atof(v[3]);
        spr.unloadFont();
        spr.loadFont(NOTO_SANS_15);
        if(cHigh == rangeMax && ! maxPrinted){
          if(i < 20 ) {
            spr.setTextDatum(ML_DATUM);
            spr.drawString(formatStringToFloat((const char*)v[2], coinPrecision), x + 5, 1);
          } else{
            spr.setTextDatum(MR_DATUM);
            spr.drawString(formatStringToFloat((const char*)v[2], coinPrecision), x - 5, 1);
          }
          maxPrinted == true;
        }
        if(cLow == rangeMin && ! minPrinted){
          if(i < 20 ) {
            spr.setTextDatum(ML_DATUM);
            spr.drawString(formatStringToFloat((const char*)v[3], coinPrecision), x + 5, candlesHeight - 8);
          } else{
            spr.setTextDatum(MR_DATUM);
            spr.drawString(formatStringToFloat((const char*)v[3], coinPrecision), x - 5, candlesHeight - 8);
          }
          minPrinted == true;
        }
        i++;
      }
      spr.unloadFont();
      spr.drawLine(0, candlesHeight, 239, candlesHeight, TFT_DARKGREY);
      spr.pushSprite(0, 0);
      showCandleFooter();
    }
  }
}

void showCandleFooter(){
  spr.createSprite(240, 135 - candlesHeight - 3);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.loadFont(NOTO_SANS_15);
  spr.setTextDatum(ML_DATUM);
  String s = symbol;
  s.toUpperCase();
  spr.drawString(s, 5, 4);
  spr.setTextDatum(MC_DATUM);
  spr.unloadFont();
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.drawString(candleInterval[selectedCandleInterval], 120, 4);
  spr.setTextDatum(MR_DATUM);
  if(oldValue.toFloat() > value.toFloat())
    spr.setTextColor(TFT_RED, TFT_BLACK);
  else
    spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  String newValue = formatStringToFloat(value, coinPrecision);
  /* Serial.println(newValue); */
  spr.setTextDatum(ML_DATUM);
  short fontWidth = 8;
  spr.drawString(newValue, (235-(fontWidth*newValue.length())) , 4);

  spr.unloadFont();
  spr.pushSprite(0, candlesHeight + 2);
}

void showLoadingCandleFooter(){
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.createSprite(240, 135 - candlesHeight);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.drawString("Loading...", 5, 2);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(candleInterval[selectedCandleInterval], 120, 2);
  spr.setTextDatum(MR_DATUM);
  if(oldValue.toFloat() > value.toFloat())
    spr.setTextColor(TFT_RED, TFT_BLACK);
  else
    spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  spr.drawString(formatStringToFloat(value, coinPrecision), 235, 2);
  spr.unloadFont();
  spr.pushSprite(0, candlesHeight + 1);
}

void showTicker() {
  String change = formatStringPercChange(percChange);
  spr.createSprite(240, 135);
  spr.fillSprite(TFT_BLACK);
  spr.drawLine(0, 90, 240, 90, TFT_DARKGREY);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.drawString("24h:", 5, 105);
  spr.drawString("Max: ", 120, 105);
  spr.drawString("Min: ", 120, 125);
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  spr.drawString(formatStringToFloat(dailyHigh, coinPrecision), 235, 105);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.drawString(formatStringToFloat(dailyLow, coinPrecision), 235, 125);
  spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
  spr.setTextDatum(MC_DATUM);
  String s = symbol;
  s.toUpperCase();
  spr.drawString(s, 120, 5);
  if(false && !charging){
    if(batteryLevel >=80){
      spr.pushImage(208, 0,  29, 14, battery_04);
    }else if(batteryLevel < 80 && batteryLevel >= 50 ){
      spr.pushImage(208, 0,  29, 14, battery_03);
    }else if(batteryLevel < 50 && batteryLevel >= 20 ){
      spr.pushImage(208, 0,  29, 14, battery_02);
    }else if(batteryLevel < 20 ){
      spr.pushImage(208, 0,  29, 14, battery_01);
    }
  }

  spr.unloadFont();
  if(change[0] == '+')
    spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  else
    spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.drawString(formatStringToFloat(valChange, coinPrecision), 45, 125);
  spr.drawString(change + "%", 45, 105);
  spr.unloadFont();
  spr.loadFont(TICKER_FONT);
  if(oldValue.toFloat() > value.toFloat())
    spr.setTextColor(TFT_RED, TFT_BLACK);
  else
    spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  String newValue = formatStringToFloat(value, coinPrecision);
  spr.setTextDatum(ML_DATUM);
  short fontWidth = 25;
  spr.drawString(newValue, (240-(fontWidth*newValue.length())) / 2, 55);
  spr.unloadFont();
  spr.pushSprite(0, 0);
}

void showCmcLoading(){
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, tableIcon);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("STATS", 120, 115);
  spr.pushSprite(0, 0);
  spr.unloadFont();
  delay(1000);
}

void showCmc(){
  spr.createSprite(240, 135);
  spr.fillSprite(TFT_BLACK);
  String perPage= "6";
  String serverPath = coingekoApiBaseUrl + "/api/v3/coins/markets.json?vs_currency=usd&order=market_cap_desc&per_page=" + perPage + "&page=" + (cmcPage + 1) + "&sparkline=false&price_change_percentage=1h%2C24h%2C7d";
  DynamicJsonDocument myResponse(8192);
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(serverPath);
  http.GET();
  deserializeJson(myResponse, http.getStream());
  http.end();
  short headerHeigh = 17;
  short lineHeight = 20;
  JsonArray myResponseObject = myResponse.as<JsonArray>();
  int i = 0;
  for(JsonVariant v : myResponseObject) {
    spr.setTextDatum(ML_DATUM);
    String s = (const char*) v["symbol"];
    String p = String(i+1+(perPage.toInt()*(cmcPage)));
    String cap = v["market_cap"];
    cap.remove(cap.length() - 6);
    String perc1 = String((double) myResponseObject[i]["price_change_percentage_1h_in_currency"]);
    String perc2 = String((double) myResponseObject[i]["price_change_percentage_24h_in_currency"]);
    String perc3 = String((double) myResponseObject[i]["price_change_percentage_7d_in_currency"]);
    perc1 = formatStringToFloat(perc1, 2);
    perc2 = formatStringToFloat(perc2, 2);
    perc3 = formatStringToFloat(perc3, 2);
    s.toUpperCase();
    spr.setTextColor(TFT_YELLOW, TFT_BLACK);
    spr.drawLine(0, headerHeigh + i*lineHeight, 240, headerHeigh + i*lineHeight, TFT_DARKGREY);
    spr.loadFont(ARIAL_BOLD_15);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.drawString(p, 1, headerHeigh + (i*lineHeight) + 11);
    spr.unloadFont();
    spr.setTextDatum(ML_DATUM);
    spr.loadFont(NOTO_SANS_BOLD_15);
    spr.setTextColor(TFT_YELLOW, TFT_BLACK);
    spr.drawString(s, 17, headerHeigh + (i*lineHeight) + 11);
    spr.unloadFont();
    spr.loadFont(ARIAL_15);
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString(cap, 99, headerHeigh + (i*lineHeight) + 11);
    spr.unloadFont();
    spr.loadFont(ARIAL_BOLD_15);
    if(perc1.toFloat() < 0)
      spr.setTextColor(TFT_RED, TFT_BLACK);
    else
      spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    spr.drawString(perc1, 146, headerHeigh + (i*lineHeight) + 11);
    if(perc2.toFloat() < 0)
      spr.setTextColor(TFT_RED, TFT_BLACK);
    else
      spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    spr.drawString(perc2, 193, headerHeigh + (i*lineHeight) + 11);
    if(perc3.toFloat() < 0)
      spr.setTextColor(TFT_RED, TFT_BLACK);
    else
      spr.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    spr.drawString(perc3, 240, headerHeigh + (i*lineHeight) + 11);
    spr.unloadFont();
    i++;
  }
  spr.loadFont(ARIAL_BOLD_15);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString("1h%", 146, 2);
  spr.drawString("24h%", 193, 2);
  spr.drawString("7d%", 240, 2);
  spr.drawString("Cap(M)", 99, 2);
  spr.unloadFont();
  spr.drawLine(0, headerHeigh, 240, headerHeigh, TFT_WHITE);
  spr.pushSprite(0, 0);
}

void showLoadingCmcHeader(){
  spr.loadFont(NOTO_SANS_BOLD_15);
  spr.createSprite(120, 16);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextDatum(ML_DATUM);
  spr.drawString("Loading...         ", 5, 1);
  spr.setTextDatum(MC_DATUM);
  spr.pushSprite(0, 0);
}

void showSymbol(){
  String newSymbol = symbol;
  newSymbol.toUpperCase();
  spr.createSprite(240, 135);
  spr.loadFont(ARIAL_BOLD_24);
  spr.fillSprite(TFT_BLACK);
  spr.pushImage(80, 10, 80, 80, pairIcon);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.drawString(newSymbol, 120, 115);
  spr.pushSprite(0, 0);
  spr.unloadFont();
}

bool detectAP(void) {
  btn2.loop();
  comingFromCP = true;
  int16_t  ns = WiFi.scanComplete();
  if (ns == WIFI_SCAN_RUNNING) {
  } else if (ns == WIFI_SCAN_FAILED) {
    if (millis() - tmDetection > scanInterval) {
      WiFi.disconnect();
      WiFi.scanNetworks(true, true, false);
    }
  } else {
    Serial.printf("scanNetworks:%d\n", ns);
    int16_t  scanResult = 0;
    while (scanResult < ns) {
      AutoConnectCredential cred;
      station_config_t  staConfig;
      if (cred.load(WiFi.SSID(scanResult++).c_str(), &staConfig) >= 0) {
        Serial.printf("AP %s ready\n", (char*)staConfig.ssid);
        WiFi.scanDelete();
        return false;
      }
    }
    Serial.println("No found known AP");
    WiFi.scanDelete();
    tmDetection = millis();
  }
  return true;
}

void readBatteryLevel(){
  Serial.println(BL.getBatteryVolts());
  if(BL.getBatteryVolts() >= MIN_USB_VOL){
    Serial.println("Charging...");
    charging = true;
  }else{
    charging = false;
    batteryLevel = BL.getBatteryChargeLevel();
  }
}
