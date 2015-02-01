#include <LedControl.h>

#define LED_CLK 6
#define LED_LOAD 7
#define LED_MOSI 5

static LedControl lc1 = LedControl(LED_MOSI, LED_CLK, LED_LOAD, 1);

void write_display(int display, int n) {
  display <<= 2;
  int i;
  for (i=3; i>=0; i--) {
    lc1.setDigit(0, display + i, n % 10, false);
    n /= 10;
    if (!n) break;
  }
  if (i) for(i--; i>=0; i--) lc1.setChar(0, display + i, ' ', false);
}

void init_lcd() {
  lc1.shutdown(0, false);
  lc1.setIntensity(0, 15);
  write_display(0, 0);
  write_display(1, 0);
}
