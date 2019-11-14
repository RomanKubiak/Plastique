#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Adafruit_Neopixel.h>

#define PIN_ENCODER_A   7
#define PIN_ENCODER_B   8
#define PIN_ENCODER_BTN 12
#define PIN_PIXELS		6
#define PIN_LED_BLUE 9
#define PIN_LED_GREEN 10
#define PIN_LED_RED 11

#define PIXELS_NUM 		16
#define PIXELS_SECTIONS 4

#include "encoder.h"

enum t_mode {
	color,section,led
};

ClickEncoder *encoder;
Adafruit_NeoPixel *pixels;
int16_t last, value;

t_mode mode_selected = color;
uint32_t blink_timer = 0;
bool blinkFlag = false;
int selected_led = 0;

void blink()
{
	blinkFlag = !blinkFlag;
	uint32_t color_orig;
	if (mode_selected == led && blinkFlag) 
	{
		color_orig = pixels->getPixelColor(selected_led);
		pixels->setPixelColor(selected_led, pixels->Color(8, 8, 8));
	}
	else if (!blinkFlag)
	{
		pixels->setPixelColor(selected_led, color_orig);	
	}

	pixels->show();
}

void timerIsr()
{
	encoder->service();
	blink_timer++;
	if (blink_timer == 200)
	{
		blink_timer = 0;
		blink();
	}
}

void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	
	Serial.begin(115200);
	encoder = new ClickEncoder(PIN_ENCODER_A, 
								PIN_ENCODER_B, 
								PIN_ENCODER_BTN,
								4, LOW, HIGH);
	Timer1.initialize(1000);
  	Timer1.attachInterrupt(timerIsr);
  	encoder->setAccelerationEnabled(true);
  	pixels = new Adafruit_NeoPixel(PIXELS_NUM, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
  	pixels->begin();
  	pixels->clear();
  	for (int i=0; i<16; i++)
  	{
  		pixels->setPixelColor(i, pixels->Color(2, 0, 0));
  	}
  	pixels->show();
  	Serial.println("begin");
  	set_encoder_color(encoder_color);
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

  		if (b == ClickEncoder::DoubleClicked)
  		{
  			Serial.print("switch mode: ");
  			switch (mode_selected)
  			{
  				case color:
  					mode_selected = section;
  					break;
  				case section:
  					mode_selected = led;
  					break;
  				case led:
  					mode_selected = color;
  					break;
  			}
  			Serial.println(mode_selected);
  		}
  	}
}

static void encoder_changed(int8_t val)
{
	if (mode_selected == led)
	{
		Serial.print("select next led ");
		if (selected_led + val < 0 || selected_led + val > PIXELS_NUM)
			selected_led = 0;
		else
			selected_led+=val;		
		Serial.println(selected_led);
	}
}
