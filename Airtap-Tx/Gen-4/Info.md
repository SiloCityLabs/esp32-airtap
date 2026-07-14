This is a new Airtap by AC Infinity. Reverse engineering notes here:

Header Pads exposed:
 - IO9 (BOOT / download strap — hold LOW + pulse EN to enter ROM bootloader)
 - GND
 - TX
 - RX
 - 3.3V
 - EN

Chip: ESP32-C6-WROOM-1
UART console: 115200 8N1 on TX/RX header

OEM boot log (normal SPI boot):
- Project: smarthome_device_airtap v2.1 (ESP-IDF v5.4.1, Apr 9 2026 20:33:54)
- Versions reported: hw 3.0 / sw 3.0.15
- Secure boot: RSA-PSS enabled (verification succeeded)
- Flash encryption: DEVELOPMENT (not secure)
- Power-on path: BLE / Blufi provisioning
- MAC seen: a0:f2:62:71:bc:e4
- Input GPIOs (pull-up): 5, 6, 7, 21, 22
- Output GPIOs: 8, 10, 11, 15, 18, 19, 23, 20, 1, 4

Download mode (UART dump/flash):
1. Hold IO9 to GND
2. Pulse EN low then high
3. Release IO9
4. esptool --port /dev/ttyUSB0 --chip esp32c6 --before no-reset --after no-reset ...
   Use baud <= 460800 on this CH340; 921600 corrupts the link.

Flash dump:
- oem-firmware-full.bin — full 8MB SPI dump (ciphertext; flash encryption enabled)
- SHA256: ff35eccd4f6936a8b0c3f98f9bc9d93c65edfefdc661ccb78f1077a39ab8743d
- oem-partition-table.bin — 0x8000..0x8C00 (also ciphertext)
- oem-security-info.txt / oem-efuse-summary.txt
- Flash size: 8MB (GD/c8 4017)
- SPI_BOOT_CRYPT_CNT=0b111 (encryption permanently on)
- RD_DIS blocks reading KEY0/KEY1 (flash encrypt key not readable via espefuse)
- Secure boot digests present; JTAG permanently disabled; DIS_DOWNLOAD_ICACHE=1
- DIS_DOWNLOAD_MANUAL_ENCRYPT=0 (download-mode encrypt still allowed)


Manual Pin traces:

IO0 - R29 -> Q5 -> ?? Maybe Buzzer
IO1 - R6/R34 -> ??
IO2 - NTC
IO3 - ??
IO4 - ??
IO5 - TTP223 TOUCH 1
IO6 - TTP223 TOUCH 2
IO7 - TTP223 TOUCH 3
IO8 - Q4(Transistor) -> CS1621(/CS)
IO9 - PAD9
IO10 - Q13(Transistor) -> CS1621(/WR)
IO11 - Q14(Transistor) -> CS1621(DATA)
IO12 - ??
IO13 - IRM3638 IR
IO15 - Motor Pin 4 (ULN2003A)
IO18 - Motor Pin 3 (ULN2003A)
IO19 - Motor Pin 2 (ULN2003A)
IO20 - ??
IO21 - TTP223 TOUCH 4
IO22 - TTP223 TOUCH 5
IO23 - Motor Pin 1 (ULN2003A)


Still missing pin matching:

PWM Control, Likely one of Q1, Q2 or Q5 Transistors
Piezzo Likely Q2 or Q5 transistors
Backlight?



CS1621:

1 - LED 1.2
2 - LED 1.3
3 - LED 1.4
4 - LED 1.5
5 - LED 1.6
6 - LED 1.7
7 - LED 1.8
8 - LED 1.9
9 - LED 1.10
10 - /CS
11 - /WR
12 - DATA
13 - GND
14 - VLCD ?
15 - VDD
16 - LED 1.14
17 - LED 1.13
18 - LED 1.12
19 - LED 1.11
20 - NC
21 - NC
22 - NC
23 - NC
24 - LED 1.1


Serial log confirmation (2026-07-13 capture → serial-logs/latest.log):
GPIO init vs traces:
- Confirmed inputs (pull-up): IO5/6/7/21/22 → matches TTP223 on 5/6/7/22; IO21 still unknown but is a 5th touch/key input in firmware
- Confirmed outputs: IO8/10/11 → matches CS1621 /CS /WR DATA
- Confirmed outputs: IO15/18/19/23 → matches ULN2003A motor pins
- IO2: neither in nor out (ADC-style) + Curve Fitting calibration immediately after → supports NTC on IO2
- Confirmed outputs still ambiguous: IO1, IO4, IO20 (buzzer / PWM / backlight candidates)
- IO0 never appears in gpio init log (may still be buzzer path, just not logged here)
- RMT RX channel enabled → supports IR RX (IRM3638 / IO13); pin number not printed
Runtime interaction in same capture:
- ct_ir_manage power 0→1 (IR remote power)
- Key_Func "Up Key" twice → ipcd_set_speed 2 then 3

Custom firmware / pinprobe:
- esphome-pinprobe.yaml cycles GPIO0(PWM) / 1(2kHz) / 4(4kHz) / 20 / 3 / 12
- FLASHED 2026-07-13 with --encrypt: write succeeded, boot FAILED
  - "Image not V2 signed?" / "RSA-PSS secure boot verification failed" bootloop
- OEM restored via firmware-dump/oem-firmware-full.bin (hash verified)
- End-goal ESPHome on THIS module is blocked without AC Infinity signing key
- Path forward: replace ESP32-C6-WROOM-1 with an unlocked module, then flash ESPHome
