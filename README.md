# S3C44B0X-S3CEV40
## Simulacion de cola de supermercado con microcontrolador S3C44B0X y placa de prototipado Embest S3CEV40 y RTOS

S3C44B0X : Microcontrolador SoC (System-on-Chip de Samsung (1 Core ARM7TDMI (66 Mhz), cache configurable, 3 buses(local, sistema, perisferico) y 26 dispositivos internos (controladores).

La placa de prototipado Embest S3CEV40 conecta el microcontrolador S·C44B0X con un conjunto de dispositivos externos a los que puede acceder.

Para este proyecto se realizo la configuracion de:
  - Display 7-seg, LEDs y pulsadores.
  - I/O (**UART** y comunicacion con un terminal serie) por canal **RS-232**
  - Gestion de interrupciones (Programacion de un reloj en tiempo real (**RTC**))
  - Medidas de tiempo (Temporizadores) para pulsadores y keypad
  - Salida por LCD (Driver LCD) -> Bitmaps
  - Controlador Bus **IIC** (Lectura y escritura de una **EEPROM** AT24C04)
  - I/O por **DMA** y bus **IIS** (Reproduccion y grabacion de sonido con un Audio Codec UDA1341TS)
  - Conversor analogico-digital **A/D** (Entrada por un touchscreen)
  - Autoarranque desde **ROM**
  - Multitarea bajo **RTOS uC/OS-II**
  - Aplicacion multitarea bajo kernel de planificacion no expropiativo.
  
![GitHub Logo](/images/micro-prot.jpeg)
  
  
  


