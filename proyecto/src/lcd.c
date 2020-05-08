
#include <s3c44b0x.h>
#include <lcd.h>

extern uint8 font[];
static uint8 lcd_buffer[LCD_BUFFER_SIZE];

static uint8 state;
/*
 ** Configura el controlador de LCD seg˙n los siguientes par·metros
 **   ResoluciÛn: 320x240
 **   Modo de barrido: 4 bit single scan
 **   Modo del lcd: 16 niveles de gris
 **   Paletas de color: no
 **   Frecuencia de refresco: 60 Hz
 **   Reordenado de bytes: activado
 **   Self-refresh: desactivado
 **   Polaridad de seÒales de sincronismo: normal
 **   ConmutaciÛn de VM: en cada frame
 **   Anchura del blanking horizontal: mÌnima
 **   Retardo y anchura de pulso de sincronismo horizontal: mÌnimos
 **   Valores de dithering: por defecto
 ** Inicializa el estado del LCD y lo apaga
 */

void lcd_init( void )
{      
    DITHMODE = 0x12210;
    DP1_2    = 0xA5A5;
    DP4_7    = 0xBA5DA65;
    DP3_5    = 0xA5A5F;
    DP2_3    = 0xD6B;
    DP5_7    = 0xEB7B5ED;
    DP3_4    = 0x7DBE;
    DP4_5    = 0x7EBDF;
    DP6_7    = 0x7FDFBFE;
    
    REDLUT   = 0x0;
    GREENLUT = 0x0;
    BLUELUT  = 0x0;

    LCDCON1  = 0x1C020;
    LCDCON2  = 0x13CEF;
    LCDCON3  = 0x0;

    LCDSADDR1 = (2 << 27) | ((uint32)lcd_buffer >> 1);
    LCDSADDR2 = (1 << 29) | (((uint32)lcd_buffer + LCD_BUFFER_SIZE) & 0x3FFFFF) >> 1;
    LCDSADDR3 = 0x50;
    
    lcd_off();
}


 // Enciende el LCD

void lcd_on( void )
{
    LCDCON1 |= 0x1;
}

 // Apaga el LCD

void lcd_off( void )
{
    LCDCON1 &= ~(0x1);
}

//  Devuelve el estado (LCD_ON/LCD_OFF) del LCD

uint8 lcd_status( void )
{
    return state;
}

// Borra el LCD

void lcd_clear( void )
{
    uint16 i ;
    for (i = 0; i < LCD_BUFFER_SIZE; i++) {
        lcd_buffer[i] = WHITE;
    }
    //... lcd_putpixel(,,WHITE)
}

//  Pone el pixel (x,y) en el color indicado

void lcd_putpixel( uint16 x, uint16 y, uint8 c)
{
    uint8 byte, bit;
    uint16 i;

    i = x/2 + y*(LCD_WIDTH/2);
    bit = (1-x%2)*4;
    
    byte = lcd_buffer[i];
    byte &= ~(0xF << bit);
    byte |= c << bit;
    lcd_buffer[i] = byte;
}


//  Devuelve el color al que est· el pixel (x,y)

// LCD_WIDTH   (320)
// LCD_HEIGHT  (240)
uint8 lcd_getpixel( uint16 x, uint16 y )
{
    return lcd_buffer[x/2 + y*(LCD_WIDTH/2)];
    
}

// Dibuja una lÌnea horizontal desde el pixel (xleft,y) hasta el pixel (xright,y) del color y grosor indicados

void lcd_draw_hline( uint16 xleft, uint16 xright, uint16 y, uint8 color, uint16 width )
{
    uint16 i , j;
    
    for (j = y; j < width + y; j++){
        for (i = xleft; i < xright; i++){
            lcd_putpixel(i, j, color);
        }
    }
}

// Dibuja una lÌnea vertical desde el pixel (x,yup) hasta el pixel (x,ydown) del color y grosor indicados

void lcd_draw_vline( uint16 yup, uint16 ydown, uint16 x, uint8 color, uint16 width )
{
    uint16 i , j;

    for (j = x; j < width + x; j++){
        for (i = yup; i < ydown; i++){
            lcd_putpixel(j, i, color);
        }
    }
}

//  Dibuja un rect·ngulo cuya esquina superior izquierda est· en el pixel (xleft,yup) y cuya esquina inferior est· en el pÌxel (xright, ydown) del color y grosor indicados

void lcd_draw_box( uint16 xleft, uint16 yup, uint16 xright, uint16 ydown, uint8 color, uint16 width )
{
    lcd_draw_hline(xleft, xright, yup, color, width); // linea horizontal de arriba
    lcd_draw_vline(yup, ydown, xleft, color, width); //linea vertical de la izq
    //lcd_clear();
    lcd_draw_hline(xleft, xright+width, ydown, color, width); // linea horizontal de abajo
    lcd_draw_vline(yup, ydown, xright, color, width);// linea vertical  derecha
}

