//secrets.h
// Enter your Wifi and OpenAI API key below
//
#pragma once  // Optional, prevents multiple includes

#define GITHUBURL "https://raw.githubusercontent.com/bill-orange/Turkey_Day_ESP32_Audio_Video/master/Data/"  // or your fork
const char *chatGPT_APIKey_txt = "your API key";
const char *chatGPT_APIKey = "Bearer your API key";

// 🔐 List of known SSIDs and passwords
const char* ssidList[] = {"ssid_1", "ssid_2", "ssid_3"};
const char* passList[] = {"pw_1", "pw_2", "pw_3"};
const int numNetworks = 3;
