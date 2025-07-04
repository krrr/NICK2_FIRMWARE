/*-------- NTP code ----------*/
#include <header.hpp>
#include <cstddef>
#include <ctime>
#include <HardwareSerial.h>
#include <ESP8266WiFi.h>


const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ; // discard any previously received packets
  // Serial.println("Transmit NTP Request");
  //  get a random server from the pool
  const char *server = json["ntp"].as<const char *>();
  if (server == NULL || server[0] == '\0')
  {
    server = ntpServerName;
  }
  Serial.print("[NTP] Connecting to server: ");
  Serial.println(server);
  if (WiFi.hostByName(server, ntpServerIP) != 1)
  {
    Serial.println("[NTP] Server not found...");
    return 0;
  }
  Serial.print("[NTP] Server IP: ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("[NTP] Receiving data...");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      // convert four bytes starting at location 40 to a long integer
      auto secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      return secsSince1900 - 2208988800UL;
    }
    delay(10);
  }

  Serial.println("[NTP] Server not responding...");
  return 0;  // return 0 if unable to get the time
}

time_t getNtpLocalTime() {
  int iterator = 0;
  time_t receivedTime = 0;

  while (iterator < 3 && receivedTime == 0) {
    receivedTime = getNtpTime();
    iterator++;
  }
  if (receivedTime == 0) {
    timeUpdateStatus = UPDATE_FAIL;
    failedAttempts += 1;
    Serial.print("[NTP] Sync failed. Attempt: ");
    Serial.println(failedAttempts);
    return 0;
  }
  Serial.print("[NTP] Sync success! Received NTP timestamp: ");
  Serial.println(receivedTime);
  timeUpdateStatus = UPDATE_SUCCESS;
  timeUpdateLastTime = millis();
  failedAttempts = 0;

  return TZ.toLocal(receivedTime);
}


void ndp_setup() {
  TimeChangeRule EDT2 = {"EDT", json["dst_week"].as<int>(), json["dst_day"].as<int>(), json["dst_month"].as<int>(), json["dst_hour"].as<int>(), json["dst_offset"].as<int>()};
  TimeChangeRule EST2 = {"EST", json["std_week"].as<int>(), json["std_day"].as<int>(), json["std_month"].as<int>(), json["std_hour"].as<int>(), json["std_offset"].as<int>()};

  if (json["dst_enable"].as<int>() == 1) {
    TZ.setRules(EDT2, EST2);
  } else {
    TZ.setRules(EST2, EST2);
  }

  Udp.begin(localPort);
  setSyncProvider(getNtpLocalTime);
  setSyncInterval(3600);
}

void ntp_cancel() {
  setSyncInterval(0);
  setSyncProvider(NULL);
}