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

byte event_queue[2048] = {};

int eq_begin = 0;
int eq_end = 0;


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

void enqueue_update(int score, int team, int source) {
  if (eq_end == 2048) eq_end = 0;
  event_queue[eq_end++] = ((score & 0x0F00) >> 8) | (source<<4) | (team<<7);
  event_queue[eq_end++] = score & 0xFF;
}

void send_event() {
  if (!client || !client.connected() || !client || !client.connected() || pwl != sizeof(password)-1) return;
  if (eq_begin == 2048 && eq_end != 2048) eq_begin = 0;
  if (eq_begin == eq_end) return;
  client.write(event_queue[eq_begin++]);
  client.write(event_queue[eq_begin++]);
}