// Usando una fuente 8x16, escribe un caracter a partir del pixel (x,y) en el color indicado

void lcd_putchar( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( line=0; line<16; line++ )
        for( row=0; row<8; row++ )                    
            if( bitmap[line] & (0x80 >> row) )
                lcd_putpixel( x+row, y+line, color );
            else
                lcd_putpixel( x+row, y+line, WHITE );
}

// Usando una fuente 8x16, escribe una cadena a partir del pixel (x,y) en el color indicado

void lcd_puts( uint16 x, uint16 y, uint8 color, char *s )
{
    uint16 i = 0;
    while (s[i] != '\0'){
        lcd_putchar(x, y, color, s[i]);
        x+=8;
        if(x == LCD_WIDTH){
        	y+=8;
        }
        i++;
    }
}

// Usando una fuente 8x16, escribe una cadena que representa en decimal al entero que toma como argumento a partir del pixel (x,y) en el color indicado

void lcd_putint( uint16 x, uint16 y, uint8 color, int32 i )
{
    char buf[32 + 1];
    char *p = buf + 8;
    uint16 c;
    int negativo = 0;
    
    *p = '\0';
    if( i < 0){
        negativo = 1;
        i = -i; //asb(i);
    }
    do {
        c = i % 10;
        *--p = '0' + c; // cast a char
        i = i / 10;
    } while( i );
    
    if(negativo){
        *--p = '-';
    }
    lcd_puts(x, y, color, p);
}

//  Usando una fuente 8x16, escribe una cadena que representa en hexadecimal al entero que toma como argumento a partir del pixel (x,y) en el color indicado

void lcd_puthex( uint16 x, uint16 y, uint8 color, uint32 i )
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
    
    lcd_puts(x, y, color, p);
}

// Usando una fuente 8x16, escribe un caracter a doble tamaÒo a partir del pixel (x,y) en el color indicado

void lcd_putchar_x2( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 line, row;
    uint8 *bitmap;
    
    bitmap = font + ch*16;
    for( line=0; line<32; line++ )
        for( row=0; row<16; row++ )
            if( bitmap[line/2] & (0x80 >> (row/2)) ){
            	 lcd_putpixel( x+row, y+line, color );
               // lcd_putpixel( x+row*2, y+line*2, color );

            }
            else
                lcd_putpixel( x+row, y+line, WHITE );
}

// Usando una fuente 8x16, escribe una cadena a doble tamaÒo a partir del pixel (x,y) en el color indicado

void lcd_puts_x2( uint16 x, uint16 y, uint8 color, char *s )
{
	uint16 i = 0;
	    while (s[i] != '\0'){
	        lcd_putchar_x2(x, y, color, s[i]);
	        x+=16;
	        if(x == LCD_WIDTH-2){
	        	y+=16;
	        }
	        i++;
	    }
}

//  Usando una fuente 8x16, escribe una cadena a doble tamaÒo que representa en decimal al entero que toma como argumento a partir del pixel (x,y) en el color indicado

void lcd_putint_x2( uint16 x, uint16 y, uint8 color, int32 i )
{
	char buf[32 + 1];
	    char *p = buf + 8;
	    uint16 c;
	    int negativo = 0;

	    *p = '\0';
	    if( i < 0){
	        negativo = 1;
	        i = -i; //asb(i);
	    }
	    do {
	        c = i % 10;
	        *--p = '0' + c; // cast a char
	        i = i / 10;
	    } while( i );

	    if(negativo){
	        *--p = '-';
	    }
	    lcd_puts_x2(x, y, color, p);
}

//  Usando una fuente 8x16, escribe una cadena a doble tamaÒo que representa en hexadecimal al entero que toma como argumento a partir del pixel (x,y) en el color indicado

void lcd_puthex_x2( uint16 x, uint16 y, uint8 color, uint32 i )
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

	    lcd_puts_x2(x, y, color, p);
}

//  Muestra un BMP de 320x240 px y 16b/px

void lcd_putWallpaper( uint8 *bmp )
{
    uint32 headerSize;

    uint16 x, ySrc, yDst;
    uint16 offsetSrc, offsetDst;

    headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);

    bmp = bmp + headerSize;
    
    for( ySrc=0, yDst=LCD_HEIGHT-1; ySrc<LCD_HEIGHT; ySrc++, yDst-- )                                                                       
    {
        offsetDst = yDst*LCD_WIDTH/2;
        offsetSrc = ySrc*LCD_WIDTH/2;
        for( x=0; x<LCD_WIDTH/2; x++ )
            lcd_buffer[offsetDst+x] = ~bmp[offsetSrc+x];
    }
}

