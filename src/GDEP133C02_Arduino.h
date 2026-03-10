#ifndef __GDEP133C02_ARDUINO_H__
#define __GDEP133C02_ARDUINO_H__

#include <Arduino.h>
#include <SPI.h>

// Color definitions
#define BLACK         0x00
#define WHITE         0x11
#define YELLOW        0x22
#define RED           0x33
#define BLUE          0x55
#define GREEN         0x66

// EPD Commands
#define PSR             0x00
#define PWR             0x01
#define POF             0x02
#define PON             0x04
#define BTST_N          0x05
#define BTST_P          0x06
#define DTM             0x10
#define DRF             0x12
#define CDI             0x50
#define TCON            0x60
#define TRES            0x61
#define PTLW            0x83
#define AN_TM           0x74
#define AGID            0x86
#define BUCK_BOOST_VDDN 0xB0
#define TFT_VCOM_POWER  0xB1
#define EN_BUF          0xB6
#define BOOST_VDDP_EN   0xB7
#define CCSET           0xE0
#define PWS             0xE3
#define CMD66           0xF0

// Status definitions
#define DONE  0
#define ERROR -1

// PTLW control
#define PTLW_ENABLE  0x01
#define PTLW_DISABLE 0x00

// Display parameters
#define EPD_WIDTH 1200
#define EPD_HEIGHT 1600
#define EPD_IMAGE_DATA_BUFFER 8192

class GDEP133C02 {
public:
    // Constructor
    GDEP133C02(uint8_t rst, uint8_t busy, uint8_t cs0, uint8_t cs1, 
               uint8_t mosi, uint8_t miso, uint8_t sclk);
    
    // Initialization
    void begin();
    void initEPD();
    
    // Basic EPD operations
    void epdHardwareReset();
    void epdDisplay();
    void epdDisplayColor(uint8_t colorSelect);
    void epdDisplayColorBar();
    
    // Image display
    void writeEpdImage(uint8_t csx, const uint8_t *imageData, uint32_t imageDataLength);
    void picDisplayTest(const uint8_t *imageData);
    
    // Partial window update
    int8_t partialWindowUpdateWithImageData(uint8_t csx, const uint8_t *imageData, 
                                             uint32_t imageDataLength,
                                             uint16_t xStart, uint16_t yStart, 
                                             uint16_t xPixel, uint16_t yLine, 
                                             bool epdDisplayEnable);
    int8_t partialWindowUpdateWithoutImageData(uint8_t csx, uint16_t xStart, 
                                                 uint16_t yStart, uint16_t xPixel, 
                                                 uint16_t yLine, bool epdDisplayEnable);
    
    // Status check
    uint8_t checkDriverICStatus();
    
private:
    // Pin definitions
    uint8_t _rst;
    uint8_t _busy;
    uint8_t _cs0;
    uint8_t _cs1;
    uint8_t _mosi;
    uint8_t _miso;
    uint8_t _sclk;
    
    SPIClass *_spi;
    SPISettings _spiSettings;
    
    // Image buffer
    uint8_t epdImageDataBuffer[EPD_IMAGE_DATA_BUFFER];
    
    // Status
    int8_t partialWindowUpdateStatus;
    
    // CS pin control
    void setPinCs(uint8_t csNumber, uint8_t level);
    void setPinCsAll(uint8_t level);
    
    // BUSY signal check
    void checkBusyHigh();
    void checkBusyLow();
    
    // EPD read/write operations
    void writeEpd(uint8_t epdCommand, const uint8_t *epdData, uint16_t epdDataLength);
    void readEpd(uint8_t epdCommand, uint8_t *epdData, uint16_t epdDataLength);
    void writeEpdCommand(uint8_t epdCommand);
    void writeEpdData(const uint8_t *epdData, uint32_t epdDataLength);
    
    // SPI operations
    void spiTransmitCommand(uint8_t commandBuf);
    void spiTransmitData(const uint8_t *dataBuffer, uint32_t dataLength);
    void spiReceiveData(uint8_t *dataBuffer, uint32_t dataLength);
    void spiTransmitLargeData(uint8_t commandBuf, const uint8_t *dataBuffer, uint32_t dataLength);
    void spiTransmit(uint8_t commandBuf, const uint8_t *dataBuffer, uint16_t dataLength);
    void spiReceive(uint8_t commandBuf, uint8_t *dataBuffer, uint16_t dataLength);
    
    // Configuration arrays
    static const uint8_t PSR_V[2];
    static const uint8_t PWR_V[6];
    static const uint8_t POF_V[1];
    static const uint8_t DRF_V[1];
    static const uint8_t CDI_V[1];
    static const uint8_t TCON_V[2];
    static const uint8_t TRES_V[4];
    static const uint8_t CMD66_V[6];
    static const uint8_t EN_BUF_V[1];
    static const uint8_t CCSET_V[1];
    static const uint8_t PWS_V[1];
    static const uint8_t AN_TM_V[9];
    static const uint8_t AGID_V[1];
    static const uint8_t BTST_P_V[2];
    static const uint8_t BOOST_VDDP_EN_V[1];
    static const uint8_t BTST_N_V[2];
    static const uint8_t BUCK_BOOST_VDDN_V[1];
    static const uint8_t TFT_VCOM_POWER_V[1];
};

#endif // __GDEP133C02_ARDUINO_H__
