
#include <s3c44b0x.h>
#include <uart.h>

void uart0_init( void )
{
    UFCON0 = (0<<2) | (0<<1) | (1);//Permite habilitar las FIFOs de datos,borrarlas . por DMA
    UMCON0 = ~(1<<4);//control flujo datos. UART Channel 0 Modem Control
    ULCON0 = (0<<6) | (0<<3) | (0<<2) | (3);//fPermite configurar el formato de los datos serie.
    UBRDIV0 = 0x22;  //dividir la seÒal para transmitir en 115200 baudios.Permite configurar la velocidad de comunicación.
    UCON0 = 0x5;//Permite configurar el modo de transmisión (por interrupciones/DMA) de la UART.
}
//UTXH0 Registro del dato a transmitir.
//URXH0 Registro del dato recibido.
//UFSTAT0 Indica el de estado de las FIFOs de datos.
//UERSTAT0 Indica el error producido en la recepción (si lo hubiera)
//UTRSTAT0 Indica de estado de la transmisión/recepción.
/**
 * Envia un caracter por la uart, espera mientras este llena
 * Esperar mientras Tx FIFO esté llena (esperar mientras UFSTAT0[9] == 1
 * Escribir en UTXH0
 */
void uart0_putchar( char ch )
{
    while(UFSTAT0 & (1<<9));
    UTXH0 = ch;
}        

/**
 * Recibir caracter de la UART y devolverlo
 */
char uart0_getchar( void )
{
    while( !(UFSTAT0 &(0xf) ));
    return URXH0 ;
}

/**
 *envia cadena de caracteres por la uart hasta alcanzar fin de cadena '\0'
 */
void uart0_puts( char *s )
{
    int i = 0;
    while(s[i] != '\0'){
        uart0_putchar(s[i]);
        i++;
    }
}
/**
 * eviar en una cadena en decimal del decimal numero recibido por param
 */
void uart0_putint( int32 i )
{
	char buf[8 + 1];
	char *p = buf + 8;
	uint8 c;
	int negativo = 0;

	*p = '\0';
	if( i < 0){
		negativo = 1;
		i = -i; //asb(i);
	}
	do {
		c = i%10;
		*--p = '0' + c; // cast a char
		i = i/10;
	} while( i );

	if(negativo){
		*--p = '-';
	}
	uart0_puts( p );
}

/**
 *Envia una cadena de caracteres por la UART . En hexadecimal el del argumento
 */
void uart0_puthex( uint32 i )
{
    char buf[8 + 1];
    char *p = buf + 8;
    uint8 c;

    *p = '\0';

    do {
        c = i & 0xf; // c = i % 16
        if( c < 10 )
            *--p = '0' + c; // cast a char
        else
            *--p = 'a' + c - 10;  //cast a char
        i = i >> 4; // i = i / 16
    } while( i );

    uart0_puts( p );
}
/**
 * recibir una cadena, de uno en uno hasta final de linea
 *Los caracteres se reciben y almacenan de uno en uno hasta
 *detectar el fin de línea LF ('\n') que no se almacena
 *En su lugar se almacena el centinela fin de cadena ('\0')
 */

void uart0_gets( char *s )
{
	uint8 i = 0;
	s[i] = uart0_getchar();
	while(s[i] != '\n'){
		i++;
		s[i] = uart0_getchar();
	}
	s[i] = '\0';

}

/**
 * Se recibe una cadena y después se recorre carácter a caractet, acumulando el valor
 *para tener el calor del caracter se resta '0'
 *puede ser negativo '-' negar el valor calculado
*/

int32 uart0_getint( void )
{
	char s[256];
	int32 dec = 0;
	int i =  0;
	int negativo = 1;

	uart0_gets( s );

	if(s[i] == '-'){
		negativo = -1;
		i++;
	}

	while( s[i] != '\0' ){

		dec *=10;
		dec += s[i] - '0'; // cast decimal

		i++;
	}
	
	return dec* negativo;
}
/**
 * recibe una cadena y recorre, y acumula mult por 16
 *para 0-9 basta con restarle '0'
 *de A a F hay que restarle 'A' y sumarle 10
 */
uint32 uart0_gethex( void )
{
    char s[256];
    int32 hex = 0;
    int i =  0;
   
    uart0_gets( s );
    
    while( s[i] != '\0' ){
        hex *=16;
        if(s[i]-'0' < 10)
            hex += s[i] - '0'; // cast decimal
        else{
        	if(s[i] < 'a')
        		hex += s[i] - 'A' + 10;
        	else
        		hex += s[i] - 'a' + 10;

        }
        i++;
    }
    
    return hex;
}

