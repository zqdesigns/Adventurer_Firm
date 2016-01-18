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
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "msp430.h"
#include "glcdfont.c"

#include "Adafruit_SSD1306.h"

// the memory buffer for the LCD
#pragma SET_DATA_SECTION(".fram_display")
uint8_t buffer[SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8];
#pragma SET_DATA_SECTION()

Adafruit_SSD1306::Adafruit_SSD1306(int16_t w, int16_t h):
  WIDTH(w), HEIGHT(h)
{
  _width    = WIDTH;
  _height   = HEIGHT;
  rotation  = 0;
  cursor_y  = cursor_x    = 0;
  textsize  = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap      = true;
}

void Adafruit_SSD1306::ssd1306_command(uint8_t c) { 
  uint8_t control = 0x00;   // Co = 0, D/C = 0
    
  while (UCB0CTLW0 & UCTXSTP);
  UCB0I2CSA = _i2caddr;                         
  UCB0CTLW0 |= UCTXSTT ;              // I2C start
  while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
  UCB0TXBUF = control;         	    //Port Configuration
  while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
  UCB0TXBUF = c;
  while (~UCB0IFG&UCTXIFG0);
  UCB0CTLW0 |= UCTXSTP;
}

void Adafruit_SSD1306::ssd1306_data(uint8_t c) {
  uint8_t control = 0x40;   // Co = 0, D/C = 1
  while (UCB0CTLW0 & UCTXSTP);
  UCB0I2CSA = _i2caddr;                         
  UCB0CTLW0 |= UCTXSTT ;              // I2C start
  while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
  UCB0TXBUF = control;         	    //Port Configuration
   while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
  UCB0TXBUF = c;
  while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
  UCB0CTLW0 |= UCTXSTP;
}

void Adafruit_SSD1306::begin(uint8_t vccstate, uint8_t i2caddr) {
  _i2caddr = i2caddr;

  P1SEL1 |= BIT6+BIT7;
  P1SEL0 &= ~(BIT6+BIT7);

  UCB0CTLW0 = UCSWRST;
  UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC + UCSSEL_2 + UCTR;      // I2C master mode Use SMCLK, keep SW reset
  UCB0BRW = 0x0014;                       // 400 kHz
  UCB0I2CSA = _i2caddr;        // address
  UCB0CTLW0 &= ~UCSWRST;
  __delay_cycles(8000000);

  #if defined SSD1306_128_32
    // Init sequence for 128x32 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x1F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x10); }
    else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
    else 
      { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
  #endif

  #if defined SSD1306_128_64
    // Init sequence for 128x64 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    ssd1306_command(0x80);                                  // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
    ssd1306_command(0x3F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    ssd1306_command(0x0);                                   // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x10); }
    else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
    ssd1306_command(0x00);                                  // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x9F); }
    else 
      { ssd1306_command(0xCF); }
    ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
    else 
      { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
  #endif
}

void Adafruit_SSD1306::sleep(uint8_t c) {
  if (c) {
    ssd1306_command(SSD1306_DISPLAYOFF);
  } else {
    ssd1306_command(SSD1306_DISPLAYON);
  }
}

void Adafruit_SSD1306::invertDisplay(uint8_t i) {
  if (i) {
    ssd1306_command(SSD1306_INVERTDISPLAY);
  } else {
    ssd1306_command(SSD1306_NORMALDISPLAY);
  }
}

// Return the size of the display (per current rotation)
int16_t Adafruit_SSD1306::width(void) {
  return _width;
}
 
int16_t Adafruit_SSD1306::height(void) {
  return _height;
}

uint8_t Adafruit_SSD1306::getRotation(void) {
  return rotation;
}
  
void Adafruit_SSD1306::setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void Adafruit_SSD1306::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void Adafruit_SSD1306::setTextColor(uint16_t c) {
  // For 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

void Adafruit_SSD1306::setTextColor(uint16_t c, uint16_t b) {
  textcolor   = c;
  textbgcolor = b; 
}

void Adafruit_SSD1306::setTextWrap(uint8_t w) {
  wrap = w;
}
  
void Adafruit_SSD1306::setRotation(uint8_t x) {
  rotation = (x & 3);
  switch(rotation) {
   case 0:
   case 2:
    _width  = WIDTH;
    _height = HEIGHT;
    break;
   case 1:
   case 3:
    _width  = HEIGHT;
    _height = WIDTH;
    break;
  }
}  

// the most basic function, set a single pixel
void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }  

  // x is which column
  if (color == WHITE) 
  {
    buffer[x+ (y/8)*SSD1306_LCDWIDTH] |= 1<<(y%8);  
  }
  else
    buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1<<(y%8)); 
}

// Bresenham's algorithm - thx wikpedia
void Adafruit_SSD1306::drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void Adafruit_SSD1306::drawFastVLine(int16_t x, int16_t y,
				 int16_t h, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void Adafruit_SSD1306::drawFastHLine(int16_t x, int16_t y,
				 int16_t w, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

// Draw a rectangle
void Adafruit_SSD1306::drawRect(int16_t x, int16_t y,
			    int16_t w, int16_t h,
			    uint16_t color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void Adafruit_SSD1306::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
			    uint16_t color) {
  // Update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}
  
// Draw a character
void Adafruit_SSD1306::drawChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t color, uint16_t bg, uint8_t size) {

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

// Draw a character
void Adafruit_SSD1306::drawBigChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t color, uint16_t bg) {

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 11) < 0) 	  	  || // Clip left
     ((y + 15) < 0))             // Clip top
    return;

  for (int8_t i=0; i<12; i++ ) {
    uint16_t line;
    if (i >= 10)
      line = 0x0;
    else
      line = big_font[(c*10)+i];
    for (int8_t j = 0; j<16; j++) {
      if (line & 0x1) {
          drawPixel(x+i, y+j, color);
      } else if (bg != color) {
          drawPixel(x+i, y+j, bg);
      }
      line >>= 1;
    }
  }
}

