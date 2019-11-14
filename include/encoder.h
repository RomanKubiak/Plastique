#define ENCODER_RED		B001
#define ENCODER_GREEN	B010
#define ENCODER_BLUE	B100
int encoder_color = ENCODER_RED;
void set_encoder_color(int color)
{
	digitalWrite(PIN_LED_RED, 	!(bitRead(color, 0)));
	digitalWrite(PIN_LED_GREEN, !(bitRead(color, 1)));
	digitalWrite(PIN_LED_BLUE, 	!(bitRead(color, 2)));

	encoder_color = color;
}

void set_next_encoder_color()
{
	switch (encoder_color)
	{
		case ENCODER_RED:
			encoder_color = ENCODER_GREEN;
			break;
		case ENCODER_GREEN:
			encoder_color = ENCODER_BLUE;
			break;
		case ENCODER_BLUE:
			encoder_color = ENCODER_RED;
			break;
		default:
			break;
	}

	set_encoder_color(encoder_color);
}
