# ESP32-2432S028 CYD, ILI9341

[English](README-en.md)

## Phạm vi

Profile thử nghiệm cho ESP32 thường, flash 4 MB, **không PSRAM**, LCD
ILI9341 240x320 xoay ngang thành 320x240. Chỉ dành cho biến thể có đèn nền
GPIO21 như hai tệp `CYD_LGFX_Test.ino` và `LGFX_CYD.hpp` do Tony Trần cung cấp.
Không dùng cho CYD màn ST7789 hoặc bo ESP32-S3.

Hỗ trợ LCD, điều chỉnh độ sáng, BOOT GPIO0, console UART qua USB,
cấu hình Wi-Fi **IPv4** và giao tiếp Hermes bằng văn bản. Profile này tắt
IPv6 để giữ đủ DRAM liên tục; dùng địa chỉ IPv4 hoặc hostname có bản ghi A.
Chưa hỗ trợ XPT2046,
microphone analog, loa DAC GPIO26, thẻ SD hoặc LED RGB. Các ngoại vi này
không được khai báo như thể đã hoạt động. Dùng console để gửi văn bản,
hủy thao tác và thay đổi cài đặt; nút BOOT không thay thế được microphone.

Đây chưa phải bản đã kiểm chứng trên phần cứng. Mẫu LovyanGFX chạy thành
công chỉ xác nhận LCD với mẫu đó, không xác nhận toàn bộ firmware Hermes.

## Cấu hình LCD

| Tín hiệu | Cấu hình |
|---|---|
| Bus | SPI2 / HSPI, mode 0, 40 MHz |
| SCLK / MOSI / MISO | GPIO14 / GPIO13 / GPIO12 |
| CS / DC | GPIO15 / GPIO2 |
| RST | -1, dùng software reset |
| Backlight | GPIO21, active-high, LEDC |
| Kích thước logic | 320x240 |
| Rotation | swap XY, mirror X, không mirror Y |
| Màu | RGB565, BGR, inversion bật |

`CYD_RGB_ORDER=3` trong mẫu được chuyển thành boolean `true`, không phải
một chế độ màu thứ ba. ESP-IDF dùng BGR tương ứng. MISO được nối như mẫu
nhưng driver không đọc pixel/ID từ LCD. Backlight dùng PWM 5 kHz của driver
chung, không dùng PWM 44.1 kHz của mẫu.

Firmware tái sử dụng managed component `espressif/esp_lcd_ili9341` đã có
trong dự án, không thêm Arduino hoặc LovyanGFX. Pin và cấu hình hướng/màu
được đối chiếu với mẫu của Tony Trần; không sao chép mã nguồn LovyanGFX hay
bảng khởi tạo của thư viện. Driver Espressif giữ bảng khởi tạo mặc định,
vì vậy vẫn cần thử màu, hướng và cold boot trên chính bo của bạn.

## Build Và Nạp

Mở terminal ESP-IDF 5.3+ đã kích hoạt môi trường; bản port được kiểm tra
bằng ESP-IDF 6.1. Chạy từ `firmware/esp32`:

```powershell
idf.py -B build-cyd -D SDKCONFIG=sdkconfig.cyd -D IDF_TARGET=esp32 -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;boards/esp32-2432s028-cyd/sdkconfig.defaults" build
idf.py -B build-cyd -p COM5 flash monitor
```

Thay `COM5` bằng cổng thực tế. Nếu không vào download mode, giữ BOOT,
nhấn RESET rồi thả BOOT khi bắt đầu kết nối. Không giữ BOOT khi boot
bình thường. Không dùng binary dành cho ESP32-S3.

Sau khi đổi `idf_component.yml` hoặc defaults, xóa **thư mục build và
sdkconfig sinh ra của profile đó** rồi chạy lại lệnh build đầy đủ. Ví dụ
`build-cyd` và `sdkconfig.cyd`; giữ nguyên các tệp `sdkconfig.defaults`.
Với PlatformIO, xóa `.pio` và `sdkconfig.esp32-2432s028-cyd` trước khi build lại.

PlatformIO dùng environment mới, với platform hỗ trợ ESP-IDF >=5.3:

```powershell
pio run -e esp32-2432s028-cyd
pio run -e esp32-2432s028-cyd -t upload --upload-port COM5
pio device monitor -b 115200 -p COM5
```

Kiểm tra dung lượng cho build ESP-IDF trực tiếp:

```powershell
python tools/check_size.py --app build-cyd/hermes_gadget.bin --partitions build-cyd/partition_table/partition-table.bin
```

