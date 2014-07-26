/*************************************************************************
* Demo sketch of Arduino Text Display Library for Multiple LCDs
* Distributed under GPL v2.0
* Copyright (c) 2013 Stanley Huang <stanleyhuangyc@live.com>
* All rights reserved.
*************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <MultiLCD.h>
#include <Streaming.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

enum { RMSG_JOYXY = 1 } RADIOMSGTYPES;
struct radiomsg {
  int type;
  union {
    struct {
      int h;
      int v;
    } joyxy;
    int dummy;
  };
};

// 1306 is itead
LCD_SSD1306 lcd; /* for SSD1306 OLED module */
//LCD_PCD8544 lcd; /* for LCD4884 shield or Nokia 5100 screen module */
//LCD_1602 lcd; /* for LCD1602 shield */
//LCD_ZTOLED lcd; /* for ZT OLED module */
//LCD_ILI9325D lcd; /* for Itead 2.8" TFT shield */

static const PROGMEM uint8_t smile[48 * 48 / 8] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xF8,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xC0,0xF0,0xFC,0xFE,0xFF,0xFF,0xFF,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0xFF,0xFF,0xFF,0xFE,0xFC,0xF0,0xC0,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x30,0xF8,0xF8,0xF8,0xF8,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF8,0xF8,0xFC,0xF8,0x30,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
0x00,0x03,0x0F,0x3F,0x7F,0xFF,0xFF,0xFF,0xFC,0xF8,0xF0,0xE1,0xC7,0x87,0x0F,0x1F,0x3F,0x3F,0x3E,0x7E,0x7C,0x7C,0x7C,0x78,0x78,0x7C,0x7C,0x7C,0x7E,0x3E,0x3F,0x3F,0x1F,0x0F,0x87,0xC7,0xE1,0xF0,0xF8,0xFC,0xFF,0xFF,0xFF,0x7F,0x3F,0x0F,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x1F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
};



byte pin_m1in1 = A0;
byte pin_m1in2 = A1;
byte pin_m1pwm = 5;
int m1pwmvalue = 0;
byte pin_m2in1 = A2;
byte pin_m2in2 = A3;
byte pin_m2pwm = 6;
int m2pwmvalue = 0;


//////////////////////////////

void display_smile()
{
	lcd.clear();
	lcd.draw(smile, 40, 8, 48, 48);
}

void display_message()
{
  	lcd.clear();
//	lcd.setCursor(0, 0);
//	lcd.setFont(FONT_SIZE_SMALL);
//	lcd.print("Hello, world!");

	lcd.setCursor(0, 0);
	lcd.setFont(FONT_SIZE_LARGE);
	lcd.print("My name is");
	lcd.setCursor(0, 2);
	lcd.setFont(FONT_SIZE_XLARGE);
	lcd.print("Tiny Tim! v0.1");

	lcd.setCursor(0, 6);
	lcd.setFont(FONT_SIZE_MEDIUM);
	lcd.print("Where's Terry?");


//	lcd.setCursor(64, 3);
//	lcd.setFont(FONT_SIZE_MEDIUM);
//	lcd.printLong(12345678);

//	lcd.setCursor(0, 4);
//	lcd.setFont(FONT_SIZE_LARGE);
//	lcd.printLong(12345678);

}

//////////////////////////////


void setup()
{
  Serial.begin(57600);
//  while (!Serial) ;
      printf_begin();

  Serial.println("starting up...");

	lcd.begin();
  Serial.println("starting radio...");
        radio.begin();
        radio.setRetries(15,15);
       
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
      
        radio.startListening();
        radio.printDetails();



        pinMode( pin_m1in1, OUTPUT );
        pinMode( pin_m1in2, OUTPUT );
        pinMode( pin_m1pwm, OUTPUT );
        digitalWrite( pin_m1in2, LOW );
        digitalWrite( pin_m1in1, HIGH );
        analogWrite( pin_m1pwm, m1pwmvalue );

        pinMode( pin_m2in1, OUTPUT );
        pinMode( pin_m2in2, OUTPUT );
        pinMode( pin_m2pwm, OUTPUT );
        digitalWrite( pin_m2in2, LOW );
        digitalWrite( pin_m2in1, HIGH );
        analogWrite( pin_m2pwm, m2pwmvalue );

  Serial.println("setup complete!");
  display_smile();
}


int displayiter = 0;

void loop()
{

    // if there is data ready
    if ( radio.available() )
    {
      Serial << "packet!!!!..." << endl;
  
      // Dump the payloads until we've gotten everything
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        struct radiomsg msg;
        msg.type = 0;
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &msg, sizeof(radiomsg) );

        printf("Got payload done:%d %d  h:%d v:%d...\n\r", done, msg.type, msg.joyxy.h, msg.joyxy.v );
        if( done ) 
        {

          m1pwmvalue = msg.joyxy.h;
          if( m1pwmvalue > 510 )
              m1pwmvalue = 510;
          m1pwmvalue /= 2;
          printf("*** m1pwmvalue:%d\n",m1pwmvalue);

          if( msg.joyxy.h < 512 ) 
          {
            m2pwmvalue = 510;
          }
          else
            m2pwmvalue = 1024 - msg.joyxy.h;
          m2pwmvalue /= 2;

          if( msg.joyxy.v < 512+ 50 )
          {
            m1pwmvalue = 0;
            m2pwmvalue = 0;
          }          
          analogWrite( pin_m1pwm, m1pwmvalue );
          analogWrite( pin_m2pwm, m2pwmvalue );
        }


	// Delay just a little bit to let the other unit
	// make the transition to receiver
	delay(20);
      }

    }

    int displayiterold = displayiter;
    int displayitermax = 2;
    unsigned long m = millis();
    if( m % 2000 == 0 )
    {
      displayiter++;
    }
    if( displayiterold != displayiter )
    {
       if( displayiter % displayitermax == 0 )
       {
         display_message();
       }
       if( displayiter % displayitermax == 1 )
       {
         display_smile();
       }
    }

//    Serial << "looping..." << endl;
//    delay(300);

  
#if 0  
	lcd.clear();
	lcd.draw(smile, 40, 8, 48, 48);
	delay(1000);

	lcd.clear();
//	lcd.setCursor(0, 0);
//	lcd.setFont(FONT_SIZE_SMALL);
//	lcd.print("Hello, world!");

	lcd.setCursor(0, 0);
	lcd.setFont(FONT_SIZE_LARGE);
	lcd.print("My name is");
	lcd.setCursor(0, 2);
	lcd.setFont(FONT_SIZE_XLARGE);
	lcd.print("Tiny Tim! v0.1");

	lcd.setCursor(0, 6);
	lcd.setFont(FONT_SIZE_MEDIUM);
	lcd.print("Where's Terry?");


//	lcd.setCursor(64, 3);
//	lcd.setFont(FONT_SIZE_MEDIUM);
//	lcd.printLong(12345678);

//	lcd.setCursor(0, 4);
//	lcd.setFont(FONT_SIZE_LARGE);
//	lcd.printLong(12345678);


	delay(3000);
#endif

}
