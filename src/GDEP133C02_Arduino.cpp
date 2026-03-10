#include "GDEP133C02_Arduino.h"

// Configuration arrays
const uint8_t GDEP133C02::PSR_V[2] = {0xDF, 0x69};
const uint8_t GDEP133C02::PWR_V[6] = {0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38};
const uint8_t GDEP133C02::POF_V[1] = {0x00};
const uint8_t GDEP133C02::DRF_V[1] = {0x01};
const uint8_t GDEP133C02::CDI_V[1] = {0xF7};
const uint8_t GDEP133C02::TCON_V[2] = {0x03, 0x03};
const uint8_t GDEP133C02::TRES_V[4] = {0x04, 0xB0, 0x03, 0x20};
const uint8_t GDEP133C02::CMD66_V[6] = {0x49, 0x55, 0x13, 0x5D, 0x05, 0x10};
const uint8_t GDEP133C02::EN_BUF_V[1] = {0x07};
const uint8_t GDEP133C02::CCSET_V[1] = {0x01};
const uint8_t GDEP133C02::PWS_V[1] = {0x22};
const uint8_t GDEP133C02::AN_TM_V[9] = {0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55};
const uint8_t GDEP133C02::AGID_V[1] = {0x10};
const uint8_t GDEP133C02::BTST_P_V[2] = {0xE8, 0x28};
const uint8_t GDEP133C02::BOOST_VDDP_EN_V[1] = {0x01};
const uint8_t GDEP133C02::BTST_N_V[2] = {0xE8, 0x28};
const uint8_t GDEP133C02::BUCK_BOOST_VDDN_V[1] = {0x01};
const uint8_t GDEP133C02::TFT_VCOM_POWER_V[1] = {0x02};

GDEP133C02::GDEP133C02(uint8_t rst, uint8_t busy, uint8_t cs0, uint8_t cs1, 
                       uint8_t mosi, uint8_t miso, uint8_t sclk) 
    : _rst(rst), _busy(busy), _cs0(cs0), _cs1(cs1), 
      _mosi(mosi), _miso(miso), _sclk(sclk),
      _spiSettings(10000000, MSBFIRST, SPI_MODE0),
      partialWindowUpdateStatus(DONE) {
}

void GDEP133C02::begin() {
    // Initialize pins
    pinMode(_rst, OUTPUT);
    pinMode(_busy, INPUT);
    pinMode(_cs0, OUTPUT);
    pinMode(_cs1, OUTPUT);
    
    digitalWrite(_rst, HIGH);
    digitalWrite(_cs0, HIGH);
    digitalWrite(_cs1, HIGH);
    
    // Initialize SPI
    _spi = &SPI;
    _spi->begin(_sclk, _miso, _mosi, -1);
    
    Serial.println("GDEP133C02: Initialization complete");
}

void GDEP133C02::setPinCs(uint8_t csNumber, uint8_t level) {
    if (csNumber == 0) {
        digitalWrite(_cs0, level);
    } else if (csNumber == 1) {
        digitalWrite(_cs1, level);
    }
}

void GDEP133C02::setPinCsAll(uint8_t level) {
    digitalWrite(_cs0, level);
    digitalWrite(_cs1, level);
}

void GDEP133C02::checkBusyHigh() {
    while (digitalRead(_busy) == LOW) {
        delay(10);
    }
}

void GDEP133C02::checkBusyLow() {
    while (digitalRead(_busy) == HIGH) {
        delay(10);
    }
}

void GDEP133C02::epdHardwareReset() {
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(20);
}

void GDEP133C02::spiTransmitCommand(uint8_t commandBuf) {
    _spi->beginTransaction(_spiSettings);
    _spi->transfer(commandBuf);
    _spi->endTransaction();
}

void GDEP133C02::spiTransmitData(const uint8_t *dataBuffer, uint32_t dataLength) {
    _spi->beginTransaction(_spiSettings);
    for (uint32_t i = 0; i < dataLength; i++) {
        _spi->transfer(dataBuffer[i]);
    }
    _spi->endTransaction();
}