Giữ partition table hiện tại với hai slot OTA, không thay đổi vị trí NVS.
`idf.py flash` nạp các thành phần tại đúng offset; không nạp app riêng vào
địa chỉ 0. Với ESP32 thường, bootloader nằm tại `0x1000`, app tại `0x20000`.

## Bộ Nhớ Và Kiểm Tra

Framebuffer RGB565 cần 153600 byte liên tục. Constructor priority 101
đặt trước vùng này trong internal heap, trước các constructor thông thường
và Wi-Fi; buffer DMA chỉ chứa 4 hàng (2560 byte). Không đặt framebuffer lớn
trong `.bss`. Stack chính vẫn 8192 byte, radio/TCP dùng buffer nhỏ hơn.
Đối tượng ứng dụng được cấp phát sau framebuffer; các đối tượng ngoại vi
không dùng không được tạo trong bản CYD. Bảng IPv6 bị loại khỏi static RAM.

Kiểm tra sau khi nạp:

1. Log có `ili9341 320x240 ready`, không có lỗi cấp phát framebuffer.
2. Chữ đúng hướng, không bị soi gương; thử đỏ/xanh lá/xanh dương/trắng/đen.
3. Độ sáng 0%, trung gian, 100%; kiểm tra timeout và đánh thức màn hình.
4. Cấu hình Wi-Fi qua console hoặc trang setup, ghép đôi Hermes, gửi văn bản.
5. Chạy `diag`, ghi lại heap trống, khối lớn nhất và stack trống; thử cả
   lúc setup, kết nối WebSocket và nhận câu trả lời dài.
6. Ngắt/cấp nguồn, mất/kết nối lại mạng; kiểm tra danh tính còn nguyên.
7. Thử OTA/rollback và chạy liên tục hai giờ trước khi dùng thường xuyên.

WSS/TLS, ảnh và nội dung dài cần thêm heap. Build thành công không chứng
minh có đủ RAM cho mọi tải; không tắt xác thực chứng chỉ để giảm RAM.
Nếu LCD bị nhiễu, thử `b.lcd.spi_mhz = 20` trong nhánh CYD của `board.cpp`.
Không đổi nhiều tham số màu/hướng cùng lúc.

Ghi rõ revision PCB, nguồn cấp, firmware, log và bước nào chưa thử.
Checklist phần cứng đầy đủ nằm trong `docs/hardware-validation.md` của repo.
Các trang web/danh sách bo ở ngoài `firmware/esp32` chưa được cập nhật trong
thay đổi giới hạn phạm vi này.

Kiểm thử cấu hình bo trên máy tính (không cần ESP-IDF, cần CMake và C++17):

```powershell
cmake -S tests -B "$env:TEMP/hermes-cyd-board-tests"
cmake --build "$env:TEMP/hermes-cyd-board-tests" --config Release
ctest --test-dir "$env:TEMP/hermes-cyd-board-tests" -C Release --output-on-failure
python -B -m unittest discover -s tests -p test_image_identity.py -v
```

Ba ca kiểm tra CYD, ESP32-S3 breadboard và CrowPanel dùng `board.cpp` thật,
chỉ thay nguồn lựa chọn Kconfig bằng định nghĩa lúc biên dịch. Chúng không
giả lập LCD, Wi-Fi hoặc heap của chip.

## Kết Quả Cục Bộ

Ngày 2026-10-09: build ESP-IDF 6.1 đạt; `check_config` và `check_size` đạt.
App 1168000 byte, còn 43% slot OTA. Ba kiểm thử cấu hình bo và bốn kiểm thử
nhận dạng binary đạt; binary thật trả về `esp32-2432s028-cyd`.
Công cụ đóng gói đã được sửa để bỏ qua thẻ `HGBOARD=` rỗng của bộ quét OTA.

Linker đặt heap thấp tại `0x3ffba150`, còn 155312 byte trước `0x3ffe0000`
(vùng stack ROM, chưa cấp phát được trước scheduler). Sau khi trừ 153600
byte framebuffer chỉ còn 1712 byte **trước overhead allocator/startup**.
Đây không phải số đo heap lúc chạy; phải xác nhận log không có
`early CYD framebuffer allocation failed` trên bo thật.

Chưa nạp/thử phần cứng, chưa chạy toàn bộ ma trận firmware hoặc PlatformIO.
Build còn cảnh báo API touch cũ trong driver dùng chung; touch không bật
trên CYD. Không khẳng định ổn định WSS, ảnh lớn, OTA hoặc chạy dài hạn.
