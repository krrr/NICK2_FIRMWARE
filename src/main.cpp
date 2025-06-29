/*
   GNU General Public License v3.0
   Copyright (c) 2022 Martin Cerny
*/

#include <header.hpp>
#include <FS.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>


#include <TimeLib.h>


// User global vars
const char* dns_name = "nick2"; // only for AP mode
const char* update_path = "/update";
const char* update_username = "nick2";
const char* update_password = "nick2";
const char* ntpServerName = "pool.ntp.org";

// const int dotsAnimationSteps = 2000; // dotsAnimationSteps * TIMER_INTERVAL_uS = one animation cycle time in microseconds


HsbColor red[] = {
  HsbColor(RgbColor(100, 0, 0)), // LOW
  HsbColor(RgbColor(150, 0, 0)), // MEDIUM
  HsbColor(RgbColor(200, 0, 0)), // HIGH
};
HsbColor green[] = {
  HsbColor(RgbColor(0, 100, 0)), // LOW
  HsbColor(RgbColor(0, 150, 0)), // MEDIUM
  HsbColor(RgbColor(0, 200, 0)), // HIGH
};
HsbColor blue[] = {
  HsbColor(RgbColor(0, 0, 100)), // LOW
  HsbColor(RgbColor(0, 0, 150)), // MEDIUM
  HsbColor(RgbColor(0, 0, 200)), // HIGH
};
HsbColor yellow[] = {
  HsbColor(RgbColor(100, 100, 0)), // LOW
  HsbColor(RgbColor(150, 150, 0)), // MEDIUM
  HsbColor(RgbColor(200, 200, 0)), // HIGH
};
HsbColor purple[] = {
  HsbColor(RgbColor(100, 0, 100)), // LOW
  HsbColor(RgbColor(150, 0, 150)), // MEDIUM
  HsbColor(RgbColor(200, 0, 200)), // HIGH
};
HsbColor azure[] = {
  HsbColor(RgbColor(0, 100, 100)), // LOW
  HsbColor(RgbColor(0, 150, 150)), // MEDIUM
  HsbColor(RgbColor(0, 200, 200)), // HIGH
};

#if defined(CLOCK_VERSION_IN16)
HsbColor colonColorDefault[] = {
  HsbColor(RgbColor(30, 70, 50)), // LOW
  HsbColor(RgbColor(50, 100, 80)), // MEDIUM
  HsbColor(RgbColor(80, 130, 100)), // HIGH
};
#else
HsbColor colonColorDefault[] = {
  HsbColor(RgbColor(30, 70, 50)), // LOW
  HsbColor(RgbColor(50, 100, 80)), // MEDIUM
  HsbColor(RgbColor(120, 220, 140)), // HIGH
};
/*
  RgbColor colonColorDefault[] = {
  RgbColor(30, 70, 50), // LOW
  RgbColor(50, 100, 80), // MEDIUM
  RgbColor(100, 200, 120), // HIGH
  };
*/
#endif

/*
  RgbColor colonColorDefault[] = {
  RgbColor(30, 6, 1), // LOW
  RgbColor(38, 8, 2), // MEDIUM
  RgbColor(50, 10, 2), // HIGH
  };
*/
RgbColor currentColor = RgbColor(0, 0, 0);
//RgbColor colonColorDefault = RgbColor(90, 27, 7);
//RgbColor colonColorDefault = RgbColor(38, 12, 2);

// MAX6921 has 20 outputs, we have 3 of them,
// closest to that is 64 (8x8)
#if defined(CLOCK_VERSION_IN16)
const uint8_t digitPins[DIGITS_COUNT][CATHODE_COUNT] = {
  {
    14,  //0
    1, //1
    3, //2
    2, //3
    4, //4
    6, //5
    5, //6
    7, //7
    0, //8
    15,  //9
  },
  {
    21, //0
    13, //1
    20, //2
    11, //3
    10, //4
    23, //5
    22, //6
    12, //7
    19, //8
    18, //9
  },
  {
    24, //0
    16, //1
    30, //2
    27, //3
    28, //4
    29, //5
    31, //6
    17, //7
    26, //8
    25, //9
  },
  {
    42, //0
    39, //1
    33, //2
    37, //3
    38, //4
    35, //5
    34, //6
    36, //7
    32, //8
    41, //9
  },

  {
    54, //0
    46, //1
    43, //2
    45, //3
    55, //4
    48, //5
    44, //6
    47, //7
    62, //8
    49, //9
  },
  {
    57, //0
    53, //1 // 56 = dot
    60, //2
    51, //3
    50, //4
    63, //5
    61, //6
    52, //7
    59, //8
    58, //9
  },
};
#else if defined(CLOCK_VERSION_ZM1040)
const uint8_t bytesToShift = 8; // we have 60 outputs, 60 / 8 = 8 bytes
const uint8_t digitPins[DIGITS_COUNT][CATHODE_COUNT] = {
  {
    6, //0
    7, //1
    14,  //2
    15, //3
    0, //4
    1, //5
    2, //6
    3, //7
    4, //8
    5, //9
  },
  {
    12, //0
    13, //1
    20, //2
    21, //3
    22, //4
    23, //5
    8, //6
    9, //7
    10, //8
    11, //9
  },
  {
    18, //0
    19, //1
    26, //2
    27, //3
    28, //4
    29, //5
    30, //6
    31, //7
    16, //8
    17, //9
  },
  {
    24, //0
    25, //1
    32, //2
    33, //3
    34, //4
    35, //5
    36, //6
    37, //7
    38, //8
    39, //9
  },
};
#endif



