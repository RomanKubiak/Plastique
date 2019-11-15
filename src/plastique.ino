#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Adafruit_Neopixel.h>
#include <EEPROM.h>

#define PIN_ENCODER_A   7
#define PIN_ENCODER_B   8
#define PIN_ENCODER_BTN 12
#define PIN_PIXELS		4
#define PIN_PIXELS_INT	6
#define PIN_LED_BLUE 9
#define PIN_LED_GREEN 10
#define PIN_LED_RED 11

#define PIXELS_NUM 		16
#define PIXELS_SECTIONS 4

#include "encoder.h"

#define NEO_COLOR_BLACK 	pixels->Color(0,0,0)
#define NEO_COLOR_WHITE 	pixels->Color(2,2,2)
#define NEO_COLOR_GREEN 	pixels->Color(0,32,0)
#define NEO_COLOR_RED	 	pixels->Color(32,0,0)

#define NEO_COLOR_RAND(i,j)	pixels->Color(random(i,j), random(i,j), random(i,j))
enum t_mode {
	color,position_select_start,position_select_end
};

ClickEncoder *encoder;
Adafruit_NeoPixel *pixels, *pixelsInternal;
int16_t last, value;

t_mode mode_selected = color;
uint32_t blink_timer = 0;
bool blinkFlag = false;
int led_start = 0, led_end = PIXELS_NUM-1;
uint32_t led_start_color, led_end_color;

void blink()
{
	blinkFlag = !blinkFlag;
	
	if ((mode_selected == position_select_start) || (mode_selected == position_select_end))
	{
		pixels->setPixelColor(led_start, blinkFlag ? NEO_COLOR_GREEN : led_start_color);
		pixels->show();
		pixels->setPixelColor(led_end, blinkFlag ? NEO_COLOR_RED : led_end_color);
		pixels->show();
		if (mode_selected == position_select_start)
			pixelsInternal->fill(blinkFlag ? NEO_COLOR_GREEN : NEO_COLOR_BLACK, 0, 15);
		if (mode_selected == position_select_end)
			pixelsInternal->fill(blinkFlag ? NEO_COLOR_RED : NEO_COLOR_BLACK, 0, 15);
		pixelsInternal->show();
	}
}

void timerIsr()
{
	encoder->service();
	blink_timer++;
	if (blink_timer == 50)
	{
		blink_timer = 0;
		blink();
	}
}

static void read_current_colors()
{
	led_start_color = pixels->getPixelColor(led_start);
	led_end_color = pixels->getPixelColor(led_end);
}

static void read_eeprom()
{
	uint8_t *ptr = pixels->getPixels();
	for (uint16_t i=0; i<pixels->getPixelsSize(); i++)
	{
		*ptr = EEPROM.read(i);
		ptr++;
	}
	pixels->show();
}

static void write_eeprom()
{
	uint8_t *ptr = pixels->getPixels();
	for (uint16_t i=0; i<pixels->getPixelsSize(); i++)
	{
		EEPROM.write(i, *ptr++);
	}
}

void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	
	Serial.begin(115200);
	Serial.println("begin");
	encoder = new ClickEncoder(PIN_ENCODER_A, 
								PIN_ENCODER_B, 
								PIN_ENCODER_BTN,
								4, LOW, HIGH);
	Timer1.initialize(1000);
  	Timer1.attachInterrupt(timerIsr);
  	encoder->setAccelerationEnabled(false);
  	pixels = new Adafruit_NeoPixel(PIXELS_NUM, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
  	pixelsInternal = new Adafruit_NeoPixel(16, PIN_PIXELS_INT, NEO_GRB + NEO_KHZ800);
  	pixels->begin();
  	pixelsInternal->begin();
  	pixelsInternal->fill(pixelsInternal->Color(8,8,8), 0, 16);
 	pixelsInternal->show();

  	set_encoder_color(encoder_color);
  	read_eeprom();
	read_current_colors();
}

void loop()
{
	value += encoder->getValue();

	if (value > last)
	{
		encoder_changed(1);
		last = value;
    }

    if (value < last)
    {
    	encoder_changed(-1);
    	last = value; 
    }

   
    ClickEncoder::Button b = encoder->getButton();
  	if (b != ClickEncoder::Open)
  	{
  		if (b == ClickEncoder::Clicked)
  		{
  			Serial.println("CLICK!");
  			if (mode_selected == color)
  			{
  				Serial.println("set next encder color");
  				set_next_encoder_color();
  			}
  		}
  		if (b == ClickEncoder::Held)
  		{
  			Serial.println("write eeprom");
  			write_eeprom();
  		}
  		if (b == ClickEncoder::DoubleClicked)
  		{
  			Serial.print("switch mode: ");
  			switch (mode_selected)
  			{
  				case position_select_start:
  					mode_selected = position_select_end;
  					break;
  				case position_select_end:
  					mode_selected = color;
  					pixels->setPixelColor(led_start, led_start_color);
  					pixels->setPixelColor(led_end, led_end_color);
  					pixels->show();
  					pixelsInternal->fill(NEO_COLOR_WHITE, 0, 15);
  					pixelsInternal->show();
  					break;
  				case color:
  					mode_selected = position_select_start;
  					break;
  			}
  			Serial.println(mode_selected);
  		}
  	}
}

static void encoder_changed(int8_t val)
{
	if (mode_selected == position_select_start)
	{
		pixels->setPixelColor(led_start, led_start_color);
		
		if (led_start + val < 0)
			led_start = 0;
		else if (led_start + val > (PIXELS_NUM-1))
			led_start = PIXELS_NUM-1;
		else
			led_start+=val;

		led_start_color = pixels->getPixelColor(led_start);

		Serial.println("section change: ");
		Serial.print(led_start);
		Serial.print("::");
		Serial.println(led_end);
	}

	if (mode_selected == position_select_end)
	{
		pixels->setPixelColor(led_end, led_end_color);

		if (led_end + val < 0)
			led_end = 0;
		else if (led_end + val > (PIXELS_NUM-1))
			led_end = PIXELS_NUM-1;
		else
			led_end+=val;

		led_end_color = pixels->getPixelColor(led_end);
		Serial.println("section change: ");
		Serial.print(led_start);
		Serial.print("::");
		Serial.println(led_end);
	}
	if (mode_selected == color)
	{
		for (int i=led_start; i<=led_end; i++)
		{
			//pixels->setPixelColor(i, )
			uint32_t color = pixels->getPixelColor(i);
			uint8_t r = (uint8_t)(color >> 16);
      		uint8_t g = (uint8_t)(color >>  8);
      		uint8_t b = (uint8_t)color;
      		if (encoder_color == ENCODER_RED)
      			r = constrain(r+val, 0, 127);
      		if (encoder_color == ENCODER_GREEN)
      			g = constrain(g+val, 0, 127);
      		if (encoder_color == ENCODER_BLUE)
      			b = constrain(b+val, 0, 127);
      		pixels->setPixelColor(i, pixels->Color(r,g,b));
		}
		pixels->show();
		
		read_current_colors();

		Serial.print("Change color to section: ");
		Serial.print(led_start);
		Serial.print("::");
		Serial.println(led_end);
	}
}
