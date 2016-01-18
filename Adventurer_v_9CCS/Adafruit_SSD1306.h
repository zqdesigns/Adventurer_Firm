/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#include <stdint.h>

#define BLACK 0
#define WHITE 1

#define SSD1306_I2C_ADDRESS   0x3C	// 011110+SA0+RW - 0x3C or 0x3D
// Address for 128x32 is 0x3C
// Address for 128x32 is 0x3D (default) or 0x3C (if SA0 is grounded)

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SSD1306_128_64  128x64 pixel display

    SSD1306_128_32  128x32 pixel display

    You also need to set the LCDWIDTH and LCDHEIGHT defines to an 
    appropriate size

    -----------------------------------------------------------------------*/
   #define SSD1306_128_64
//   #define SSD1306_128_32
/*=========================================================================*/

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH              128
  #define SSD1306_LCDHEIGHT             64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH              128
  #define SSD1306_LCDHEIGHT             32
#endif

#define SSD1306_SETCONTRAST             0x81
#define SSD1306_DISPLAYALLON_RESUME     0xA4
#define SSD1306_DISPLAYALLON            0xA5
#define SSD1306_NORMALDISPLAY           0xA6
#define SSD1306_INVERTDISPLAY           0xA7
#define SSD1306_DISPLAYOFF              0xAE
#define SSD1306_DISPLAYON               0xAF

#define SSD1306_SETDISPLAYOFFSET        0xD3
#define SSD1306_SETCOMPINS              0xDA

#define SSD1306_SETVCOMDETECT           0xDB

#define SSD1306_SETDISPLAYCLOCKDIV      0xD5
#define SSD1306_SETPRECHARGE            0xD9

#define SSD1306_SETMULTIPLEX            0xA8

#define SSD1306_SETLOWCOLUMN            0x00
#define SSD1306_SETHIGHCOLUMN           0x10

#define SSD1306_SETSTARTLINE            0x40

#define SSD1306_MEMORYMODE              0x20

#define SSD1306_COMSCANINC              0xC0
#define SSD1306_COMSCANDEC              0xC8

#define SSD1306_SEGREMAP                0xA0

#define SSD1306_CHARGEPUMP              0x8D

#define SSD1306_EXTERNALVCC             0x1
#define SSD1306_SWITCHCAPVCC            0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL         0x2F
#define SSD1306_DEACTIVATE_SCROLL       0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL  0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A
      
#define swap(a, b) { int16_t t = a; a = b; b = t; }

class Adafruit_SSD1306 {
 public:
  Adafruit_SSD1306(int16_t w, int16_t h);

  void begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = SSD1306_I2C_ADDRESS);
  void ssd1306_command(uint8_t c);
  void ssd1306_data(uint8_t c);

  void clearDisplay(void);
  void sleep(uint8_t c);
  void invertDisplay(uint8_t i);
  void display();

  int16_t width(void);
  int16_t height (void);
  uint8_t getRotation(void);
  void 
  setCursor(int16_t x, int16_t y),
  setTextColor(uint16_t c),
  setTextColor(uint16_t c, uint16_t bg),
  setTextSize(uint8_t s),
  setTextWrap(uint8_t w),
  setRotation(uint8_t r),
  drawPixel(int16_t x, int16_t y, uint16_t color),
  drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
  fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
  drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
  drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
  drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
  write(char *c),
  writeBig(char *c),
  drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,uint16_t bg, uint8_t size),
  drawBigChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg),
  fillScreen(uint16_t color),
  drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
  drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color),
  fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
  fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color),
  drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),
  fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),
  drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
  fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  int8_t getCursorX() const       { return cursor_x;  }
  int8_t getCursorY() const       { return cursor_y;  }

 private:
  int8_t _i2caddr;
  
 protected:
  const uint8_t
    WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
  uint8_t
    _width, _height, // Display w/h as modified by current rotation
    cursor_x, cursor_y;
  uint16_t
    textcolor, textbgcolor;
  uint8_t
    textsize,
    rotation;
  uint8_t
    wrap; // If set, 'wrap' text at right edge of display 
};
