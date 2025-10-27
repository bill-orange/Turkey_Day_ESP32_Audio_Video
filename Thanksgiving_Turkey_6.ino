/* Thanksgiving Turkey using OpenAI TTS and ChatGPT

  Must run on ESP32S3 with PSRAM 

   William E Webb (c) released under MIT license blease observe all included licenses
   
   This project uses TFT_eSPI by Bodmer The original starting point for this library was the
   Adafruit_ILI9341 library in January 2015. MIT License 
   
   This project uses EST32-audioI2S by schreibfaul1 under GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007

   This project uses ChatGPTuino by programming-electronics-acadamy 
   GNU Lesser General Public License v2.1

   This project uses button-debounce by Aaron Kimball Copyright 2022
   under BSD 3-Clause "New" or "Revised" License

   Soundclip of axe fall: 
    100929_monster_without_arni.wav by Gzmo -- https://freesound.org/s/140256/ -- License: Creative Commons 0

   10/24/2025 First Tests with Graphics
   10/25/2025 Graphics works,TTS works, flow control testing
   10/26/2025 Flow control works now, adding updated payload construction and speechengine enhancements

   */

#include <WiFi.h>               // 📡 Wi-Fi connectivity for HTTP requests and cloud APIs
#include <HTTPClient.h>         // 🌐 HTTP client for making REST API calls (e.g., OpenAI TTS)
#include <AudioFileSource.h>    // 🎵 Base class for audio file sources (used by ESP8266Audio)
#include <AudioGeneratorMP3.h>  // 🔊 MP3 decoder for streaming and playback
#include <AudioOutputI2S.h>     // 🔈 I2S audio output driver (handles DAC or I2S peripheral)
#include <TFT_eSPI.h>           // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();      // For Display
#include "support_functions.h"  // For PNG
#include <ChatGPTuino.h>        // 🤖 ChatGPT integration for generating dynamic speech or quips
#include <debounce.h>           // Button debounce
#include "MemoryStream.h"       // 🧠 In-memory stream wrapper for audio playback from RAM/PSRAM
#include "secrets.h"            // 🔐 API keys and secrets 

SET_LOOP_TASK_STACK_SIZE(14 * 1024);  // needed to handle really long strings

#define USE_LINE_BUFFER  // Enable for faster rendering
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// Connect a button to some GPIO pin; digital pin 6 in this example.
static constexpr int PIN = 19;

const char* ssid = ssid_1;
const char* password = password_1;

const char* apiKey = chatGPT_APIKey;  // Replace with your actual key
const int TOKENS = 100;               // How lengthy a response you want, every token is about 3/4 a word
const int NUM_MESSAGES = 20;          // Another budget limit
//const char *model = "gpt-4o-mini";  // OpenAI Model being used
const char* model = "gpt-4o";              // OpenAI Model being used
ChatGPTuino chat{ TOKENS, NUM_MESSAGES };  // Will store and send your most recent messages (up to NUM_MESSAGES)
int nexToSay = 0;
int allReadySaid1 = 0;
int allReadySaid2 = 0;
int allReadySaid3 = 0;
int allReadySaid4 = 0;
int audioLock = 1;

unsigned long audioStartTime = 0;
bool isPlaying = false;

/*-------------------------------- Button Handler ----------------------------------------*/
static void buttonHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    Serial.println("Pushed button");
    //AxeFall();
  } else {
    // btnState == BTN_OPEN.
    Serial.println("Released button");
    allReadySaid2 = 0;
    allReadySaid3 = 0;
    nexToSay = 2;
    audioLock = 1;
  }
}

/*-------------------------------- END Button Handler ------------------------------------*/

static Button myButton(0, buttonHandler);

/*-------------------------------- Display Graphics ----------------------------------------*/
void showGraphic(String(image)) {
  uint32_t t = millis();

  setPngPosition(0, 0);
  String githubURL = GITHUBURL + image;
  const char* URLStr = githubURL.c_str();
  Serial.println(URLStr);
  load_png(URLStr);
  t = millis() - t;
  Serial.print(t);
  Serial.println(" ms to load URL");
}
/*--------------------------------------------------------------------------------------------*/

