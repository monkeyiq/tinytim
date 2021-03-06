#include <Arduino.h>
#include <Wire.h>
#include <MultiLCD.h>
#include <Streaming.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <Adafruit_NeoPixel.h>

byte neopixel_pin = 8;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, neopixel_pin, NEO_GRB + NEO_KHZ800);

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

static const PROGMEM uint8_t smile[48 * 48 / 8] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xF8,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xC0,0xF0,0xFC,0xFE,0xFF,0xFF,0xFF,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0xFF,0xFF,0xFF,0xFE,0xFC,0xF0,0xC0,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x30,0xF8,0xF8,0xF8,0xF8,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF8,0xF8,0xFC,0xF8,0x30,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
0x00,0x03,0x0F,0x3F,0x7F,0xFF,0xFF,0xFF,0xFC,0xF8,0xF0,0xE1,0xC7,0x87,0x0F,0x1F,0x3F,0x3F,0x3E,0x7E,0x7C,0x7C,0x7C,0x78,0x78,0x7C,0x7C,0x7C,0x7E,0x3E,0x3F,0x3F,0x1F,0x0F,0x87,0xC7,0xE1,0xF0,0xF8,0xFC,0xFF,0xFF,0xFF,0x7F,0x3F,0x0F,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x1F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
};



byte pin_m1in1  = A2;
byte pin_m1in2  = A3;
byte pin_m1pwm  = 6;
int  m1pwmvalue = 0;
byte pin_m2in1  = A0;
byte pin_m2in2  = A1;
byte pin_m2pwm  = 5;
int  m2pwmvalue = 0;



/* byte pin_m1in1  = A0; */
/* byte pin_m1in2  = A1; */
/* byte pin_m1pwm  = 6; */
/* int  m1pwmvalue = 0; */
/* byte pin_m2in1  = A2; */
/* byte pin_m2in2  = A3; */
/* byte pin_m2pwm  = 5; */
/* int  m2pwmvalue = 0; */


//////////////////////////////

void display_smile()
{
	lcd.clear();
	lcd.draw(smile, 40, 8, 48, 48);
}

void display_message()
{
  	lcd.clear();

    lcd.setCursor(0, 0);
	lcd.setFont(FONT_SIZE_LARGE);
	lcd.print("My name is");
	lcd.setCursor(0, 2);
	lcd.setFont(FONT_SIZE_XLARGE);
	lcd.print("Tiny Tim! v0.2");

	lcd.setCursor(0, 6);
	lcd.setFont(FONT_SIZE_MEDIUM);
	lcd.print("Where's Terry?");
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
    
    pixels.begin();
    pixels.show();
    pixels.setPixelColor(0, 200, 0, 0 );
    pixels.setPixelColor(1, 200, 0, 0 );
    pixels.show();

}


int displayiter = 0;

/**
 * pin is the pin to pwm the signal on,
 * 
 * speed is between -255 and 255 to be forward/reverse at up to full
 * speed in each direction.
 */
void setMotorSpeed( byte pin, int speed )
{
    int pin_in1 = (pin == pin_m1pwm) ? pin_m1in1 : pin_m2in1;
    int pin_in2 = (pin == pin_m1pwm) ? pin_m1in2 : pin_m2in2;

    if( speed < -255 )
        speed = -255;
    if( speed > 255 )
        speed = 255;
    
    if( speed >= 0 ) 
    {
        digitalWrite( pin_in2, LOW );
        digitalWrite( pin_in1, HIGH );
    }
    else
    {
        digitalWrite( pin_in2, HIGH );
        digitalWrite( pin_in1, LOW );
    }
    analogWrite( pin, abs(speed) );
}

void setPixelColor( byte pix, int pwm, byte g = 0, byte b = 0 )
{
    byte r = 200;

    
    if( pwm >= 0 ) 
    {
        if( pix == 0 )
            printf("setPixelColor() p %d g %d b %d\n", pwm, g, b );
        pixels.setPixelColor(pix, r, g, b );
    }
    else
    {
        int rr = r;
        int gg = 0;
        int bb = 0;
        if( pix == 0 ) 
        {
            rr = 254-bb;
            bb = 254-g;
            gg = bb;
        }
        if( pix == 1 )
        {
            rr = 254-bb;
            gg = 254-b;
            bb = gg;
        }
        
        
        /* if( pix == 0 ) */
        /*     printf("setPixelColor() p %d g %d b %d  gg:%d\n", pwm, g, b, gg ); */

        
        pixels.setPixelColor(pix, r, gg, bb );
    }
    

}


void loop()
{

    // if there is data ready
    if ( radio.available() )
    {
//        Serial << "packet!!!!..." << endl;
  
        // Dump the payloads until we've gotten everything
        unsigned long got_time;
        bool done = false;
        while (!done)
        {
            struct radiomsg msg;
            msg.type = 0;
            // Fetch the payload, and see if this was the last one.
            done = radio.read( &msg, sizeof(radiomsg) );

//            printf("Got payload done:%d %d  h:%d v:%d...\n\r", done, msg.type, msg.joyxy.h, msg.joyxy.v );
            if( msg.type == RMSG_JOYXY ) 
            {

                m1pwmvalue = msg.joyxy.h;
                if( m1pwmvalue > 510 )
                    m1pwmvalue = 510;
                m1pwmvalue -= 255;
//                m1pwmvalue /= 2;
                printf("*** m1pwmvalue:%d\n",m1pwmvalue);

                if( msg.joyxy.h < 512 ) 
                {
                    m2pwmvalue = 510;
                }
                else
                    m2pwmvalue = 1024 - msg.joyxy.h;
                m2pwmvalue -= 255;
//                m2pwmvalue /= 2;

                // if the joystick is not forward enough, stop
                byte deadZone = 50;
                if( msg.joyxy.v < (512 + deadZone) && msg.joyxy.v > (512 - deadZone) )
                {
                    m1pwmvalue = 0;
                    m2pwmvalue = 0;
                }
                // reverse.
                if( msg.joyxy.v  < (512 - deadZone))
                {
                    m1pwmvalue *= -1;
                    m2pwmvalue *= -1;
                }
                
                setMotorSpeed( pin_m1pwm, m1pwmvalue );
                setMotorSpeed( pin_m2pwm, m2pwmvalue );
                
                int m1led = m1pwmvalue % 255;
                int m2led = m2pwmvalue % 255;
                if( !m1pwmvalue )
                  m1led = 0;
                if( !m2pwmvalue )
                  m2led = 0;      
                if( m1pwmvalue > 254 )
                  m1led = 254;
                if( m2pwmvalue > 254 )
                  m2led = 254;

                setPixelColor( 0, m1pwmvalue, m1led, 0 );
                setPixelColor( 1, m2pwmvalue, 0, m2led );
                pixels.show();                
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
}