void GDEP133C02::spiReceiveData(uint8_t *dataBuffer, uint32_t dataLength) {
    _spi->beginTransaction(_spiSettings);
    for (uint32_t i = 0; i < dataLength; i++) {
        dataBuffer[i] = _spi->transfer(0x00);
    }
    _spi->endTransaction();
}

void GDEP133C02::spiTransmitLargeData(uint8_t commandBuf, const uint8_t *dataBuffer, uint32_t dataLength) {
    _spi->beginTransaction(_spiSettings);
    
    // Send command first
    _spi->transfer(commandBuf);
    
    // Send data
    for (uint32_t i = 0; i < dataLength; i++) {
        _spi->transfer(dataBuffer[i]);
    }
    
    _spi->endTransaction();
}

void GDEP133C02::spiTransmit(uint8_t commandBuf, const uint8_t *dataBuffer, uint16_t dataLength) {
    _spi->beginTransaction(_spiSettings);
    
    // Send command
    _spi->transfer(commandBuf);
    
    // Send data
    for (uint16_t i = 0; i < dataLength; i++) {
        _spi->transfer(dataBuffer[i]);
    }
    
    _spi->endTransaction();
}

void GDEP133C02::spiReceive(uint8_t commandBuf, uint8_t *dataBuffer, uint16_t dataLength) {
    _spi->beginTransaction(_spiSettings);
    
    // Send command
    _spi->transfer(commandBuf);
    
    // Receive data
    for (uint16_t i = 0; i < dataLength; i++) {
        dataBuffer[i] = _spi->transfer(0x00);
    }
    
    _spi->endTransaction();
}

void GDEP133C02::writeEpdCommand(uint8_t epdCommand) {
    spiTransmitCommand(epdCommand);
}

void GDEP133C02::writeEpdData(const uint8_t *epdData, uint32_t epdDataLength) {
    spiTransmitData(epdData, epdDataLength);
}

void GDEP133C02::writeEpd(uint8_t epdCommand, const uint8_t *epdData, uint16_t epdDataLength) {
    spiTransmit(epdCommand, epdData, epdDataLength);
}

void GDEP133C02::readEpd(uint8_t epdCommand, uint8_t *epdData, uint16_t epdDataLength) {
    spiReceive(epdCommand, epdData, epdDataLength);
}

