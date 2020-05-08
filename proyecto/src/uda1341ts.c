
#include <l3.h>
#include <uda1341ts.h>

#define ADDRESS (0x05)

#define DATA0  (0x0)
#define DATA1  (0x1)
#define STATUS (0x2)

#define EA (0x18 << 3)
#define ED (0x7 << 5)

static uint8 volume;
static uint8 state;

/*
 ** Inicializa el interfaz L3
 ** Resetea el audio codec
 ** Configura el audio codec seg˙n los siguientes par·metros:
 **   CODECLK = 256fs
 **   Protocolo de trasmisiÛn de audio: iis
 **   Volumen de reproducciÛn m·ximo
 **   Selecciona el canal 1 como entrada
 **   Habilita el ADC y DAC con 6 dB de ganancia de entrada
 **   Fija el volumen m·ximo
 */
void uda1341ts_init( void )
{
    L3_init();     
    
    L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE );
    L3_putByte( (1 << 6) | (2 << 4), L3_DATA_MODE );
    L3_putByte( (2 << 4), L3_DATA_MODE );

    L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE  ); 
    L3_putByte( EA | (2), L3_DATA_MODE ); 
    L3_putByte( ED | 1, L3_DATA_MODE );
  
    uda1341ts_setvol( VOL_MAX );

    uda1341ts_on( UDA_DAC );
    uda1341ts_on( UDA_ADC );
}

/*
 ** Habilita/desabilita el silenciado del audio codec
 */
void uda1341ts_mute( uint8 on )
{
    L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE );
    L3_putByte( (1 << 7) | (on << 2), L3_DATA_MODE ) ;
};

/*
 ** Enciende el conversor indicado
 */
void uda1341ts_on( uint8 converter )
{
    state |= converter;
    L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE );
    L3_putByte( (1 << 7) | (1 << 6) | (1<<5) | state, L3_DATA_MODE );
}

/*
 ** Apaga el conversor indicado
 */
void uda1341ts_off( uint8 converter )
{
    state &= ~converter;
    L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE );
    L3_putByte( (1 << 7) | (1 << 6) | (1<<5) | state, L3_DATA_MODE );
}

/*
 ** Devuelve el estado del conversor indicado
 */
uint8 uda1341ts_status( uint8 converter )
{
    return state & converter;
}

/*
 ** Fija el volumen de reproducciÛn
 */
void uda1341ts_setvol( uint8 vol )
{
    volume = 0x3f - vol;
    L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE );
    L3_putByte( (volume & ~(0x3 << 6)), L3_DATA_MODE ) ;
}

/*
 ** Devuelve el volumen de reproducciÛn
 */
uint8 uda1341ts_getvol( void )
{
    return volume;
}

