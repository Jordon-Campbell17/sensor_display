//##################################################################################################################
//##                           SENSOR DATA ACQUISITION AND TFT DISPLAY SYSTEM                                     ##
//##                                                                                                              ##
//##  Sensors:                                                                                                    ##
//##  - DHT22  : Temperature and Humidity         -> GPIO4                                                       ##
//##  - BMP280 : Atmospheric Pressure             -> SDA:GPIO21, SCL:GPIO22                                      ##
//##  - Soil Moisture Sensor : Soil Moisture      -> GPIO34                                                      ##
//##                                                                                                              ##
//##  Display:                                                                                                    ##
//##  - TFT ILI9341 : 240x320 SPI Display         -> CS:5, DC:17, RST:16, MOSI:23, SCK:18, MISO:19              ##
//##################################################################################################################


// ############### LIBRARIES ###############
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>


// ############### TFT PIN DEFINITIONS ###############
#define TFT_CS    5
#define TFT_RST   16
#define TFT_DC    17
#define TFT_MOSI  23
#define TFT_SCK   18
#define TFT_MISO  19


// ############### SENSOR PIN DEFINITIONS ###############
#define DHT_PIN       4       // DHT22 Data Pin
#define DHT_TYPE      DHT22   // DHT22 Sensor Type
#define SOIL_PIN      34      // Soil Moisture Analog Pin


// ############### SENSOR CONSTANTS ###############
#define SOIL_DRY      4095    // Analog value when soil is completely dry
#define SOIL_WET      0       // Analog value when soil is completely wet


// ############### COLOUR DEFINITIONS ###############
#define BACKGROUND    ILI9341_BLACK
#define HEADER_BG     0x1B4F  // Dark blue
#define CARD_BG       0x2104  // Dark grey
#define TEXT_WHITE    ILI9341_WHITE
#define TEXT_CYAN     ILI9341_CYAN
#define TEXT_YELLOW   ILI9341_YELLOW
#define TEXT_GREEN    ILI9341_GREEN
#define TEXT_RED      ILI9341_RED
#define TEXT_ORANGE   0xFD20  // Orange
#define ACCENT        0x055F  // Teal


// ############### OBJECTS ###############
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;


// ############### VARIABLES ###############
float temperature   = 0.0;
float humidity      = 0.0;
float pressure      = 0.0;
float heatIndex     = 0.0;
int   soilRaw       = 0;
int   soilPercent   = 0;

float prevTemperature   = -999;
float prevHumidity      = -999;
float prevPressure      = -999;
float prevHeatIndex     = -999;
int   prevSoilPercent   = -999;


// ############### FUNCTION DECLARATIONS ###############
void drawUI(void);
void drawHeader(void);
void updateSensorValues(void);
void updateDisplay(void);
String getSoilStatus(int percent);
uint16_t getSoilColor(int percent);
uint16_t getTempColor(float temp);
uint16_t getHumidityColor(float hum);


// ############### SETUP ###############
void setup() {
    Serial.begin(115200);
    Serial.println("\n\nSensor Display System Starting...");

    // INIT TFT
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(BACKGROUND);
    Serial.println("TFT Initialized");

    // INIT DHT22
    dht.begin();
    Serial.println("DHT22 Initialized");

    // INIT BMP280
    if (!bmp.begin(0x76)) {
        if (!bmp.begin(0x77)) {
            Serial.println("BMP280 not found! Check wiring.");
        } else {
            Serial.println("BMP280 Initialized at 0x77");
        }
    } else {
        Serial.println("BMP280 Initialized at 0x76");
    }

    // CONFIGURE BMP280
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);

    // CONFIGURE SOIL MOISTURE PIN
    pinMode(SOIL_PIN, INPUT);

    // DRAW INITIAL UI
    drawUI();

    Serial.println("Setup Complete!");
}


// ############### MAIN LOOP ###############
void loop() {
    // READ ALL SENSORS
    updateSensorValues();

    // UPDATE DISPLAY WITH NEW VALUES
    updateDisplay();

    // PRINT TO SERIAL FOR DEBUGGING
    Serial.printf("Temp: %.1f C  Humidity: %.1f%%  Pressure: %.1f hPa  HeatIndex: %.1f C  Soil: %d%%\n",
        temperature, humidity, pressure, heatIndex, soilPercent);

    delay(2000);
}


// ############### READ ALL SENSOR VALUES ###############
void updateSensorValues() {
    // READ DHT22
    float newTemp = dht.readTemperature();
    float newHum  = dht.readHumidity();

    if (!isnan(newTemp)) temperature = newTemp;
    if (!isnan(newHum))  humidity    = newHum;

    // CALCULATE HEAT INDEX
    if (!isnan(temperature) && !isnan(humidity)) {
        heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    }

    // READ BMP280
    float newPressure = bmp.readPressure() / 100.0F;
    if (newPressure > 0) pressure = newPressure;

    // READ SOIL MOISTURE
    soilRaw     = analogRead(SOIL_PIN);
    soilPercent = map(soilRaw, SOIL_DRY, SOIL_WET, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);
}


