# RC Servo Pulse Monitor

Display the pulse length of a RC servo output.
The pulse length and its corresponding percentage value (-100% ... +100%) is
shown on a common 128 x 32 I2C OLED display (with SSD1306 controller).

## Wiring scheme

| OLED | Arduino |
|------|---------|
| SDA  | A4      |
| SCL  | A5      |
| VCC  | +5V     |
| GND  | GND     |

| Servo | Arduino |
|-------|---------|
| Sig.  | 11      |
| VCC   | +5V     |
| GND   | GND     |

Tipp: Pin 11 is also available on the ISP header.

## Dependencies

Requires the following Arduino libraries:
- Adafruit_GFX
- Adafruit_SSD1306
