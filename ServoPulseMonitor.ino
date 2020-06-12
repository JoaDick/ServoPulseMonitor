
/*******************************************************************************

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

*******************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//------------------------------------------------------------------------------

// Read servo pulses from that pin.
#define SERVO_IN 11

// LED for indicating invalid pulses.
#define FAIL_LED LED_BUILTIN

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//------------------------------------------------------------------------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

//------------------------------------------------------------------------------

// Pulse length for -100%
const int16_t t_min = 1000;

// Pulse length for +100%
const int16_t t_max = 2000;

// Pulse length for 0%
const int16_t t_0 = (t_min + t_max) / 2;

// X drawing coordinate for 0%
const int16_t x_0 = display.width() / 2;

// X drawing coordinate for -100%
const int16_t x_min = x_0 - 50;

// X drawing coordinate for +100%
const int16_t x_max = x_min + 100;

// Helper for smoothing pulse value.
float filteredPulse = t_0;

//------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  pinMode(SERVO_IN, INPUT_PULLUP);
  pinMode(FAIL_LED, OUTPUT);
  digitalWrite(FAIL_LED, 1);

  delay(600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("Failed to initialize display."));
    for (;;)
    {
      delay(100);
      digitalWrite(FAIL_LED, 0);
      delay(100);
      digitalWrite(FAIL_LED, 1);
    }
  }

  //display.display();
  //delay(500);
}

//------------------------------------------------------------------------------

void loop()
{
  noInterrupts();
  const float currentPulse = pulseIn(SERVO_IN, HIGH, 50 * 1000ul);
  interrupts();

  // valid pulse?
  if (currentPulse <= 2500 &&
      currentPulse >= 500)
  {
    filteredPulse = filteredPulse + 0.9 * (currentPulse - filteredPulse);
    int16_t pulse = int16_t(filteredPulse + 0.5);

    digitalWrite(FAIL_LED, 0);
    drawPulse(pulse);
    Serial.println(pulse);
  }
  // invalid pulse
  else
  {
    filteredPulse = t_0;

    digitalWrite(FAIL_LED, 1);
    drawPulse(0);
    Serial.println(0);
  }
}

//------------------------------------------------------------------------------

void drawPulse(int16_t length_us)
{
  const int16_t percent = map(length_us, t_min, t_max, -100, 100);

  int16_t y0 = 0;

  display.clearDisplay();
  display.setTextColor(WHITE);
  drawPulseLimitText(y0);
  y0 += drawPulseText(y0, length_us) + 2;

#if (0)
  int16_t height = 23;
  drawScale(y0, height);
  if (length_us)
  {
    drawBar(y0 + 1, height - 2, percent);
  }
  display.setTextColor(INVERSE);
  drawPercentText(y0 + 5, percent);
#else
  int16_t height = 8;
  drawScale(y0, height);
  if (length_us)
  {
    drawBar(y0 + 1, height - 2, percent);
  }
  y0 += height + 1;
  // drawPercentLimitText(y0);
  drawPercentText(y0, percent);
#endif

  display.display();
}

//------------------------------------------------------------------------------

int16_t drawPulseText(int16_t y0, int16_t us)
{
  String text(us);
  if (us == 0)
    text = "-";

  display.setTextSize(1);
  display.setCursor(x_0 + 1 - (text.length() * 6) / 2, y0);
  display.print(text);

  return 7;
}

//------------------------------------------------------------------------------

int16_t drawPulseLimitText(int16_t y0)
{
  display.setTextSize(1);
  display.setCursor(x_min + 1 - 2 * 6, y0);
  display.print(1000);
  display.setCursor(x_max + 1 - 2 * 6, y0);
  display.print(2000);

  return 7;
}

//------------------------------------------------------------------------------

int16_t drawPercentText(int16_t y0, int16_t percent)
{
  String text(percent);

  if (percent <= 200 &&
      percent >= -200)
  {
    if (percent == 0)
    {
      text = ' ' + text;
    }
    else if (percent > 0)
    {
      text = '+' + text;
    }
    text += '%';
  }
  else
  {
    text = "No Pulse";
  }

  display.setTextSize(2);
  display.setCursor(x_0 + 1 - (text.length() * 12) / 2, y0);
  display.print(text);

  return 15;
}

//------------------------------------------------------------------------------

int16_t drawPercentLimitText(int16_t y0)
{
  display.setTextSize(1);
  display.setCursor(x_min + 1 - 2 * 6, y0);
  display.print("-100");
  display.setCursor(x_max + 1 - 2 * 6, y0);
  display.print("+100");

  return 7;
}

//------------------------------------------------------------------------------

int16_t drawBar(int16_t y0, int16_t height, int16_t percent)
{
  const int16_t x = getX(percent);
  int16_t width = x - x_0;
  if (width > 0)
  {
    display.fillRect(x_0 + 1, y0, width, height, INVERSE);
  }
  else if (width < 0)
  {
    width = -width;
    display.fillRect(x_0 - width, y0, width, height, INVERSE);
  }
  else
  {
    display.drawLine(x_0, y0, x_0, y0 + height, WHITE);
  }
  return height;
}

//------------------------------------------------------------------------------

int16_t drawScale(int16_t y0, int16_t height)
{
#if (1)
  const int16_t width = 1 + x_max - x_min + 2;
  const int16_t y_max = y0 + height - 1;

  display.drawLine(x_0, y0, x_0, y_max, WHITE);
  display.drawRect(x_min - 1, y0, width, height, WHITE);

  return height;

#else
  const int16_t y_min = y0;
  const int16_t y_max = y0 + height - 1;

  display.drawLine(x_0, y_min, x_0, y_max, WHITE);
  display.drawLine(x_min, y_min, x_min, y_max, WHITE);
  display.drawLine(x_max, y_min, x_max, y_max, WHITE);
  //display.drawLine(x_min + 1, y_min, x_max - 1, y_min, INVERSE);
  return height;
#endif
}

//------------------------------------------------------------------------------

int16_t getX(int16_t percent)
{
  return map(percent, -100, 100, x_min, x_max);
}

//------------------------------------------------------------------------------
