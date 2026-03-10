

#ifndef __PINDEFINE_H__
#define __PINDEFINE_H__

//==============  Standard SPI Setting   ==============//
//Please modify the pin number
#define EPD_SPI_CS0		18
#define EPD_SPI_CS1		17
#define EPD_SPI_CLK		9
#define EPD_SPI_Data0	41
#define EPD_SPI_Data1	40
#define EPD_SPI_Data2	39
#define EPD_SPI_Data3	38

//==============   GPIO Setting   ==============//
//Please modify the pin number
#define EPD_BUSY	7   // Please set it as input pin
#define EPD_RST		6   // Please set it as output pin
#define LOAD_SW		45  // Please set it as output pin

//===============================================

#define GPIO_LOW	0
#define GPIO_HIGH	1

#endif //#ifndef __PINDEFINE_H__
