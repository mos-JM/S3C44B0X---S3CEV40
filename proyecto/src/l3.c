
#include <s3c44b0x.h>
#include <l3.h>
#include <leds.h>

#define SHORT_DELAY    { int8 j; for( j=0; j<4; j++ ); }

/*
 ** Inicializa a 1 las lineas L3CLOCK y L3MODE
 */
void L3_init( void )
{
    PCONA &= ~(1 << 9);
    PCONB &= ~(3 << 4);
    PDATB = ((1 << 5)|(1 << 4));
}

/*
 ** EnvÌa un byte por el interfaz L3 en el modo (ADDR/DATA) indicado
 */
void L3_putByte( uint8 byte, uint8 mode )
{
    uint8 i;
    uint8 rled, lled;
    
    rled = !led_status( RIGHT_LED );
    lled = !led_status( LEFT_LED );    
   
    PDATB =  (rled << 10) | (lled << 9) | (1 << 5) | (mode << 4);
    SHORT_DELAY;

    for( i=0; i<8; i++ )
    {
        PDATB = (mode << 4); // Baja la señal de reloj: L3CLOCK=0 y L3MODE=mode
        PDATA = ((byte & (1 << i)) >> i) << 9; //Pone el bit a transmitir: L3DATA = byte[i]
        SHORT_DELAY;     //espera tCLK(L3)L > 250ns
        PDATB = (1 << 5) | (mode << 4); // Sube la señal de reloj: L3CLOCK=1 y L3MODE=mode
        SHORT_DELAY; // espera tCLK(L3)H > 250ns
    }
    PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (1 << 4);   
}

