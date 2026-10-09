# ESP32-2432S028 CYD, ILI9341

[Tiếng Việt](README.md)

## Scope

Experimental profile for the classic ESP32, 4 MB flash, **no PSRAM**, and
an ILI9341 240x320 panel rotated to 320x240. It targets the GPIO21-backlight
variant in Tony Tran's working `CYD_LGFX_Test.ino` and `LGFX_CYD.hpp`.
Do not use it on ST7789 variants or ESP32-S3 boards.

Enabled: LCD, brightness, BOOT GPIO0, UART console through USB, IPv4 Wi-Fi
setup, and text interaction with Hermes. IPv6 is disabled to retain enough
contiguous DRAM; use IPv4 addresses or hostnames with an A record.
XPT2046 touch, analog microphone, GPIO26
DAC speaker, SD card and RGB LED are not enabled or advertised. Use the
console for text, cancel and settings. BOOT is not a microphone substitute.

The working LovyanGFX sketch is evidence for that sketch's LCD output only.
This Hermes port still requires physical validation.

## Display Mapping

| Signal | Configuration |
|---|---|
| Bus | SPI2 / HSPI, mode 0, 40 MHz |
| SCLK / MOSI / MISO | GPIO14 / GPIO13 / GPIO12 |
| CS / DC | GPIO15 / GPIO2 |
| RST | -1, software reset |
| Backlight | GPIO21, active-high LEDC |
| Logical dimensions | 320x240 |
| Orientation | swap XY, mirror X, no mirror Y |
| Color | RGB565, BGR, inversion enabled |

The sketch converts `CYD_RGB_ORDER=3` to boolean `true`; it is not a third
color mode. MISO matches the wiring but the driver does not read panel
pixels or ID. Backlight uses the shared driver's 5 kHz PWM, not 44.1 kHz.

The port reuses the existing managed `espressif/esp_lcd_ili9341` component.
No Arduino, LovyanGFX code or LovyanGFX initialization table is copied.
Hardware settings are credited to Tony Tran's supplied sketch. Espressif's
driver retains its default initialization, so check cold boot, colors and
orientation on the actual board.

## Build And Flash

Use an activated ESP-IDF 5.3+ terminal from `firmware/esp32`. The local
verification uses ESP-IDF 6.1:

```powershell
idf.py -B build-cyd -D SDKCONFIG=sdkconfig.cyd -D IDF_TARGET=esp32 -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;boards/esp32-2432s028-cyd/sdkconfig.defaults" build
idf.py -B build-cyd -p COM5 flash monitor
python tools/check_size.py --app build-cyd/hermes_gadget.bin --partitions build-cyd/partition_table/partition-table.bin
```

Replace COM5 with the actual port. For manual download mode, hold BOOT,
press RESET, and release BOOT when connecting. Do not hold BOOT during
normal startup.

After changing the component manifest or defaults, delete the generated
profile build directory and sdkconfig (here `build-cyd` and `sdkconfig.cyd`)
and rerun the full build command. Never delete `sdkconfig.defaults`.
For PlatformIO, clean `.pio` and `sdkconfig.esp32-2432s028-cyd` first.

With a PlatformIO platform providing ESP-IDF >=5.3:

```powershell
pio run -e esp32-2432s028-cyd
pio run -e esp32-2432s028-cyd -t upload --upload-port COM5
pio device monitor -b 115200 -p COM5
```

The existing dual-slot OTA partition table and NVS offsets are unchanged.
`idf.py flash` writes the correct offsets. Do not flash an app-only binary
at address zero: classic ESP32 uses bootloader `0x1000` and app `0x20000`.

## Memory And Validation

A priority-101 constructor reserves the 153600-byte RGB565 framebuffer in
internal heap before ordinary constructors and Wi-Fi fragment it. DMA
staging is four rows (2560 bytes). The framebuffer is not a large `.bss`
array. The main stack stays at 8192 bytes; radio/TCP buffers are reduced.
The app object is allocated after the framebuffer, unused peripheral
instances are omitted, and IPv6 tables no longer occupy static RAM.

Check readable, unmirrored output and red/green/blue/white/black; brightness
at zero/intermediate/full, timeout and wake; console and phone Wi-Fi setup;
pairing, text replies, reconnect and identity persistence across power loss.
Use `diag` to record free heap, largest block and stack headroom during
setup, WebSocket connection and long replies. Test OTA/rollback and a
two-hour session before regular use.

TLS/WSS, images and long messages need additional heap. Compilation alone
does not establish runtime memory sufficiency. Do not disable certificate
verification to save RAM. For display noise, try `b.lcd.spi_mhz = 20` in
the CYD branch of `board.cpp`; change only one display setting at a time.

Record PCB revision, power supply, firmware, logs and untested steps.
See the repository's `docs/hardware-validation.md` for the full checklist.
Website pages and board lists outside `firmware/esp32` are intentionally
unchanged in this scoped port.

Native board configuration tests (CMake and C++17, no ESP-IDF required):

```powershell
cmake -S tests -B "$env:TEMP/hermes-cyd-board-tests"
cmake --build "$env:TEMP/hermes-cyd-board-tests" --config Release
ctest --test-dir "$env:TEMP/hermes-cyd-board-tests" -C Release --output-on-failure
python -B -m unittest discover -s tests -p test_image_identity.py -v
```

These three cases run the real `board.cpp` for CYD, S3 breadboard and
CrowPanel with compile-time board choices instead of generated Kconfig.
They do not simulate LCD, Wi-Fi or the chip's heap.

## Local Results

2026-10-09: ESP-IDF 6.1 build, `check_config` and `check_size` passed.
The app is 1168000 bytes with 43% of its OTA slot free. Three board tests
and four image identity tests passed; the actual binary identifies as
`esp32-2432s028-cyd`. Packaging now skips the OTA scanner's empty `HGBOARD=`
literal when finding the image's board name.

The linker places the low heap at `0x3ffba150`, leaving 155312 bytes before
`0x3ffe0000` (ROM stack memory unavailable before the scheduler starts).
That leaves 1712 bytes beyond the 153600-byte framebuffer **before
allocator/startup overhead**. This is not a runtime heap measurement:
verify that boot does not log `early CYD framebuffer allocation failed`.

No physical flashing/testing, full firmware matrix or PlatformIO run has
been performed. A shared touch driver still emits a deprecated-API warning;
touch is disabled on CYD. WSS, large images, OTA and long-run stability are
not verified.
