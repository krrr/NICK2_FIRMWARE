#ifndef GLOBAL_H
#define GLOBAL_H
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "ESP8266TimerInterrupt.h"
#include <Ticker.h>
#include <WString.h>
#include <Timezone.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

////////////// main
// Pick a clock version below!
#define CLOCK_VERSION_IN16
//#define CLOCK_VERSION_ZM1040

#if defined(CLOCK_VERSION_IN16)
#define DIGITS_COUNT 6  // number of digits
#define CLOCK_6_DIGIT
#else
#define DIGITS_COUNT 4  // number of digits
#define CLOCK_4_DIGIT
#endif
#define CATHODE_COUNT 10  // number of cathodes per digit

#if !defined(CLOCK_VERSION_IN16) && !defined(CLOCK_VERSION_ZM1040)
#error "You have to select a clock version! Line 25"
#endif

#define AP_NAME "NICK2_"
#define FW_NAME "Nick2_Fork"
#define FW_VERSION "1.0.0"
#define CONFIG_TIMEOUT 300000 // 300000 = 5 minutes

// ONLY CHANGE DEFINES BELOW IF YOU KNOW WHAT YOU'RE DOING!
#define NORMAL_MODE 0
#define OTA_MODE 1
#define CONFIG_MODE 2
#define CONFIG_MODE_LOCAL 3
#define CONNECTION_FAIL 4
#define UPDATE_SUCCESS 1
#define UPDATE_FAIL 2
#define DATA 13
#define CLOCK 14
#define LATCH 15
#define COLON_PIN 4
#define TIMER_INTERVAL_uS 250 // screen pwm interval in microseconds, 300 = safe value for 6 digits. You can go down to 150-200 for 4-digit one. Going too low will cause crashes.
#if defined(CLOCK_6_DIGIT)
#define TIMER_INTERVAL_uS 300
#endif
#define MINIMAL_CROSSFADE_BRIGHTNESS 8 // crossfade will be disabled below this brightness because there's not enough steps for smooth transition
#define PIXEL_COUNT 14 // Addressable LED count
#define BYTES_TO_SHIFT 8 // we have 60 outputs, 60 / 8 = 8 bytes
#define HEALING_CYCLE_TIMES_A_SECOND 10

// User global vars
extern const char* dns_name;
extern const char* update_path;
extern const char* update_username;
extern const char* update_password;
extern const char* ntpServerName;
// global vars
extern JsonDocument json;
extern Ticker fade_animation_ticker;
extern Ticker onceTicker;
extern Ticker colonTicker;
extern ESP8266Timer ITimer;
extern DNSServer dnsServer;
extern ESP8266WebServer server;
extern WiFiUDP Udp;
extern ESP8266HTTPUpdateServer httpUpdateServer;
extern RgbColor currentColor;
extern NeoGamma<NeoGammaTableMethod> colorGamma;
extern NeoPixelAnimator animations;
extern const int dotsAnimationSteps;
extern HsbColor red[];
extern HsbColor blue[];
extern HsbColor green[];
extern HsbColor yellow[];
extern HsbColor purple[];
extern HsbColor azure[];
extern HsbColor colonColorDefault[];
extern RgbColor currentColor;
extern const uint8_t digitPins[DIGITS_COUNT][CATHODE_COUNT];
extern Timezone TZ;
#if defined(CLOCK_VERSION_IN16)
extern NeoPixelBus<NeoGrbFeature, NeoWs2813InvertedMethod> strip;
#else
extern NeoPixelBus<NeoGrbFeature, NeoWs2813Method> strip;
#endif
extern unsigned int localPort;
extern volatile uint8_t currentCathode[];
extern volatile uint8_t targetCathode[];
extern volatile uint8_t crossFadeState[];
extern volatile uint8_t currentNeonBrightness;
extern volatile uint8_t targetNeonBrightness;
extern volatile uint8_t shiftedDutyState[];
extern const uint8_t pwmResolution;
extern const uint8_t dimmingSteps;
extern uint8_t healPattern[6][10];
extern uint8_t bri_vals_separate[3][6];
extern volatile bool isPoweredOn;
extern unsigned long configStartMillis, prevDisplayMillis;
// extern volatile int activeDot;
extern uint8_t deviceMode;
extern bool timeUpdateFirst;
extern volatile bool toggleSeconds;
// extern bool breatheState;
extern byte mac[];
// extern volatile int dutyState;
extern volatile byte bytes[];
extern volatile byte prevBytes[];
extern volatile uint8_t bri;
extern volatile int crossFadeTime;
extern volatile uint8_t fadeIterator;
extern uint8_t timeUpdateStatus;
extern uint8_t failedAttempts;
extern volatile bool enableDotsAnimation;
// extern volatile unsigned short dotsAnimationState;
extern RgbColor colonColor;
extern IPAddress ip_addr;
//////////////

////////////// fns
String macToStr(const uint8_t* mac);
String macLastThreeSegments(const uint8_t* mac);
bool readConfig();
bool saveConfig();
String toStringIp(IPAddress ip);
boolean isIp(String str);
//////////////

////////////// ntp fns
void ndp_setup();
time_t getNtpTime();
time_t getNtpLocalTime();
//////////////

////////////// vfd fns
void initScreen();
void enableScreen();
void disableScreen();
void setupBriBalance();
void setupCrossFade();
void setAllDigitsTo(uint16_t value);
void cycleDigits();
void showIP(int delay_ms);
void setupPhaseShift();
void toggleNightMode();
void healingCycle();
void showTime();
void setNeon(bool state);
void setDigit(uint8_t digit, uint8_t value);
void handleFade();
void blankAllDigits();
//////////////


///////////// neopixel fns

void initStrip();
void strip_show();
void setTemporaryColonColor(int seconds, RgbColor color);
void resetColonColor();
void initRgbColon();
void updateColonColor(RgbColor color);
void handleColon();
void AnimUpdate(const AnimationParam& param);
void SetupAnimations(RgbColor StartingColor, RgbColor EndingColor, int duration);

//////////////


///////////// config_portal
void startConfigPortal();
void startServer();
void handleRoot();
void handleDiyHueDetect();
void handleDiyHueSet();
void handleNotFound();
void handleJson();
void handleDiyHueGet();
void handleHeal();

#endif // GLOBAL_H