/*
  struct Digits {
  uint8_t current_cathode;
  uint8_t target_cathode;
  uint8_t current_brightness;
  uint8_t target_brightness;
  uint8_t cf_state;
  };
*/
volatile uint8_t currentCathode[DIGITS_COUNT];
volatile uint8_t targetCathode[DIGITS_COUNT];
volatile uint8_t crossFadeState[DIGITS_COUNT];
volatile uint8_t currentNeonBrightness;
volatile uint8_t targetNeonBrightness;

// 32 steps @ 220uS => 142hz
// 48 steps @ 220uS => 95.24hz
// 48 steps @ 300uS => 69hz
// 64 steps @ 220uS => 71.4hz
// 64 steps @ 300uS => 52hz
volatile uint8_t shiftedDutyState[DIGITS_COUNT];
const uint8_t pwmResolution = 64; // should be in the multiples of dimmingSteps to enable smooth crossfade
const uint8_t dimmingSteps = 2;

// Cathode poisoning prevention pattern
uint8_t healPattern[6][10] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
  {4, 5, 6, 7, 8, 9, 0, 1, 2, 3},
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
  {3, 4, 5, 6, 7, 8, 9, 0, 1, 2},
  {6, 7, 8, 7, 9, 6, 7, 8, 9, 7},  // special
  {2, 3, 4, 5, 6, 7, 8, 9, 0, 1},
};

// MAX BRIGHTNESS PER DIGIT
// These need to be multiples of 8 to enable crossfade! Must be less or equal as pwmResolution.
// Set maximum brightness for reach digit separately. This can be used to normalize brightness between new and burned out tubes.
// Last two values are ignored in 4-digit clock
uint8_t bri_vals_separate[3][6] = {
  {8, 8, 8, 8, 8, 8}, // Low brightness
  {24, 24, 24, 24, 24, 24}, // Medium brightness
  {64, 64, 64, 64, 64, 64}, // High brightness
};


// Better left alone global vars
volatile bool isPoweredOn = true;
unsigned long configStartMillis, prevDisplayMillis = 0;
// volatile int activeDot;
uint8_t deviceMode = NORMAL_MODE;
volatile bool toggleSeconds;
// bool breatheState;
byte mac[6];
// volatile int dutyState = 0;
// volatile uint8_t digitsCache[] = {0, 0, 0, 0};
volatile byte bytes[BYTES_TO_SHIFT];
// volatile byte prevBytes[BYTES_TO_SHIFT];
volatile uint8_t bri = 0;
volatile int crossFadeTime = 0;
volatile uint8_t fadeIterator;
uint8_t timeUpdateStatus = 0; // 0 = no update, 1 = update success, 2 = update fail,
unsigned long timeUpdateLastTime = 0;  // store the time when NTP was last updated
uint8_t failedAttempts = 0;
volatile bool enableDotsAnimation;
// volatile unsigned short dotsAnimationState;
RgbColor colonColor;
IPAddress ip_addr;

