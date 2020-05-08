
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <timers.h>

extern void isr_TIMER0_dummy( void );

static uint32 loop_ms = 0;
static uint32 loop_s = 0;

static void sw_delay_init( void );

/*
 ** Pone a 0 los registros de configuración
 ** Pone a 0 todos los búfferes y registros de cuenta y comparación
 ** Para todos los temporizadores
 ** Inicializa las variables para retardos software
 */
void timers_init( void )
{
    TCFG0 = 0x0; //pone a 0 los registros de configuración
    TCFG1 = 0x0; //pone a 0 los registros de configuración

    TCNTB0 = 0x0; //pone a 0 los countbuffer de todos los temporizadores
    TCMPB0 = 0x0; //pone a 0 los compare buffer de todos los temporizadores
    TCNTB1 = 0x0;
    TCMPB1 = 0x0;
    TCNTB2 = 0x0;
    TCMPB2 = 0x0;
    TCNTB3 = 0x0;
    TCMPB3 = 0x0;
    TCNTB4 = 0x0;
    TCMPB4 = 0x0;
    TCNTB5 = 0x0;

    TCON |= (1<<1) | (1<<9) | (1<<13) | (1<<17) | (1<<21) | (1<<25); //carga y para todos los TCNTx
    TCON = 0x0; //no carga y para todos los TCNTx

    sw_delay_init();
}

static void sw_delay_init( void )
{
    uint32 i;
    
    timer3_start();
    for( i=1000000; i; i--);
    loop_s = ((uint64)1000000*10000)/timer3_stop();
    loop_ms = loop_s / 1000;
};

/*
 ** Realiza una espera de n milisegundos usando el timer3
 */
void timer3_delay_ms( uint16 n )
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (31 << 8);
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (0 >> 12);
    for( ; n; n-- )
    {
		TCNTB3 = 1000; // -> esta bien ?
		TCON = (TCON & ~(0xf << 16)) | (1 << 17);
		TCON = (TCON & ~(0xf << 16)) | (1 << 16);
		while( !TCNTO3 );
		while( TCNTO3 );

    }
}

/*
 ** Realiza una espera de n milisegundos sin usar temporizadores
 */
void sw_delay_ms( uint16 n )
{
    uint32 i;
    
    for( i=loop_ms*n; i; i-- ); // loop_ms = 487???
}

/*
 ** Realiza una espera de n segundos usando el timer3
 */
void timer3_delay_s( uint16 n )
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (63 << 8);
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (4 << 12);
	for( ; n; n--){
		TCNTB3 = 31250;
		TCON = (TCON & ~(0xf << 16)) | (1 << 17);
		TCON = (TCON & ~(0xf << 16)) | (1 << 16);
		while( !TCNTO3 );
		while( TCNTO3 );
	};
}

/*
 ** Realiza una espera de n segundos sin usar temporizadores
 */
void sw_delay_s( uint16 n )
{
    uint32 i;
    for( i=loop_s*n; i; i-- ); //loop_s = ????
}

/*
 ** Arranca el timer3 a una frecuencia de 0,01 MHz
 ** Permitirá medir tiempos con una resolución de 0,1 ms (100 us) hasta un máximo de 6.55s
 */
void timer3_start( void ) 
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (199 << 8);    
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (4 << 12);
    
    TCNTB3 = 0xffff; 
    TCON = (TCON & ~(0xf << 16)) | (1 << 17);
    TCON = (TCON & ~(0xf << 16)) | (1 << 16);
    while( !TCNTO3 );
}


/*
 ** Detiene el timer3, devolviendo el número de décimas de milisegundo transcurridas desde que arrancó hasta un máximo de 6.55s
 */
uint16 timer3_stop( void )
{
    TCON &= ~(1 << 16);
    return 0xffff - TCNTO3;
}


/*
 ** Arranca el timer3 a una frecuencia de 0,01 MHz
 ** Permitirá contar n décimas de milisegundo (0,1 ms = 100 us) hasta un máximo de 6.55s
 */
void timer3_start_timeout( uint16 n ) 
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (199 << 8);          
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (4 << 12);
    
    TCNTB3 = n; 
    TCON = (TCON & ~(0xf << 16)) | (1 << 17);
    TCON = (TCON & ~(0xf << 16)) | (1 << 16);
    while( !TCNTO3 );
}


/*
 ** Indica si el timer3 ha finalizado su cuenta
 */
