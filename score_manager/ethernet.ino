#include <SPI.h>
#include <EthernetServer.h>
#include <Ethernet.h>
#include <EthernetClient.h>

static byte mac[] = {0x24, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static byte ip[] = {192, 168, 6, 250 };

static const char password[] = "VerySecret";
static int pwl = 0;

EthernetClient client = 0;
EthernetServer server = EthernetServer(2400);

void init_ethernet() {
  Ethernet.begin(mac, ip);
  
  server.begin();
}

int getclient() {
  if (client && client.connected()) {
    if (pwl == sizeof(password)-1)
      return client.available();
  }
  else {
    pwl = 0;
    client = 0;
  }
  client = server.available();
  if (!client) return 0;
  if (client.available()) {
    char br = client.read();
    if (password[pwl] != br) {
      client.stop();
      client = 0;
      return 0;
    }
    pwl++;
 }
 if (pwl == sizeof(password)-1)
   return client.available();
 return 0;
}

int check_ethernet() {
  if (!getclient()) return -1;
  byte data = client.read();
  return data;
}

void send_update(int score, int team, int source) {
  if (!client || !client.connected()) return;
  client.write(((score & 0x0F00) >> 8) | (source<<4) | (team<<7));
  client.write(score & 0xFF);
}
