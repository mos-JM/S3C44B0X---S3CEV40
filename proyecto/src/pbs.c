
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <pbs.h>
#include <timers.h>

extern void isr_PB_dummy( void );

void pbs_init( void )
{
    timers_init();
}

/*
 ** Si hay un pulsador presionado devuelve su scancode, en otro caso devuelve PB_FAILURE
 */
uint8 pb_scan( void )
{
    /*PDATG Leer el estado del pulsador de lospines6y7dePDATG
    • 0 pulsado
    • 1 no pulsado
     */
    if( !(PDATG & PB_LEFT)) // PDATG fichero s3c44b0x.h
        return PB_LEFT;
    else if(!(PDATG & PB_RIGHT))
        return PB_RIGHT;
    else
        return PB_FAILURE;
}


/*
 ** Devuelve el estado del pulsador indicado
 */
uint8 pb_status( uint8 scancode )
{
    if( (scancode == PB_LEFT) && (pb_scan() == PB_LEFT)) //si corresponde  al scan code y pulsado, devuelvo pulsado,
        return PB_DOWN;
    else if( (scancode == PB_LEFT) && (pb_scan() != PB_LEFT)) //si corresponde  al scancode y no pulsado, devuelvo nop pulsado,
        return PB_UP;
    else if( (scancode == PB_RIGHT) && (pb_scan() == PB_RIGHT)) //si corresponde  al scancode y  pulsado , devuelvo pulsado,
        return PB_DOWN;
    else if( (scancode == PB_RIGHT) && (pb_scan() != PB_RIGHT)) //si corresponde  al scancode y no pulsado, devuelvo no  pulsado,
        return PB_UP;
    else
        return PB_FAILURE;
}

/*
 ** Espera a que se presione el pulsador indicado
 */
void pb_wait_keydown( uint8 scancode )
{
	while( 1 )
	 {
		pb_wait_any_keydown( );
		if ( scancode == pb_scan() )
			return;
		while(!(PDATG & PB_LEFT) && !(PDATG & PB_RIGHT));			//Si no lo es, espera depresi�n y vuelve a empezar
		sw_delay_ms( PB_KEYUP_DELAY );
	}
}

/*
 ** Espera a que se presione y despresione el pulsador indicado
 */
void pb_wait_keyup( uint8 scancode )
{
	 pb_wait_keydown( scancode );
	 while(!(PDATG & scancode)) ;
	 sw_delay_ms( PB_KEYUP_DELAY );

}

/*
 ** Espera a que se presione cualquier pulsador
 */
void pb_wait_any_keydown( void )
{
    while( (PDATG & PB_LEFT)  && (PDATG & PB_RIGHT));
    
    sw_delay_ms( PB_KEYDOWN_DELAY );
}

/*
 ** Espera a que se presione y depresione cualquier pulsador
 */
void pb_wait_any_keyup( void )
{
    pb_wait_any_keydown();
    while(!(PDATG & PB_LEFT) && !(PDATG & PB_RIGHT));
    
    sw_delay_ms( PB_KEYUP_DELAY );
}

/*
 ** Espera la presiÛn y depresiÛn de un pulsador y devuelve su scancode
 */
uint8 pb_getchar( void )
{
    uint8 scancode;
    pb_wait_any_keydown();
    scancode = pb_scan();
    pb_wait_any_keyup();
    
    return scancode;
}

/*
 ** Espera la presiÛn y depresiÛn de un pulsador y devuelve su scancode y el intervalo en ms que ha estado pulsado (max. 6553ms)
 */
uint8 pb_getchartime( uint16 *ms )
{
    uint8 scancode;
    
    while((PDATG & PB_LEFT) && (PDATG & PB_RIGHT)); // mientras no sea pulsado
    timer3_start();
    sw_delay_ms( PB_KEYDOWN_DELAY );
    
    scancode = pb_scan();
    
    while(!(PDATG & PB_LEFT) || !(PDATG & PB_RIGHT));
    *ms = timer3_stop() / 10;
    sw_delay_ms( PB_KEYUP_DELAY );

    return scancode;
}

/*
 ** Espera un m·ximo de ms (no mayor de 6553ms) la presiÛn y depresiÛn de un pulsador y devuelve su scancode, en caso contrario devuelve PB_TIMEOUT
 */
uint8 pb_timeout_getchar( uint16 ms )
{
    uint8 scancode;
    timer3_start_timeout(ms*10);
	while ( (PDATG & PB_LEFT)  && (PDATG & PB_RIGHT) && !timer3_timeout());
	if (timer3_timeout() ){//si timer out return timer out sino

		return PB_TIMEOUT;
	}
	else{
		return  pb_getchar();
	}
	
	//return pb_getchar();

}


/*
 ** Instala, en la tabla de vectores de interrupciÛn, la funciÛn isr como RTI de interrupciones por presiÛn de un pulsador
 ** Borra interrupciones pendientes por presiÛn de un pulsador
 ** Desenmascara globalmente las interrupciones y especÌficamente las interrupciones por presiÛn de un pulsador
 */
void pbs_open( void (*isr)(void) )
{
    pISR_PB   = (uint32)isr;
    EXTINTPND = ~0;//BIT_EINT4 | BIT_EINT5 | BIT_EINT6 | BIT_EINT7; //borra flag de interrup pendiente por interup externas
    I_ISPC    = BIT_EINT4567;
    INTMSK   &= ~(BIT_GLOBAL | BIT_EINT4567); //añado
}

void pbs_close( void )
{
    INTMSK  |= BIT_GLOBAL | BIT_EINT4567; //añado
    pISR_PB  = (uint32)isr_PB_dummy; //añado
}