AudioGeneratorMP3* mp3;
AudioOutputI2S* out;
MemoryStream* file;
uint8_t* mp3Buffer = nullptr;
size_t mp3Size = 0;

/*-----------------------------------------SETUP---------------------------------------------*/

void setup() {

  Serial.begin(115200);
  delay(1000);
  pinMode(PIN, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("Initialize Chatgpt");
  chat.init(chatGPT_APIKey_txt, model);  // Initialize AI chat
  Serial.println("DONE-Initialize Chatgpt");
  tft.begin();
  tft.invertDisplay(1);
  tft.fillScreen(0);
  tft.setRotation(2);
  fileInfo();

  if (!psramInit()) {
    Serial.println("❌ PSRAM init failed");
    return;
  }
  Serial.printf("✅ PSRAM total: %d bytes\n", ESP.getPsramSize());
  Serial.printf("✅ PSRAM free: %d bytes\n", ESP.getFreePsram());
  
  out = new AudioOutputI2S();
  out->SetPinout(20, 6, 21);
  out->SetRate(24000);  // Try 24000 or 22050
  out->begin();
  
}

/*----------------------------------------------------------LOOP--------------------------------------------------------*/
void loop() {

  pollButtons();  // Poll your buttons every loop.
  unsigned long timer = millis();

  if (nexToSay == 0 && allReadySaid1 == 0 && audioLock == 1) {
    delay(100);
    showGraphic("Thanksgiving.png");
    speechEngine("Happy Thanksgiving if you are not a Turkey", "echo", 0.80, "Speak with a cheerful excessively upbeat voice.");  // Google TTS
    allReadySaid1 = 0;
    nexToSay = 1;
    audioLock = 0;
  }

  if (nexToSay == 1 && allReadySaid1 == 0 && audioLock == 1 && !isPlaying) {
    delay(100);
    showGraphic("Axe.png");
    speechEngine("Press the Axe for an unpleasant surprise for the Turkey", "echo", 1.00,"Speak in an ominious threatening tone.");  // ChatGPT TTS
    allReadySaid1 = 1;
    nexToSay = 5;
    audioLock = 0;
  }

  if (nexToSay == 2 && allReadySaid2 == 0 && audioLock == 1 && !isPlaying) {
    delay(100);
    showGraphic("Startled_Turkey.png");
    speechEngine(quip(), "shimmer", 1.00,"Speak in a tone filled with fear, dread and anxiety.");
    //speechEngine("Quip Placeholder");  // ChatGPT TTS
    allReadySaid2 = 1;
    nexToSay = 3;
    audioLock = 0;
  }

  if (nexToSay == 3 && allReadySaid3 == 0 && audioLock == 1 && !isPlaying) {
    delay(100);
    showGraphic("Falling_Axe.png");
    AxeFall();
    allReadySaid3 = 1;
    nexToSay = 4;
    audioLock = 0;
  }

  if (nexToSay == 4 && allReadySaid4 == 0 && audioLock == 1 && !isPlaying) {
    delay(2000);
    showGraphic("cooked_turkey.png");
    if (millis() % 3 == 0) {
      speechEngine("Here's your Turkey. Need another?", "echo", 1.00, "Speak in a happy tone refelecting gallows humor");  // Google TTS}
    } else {
      speechEngine("Your Turkey is ready! If more guests are coming, prepare another.", "echo", 1.00, "Speak in a happy tone refelecting gallows humor");  // Google TTS }
    }
    allReadySaid4 = 1;
    nexToSay = 5;
    audioLock = 0;
  }

  if (mp3 && mp3->isRunning()) {
    mp3->loop();
    if (file->isFinished()) {
      Serial.println("🔕 MP3 playback finished (stream exhausted)");
      mp3->stop();
      delete mp3;
      mp3 = nullptr;
      isPlaying = false;
      audioLock = 1;
    }
  }

}

/*---------------------------- File information  ------------------------------------------*/
void fileInfo() {  // uesful to figure our what software is running

  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);  // Print to TFT display, White color
  tft.setTextSize(1);
  tft.drawString("       Thanksgiving_Turkey", 8, 60);
  tft.drawString("       OpenAI Voice Test", 10, 70);
  tft.setTextSize(1);
  tft.drawString(__FILENAME__, 30, 110);
  tft.drawString(__DATE__, 35, 140);
  tft.drawString(__TIME__, 125, 140);
  delay(3000);
}