TimeChangeRule EDT = {"EDT", Last, Sun, Mar, 1, 120};  //UTC + 2 hours
TimeChangeRule EST = {"EST", Last, Sun, Oct, 1, 60};  //UTC + 1 hours
Timezone TZ(EDT, EST);
#if defined(CLOCK_VERSION_IN16)
NeoPixelBus<NeoGrbFeature, NeoWs2813InvertedMethod> strip(PIXEL_COUNT);
#else
NeoPixelBus<NeoGrbFeature, NeoWs2813Method> strip(PIXEL_COUNT);
#endif
NeoGamma<NeoGammaTableMethod> colorGamma;
NeoPixelAnimator animations(PIXEL_COUNT);
JsonDocument json; // config buffer
Ticker fade_animation_ticker;
Ticker onceTicker;
Ticker colonTicker;
ESP8266Timer ITimer;
DNSServer dnsServer;
ESP8266WebServer server(80);
WiFiUDP Udp;
ESP8266HTTPUpdateServer httpUpdateServer;
unsigned int localPort = 8888;  // local port to listen for UDP packets


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println("");

  if (!SPIFFS.begin()) {
    Serial.println("[CONF] Failed to mount file system");
  }

  pinMode(COLON_PIN, OUTPUT);
  digitalWrite(COLON_PIN, 0);

  readConfig();

  initStrip();
  initRgbColon();
  initScreen();

  WiFi.macAddress(mac);

  auto ssid = json["ssid"].as<const char*>();
  auto pass = json["pass"].as<const char*>();
  auto ip = json["ip"].as<const char*>();
  auto gw = json["gw"].as<const char*>();
  auto sn = json["sn"].as<const char*>();

  if (ssid != NULL && pass != NULL && ssid[0] != '\0' && pass[0] != '\0') {
    Serial.println("[WIFI] Connecting to: " + String(ssid));
    WiFi.mode(WIFI_STA);

    if (ip != NULL && gw != NULL && sn != NULL && ip[0] != '\0' && gw[0] != '\0' && sn[0] != '\0') {
      IPAddress ip_address, gateway_ip, subnet_mask;
      if (!ip_address.fromString(ip) || !gateway_ip.fromString(gw) || !subnet_mask.fromString(sn)) {
        Serial.println("[WIFI] Error setting up static IP, using auto IP instead. Check your configuration.");
      } else {
        WiFi.config(ip_address, gateway_ip, subnet_mask);
      }
    }
    // serializeJson(json, Serial);

    enableDotsAnimation = true; // Start the dots animation

    updateColonColor(yellow[bri]);
    strip_show();

    WiFi.hostname(AP_NAME + macLastThreeSegments(mac));
    WiFi.begin(ssid, pass);

    //startBlinking(200, colorWifiConnecting);

    int wifi_timeout = json["wifi_timeout"].as<int>();
    if (wifi_timeout < 10 || wifi_timeout > 360) {
      wifi_timeout = 20;  // defult 20s timeout
    }
    wifi_timeout *= 10;
    for (int i = 0; ; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        if (i > wifi_timeout) {
          enableDotsAnimation = false;
          deviceMode = CONFIG_MODE;
          updateColonColor(red[bri]);
          strip_show();
          Serial.print("[WIFI] Failed to connect to: " + String(ssid) + ", going into config mode.");
          delay(500);
          break;
        }
        delay(100);
      } else {
        updateColonColor(green[bri]);
        enableDotsAnimation = false;
        strip_show();
        Serial.println("[WIFI] Successfully connected to: " + WiFi.SSID());
        Serial.println("[WIFI] Mac address: " + WiFi.macAddress());
        Serial.println("[WIFI] IP address: " + WiFi.localIP().toString());
        delay(1000);
        break;
      }
    }
  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("[CONF] No credentials set, going to config mode.");
  }

  Serial.println("[SYS] FreeSketchSpace: " + String(ESP.getFreeSketchSpace() / 1024) + "KB");

  if (deviceMode == CONFIG_MODE) {
    startConfigPortal(); // Blocking loop
  } else {
    ndp_setup();
    startServer();
  }

  if (json["rst_cycle"].as<unsigned int>() == 1) {
    cycleDigits();
    delay(500);
  }

  if (json["rst_ip"].as<unsigned int>() == 1) {
    showIP(5000);
    delay(500);
  }

  /*
    if (!MDNS.begin(dns_name)) {
      Serial.println("[ERROR] MDNS responder did not setup");
    } else {
      Serial.println("[INFO] MDNS setup is successful!");
      MDNS.addService("http", "tcp", 80);
    }
  */
}

// the loop function runs over and over again forever
void loop() {
  if (timeUpdateLastTime == 0 && timeUpdateStatus == UPDATE_FAIL) {
    setAllDigitsTo(0);
    updateColonColor(red[bri]); // red
    strip_show();
    delay(10);
    return;
  }

  auto millis_ = millis();
  if (millis_ - prevDisplayMillis >= 1000) {  // update the display only if time has changed
    prevDisplayMillis = millis_;
    toggleNightMode();

    int cathode = json["cathode"].as<int>();
    auto now_t = now();
    if (
      (cathode == 1 && (hour(now_t) >= 2 && hour(now_t) <= 6) && minute(now_t) < 10) ||
      (cathode == 2 && (((hour(now_t) >= 2 && hour(now_t) <= 6) && minute(now_t) < 10) || minute(now_t) < 1)) ||
      (cathode == 3 && (minute(now_t) % 2 == 0 && second(now_t) < 10))
    ) {
      healingCycle();  // do healing loop if the time is right :)
    } else {
      if (timeUpdateStatus) {
        if (timeUpdateStatus == UPDATE_SUCCESS) {
          setTemporaryColonColor(5, green[bri]);
        } else if (timeUpdateStatus == UPDATE_FAIL) {
          if (failedAttempts > 2) {
            colonColor = red[bri];
          } else {
            setTemporaryColonColor(5, red[bri]);
          }
        }
        timeUpdateStatus = 0;
      }

      handleColon();
      showTime();
    }
  }

  // Offline mode: disconnect WiFi after NTP update and 5 minutes
  if (WiFi.status() == WL_CONNECTED && json["offline_mode"].as<int>() == 1 && (millis_ - timeUpdateLastTime) >= 300000) { // 300000 ms = 5 minutes
    Serial.println("[WIFI] Offline mode enabled: Disconnecting WiFi.");
    ntp_cancel();
    WiFi.disconnect(true);
  }

  animations.UpdateAnimations();
  strip_show();

  //MDNS.update();
  server.handleClient();
  delay(5); // Simple delay doesn't look like much but it keeps the ESP cold and saves power
}