void Adafruit_SSD1306::write(char *c) {
  for (uint8_t i = 0; i < strlen(c); i ++)
  {
    if (c[i] == '\n') {
      cursor_y += textsize*8;
      cursor_x  = 0;
    } else if (c[i] == '\r') {
      // skip em
    } else {
      drawChar(cursor_x, cursor_y, c[i], textcolor, textbgcolor, textsize);
      cursor_x += textsize*6;
      if (wrap && (cursor_x > (_width - textsize*6))) {
        cursor_y += textsize*8;
        cursor_x = 0;
      }
    }  
  }
}

void Adafruit_SSD1306::writeBig(char *c) {
  uint8_t index;
  for (uint8_t i = 0; i < strlen(c); i ++)
  {
    if ((c[i] >= '0')&&(c[i] <= '9'))
      index = c[i] - 0x30;
    else if (c[i] == 'C')
      index = 10;
    else if (c[i] == 'F')
      index = 11;
    else if (c[i] == '�')
      index = 12;
    else if (c[i] == 't')
      index = 13;
    else if (c[i] == 'm')
      index = 14;
    else if (c[i] == '-')
      index = 15;
    drawBigChar(cursor_x, cursor_y, index, textcolor, textbgcolor);
    cursor_x += 12;
    if (wrap && (cursor_x > (_width - 12)))
    {
        cursor_y += 16;
        cursor_x = 0;
    }
  }
}

void Adafruit_SSD1306::display() {
  uint16_t MAX;
  
  ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0
  
  MAX = SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8;
  for (uint16_t i=0; i<MAX; i++) {
  // send a bunch of data in one xmission
      while (UCB0CTLW0 & UCTXSTP);
      UCB0I2CSA = _i2caddr;                         
      UCB0CTLW0 |= UCTXSTT ;              // I2C start
      while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
      UCB0TXBUF = 0x40; 
      while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
  
      for (uint8_t x=0; x<16; x++) {
	    UCB0TXBUF = buffer[i];
        while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
	    i++;
      }
      i--;
      UCB0CTLW0 |= UCTXSTP;
    }
}

// clear everything
void Adafruit_SSD1306::clearDisplay(void) {
  uint16_t MAX = SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8;
  for (uint16_t i=0; i<MAX; i++) {
    buffer[i] = 0x00;
  }
}

void Adafruit_SSD1306::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}

// Draw a circle outline
void Adafruit_SSD1306::drawCircle(int16_t x0, int16_t y0, int16_t r,
    uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void Adafruit_SSD1306::drawCircleHelper( int16_t x0, int16_t y0,
               int16_t r, uint8_t cornername, uint16_t color) {
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void Adafruit_SSD1306::fillCircle(int16_t x0, int16_t y0, int16_t r,
			      uint16_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void Adafruit_SSD1306::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Draw a triangle
void Adafruit_SSD1306::drawTriangle(int16_t x0, int16_t y0,
				int16_t x1, int16_t y1,
				int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void Adafruit_SSD1306::fillTriangle ( int16_t x0, int16_t y0,
				  int16_t x1, int16_t y1,
				  int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
}

// Draw a rounded rectangle
void Adafruit_SSD1306::drawRoundRect(int16_t x, int16_t y, int16_t w,
  int16_t h, int16_t r, uint16_t color) {
  // smarter version
  drawFastHLine(x+r  , y    , w-2*r, color); // Top
  drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
  drawFastVLine(x    , y+r  , h-2*r, color); // Left
  drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r    , y+r    , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void Adafruit_SSD1306::fillRoundRect(int16_t x, int16_t y, int16_t w,
				 int16_t h, int16_t r, uint16_t color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}