void GDEP133C02::initEPD() {
    epdHardwareReset();
    checkBusyHigh();
    
    setPinCs(0, LOW);
    writeEpd(AN_TM, AN_TM_V, sizeof(AN_TM_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(CMD66, CMD66_V, sizeof(CMD66_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(PSR, PSR_V, sizeof(PSR_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(CDI, CDI_V, sizeof(CDI_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(TCON, TCON_V, sizeof(TCON_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(AGID, AGID_V, sizeof(AGID_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(PWS, PWS_V, sizeof(PWS_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(CCSET, CCSET_V, sizeof(CCSET_V));
    setPinCsAll(HIGH);
    
    setPinCsAll(LOW);
    writeEpd(TRES, TRES_V, sizeof(TRES_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(PWR, PWR_V, sizeof(PWR_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(BTST_P, BTST_P_V, sizeof(BTST_P_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(BTST_N, BTST_N_V, sizeof(BTST_N_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V));
    setPinCsAll(HIGH);
    
    setPinCs(0, LOW);
    writeEpd(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V));
    setPinCsAll(HIGH);
    
    Serial.println("initEPD() complete");
}

uint8_t GDEP133C02::checkDriverICStatus() {
    uint8_t status = DONE;
    uint8_t dataBuf[3];
    
    for (uint8_t csx = 0; csx < 2; csx++) {
        memset(dataBuf, 0, sizeof(dataBuf));
        setPinCs(csx, LOW);
        readEpd(0xF2, dataBuf, sizeof(dataBuf));
        setPinCs(csx, HIGH);
        
        Serial.printf("Driver IC [%d] = 0x%02X 0x%02X 0x%02X\n", csx, dataBuf[0], dataBuf[1], dataBuf[2]);
        
        if ((dataBuf[0] & 0x01) == 0x01) {
            Serial.printf("Driver IC [%d] is ready.\n", csx);
        } else {
            Serial.printf("Driver IC [%d] did not reply.\n", csx);
            status = ERROR;
        }
    }
    
    return status;
}

void GDEP133C02::epdDisplay() {
    Serial.println("Write PON");
    setPinCsAll(LOW);
    writeEpdCommand(PON);
    checkBusyHigh();
    setPinCsAll(HIGH);
    
    Serial.println("Write DRF");
    setPinCsAll(LOW);
    delay(30);
    writeEpd(DRF, DRF_V, sizeof(DRF_V));
    checkBusyHigh();
    setPinCsAll(HIGH);
    
    Serial.println("Write POF");
    setPinCsAll(LOW);
    writeEpd(POF, POF_V, sizeof(POF_V));
    checkBusyHigh();
    setPinCsAll(HIGH);
    
    Serial.println("Display Done!");
}

void GDEP133C02::epdDisplayColor(uint8_t colorSelect) {
    uint32_t totalSize = 480000;
    
    memset(epdImageDataBuffer, colorSelect, EPD_IMAGE_DATA_BUFFER);
    
    setPinCsAll(LOW);
    writeEpdCommand(DTM);
    
    for (uint32_t i = 0; i < totalSize / EPD_IMAGE_DATA_BUFFER; i++) {
        writeEpdData(epdImageDataBuffer, EPD_IMAGE_DATA_BUFFER);
    }
    
    uint32_t remaining = totalSize % EPD_IMAGE_DATA_BUFFER;
    if (remaining > 0) {
        writeEpdData(epdImageDataBuffer, remaining);
    }
    
    setPinCsAll(HIGH);
    
    epdDisplay();
    
    Serial.println("Display color complete");
}

void GDEP133C02::epdDisplayColorBar() {
    const uint8_t colors[] = {BLACK, WHITE, YELLOW, RED, BLUE, GREEN};
    const uint32_t segmentSize = 80000;
    
    setPinCsAll(LOW);
    writeEpdCommand(DTM);
    
    for (uint8_t colorIdx = 0; colorIdx < 6; colorIdx++) {
        memset(epdImageDataBuffer, colors[colorIdx], EPD_IMAGE_DATA_BUFFER);
        
        for (uint32_t i = 0; i < segmentSize / EPD_IMAGE_DATA_BUFFER; i++) {
            writeEpdData(epdImageDataBuffer, EPD_IMAGE_DATA_BUFFER);
        }
        
        uint32_t remaining = segmentSize % EPD_IMAGE_DATA_BUFFER;
        if (remaining > 0) {
            writeEpdData(epdImageDataBuffer, remaining);
        }
    }
    
    setPinCsAll(HIGH);
    
    epdDisplay();
    
    Serial.println("Display color bar complete");
}

void GDEP133C02::writeEpdImage(uint8_t csx, const uint8_t *imageData, uint32_t imageDataLength) {
    setPinCs(csx, LOW);
    spiTransmitLargeData(DTM, imageData, imageDataLength);
    setPinCs(csx, HIGH);
    
    Serial.println("Writing data completed");
}

void GDEP133C02::picDisplayTest(const uint8_t *imageData) {
    uint16_t width = (EPD_WIDTH % 2 == 0) ? (EPD_WIDTH / 2) : (EPD_WIDTH / 2 + 1);
    uint16_t width1 = (width % 2 == 0) ? (width / 2) : (width / 2 + 1);
    uint16_t height = EPD_HEIGHT;
    
    const uint32_t firstPackSize = 480000;
    const uint32_t secondPackSize = 480000;
    
    // First section
    setPinCsAll(HIGH);
    setPinCs(0, LOW);
    writeEpdCommand(DTM);
    writeEpdData(imageData, firstPackSize);
    setPinCs(0, HIGH);
    
    // Second section
    setPinCs(1, LOW);
    writeEpdCommand(DTM);
    writeEpdData(imageData + firstPackSize, secondPackSize);
    setPinCs(1, HIGH);
    
    epdDisplay();
    
    Serial.println("Picture display test complete");
}

int8_t GDEP133C02::partialWindowUpdateWithImageData(uint8_t csx, const uint8_t *imageData, 
                                                      uint32_t imageDataLength,
                                                      uint16_t xStart, uint16_t yStart, 
                                                      uint16_t xPixel, uint16_t yLine, 
                                                      bool epdDisplayEnable) {
    int8_t status = DONE;
    uint16_t HRST, HRED, VRST, VRED;
    uint8_t partialWindowData[9];
    
    HRST = xStart * 2;
    HRED = (xStart + xPixel) * 2 - 1;
    VRST = yStart / 2;
    VRED = (yStart + yLine) / 2 - 1;
    
    Serial.printf("csx=%d HRST=%d HRED=%d VRST=%d VRED=%d\n", csx, HRST, HRED, VRST, VRED);
    
    // Validation
    if (HRST % 8 != 0) {
        status = -1;
        Serial.println("Error: xStart problem");
    } else if ((HRED - 7) % 8 != 0) {
        status = -2;
        Serial.println("Error: xPixel problem");
    } else if ((xStart > 584) || (xPixel > 600)) {
        status = -3;
        Serial.println("Error: xStart or xPixel out of range");
    } else if ((HRED - HRST + 1 < 32) || (HRED + 1 > 1200)) {
        status = -4;
        Serial.println("Error: xStart & xPixel problem");
    } else if ((yStart + yLine) % 2 != 0) {
        status = -5;
        Serial.println("Error: yStart + yLine must be even");
    } else if ((yStart > 1596) || (yLine > 1600)) {
        status = -6;
        Serial.println("Error: yStart or yLine out of range");
    } else if (((int)(VRED - VRST) + 1 <= 0) || (VRED + 1 > 800)) {
        status = -7;
        Serial.println("Error: yStart & yLine problem");
    } else if (csx > 1) {
        status = -8;
        Serial.println("Error: csx problem");
    } else {
        memset(partialWindowData, 0, sizeof(partialWindowData));
        partialWindowData[0] = (uint8_t)(HRST >> 8);
        partialWindowData[1] = (uint8_t)(HRST);
        partialWindowData[2] = (uint8_t)(HRED >> 8);
        partialWindowData[3] = (uint8_t)(HRED);
        partialWindowData[4] = (uint8_t)(VRST >> 8);
        partialWindowData[5] = (uint8_t)(VRST);
        partialWindowData[6] = (uint8_t)(VRED >> 8);
        partialWindowData[7] = (uint8_t)(VRED);
        partialWindowData[8] = PTLW_ENABLE;
        
        setPinCs(csx, LOW);
        writeEpd(CMD66, CMD66_V, sizeof(CMD66_V));
        setPinCs(csx, HIGH);
        
        setPinCs(csx, LOW);
        writeEpd(PTLW, partialWindowData, sizeof(partialWindowData));
        setPinCs(csx, HIGH);
        
        setPinCs(csx, LOW);
        spiTransmitLargeData(DTM, imageData, imageDataLength);
        setPinCs(csx, HIGH);
    }
    
    if (status != DONE) {
        partialWindowUpdateStatus = ERROR;
        Serial.println("partialWindowUpdateStatus = ERROR");
    }
    
    if (epdDisplayEnable) {
        if (partialWindowUpdateStatus == DONE) {
            epdDisplay();
        }
        
        delay(300);
        
        // Turn off PTLW
        memset(partialWindowData, 0, sizeof(partialWindowData));
        partialWindowData[8] = PTLW_DISABLE;
        partialWindowUpdateStatus = DONE;
        
        setPinCsAll(LOW);
        writeEpd(PTLW, partialWindowData, sizeof(partialWindowData));
        setPinCsAll(HIGH);
    }
    
    return status;
}

int8_t GDEP133C02::partialWindowUpdateWithoutImageData(uint8_t csx, uint16_t xStart, 
                                                         uint16_t yStart, uint16_t xPixel, 
                                                         uint16_t yLine, bool epdDisplayEnable) {
    int8_t status = DONE;
    uint16_t HRST, HRED, VRST, VRED;
    uint8_t partialWindowData[9];
    
    HRST = xStart * 2;
    HRED = (xStart + xPixel) * 2 - 1;
    VRST = yStart / 2;
    VRED = (yStart + yLine) / 2 - 1;
    
    Serial.printf("csx=%d HRST=%d HRED=%d VRST=%d VRED=%d\n", csx, HRST, HRED, VRST, VRED);
    
    // Validation (same as above)
    if (HRST % 8 != 0) {
        status = -1;
        Serial.println("Error: xStart problem");
    } else if ((HRED - 7) % 8 != 0) {
        status = -2;
        Serial.println("Error: xPixel problem");
    } else if ((xStart > 584) || (xPixel > 600)) {
        status = -3;
        Serial.println("Error: xStart or xPixel out of range");
    } else if ((HRED - HRST + 1 < 32) || (HRED + 1 > 1200)) {
        status = -4;
        Serial.println("Error: xStart & xPixel problem");
    } else if ((yStart + yLine) % 2 != 0) {
        status = -5;
        Serial.println("Error: yStart + yLine must be even");
    } else if ((yStart > 1596) || (yLine > 1600)) {
        status = -6;
        Serial.println("Error: yStart or yLine out of range");
    } else if (((int)(VRED - VRST) + 1 <= 0) || (VRED + 1 > 800)) {
        status = -7;
        Serial.println("Error: yStart & yLine problem");
    } else if (csx > 1) {
        status = -8;
        Serial.println("Error: csx problem");
    } else {
        memset(partialWindowData, 0, sizeof(partialWindowData));
        partialWindowData[0] = (uint8_t)(HRST >> 8);
        partialWindowData[1] = (uint8_t)(HRST);
        partialWindowData[2] = (uint8_t)(HRED >> 8);
        partialWindowData[3] = (uint8_t)(HRED);
        partialWindowData[4] = (uint8_t)(VRST >> 8);
        partialWindowData[5] = (uint8_t)(VRST);
        partialWindowData[6] = (uint8_t)(VRED >> 8);
        partialWindowData[7] = (uint8_t)(VRED);
        partialWindowData[8] = PTLW_ENABLE;
        
        setPinCs(csx, LOW);
        writeEpd(CMD66, CMD66_V, sizeof(CMD66_V));
        setPinCs(csx, HIGH);
        
        setPinCs(csx, LOW);
        writeEpd(PTLW, partialWindowData, sizeof(partialWindowData));
        setPinCs(csx, HIGH);
    }
    
    if (status != DONE) {
        partialWindowUpdateStatus = ERROR;
        Serial.println("partialWindowUpdateStatus = ERROR");
    }
    
    if (epdDisplayEnable) {
        if (partialWindowUpdateStatus == DONE) {
            epdDisplay();
        }
        
        delay(300);
        
        // Turn off PTLW
        memset(partialWindowData, 0, sizeof(partialWindowData));
        partialWindowData[8] = PTLW_DISABLE;
        partialWindowUpdateStatus = DONE;
        
        setPinCsAll(LOW);
        writeEpd(PTLW, partialWindowData, sizeof(partialWindowData));
        setPinCsAll(HIGH);
    }
    
    return status;
}
