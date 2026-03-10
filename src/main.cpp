#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WifiPortal.h>

#define SHOW_LOG 1

#include "pindefine.h"

#include "GDEP133C02_Arduino.h"
#include "html.h"

#define SPI_MOSI EPD_SPI_Data0
#define SPI_MISO EPD_SPI_Data1
#define SPI_SCLK EPD_SPI_CLK
#define SPI_CS0 EPD_SPI_CS0
#define SPI_CS1 EPD_SPI_CS1

GDEP133C02 epd(EPD_RST, EPD_BUSY, SPI_CS0, SPI_CS1, SPI_MOSI, SPI_MISO, SPI_SCLK);

WifiPortal wifiPortal("app");

WebServer server(80);

uint8_t *imageBuffer = nullptr;
const uint32_t IMAGE_SIZE = 960000;
uint32_t receivedTotal = 0;

// Image buffer
bool allocateImageBuffer()
{
    if (imageBuffer == nullptr)
    {
        imageBuffer = (uint8_t *)ps_malloc(IMAGE_SIZE);
        if (imageBuffer == nullptr)
        {
            Serial.println("ERROR: Failed to allocate");
            return false;
        }
        Serial.printf("✓ Buffer: %d bytes\n", IMAGE_SIZE);
    }
    return true;
}

// Display image on EPD
void displayImage()
{
    Serial.println("\n=== Display ===");

    if (receivedTotal != IMAGE_SIZE)
    {
        Serial.printf("ERROR: %d/%d bytes\n", receivedTotal, IMAGE_SIZE);
        return;
    }

    Serial.printf("First: %02X %02X %02X %02X\n",
                  imageBuffer[0], imageBuffer[1], imageBuffer[2], imageBuffer[3]);

    Serial.println("Displaying...");
    unsigned long t = millis();

    epd.epdDisplayColor(0x11); // 白で全消去
    delay(1000);
    epd.picDisplayTest(imageBuffer);

    Serial.printf("✓ Done in %lu ms\n\n", millis() - t);

    receivedTotal = 0;
}


// HTTP Handlers
void handleRoot()
{
    server.send(200, "text/html", htmlPage);
}

void handleUpload()
{
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("UploadStart: %s\n", upload.filename.c_str());
        receivedTotal = 0;
        allocateImageBuffer();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        Serial.printf("UploadWrite: %s, Size: %d\n", upload.filename.c_str(), upload.currentSize);
        if (imageBuffer && receivedTotal + upload.currentSize <= IMAGE_SIZE)
        {
            memcpy(imageBuffer + receivedTotal, upload.buf, upload.currentSize);
            receivedTotal += upload.currentSize;
        }
        yield();
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        Serial.printf("UploadEnd: %s, Size: %d\n", upload.filename.c_str(), upload.totalSize);
        displayImage();
    }
}

// Arduino setup and loop
void setup()
{
    Serial.begin(115200);
    delay(1000);

    // EPD initialization
    Serial.println("Initializing EPD...");
    epd.begin();
    epd.initEPD();

    if (epd.checkDriverICStatus() == 0)
    {
        Serial.println("✓ EPD initialized successfully");
    }
    else
    {
        Serial.println("✗ EPD initialization failed");
    }

    // WiFi setup
    wifiPortal.startConfigPortal({"host_name"});

    Preferences pref;
    pref.begin("app", true);
    String hostName = pref.getString("host_name", "epd133");
    pref.end();


    // mDNS responder
    if (MDNS.begin(hostName.c_str()))
    {
        Serial.println("✓ mDNS responder started: http://" + hostName + ".local/");
    }

    // Allocate image buffer
    allocateImageBuffer();

    // HTTP server setup
    server.on("/", handleRoot);
    server.on("/upload", HTTP_POST, []()
              { server.send(200, "text/plain", "OK"); }, handleUpload);

    server.begin();
    Serial.println("✓ Server started: http://" + WiFi.localIP().toString());
}

void loop()
{
    server.handleClient();
}
