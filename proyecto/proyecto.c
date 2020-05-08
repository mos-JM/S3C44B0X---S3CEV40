#include <s3c44b0x.h>
#include <system.h>
#include <uart.h>
#include <timers.h>
#include <pbs.h>
#include <lcd.h>
#include <uda1341ts.h>
#include <iis.h>

 /* Macros */
 
#define SPS      (16000)                  // Frecuencia de muestreo
#define BUFFER   ((int16 *)0x0c200000)    // Direcci�n de inicio del buffer      
#define MAXBYTES (640000)                 // Capacidad m�xima del buffer = 10 s : (10 s) * (16000 muestras/s) * (2 canales) * (2 B/muestra) = 640000B 

 /* Definici�n de tipos */

typedef enum {waiting, recording, playing} state_t;    // Estados en los que puede estar el sistema

typedef struct {
	int16  *buffer;    // Puntero al inicio del buffer de muestras
    uint32 maxsize;    // Tama�o m�ximo del buffer (en numero de muestras, 1 muestra = 2B)
	uint32 size;       // Numero de muestras grabadas
    uint32 index;      // Numero de muestras reproducidas 
	uint16 full;       // Flag para indicar si el buffer ha sido completamente grabado
	uint16 played;     // Flag para indicar si el buffer ha sido completamente reproducido
} audioBuffer_t;

typedef struct {
	volatile uint16  numSecs;    // Numero de segundos transcurridos desde el inicio de la grabaci�n/reproducci�n
    volatile boolean updated;    // Flag para indicar si numSeconds se ha actualizado
} time_t;

/* Declaraci�n de funciones */

void timer0_tick( void ) __attribute__ ((interrupt ("IRQ")));

/* Declaraci�n de variables globales para comunicaci�n foreground/background  */

state_t state;

volatile audioBuffer_t audioBlock;
volatile time_t        time;

/************************************/

void main( void )
{
    sys_init();
    timers_init();
    pbs_init();
    lcd_init();
    uda1341ts_init();
    iis_init( IIS_POLLING );

    lcd_clear();
    lcd_on();

    while( 1 )
    {

        /************************************/

        state = waiting;
        
        lcd_puts_x2( 0, 50, BLACK, "  Esperando...      " );
        lcd_puts( 0, 82, BLACK, "    pulse para empezar la grabacion   " );
        pb_wait_keyup( PB_LEFT );

      	/************************************/

        state = recording;

        audioBlock.buffer  = BUFFER;                                            
        audioBlock.maxsize = MAXBYTES/2;
        audioBlock.size    = 0;
        audioBlock.index   = 0;
        audioBlock.full    = FALSE;
        audioBlock.played  = FALSE;
    
        time.numSecs = 0;
        time.updated = FALSE;

        lcd_puts_x2( 0, 50, BLACK, "  GRABANDO:0        " );
        lcd_puts( 0, 82, BLACK, "    pulse para parar la grabacion     " );
        timer0_open_tick( timer0_tick, SPS );                                   // Inicia la generaci�n de interrupciones periodicas para muestreo del audio codec
        while( !pb_status( PB_LEFT ) && !audioBlock.full )                      // Espera la presi�n de un pulsador o el llenado completo del buffer de audio
            if( time.updated )                                                  // Si ha transcurrido un segundo de grabaci�n...
            {
                lcd_puts_x2( 176, 50, BLACK, "  " );                            // ...actualiza en pantalla la cuenta de segundos
                lcd_putint_x2( 176, 50, BLACK, time.numSecs );
                time.updated = FALSE;
            }
        timer0_close();                                                         // Finaliza la generaci�n de interrupciones peri�dicas
        if( pb_status( PB_LEFT ) )                                              // Si finaliz� la toma de muestras por presi�n de un pulsador...
            pb_wait_keyup( PB_LEFT );                                           // ...espera su depresi�n
        
      	/************************************/

        state = waiting;
        
        lcd_puts_x2( 0, 50, BLACK, "  Esperando...      " );
        lcd_puts( 0, 82, BLACK, "    pulse para empezar la reproduccion" );
        pb_wait_keyup( PB_LEFT );

       	/************************************/

        state = playing;
               
        time.numSecs = 0;
        time.updated = FALSE;
        
        lcd_puts_x2( 0, 50, BLACK, "  REPRODUCIENDO:0   " );
        lcd_puts( 0, 82, BLACK, "    pulse para parar la reproduccion  " );
        timer0_open_tick( timer0_tick, SPS );                                   // Inicia la generaci�n de interrupciones periodicas para muestreo del audio codec
        while( !pb_status( PB_LEFT ) && !audioBlock.played )                    // Espera la presi�n de un pulsador o la reproducci�n completa del buffer de audio
            if( time.updated )                                                  // Si ha transcurrido un segundo de reproducci�n...
            {
                lcd_puts_x2( 256, 50, BLACK, "  " );                            // ...actualiza en pantalla la cuenta de segundos
                lcd_putint_x2( 256, 50, BLACK, time.numSecs );
                time.updated = FALSE;
            }
        timer0_close();                                                         // Finaliza la generaci�n de interrupciones peri�dicas
        if( pb_status( PB_LEFT ) )                                              // Si finaliz� la reproducci�n de muestras por presi�n de un pulsador...
            pb_wait_keyup( PB_LEFT );                                           // ...espera su depresi�n       

    }
}

void timer0_tick( void )
{
    static uint16 numTicks = SPS;
	int16 ch0, ch1;
 
    switch( state )
    {
        case waiting:
            break;
        case recording:
            if( audioBlock.size < audioBlock.maxsize ) {                        // Si el sistema esta grabando sonido y el buffer de audio no esta completo...
                iis_getSample( &ch0, &ch1 );                                    // ...recibe una muestra por canal y
                audioBlock.buffer[audioBlock.size++] = ch0;                     // ...las almacena en el audio buffer
                audioBlock.buffer[audioBlock.size++] = ch1;                
                if( !--numTicks ) {                                             // Decrementa el contador de ticks, y si llega a cero...
                    numTicks = SPS;
                    time.numSecs++;                                             // ...actualiza y se�aliza el n�mero de segundos de grabaci�n
                    time.updated = TRUE;
                }
            } else {
            	audioBlock.full = TRUE;                                         // Si el buffer esta completo lo se�aliza
                numTicks = SPS;
            }
            break;
        case playing:
            if( audioBlock.index < audioBlock.size ) {                          // Si el sistema esta reproducciendo sonido y quedan muestras por enviar...
                ch0 = audioBlock.buffer[audioBlock.index++];                    // ...lee las muestras del audio buffer y 
                ch1 = audioBlock.buffer[audioBlock.index++];
                iis_putSample( ch0, ch1 );                                      // ...las envia
                if( !--numTicks ) {                                             // Decrementa el contador de ticks, y si llega a cero...
                    numTicks = SPS;
                    time.numSecs++;                                             // ...actualiza y se�aliza el n�mero de segundos de reproducci�pn
                    time.updated = TRUE;
                }
                break;                
            } else {
            	audioBlock.played = TRUE;                                       // Si se ha finalizado la reproducci�n del buffer lo se�aliza
                numTicks = SPS;
            }

    }
    I_ISPC = BIT_TIMER0;
}