/*------------------------------Speech Engine---------------------------------------------*/

void speechEngine(char* phrase, char* voice, float speed, char* instructions ) {

  isPlaying = true;
  audioStartTime = millis();

  HTTPClient http;
  http.begin("https://api.openai.com/v1/audio/speech");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKey);

  String payload = "{"
                   "\"model\": \"gpt-4o-mini-tts\","
                   "\"input\": \""
                   + String(phrase) + "\","
                                      "\"voice\": \""
                   + String(voice) + "\","
                                     "\"speed\": "
                   + String(speed, 2) + ","
                                        "\"instructions\": \""
                   + String(instructions) + "\","
                                        "\"response_format\": \"mp3\""
                                        "}";

  Serial.println("📡 Sending POST to OpenAI TTS...");
  int httpCode = http.POST(payload);
  Serial.print("🌐 HTTP response code: ");
  Serial.println(httpCode);
  if (httpCode != 200) {
    Serial.println("❌ Request failed");
    Serial.println(http.getString());
    return;
  }

  String contentType = http.header("Content-Type");
  Serial.print("📦 Content-Type: ");
  Serial.println(contentType);

  WiFiClient* stream = http.getStreamPtr();
  const size_t chunkSize = 4096;
  size_t capacity = 32768;
  size_t offset = 0;

  mp3Buffer = (uint8_t*)heap_caps_malloc(capacity, MALLOC_CAP_SPIRAM);
  if (!mp3Buffer) {
    Serial.println("❌ Initial PSRAM allocation failed");
    return;
  }

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    if (stream->available()) {
      if (offset + chunkSize > capacity) {
        capacity *= 2;
        uint8_t* newBuffer = (uint8_t*)heap_caps_realloc(mp3Buffer, capacity, MALLOC_CAP_SPIRAM);
        if (!newBuffer) {
          Serial.println("❌ PSRAM reallocation failed");
          free(mp3Buffer);
          return;
        }
        mp3Buffer = newBuffer;
      }
      int len = stream->read(mp3Buffer + offset, chunkSize);
      if (len > 0) offset += len;
      startTime = millis();  // Reset timeout
    } else {
      delay(10);
    }
  }

  mp3Size = offset;
  Serial.printf("✅ Downloaded %d bytes to PSRAM\n", mp3Size);
  Serial.println();

  file = new MemoryStream(mp3Buffer, mp3Size);

  mp3 = new AudioGeneratorMP3();
  if (mp3->begin(file, out)) {
    Serial.println("🔊 Playback started");
  } else {
    Serial.println("❌ MP3 begin failed");
  }
}

/*--------------------------------------------AxeFall-------------------------------------------------*/

