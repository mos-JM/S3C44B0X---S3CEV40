
#include <s3c44b0x.h>
#include <leds.h>

static uint8 status = 0;
//inicializo leds y los apago
void leds_init( void )
{
	PCONB &= ~(1<<10) & ~(1<<9);
	led_off(LEFT_LED);
	led_off(RIGHT_LED);
}

void led_on( uint8 led )
{
	PDATB &= ~(led << 9); //8
	status |=  led;
}

void led_off( uint8 led )
{
	PDATB |= (led << 9);
	status &=  ~led;

}
//Conmuta el led indicado
void led_toggle( uint8 led )
{
	status ^= (led); //and + inv
	PDATB ^= (led << 9);

	/*if(led_status(led))
			PDATB |=  (led << 9);
	else
		PDATB &= ~(led << 9);*/

}
//devuelve el led(on/off) indicado 
uint8 led_status( uint8 led )
{
    return (status & led);
}
