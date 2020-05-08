
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <iis.h>
#include <dma.h>

static void isr_bdma0( void ) __attribute__ ((interrupt ("IRQ")));

static uint8 flag;
static uint8 iomode;

/*
 ** Configura el controlador de IIS segË™n los siguientes parÂ·metros
 **   Master mode en reposo (no transfer y todo desabilitado)
 **   fs: 16000 KHz
 **   CODECLK: 256fs
 **   SDCLK: 32fs
 **   Bits por canal: 16
 **   Protocolo de trasmisiÃ›n de audio: iis
 **   SeÃ’alizacion de canal izquierdo: a baja
 ** Si mode = IIS_DMA
 **   No transfer mode
 **   Deshabilita Tx/Rx FIFOs
 **   Deshabilita prescaler e IIS
 **   Inicializa el BDMA0
 **   Abre las interrupciones por BDMA0
 ** Si mode = IIS_POLLING
 **   Transmit and receibe mode
 **   Tx/Rx por pooling
 **   Habilita Tx/Rx FIFOs
 **   Habilita prescaler e IIS
 */
void iis_init( uint8 mode )
{
    iomode = mode;

    if( mode == IIS_POLLING )
    {
        //para operar con polling
        IISPSR  = (7 << 4) | (7 << 0);
        IISMOD  = (3 << 6) | (1 << 3) | (1 << 0);
        IISFCON = (3 << 8);
        IISCON  = (3 << 0);
    }
    if( mode == IIS_DMA )
    {
        //por DMA en modo reposo
        IISPSR  = (7 << 4) | (7 << 0);
        IISMOD  = ~(0x7 << 6);
        IISFCON = 0;
        IISCON  = 0;

        bdma0_init();
        bdma0_open( isr_bdma0 );
        flag = OFF;
    }
}

static void isr_bdma0( void )
{
    IISCON &= ~1;
    flag = OFF;
    I_ISPC = BIT_BDMA0; 
}

/*
 ** EnvÃŒa por el bus IIS una muestra por pooling
 */
inline void iis_putSample( int16 ch0, int16 ch1 )
{
    while (((IISFCON & 0xf0) >> 4) > 6) ; //2huecos
    IISFIF = ch0;
    IISFIF = ch1;
}

/*
 ** Almacena por pooling una muestra recibida por el bus IIS
 */
inline void iis_getSample( int16 *ch0, int16 *ch1 )
{
	while ((IISFCON & (0xf)) < 2); //( 2 datos
	*ch0 = IISFIF;
	*ch1 = IISFIF;
}

/*
 ** EnvÃŒa por el bus IIS un flujo de length/2 muestras almacenado en el buffer indicado
 ** Si mode = IIS_POOLING
 **   Transmite las muestras de 1 en 1 leyÃˆndolas del buffer
 ** Si mode = IIS_DMA
 **   Programa una transferencia por BDMA0 de length bits de buffer a IISFIF
 **   Transmit mode
 **   Habilita Tx FIFO
 **   Tx por DMA
 **   Habilita Tx DMA request, prescaler e IIS
 ** El parametro loop (ON/OFF) permite indicar reproducciÃ›n continua en caso de DMA. No aplica a pooling.
 */
void iis_play( int16 *buffer, uint32 length, uint8 loop )
{
    uint32 i;
    int16 ch1, ch2;

    if( iomode == IIS_POLLING )
        for( i=0; i<length/2; )
        {
            ch1 = buffer[i++];
            ch2 = buffer[i++];
            iis_putSample( ch1, ch2 );
        }
    if( iomode == IIS_DMA ) {
        while (flag != OFF);
        BDISRC0 = (1 << 30) | (1 << 28) | (uint32)  buffer;
        BDIDES0 = (1 << 30) | (3 << 28) | (uint32) &IISFIF;
        BDCON0 = 0;
        if(loop)
            BDICNT0 = (1 << 30) | (1 << 26) | (1 << 21) | (1 << 20) | length;
        else
            BDICNT0 = (1 << 30) | (1 << 26) | (3 << 22) | (1 << 20) | length;
        IISMOD = (1 << 7) | (1 << 3) | (1 << 0);
        IISFCON = (1 << 11) | (1 << 9);
        IISCON = (1 << 5) | (3 << 0);
    
        flag = ON;
    }
}

/*
 ** Almacena en el buffer indicado por DMA un flujo de length/2 recibidas por el bus IIS
 ** utilizando el mÃˆtodo de transferencia indicado al inicializar el dispositivo.
 ** Si mode = IIS_POOLING
 **   Recibe las muestras de 1 en 1 almacenÂ·ndolas en el buffer
 ** Si mode = IIS_DMA
 **   Programa una transferencia por BDMA0 de length bits de buffer a IISFIF
 **   Reciebe mode
 **   Habilita Rx FIFO
 **   Rx por DMA
 **   Habilita Rx DMA request, prescaler e IIS
 */
void iis_rec( int16 *buffer, uint32 length )
{
    uint32 i;
    int16 ch1, ch2;

    if( iomode == IIS_POLLING )
        for( i=0; i<length/2; ){
            iis_getSample(&ch1, &ch2); // recibo
            buffer[i++] = ch1;
            buffer[i++] = ch2;
        }
    if( iomode == IIS_DMA )
    {
        while( flag != OFF );
        BDISRC0  = (1 << 30) | (3 << 28) | (uint32) &IISFIF;
        BDIDES0  = (2 << 30) | (1 << 28) | (uint32) buffer;      
        BDCON0   = 0;
        BDICNT0  = (1 << 30) | (1 << 26) | (3 << 22) | (0xfffff & length); 
        BDICNT0 |= (1 << 20);

        //Configura el controlador de IIS para operar por DMA en modo solo recepcioÌ�n
        IISMOD  = (1 << 6) | (1 << 3) | (1 << 0);
        IISFCON = (1 << 10) | (1 << 8);
        IISCON  = (1 << 4) | (3 << 0);
        flag = ON;
    }
}

/*
 ** Pausa la recepcion/transmision de muestras por bus IIS
 ** Aplica solo al caso de transferencias por DMA
 */
void iis_pause( void )
{
    IISCON &= ~(1 << 0);
    flag = OFF;
}

/*
 ** Continua la recepcion/transmision de muestras por bus IIS
 ** Aplica solo al caso de transferencias por DMA
 */
void iis_continue( void )
{
    IISCON |= (1 << 0);
    flag = ON; //
}

/*
 ** Devuelve (ON/OFF) para indicar si se estÂ· reproduciendo o no sonido
 ** Aplica solo al caso de transferencias por DMA
 */
uint8 iis_status( void )
{
    return flag;
}

/*
 ** Reproduce un fichero en formato WAV cargado en memoria a partir de la direcciÃ›n indicada
 */
void iis_playwawFile( uint8 *fileAddr )
{
    uint32 size;

    while ( !(fileAddr[0] == 'd' && fileAddr[1] == 'a' && fileAddr[2] == 't' && fileAddr[3] == 'a') )
        fileAddr++;
    fileAddr += 4;

    size = (uint32) fileAddr[0];
    size += (uint32) fileAddr[1] << 8;
    size += (uint32) fileAddr[2] << 16;
    size += (uint32) fileAddr[3] << 24;
    fileAddr += 4;

    iis_play( (int16 *)fileAddr, size, OFF );
}
