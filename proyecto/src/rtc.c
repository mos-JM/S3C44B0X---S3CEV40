
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <rtc.h>

extern void isr_TICK_dummy( void );

void rtc_init( void )
{
    TICNT   = 0x0; //deshabilita en el RTC la generacioÌ�n de ticks
    RTCALM  = 0x0;
    RTCRST  = 0x0;
        
    RTCCON  = 0x1;
    
    BCDYEAR = 0x13;
    BCDMON  = 0x1;
    BCDDAY  = 0x3; // 1 enero 3 dia del mes, he buscado y pone que se cuenta a partir del dom
    BCDDATE = 0x1; // semana
    BCDHOUR = 0x0;
    BCDMIN  = 0x0;
    BCDSEC  = 0x0;

    ALMYEAR = 0x0;
    ALMMON  = 0x0;
    ALMDAY  = 0x0;
    ALMHOUR = 0x0;
    ALMMIN  = 0x0;
    ALMSEC  = 0x0;

    RTCCON &= 0x0;
}

/*uint8_t rtcDecToBCD(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}


uint8_t rtcBCDToDec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}
*/


//actualizacion
void rtc_puttime( rtc_time_t *rtc_time )
{
    RTCCON |= 0x1; //Habilita la posibilidad de leer/escribir los registros de hora/fecha del RTC
    //Actualiza la hora y fecha del RTC a la indicada por el argumento. Debe hacer una conversioÌ�n binario â€�> BCD de los datosâ€�

    BCDYEAR = ((rtc_time->year / 10) << 4) | (rtc_time->year % 10);
    BCDMON  = ((rtc_time->mon / 10) << 4) | (rtc_time->mon % 10);;
    BCDDAY  = ((rtc_time->mday / 10) << 4) | (rtc_time->mday % 10);;
    BCDDATE = ((rtc_time->wday / 10) << 4) | (rtc_time->wday % 10);;
    BCDHOUR = ((rtc_time->hour / 10) << 4) | (rtc_time->hour % 10);;
    BCDMIN  = ((rtc_time->min / 10) << 4) | (rtc_time->min % 10);;
    BCDSEC  = ((rtc_time->sec / 10) << 4) | (rtc_time->sec % 10);;
        
    RTCCON &= 0x0;
}

void rtc_gettime( rtc_time_t *rtc_time )
{
    RTCCON |= 0x1;
    
    rtc_time->year = (BCDYEAR >> 4) * 10 + (BCDYEAR & 0x0F);
    rtc_time->mon  = (BCDMON >> 4) * 10 + (BCDMON & 0x0F);
    rtc_time->mday = (BCDDAY >> 4) * 10 + (BCDDAY & 0x0F);
    rtc_time->wday = (BCDDATE >> 4) * 10 + (BCDDATE & 0x0F);
    rtc_time->hour = (BCDHOUR >> 4) * 10 + (BCDHOUR & 0x0F);
    rtc_time->min  = (BCDMIN >> 4) * 10 + (BCDMIN & 0x0F);
    rtc_time->sec  = (BCDSEC >> 4) * 10 + (BCDSEC & 0x0F);
    if( ! rtc_time->sec ){
    	rtc_time->year = (BCDYEAR >> 4) * 10 + (BCDYEAR & 0x0F);
		rtc_time->mon  = (BCDMON >> 4) * 10 + (BCDMON & 0x0F);
		rtc_time->mday = (BCDDAY >> 4) * 10 + (BCDDAY & 0x0F);
		rtc_time->wday = (BCDDATE >> 4) * 10 + (BCDDATE & 0x0F);
		rtc_time->hour = (BCDHOUR >> 4) * 10 + (BCDHOUR & 0x0F);
		rtc_time->min  = (BCDMIN >> 4) * 10 + (BCDMIN & 0x0F);
		rtc_time->sec  = (BCDSEC >> 4) * 10 + (BCDSEC & 0x0F);
    };

    RTCCON &= 0x0;
}


void rtc_open( void (*isr)(void), uint8 tick_count )
{

    pISR_TICK = (uint32) isr; //
    I_ISPC    |= (BIT_TICK);  // poner a 1, borra flag de interrupciones por tick de rtc
    INTMSK   &= ~((BIT_GLOBAL) | (BIT_TICK)); //ponemos a 0 desenmascara glo interrup y tick
    TICNT     |= (1 << 7) | tick_count ; // habilita en RTC generacion de ticks y fija valor del contador
}

void rtc_close( void )
{
    TICNT  &= ~ (1 << 7);
    INTMSK   |=  (BIT_TICK);
    pISR_TICK = (uint32) isr_TICK_dummy;
}