uint16 timer3_timeout( )
{
    return !TCNTO3;
}    


/*
 ** Instala, en la tabla de vectores de interrupción, la función isr como RTI de interrupciones del timer0
 ** Borra interrupciones pendientes del timer0
 ** Desenmascara globalmente las interrupciones y específicamente las interrupciones del timer0
 ** Configura el timer0 para que genere tps interrupciones por segundo
 */
void timer0_open_tick( void (*isr)(void), uint16 tps )
{
    pISR_TIMER0 = (uint32)isr; //instala RTI en la tabla virtual de vectores IRQ
    I_ISPC      = BIT_TIMER0; //borra flag de interrup pendiente por interrup ddel timer0
    INTMSK     &= ~(BIT_GLOBAL | BIT_TIMER0); //desenmascara globalmente interrup e interrup del timer0

    if( tps > 0 && tps <= 10 ) {
        TCFG0  = (TCFG0 & ~(0xff << 0)) | (199 << 0);//programa el T0 con 25 microS (40Khz)
        TCFG1  = (TCFG1 & ~(0xf << 0)) | (2 << 0);
        TCNTB0 = (40000U / tps); //obtiene el num de tocks indicado
    } else if( tps > 10 && tps <= 100 ) {
        TCFG0  = (TCFG0 & ~(0xff << 0)) | (79 << 0); // programa el T0 con 2,5 microS (400KHz)
        TCFG1  = (TCFG1 & ~(0xf << 0));
        TCNTB0 = (400000U / (uint32) tps);
    } else if( tps > 100 && tps <= 1000 ) { // 0,25
        TCFG0  = (TCFG0 & ~(0xff << 0)) | (7 << 0);
        TCFG1  = (TCFG1 & ~(0xf << 0));
        TCNTB0 = (4000000U / (uint32) tps);
    } else if ( tps > 1000 ) { // 31,25
        TCFG0  = (TCFG0 & ~(0xff << 0)) | (249 << 0);
        TCFG1  = (TCFG1 & ~(0xf << 0)) | (2 << 0);
        TCNTB0 = (32000000U / (uint32) tps);
    }

    TCON = (TCON & ~(1 << 0)) | (1 << 1) | (1 << 3); // interval carga TCNT0 stop t0
    TCON = (TCON & ~(1 << 1)) | (1 << 0) | (1 << 3) ; // = no carga, start t0
}

/*
 ** Instala, en la tabla de vectores de interrupción, la función isr como RTI de interrupciones del timer0
 ** Borra interrupciones pendientes del timer0
 ** Desenmascara globalmente las interrupciones y específicamente las interrupciones del timer0
 ** Configura el timer0 para que genere interrupciones en el modo y con la periodicidad indicadas
 ** Configura el timer0 para que genere interrupciones en el modo y con la periodicidad indicadas
 */
void timer0_open_ms( void (*isr)(void), uint16 ms, uint8 mode )
{
    pISR_TIMER0 = (uint32)isr;
    I_ISPC      = BIT_TIMER0;
    INTMSK     &= ~(BIT_GLOBAL | BIT_TIMER0);

    TCFG0 = (TCFG0 & ~(0xff << 0)) | (199 << 0);
    TCFG1 = (TCFG1 & ~(0xf << 0)) | (4 << 0);
    TCNTB0 = 10*ms;

    TCON = (TCON & ~(1 << 0)) | (1 << 1) | (mode << 3); // mode , carga TCNT0, stop t0
    TCON = (TCON & ~(1 << 1)) | (1 << 0) | (mode << 3) ; //mode no carga TCNT0 start t0
}


/*
 ** Para y pone a 0 todos sus bufferes y registros del timer0
 ** Deshabilita las interrupciones del timer0
 ** Desinstala la RTI del timer0
 */
void timer0_close( void )
{
    TCNTB0 = 0x0; //pone a cero el countbuffer del timer0
    TCMPB0 = 0x0; //pone a cero el compare buffer del timer0

    TCON = (TCON & ~(1 << 0)) | (1 << 1);  //carga TCNT0, stop T0
    TCON &= ~( (1<<0) | (1<<1) ); //no carga TCNT0, stop T0
    
    INTMSK     |= BIT_TIMER0; //enmascara interrupciones por fin de timer0

    pISR_TIMER0 = isr_TIMER0_dummy; //instala isr_TIMER0_dummy en la tabla virtual de vectores de interrupción
}
