#include "board.hpp"

#include <cstdio>
#include <cstring>

// Unlike assert(), these checks remain enabled in Release builds.
#define CHECK(condition) do { \
  if (!(condition)) { \
    std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    return 1; \
  } \
} while (0)

int main() {
  const auto& b = hgp::board_config();
  CHECK(b.lcd.enabled);
  CHECK(hgp::lcd_power_pin(b) == -1);
#if CONFIG_HG_BOARD_ESP32_CYD
  CHECK(std::strcmp(b.name, "esp32-2432s028-cyd") == 0);
  CHECK(b.lcd.controller == hgp::LcdController::Ili9341);
  CHECK(b.lcd.bus.type == hgp::LcdBus::Type::Spi);
  CHECK(b.lcd.width == 320 && b.lcd.height == 240);
  CHECK(b.lcd.swap_xy && b.lcd.mirror_x && !b.lcd.mirror_y);
  CHECK(b.lcd.invert && b.lcd.bgr && !b.lcd.backlight_invert);
  CHECK(b.lcd.gap_x == 0 && b.lcd.gap_y == 0);
  CHECK(b.lcd.mosi == 13 && b.lcd.miso == 12 && b.lcd.sclk == 14);
  CHECK(b.lcd.cs == 15 && b.lcd.dc == 2 && b.lcd.rst == -1);
  CHECK(b.lcd.backlight == 21 && b.lcd.spi_mhz == 40);
  CHECK(b.buttons.talk == 0 && b.buttons.cancel == -1);
  CHECK(b.buttons.up == -1 && b.buttons.down == -1);
  CHECK(!b.mic.enabled && !b.speaker.enabled && !b.codec.enabled);
  CHECK(!b.touch.enabled && !b.amoled.enabled && !b.latch_power.enabled);
  CHECK(!b.axp2101 && !b.cores3 && b.status_led == -1);
  CHECK(b.i2c.sda == -1 && b.i2c.scl == -1);
  const int used[] = {b.lcd.mosi, b.lcd.miso, b.lcd.sclk, b.lcd.cs,
                      b.lcd.dc, b.lcd.backlight, b.buttons.talk};
  for (unsigned i = 0; i < sizeof(used) / sizeof(used[0]); ++i)
    for (unsigned j = i + 1; j < sizeof(used) / sizeof(used[0]); ++j)
      CHECK(used[i] != used[j]);
#elif CONFIG_HG_BOARD_ESP32S3_BREADBOARD
  CHECK(std::strcmp(b.name, "esp32s3-breadboard") == 0);
  CHECK(b.lcd.controller == hgp::LcdController::St7789);
  CHECK(b.lcd.width == 320 && b.lcd.height == 240);
  CHECK(b.lcd.mosi == 11 && b.lcd.sclk == 12 && b.lcd.miso == -1);
  CHECK(b.lcd.cs == 10 && b.lcd.dc == 9 && b.lcd.backlight == 7);
  CHECK(b.mic.enabled && b.speaker.enabled);
#elif CONFIG_HG_BOARD_CROWPANEL_21
  CHECK(std::strcmp(b.name, "crowpanel-2.1") == 0);
  CHECK(b.lcd.bus.type == hgp::LcdBus::Type::Rgb);
  CHECK(b.lcd.width == 480 && b.lcd.height == 480 && b.lcd.round);
  CHECK(b.lcd.backlight == 6 && b.lcd.miso == -1);
  CHECK(b.lcd.rgb.pclk == 41 && b.lcd.rgb.i2c_expander == 0x21);
  CHECK(b.touch.enabled && b.encoder.a == 42 && b.encoder.b == 4);
#else
#error Unsupported test board
#endif
  std::printf("%s: board configuration passed\n", b.name);
  return 0;
}