void AxeFall() {

  isPlaying = true;
  audioStartTime = millis();
  HTTPClient http;
  http.begin("https://raw.githubusercontent.com/bill-orange/Turkey_Day_ESP32_Audio_Video/master/Data/gzmo__1.mp3");
  Serial.println("📡 Sending GET request for MP3...");
  int httpCode = http.GET();
  Serial.print("🌐 HTTP response code: ");
  Serial.println(httpCode);
  if (httpCode != 200) {
    Serial.println("❌ Request failed");
    Serial.println(http.getString());
    return;
  }

  String contentType = http.header("Content-Type");
  Serial.print("📦 Content-Type: ");
  Serial.println(contentType);

  WiFiClient* stream = http.getStreamPtr();
  const size_t chunkSize = 4096;
  size_t capacity = 32768;
  size_t offset = 0;

  mp3Buffer = (uint8_t*)heap_caps_malloc(capacity, MALLOC_CAP_SPIRAM);
  if (!mp3Buffer) {
    Serial.println("❌ Initial PSRAM allocation failed");
    return;
  }

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    if (stream->available()) {
      if (offset + chunkSize > capacity) {
        capacity *= 2;
        uint8_t* newBuffer = (uint8_t*)heap_caps_realloc(mp3Buffer, capacity, MALLOC_CAP_SPIRAM);
        if (!newBuffer) {
          Serial.println("❌ PSRAM reallocation failed");
          free(mp3Buffer);
          return;
        }
        mp3Buffer = newBuffer;
      }
      int len = stream->read(mp3Buffer + offset, chunkSize);
      if (len > 0) offset += len;
      startTime = millis();  // Reset timeout
    } else {
      delay(10);
    }
  }

  mp3Size = offset;
  Serial.printf("✅ Downloaded %d bytes to PSRAM\n", mp3Size);

  Serial.println("🔍 First 64 bytes (hex):");
  for (int i = 0; i < 64 && i < mp3Size; i++) {
    Serial.printf("%02X ", mp3Buffer[i]);
    if ((i + 1) % 16 == 0) Serial.println();
  }

  Serial.println("\n🔍 First 64 bytes (ASCII):");
  for (int i = 0; i < 64 && i < mp3Size; i++) {
    char c = mp3Buffer[i];
    Serial.print((c >= 32 && c <= 126) ? c : '.');
  }
  Serial.println();

  file = new MemoryStream(mp3Buffer, mp3Size);

  mp3 = new AudioGeneratorMP3();
  if (mp3->begin(file, out)) {
    Serial.println("🔊 Playback started");
  } else {
    Serial.println("❌ MP3 begin failed");
  }
}

/*------------------------------------Button Debounce------------------------------------------*/

static void pollButtons() {
  // update() will call buttonHandler() if PIN transitions to a new state and stays there
  // for multiple reads over 25+ ms.
  myButton.update(digitalRead(PIN));
}

/*-------------------------------- Gets Prompt from Github -------------------------------------*/
String prompt() {  // Mostly from example
  uint32_t t = millis();
  HTTPClient http;
  http.setTimeout(120000);
  http.begin("https://raw.githubusercontent.com/bill-orange/Turkey_Day_ESP32_Audio_Video/master/Prompt.txt");

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP ERROR: %d\n", httpCode);
    showGraphic("error.png");
    http.end();
    return "   ";
  }

  String payload = http.getString();  // Read response into a String
  http.end();                         // Close connection

  t = millis() - t;
  Serial.print(t);
  Serial.println(" ms to load URL");
  yield();
  return payload;
}

/*----------------------------------- Process AI Call ----------------------------------------*/
char* GetAIReply(char* message) {  // Function taken from libary example
 
  //message = "hello chatgpt";
  chat.putMessage(message, strlen(message));
  chat.getResponse();
  return chat.getLastMessageContent();
}

/*------------------------------- Get Quip ------------------------------------------------*/

char* quip() {

  char* output = " ";
  showGraphic("Startled_Turkey.png");
  String AImessage = prompt();
  Serial.println(AImessage);
  int str_len = AImessage.length() + 1;
  char AIPrompt[str_len];
  AImessage.toCharArray(AIPrompt, str_len);
  output = GetAIReply(AIPrompt);
  Serial.println(output);
  return (sanitizeQuip(output));
}

/*------------------------------- Sanitize ------------------------------------------------*/

char* sanitizeQuip(const char* input) {
  String safeQuip = String(input);
  safeQuip.replace("\\", "\\\\");
  safeQuip.replace("\"", "\\\"");
  safeQuip.replace("\n", " ");

  int len = safeQuip.length() + 1;
  char* result = (char*)malloc(len);
  if (result) {
    safeQuip.toCharArray(result, len);
  }
  return result;
}