// ############### DRAW FULL UI LAYOUT ###############
void drawUI() {
    tft.fillScreen(BACKGROUND);

    // DRAW HEADER
    drawHeader();

    // DRAW SENSOR CARDS
    // Temperature
    tft.fillRoundRect(5, 75, 230, 50, 6, CARD_BG);
    tft.drawRoundRect(5, 75, 230, 50, 6, TEXT_CYAN);

    // Humidity
    tft.fillRoundRect(5, 133, 230, 50, 6, CARD_BG);
    tft.drawRoundRect(5, 133, 230, 50, 6, TEXT_CYAN);

    // Pressure
    tft.fillRoundRect(5, 191, 230, 50, 6, CARD_BG);
    tft.drawRoundRect(5, 191, 230, 50, 6, TEXT_CYAN);

    // Heat Index
    tft.fillRoundRect(5, 249, 110, 50, 6, CARD_BG);
    tft.drawRoundRect(5, 249, 110, 50, 6, TEXT_CYAN);

    // Soil Moisture
    tft.fillRoundRect(125, 249, 110, 50, 6, CARD_BG);
    tft.drawRoundRect(125, 249, 110, 50, 6, TEXT_CYAN);

    // DRAW CARD LABELS
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextSize(1);

    tft.setTextColor(TEXT_CYAN);
    tft.setCursor(12, 92);
    tft.print("Temperature");

    tft.setCursor(12, 150);
    tft.print("Humidity");

    tft.setCursor(12, 208);
    tft.print("Pressure");

    tft.setCursor(12, 266);
    tft.print("Heat Idx");

    tft.setCursor(132, 266);
    tft.print("Soil");
}


// ############### DRAW HEADER ###############
void drawHeader() {
    tft.fillRect(0, 0, 240, 70, HEADER_BG);
    tft.drawRect(0, 0, 240, 70, ACCENT);

    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(TEXT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(20, 25);
    tft.print("Sensor Monitor");

    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(TEXT_CYAN);
    tft.setCursor(35, 50);
    tft.print("ELET2415 - 620162191");
}


// ############### UPDATE DISPLAY VALUES ###############
void updateDisplay() {
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextSize(1);

    // UPDATE TEMPERATURE
    if (temperature != prevTemperature) {
        tft.fillRect(10, 96, 220, 22, CARD_BG);
        tft.setTextColor(getTempColor(temperature));
        tft.setCursor(12, 113);
        tft.printf("%.1f C", temperature);
        prevTemperature = temperature;
    }

    // UPDATE HUMIDITY
    if (humidity != prevHumidity) {
        tft.fillRect(10, 154, 220, 22, CARD_BG);
        tft.setTextColor(getHumidityColor(humidity));
        tft.setCursor(12, 171);
        tft.printf("%.1f %%", humidity);
        prevHumidity = humidity;
    }

    // UPDATE PRESSURE
    if (pressure != prevPressure) {
        tft.fillRect(10, 212, 220, 22, CARD_BG);
        tft.setTextColor(TEXT_YELLOW);
        tft.setCursor(12, 229);
        tft.printf("%.1f hPa", pressure);
        prevPressure = pressure;
    }

    // UPDATE HEAT INDEX
    if (heatIndex != prevHeatIndex) {
        tft.fillRect(10, 270, 100, 22, CARD_BG);
        tft.setTextColor(TEXT_ORANGE);
        tft.setCursor(12, 287);
        tft.printf("%.1f C", heatIndex);
        prevHeatIndex = heatIndex;
    }

    // UPDATE SOIL MOISTURE
    if (soilPercent != prevSoilPercent) {
        tft.fillRect(130, 270, 100, 22, CARD_BG);
        tft.setTextColor(getSoilColor(soilPercent));
        tft.setCursor(132, 287);
        tft.printf("%d%%", soilPercent);
        prevSoilPercent = soilPercent;
    }
}


// ############### HELPER FUNCTIONS ###############

uint16_t getTempColor(float temp) {
    if (temp < 18.0)  return ILI9341_BLUE;
    if (temp < 25.0)  return TEXT_GREEN;
    if (temp < 30.0)  return TEXT_YELLOW;
    return TEXT_RED;
}

uint16_t getHumidityColor(float hum) {
    if (hum < 30.0)  return TEXT_YELLOW;
    if (hum < 60.0)  return TEXT_GREEN;
    return ILI9341_BLUE;
}

uint16_t getSoilColor(int percent) {
    if (percent < 20)  return TEXT_RED;
    if (percent < 40)  return TEXT_YELLOW;
    if (percent < 70)  return TEXT_GREEN;
    return ILI9341_BLUE;
}

String getSoilStatus(int percent) {
    if (percent < 20)  return "Very Dry";
    if (percent < 40)  return "Dry";
    if (percent < 70)  return "Moist";
    return "Wet";
}