#include <s3c44b0x.h>
#include <s3cev40.h>
#include <timers.h>
#include <keypad.h>

extern void isr_KEYPAD_dummy( void );

void keypad_init( void )
{
	timers_init();
};

uint8 keypad_scan( void )
{
    uint8 aux;

    aux = *( KEYPAD_ADDR + 0x1c );
    if( (aux & 0x0f) != 0x0f )
    {
        if( (aux & 0x8) == 0 )
            return KEYPAD_KEY0;
        else if( (aux & 0x4) == 0 )
            return KEYPAD_KEY1;
        else if( (aux & 0x2) == 0 )
            return KEYPAD_KEY2;
        else if( (aux & 0x1) == 0 )
            return KEYPAD_KEY3;
    }
    aux = *( KEYPAD_ADDR + 0x1a );
    if( (aux & 0x0f) != 0x0f )
    {
        if( (aux & 0x8) == 0 )
            return KEYPAD_KEY4;
        else if( (aux & 0x4) == 0 )
            return KEYPAD_KEY5;
        else if( (aux & 0x2) == 0 )
            return KEYPAD_KEY6;
        else if( (aux & 0x1) == 0 )
            return KEYPAD_KEY7;
    }
    aux = *( KEYPAD_ADDR + 0x16 );
   if( (aux & 0x0f) != 0x0f )
   {
	   if( (aux & 0x8) == 0 )
		   return KEYPAD_KEY8;
	   else if( (aux & 0x4) == 0 )
		   return KEYPAD_KEY9;
	   else if( (aux & 0x2) == 0 )
		   return KEYPAD_KEYA;
	   else if( (aux & 0x1) == 0 )
		   return KEYPAD_KEYB;
   }
   aux = *( KEYPAD_ADDR + 0x0e );
	if( (aux & 0x0f) != 0x0f )
	{
		 if( (aux & 0x8) == 0 )
			 return KEYPAD_KEYC;
		 else if( (aux & 0x4) == 0 )
			 return KEYPAD_KEYD;
		 else if( (aux & 0x2) == 0 )
			 return KEYPAD_KEYE;
		 else if( (aux & 0x1) == 0 )
			 return KEYPAD_KEYF;
	}
    return KEYPAD_FAILURE;
}


/*
 ** Devuelve el estado de la tecla indicada
 */
uint8 keypad_status( uint8 scancode )
{
    if(scancode == keypad_scan())
        return KEY_DOWN;
    else
        return KEY_UP;
}

void keypad_wait_keydown( uint8 scancode )
{
     while( 1 ) 
     {
        while(keypad_scan() != scancode);	//Espera la presión del keypad
        sw_delay_ms( KEYPAD_KEYDOWN_DELAY );
        if ( scancode == keypad_scan() )
            return;
        while(!(PDATG & (0x1<<1)));			//Si no lo es, espera depresión y vuelve a empezar
        sw_delay_ms( KEYPAD_KEYUP_DELAY );
    }         
}

void keypad_wait_keyup( uint8 scancode )
{
	keypad_wait_keydown(scancode);
	while(!(PDATG & (0x1<<1)));
	sw_delay_ms(KEYPAD_KEYUP_DELAY);

}

void keypad_wait_any_keydown( void )
{
	while((PDATG & (0x1<<1)));
	sw_delay_ms(KEYPAD_KEYDOWN_DELAY);
}

void keypad_wait_any_keyup( void )
{
	keypad_wait_any_keydown();
	while(!(PDATG & (0x1<<1)));
	sw_delay_ms(KEYPAD_KEYUP_DELAY);
}

uint8 keypad_getchar( void )
{
	keypad_wait_any_keydown();
	return keypad_scan();
}

uint8 keypad_getchartime( uint16 *ms )
{
	 uint8 scancode;

	while((PDATG & (0x1<<1))); // mientras no sea pulsado
	timer3_start();
	sw_delay_ms( KEYPAD_KEYDOWN_DELAY );

	scancode = keypad_getchar();

	while(!(PDATG & (0x1<<1)));
	*ms = timer3_stop() / 10;
	sw_delay_ms( KEYPAD_KEYUP_DELAY );

	return scancode;

}

//Espera ms maximo a la pulsacion
uint8 keypad_timeout_getchar( uint16 ms )
{
	timer3_start_timeout(ms*10);
	while ((!(timer3_timeout())) || (!(PDATG & (1<<1))));
	timer3_start();
	sw_delay_ms(PB_KEYDOWN_DELAY);
	return keypad_getchar();
}

void keypad_open( void (*isr)(void) )
{
	pISR_KEYPAD= ((uint32)isr);
	EXTINTPND = ~0;
	I_ISPC = BIT_KEYPAD;
	INTMSK &= ~(BIT_GLOBAL | BIT_KEYPAD);

	
}

void keypad_close( void )
{
	INTMSK |= (BIT_KEYPAD);
	pISR_KEYPAD= (uint32)isr_KEYPAD_dummy;
}

