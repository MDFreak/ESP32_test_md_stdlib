#include <Arduino.h>
#include <main.h>

#if (ESP32_TEST_MD_LIB > 0) // compiling only enabled inside own workspace
  // ----------------------------------------------------------------
  // --- tests for temporary usage
  // ----------------------------------------------------------------
    #define SCAN_I2C
    //#define TEST_OLED
  // ----------------------------------------------------------------
  // --- declarations
  // ----------------------------------------------------------------
  // --- system global control parameters
    static uint64_t anzUsCycles = 0ul;
    static uint64_t usLast      = 0ul;
    static uint64_t usTmp       = 0ul;
    static uint64_t usPerCycle  = 0ul;
    static uint32_t freeHeap    = 10000000;
    static uint8_t  firstrun    = true;
  // --- system local usable values
    int8_t          tmp_i8  = 0;
    int16_t         tmp_i16 = 0;
    int32_t         tmp_i32 = 0;
  // --- system devices
    // i2C devices
    #if defined(USE_I2C)
      // OLED
        #ifdef I2C1
            TwoWire i2c1 = TwoWire(0);
          #endif
        #ifdef I2C2
            TwoWire i2c2 = TwoWire(1);
          #endif
      #endif // USE_I2C
    // SPI devices
    #if ( DEV_VSPI > OFF )
        //SPIClass pVSPI(VSPI);
      #endif
    #if ( DEV_HSPI > OFF )
        //SPIClass pHSPI(HSPI);
      #endif
  // --- project devices
    // oled
      #if defined (USE_OLED_I2C)
          #if (OLED_DRV == OLED_DRV_1306)
              SSD1306Wire oled(OLED_I2C_ADDR, OLED_I2C_SDA, OLED_I2C_SCL, (OLEDDISPLAY_GEOMETRY) OLED_GEO);
            #else
              //SH1106Wire oled(OLED_I2C_ADDR, OLED_I2C_SDA, OLED_I2C_SCL, (OLEDDISPLAY_GEOMETRY) OLED_GEO);
              SH1106Wire oled(OLED_I2C_ADDR, OLED_I2C_SDA, OLED_I2C_SCL, (OLEDDISPLAY_GEOMETRY) OLED_GEO);
            #endif // OLED_DRV
          #ifdef TEST_OLED
              int oledCnt = 1;
            #endif
        #endif // USE_OLED_I2C
    // memory
      #if (USE_FRAM_I2C > OFF)
          #define FRAM1_I2C_ADDR      I2C_FRAM_50
          #define FRAM1_SIZE          0x8000
          #define FRAM1_I2C           I2C1
        #endif // USE_FRAM_I2C
    // sensors
      #if (USE_CCS811_I2C > OFF )
          #define CCS811_I2C_ADDR     I2C_CSS811_
          #define CCS811_I2C          I2C1
        #endif // USE_CCS811_I2C
      #if (USE_BME280_I2C > OFF )
          #define BME280_I2C         I2C1
        #endif // USE_BME280_I2C
      #if (USE_BME680_I2C > OFF )
          #define BME680_I2C         I2C1
        #endif // USE_BME680_I2C








// ----------------------------------------------------------------
// --- prototypes only for this file -----------------------------------
// ----------------------------------------------------------------
  // --- system --------------------------
    void heapFree(const char* text);
  // --- display
    #ifdef USE_DISP
        void dispStatus(String msg, bool direct);
        void dispStatus(const char* msg, bool direct);
        void dispText(String msg, uint8_t col, uint8_t row, uint8_t len);
        void dispText(char* msg, uint8_t col, uint8_t row, uint8_t len);
        void startDisp();
        #ifdef TEST_OLED
            void test_oled();
          #endif
      #endif // USE_OLED_I2C
// ----------------------------------------------------------------
// --- system setup -----------------------------------
// ----------------------------------------------------------------
  void setup()
    {
      // start system
        Serial.begin(SER_BAUDRATE);
        usleep(3000); // power-up safety delay
        STXT();
        STXT(" setup start ...");
      // disable watchdog
        STXT(" ... disable WD ...");
        disableCore0WDT();
        //disableCore1WDT();
        disableLoopWDT();
      // start I2C
        #ifdef SCAN_I2C
            scanI2C(&i2c1, PIN_I2C1_SDA, PIN_I2C1_SCL);
          #endif
      // start display
        #ifdef USE_DISP
            startDisp();
          #endif
      STXT(" ... setup finished");
    }

  void loop()
    {
      // put your main code here, to run repeatedly:
    }


// ----------------------------------------------------------------
// --- subroutine and drivers ----------------
// ----------------------------------------------------------------
  // --- system --------------------------
    // --- heap output
      void heapFree(const char* text)
        {
          uint32_t tmp32 = ESP.getFreeHeap();
          //uint32_t tmp32 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
          SVAL(text, tmp32);
        }
  // --- user output ---------------------
    // --- display
      #ifdef USE_DISP
          void clearDisp()
            {
              #if (USE_OLED_I2C > OFF)
                  oled.clear();
                #endif
              // #if (tft)
            }
          void dispStatus(String msg, bool direct)
            {
              #ifdef USE_STATUS
                  size_t statLen = msg.length();
                  bool   doIt    = false;
                  bool   info    = false;

                  if (statLen)
                    {
                      if ( statLen > OLED1_MAXCOLS)
                        {
                          msg.remove(OLED1_MAXCOLS);
                        }
                      statOn = true;
                      statT.startT();
                      doIt = true;    // output statOut
                      statT.startT();
                    }
                  else // empty input
                    {
                      if (statOn && statT.TOut())
                        statOn = false;
                    }
                  if (!statOn) // disp def val and actual time
                    {
                      if (statN.TOut())
                        {
                          statN.startT();
                          #if (USE_NTP_SERVER > OFF)
                              sprintf(statOut,"%02d.%02d. %02d:%02d:%02d ", day(), month(), hour(), minute(), second());
                              msg = statOut;
                              msg.concat(" ");
                              msg.concat((unsigned long) usPerCycle);
                              msg.concat("us");
                              info = true;
                              doIt = true;
                            #endif
                        }
                    }
                  if (doIt)
                    {
                      #if (USE_TOUCHSCREEN > OFF)
                          touch.wrStatus(msg);
                        #endif
                      #if (USE_OLED_I2C > OFF)
                          #if ( OLED1_STATUS > OFF )
                              oled1.wrStatus(msg);
                            #endif
                          #if ( USE_STATUS2 > OFF)
                              oled2.wrStatus(msg);
                            #endif
                        #endif
                      #if (USE_DISP_TFT > 0)
                          #if !(DISP_TFT ^ MC_UO_TFT1602_GPIO_RO)
                              mlcd.wrStatus((char*) statOut);
                            #endif
                                  //#if !(DISP_TFT ^ MC_UO_TOUCHXPT2046_AZ)
                                       //if (info)
                                       //  {
                                       //    #if ( USE_BME280_I2C > OFF )
                                       //        outStr[0] = 0;
                                       //        outStr.concat(bmeT.getVal());
                                       //        outStr.concat("° ");
                                       //        outStr.concat(bmeH.getVal());
                                       //        outStr.concat("% ");
                                       //        outStr.concat(bmeP.getVal());
                                       //        outStr.concat("mb  ");
                                       //      #endif
                                       //  }
                                       // outStr.concat((char*) statOut);
                                       // if (info)
                                       // {
                                       //   #if (USE_WEBSERVER > OFF)
                                       //       outStr.concat(" ");
                                       //       outStr.concat(WiFi.localIP().toString());
                                       //     #endif
                                       // }
                                      // #endif
                        #endif // USE_DISP
                      info = false;
                    }
                #endif // USE_STATUS
            }
          void dispStatus(const char* msg, bool direct)
            {
              //dispStatus((String) msg);
            }
          void dispText(String msg, uint8_t col, uint8_t row, uint8_t len)
            {
              #if (USE_OLED_I2C > OFF)
                  //oled.wrText(msg, col, row, len);
                #endif
              #if (USE_DISP_TFT > 0)
                  #if !(DISP_TFT ^ MC_UO_TFT1602_GPIO_RO)
                      mlcd.wrText(msg, row, col);
                    #endif
                  #if (USE_TOUCHSCREEN > OFF)
                      touch.wrText(msg, col, row, len);
                    #endif
                #endif
            }
          void dispText(char* msg, uint8_t col, uint8_t row, uint8_t len)
            {
              dispText((String) msg, col, row, len);
                /*
                    #if (USE_DISP > 0)
                        #if (USE_OLED1_I2C > OFF)
                            oled1.wrText(msg, col, row, len);
                          #endif
                        #if defined(OLED2)
                            oled2.wrText(msg, col, row, len);
                          #endif
                        #if (USE_DISP_TFT > 0)
                            #if !(DISP_TFT ^ MC_UO_TFT1602_GPIO_RO)
                                mlcd.wrText(msg, row, col);
                              #endif
                            #if !(DISP_TFT ^ MC_UO_TOUCHXPT2046_AZ)
                                touch.wrText(msg, col, row, len);
                              #endif
                          #endif
                        #if (USE_TOUCHSCREEN > OFF)
                          #endif
                      #endif
                  */
            }
          void startDisp()
            {
              String _str = "";
              #if defined (USE_OLED_I2C)
                  SOUT(" ... init OLED ...");
                  tmp_i8 = oled.init();
                  SVAL(" init returns ",tmp_i8);
                  #if (OLED_FLIP_VERTICAL > OFF)
                      oled.flipScreenVertically();
                    #endif
                  oled.setContrast(255);
                  oled.clear();
                  S2VAL(" ... OLED ... wide high ", oled.getWidth(), oled.getHeight());
                  oled.setFont(ArialMT_Plain_10);
                  oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  oled.drawString(0, oled.getHeight() - 10, "Hallo OLED"); oled.display();
                  STXT(" ... OLED is init");
                  #ifdef TEST_OLED
                      test_oled();
                    #endif
                #endif // USE_OLED_I2C
              #ifdef USE_STATUS
                statOut[OLED1_MAXCOLS] = 0;  // limit strlen
                #endif
              #if (USE_DISP_TFT > 0)
                  #if !(DISP_TFT ^ MC_UO_TFT1602_GPIO_RO)
                      mlcd.start(plcd);
                    #endif
                  #if (USE_TOUCHSCREEN_SPI > OFF)
                      touch.start(DISP_ORIENT, DISP_BCOL);
                          #if (DEBUG_MODE >= CFG_DEBUG_DETAILS)
                            STXT(" startTouch ");
                          #endif
                    #endif
                #endif
            }
          #ifdef TEST_OLED
              void drawLines()
                {
                  for (int16_t i = 0; i < oled.getWidth(); i += 4)
                    {
                      oled.drawLine(0, 0, i, oled.getHeight() - 1);
                      oled.display();
                      delay(10);
                    }
                  for (int16_t i = 0; i < oled.getHeight(); i += 4)
                    {
                      oled.drawLine(0, 0, oled.getWidth() - 1, i);
                      oled.display();
                      delay(10);
                    }
                  usleep(250000);
                  oled.clear();
                  for (int16_t i = 0; i < oled.getWidth(); i += 4)
                    {
                      oled.drawLine(0, oled.getHeight() - 1, i, 0);
                      oled.display();
                      delay(10);
                    }
                  for (int16_t i = oled.getHeight() - 1; i >= 0; i -= 4)
                    {
                      oled.drawLine(0, oled.getHeight() - 1, oled.getWidth() - 1, i);
                      oled.display();
                      delay(10);
                    }
                  usleep(250000);
                  oled.clear();
                  for (int16_t i = oled.getWidth() - 1; i >= 0; i -= 4)
                    {
                      oled.drawLine(oled.getWidth() - 1, oled.getHeight() - 1, i, 0);
                      oled.display();
                      delay(10);
                    }
                  for (int16_t i = oled.getHeight() - 1; i >= 0; i -= 4)
                    {
                      oled.drawLine(oled.getWidth() - 1, oled.getHeight() - 1, 0, i);
                      oled.display();
                      delay(10);
                    }
                  usleep(250000);
                  oled.clear();
                  for (int16_t i = 0; i < oled.getHeight(); i += 4)
                    {
                      oled.drawLine(oled.getWidth() - 1, 0, 0, i);
                      oled.display();
                      delay(10);
                    }
                  for (int16_t i = 0; i < oled.getWidth(); i += 4)
                    {
                      oled.drawLine(oled.getWidth() - 1, 0, i, oled.getHeight() - 1);
                      oled.display();
                      delay(10);
                    }
                  usleep(250000);
                }
              // Adapted from Adafruit_SSD1306
              void drawRect(void)
                {
                  for (int16_t i = 0; i < oled.getHeight() / 2; i += 2)
                    {
                      oled.drawRect(i, i, oled.getWidth() - 2 * i, oled.getHeight() - 2 * i);
                      oled.display();
                      usleep(10000);
                    }
                }
              // Adapted from Adafruit_SSD1306
              void fillRect(void)
                {
                  uint8_t color = 1;
                  for (int16_t i = 0; i < oled.getHeight() / 2; i += 3)
                    {
                      oled.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
                      oled.fillRect(i, i, oled.getWidth() - i * 2, oled.getHeight() - i * 2);
                      oled.display();
                      usleep(10000);
                      color++;
                    }
                  // Reset back to WHITE
                  oled.setColor(WHITE);
                }
              // Adapted from Adafruit_SSD1306
              void drawCircle(void)
                {
                  for (int16_t i = 0; i < oled.getHeight(); i += 2)
                    {
                      oled.drawCircle(oled.getWidth() / 2, oled.getHeight() / 2, i);
                      oled.display();
                      usleep(10000);
                   }
                  sleep(1);
                  oled.clear();
                  // This will draw the part of the circel in quadrant 1
                  // Quadrants are numberd like this:
                  //   0010 | 0001
                  //  ------|-----
                  //   0100 | 1000
                  //
                  oled.drawCircleQuads(oled.getWidth() / 2, oled.getHeight() / 2, oled.getHeight() / 4, 0b00000001);
                  oled.display();
                  usleep(200000);
                  oled.drawCircleQuads(oled.getWidth() / 2, oled.getHeight() / 2, oled.getHeight() / 4, 0b00000011);
                  oled.display();
                  usleep(200000);
                  oled.drawCircleQuads(oled.getWidth() / 2, oled.getHeight() / 2, oled.getHeight() / 4, 0b00000111);
                  oled.display();
                  usleep(200000);
                  oled.drawCircleQuads(oled.getWidth() / 2, oled.getHeight() / 2, oled.getHeight() / 4, 0b00001111);
                  oled.display();
                }
              void printBuffer(void)
                {
                  // Initialize the log buffer
                  // allocate memory to store 8 lines of text and 30 chars per line.
                  oled.setLogBuffer(5, 30);
                  // Some test data
                  const char* test[] =
                    {
                      "Hello",
                      "World" ,
                      "----",
                      "Show off",
                      "how",
                      "the log buffer",
                      "is",
                      "working.",
                      "Even",
                      "scrolling is",
                      "working"
                    };
                  for (uint8_t i = 0; i < 11; i++)
                    {
                      oled.clear();
                      // Print to the screen
                      oled.println(test[i]);
                      // Draw it to the internal screen buffer
                      oled.drawLogBuffer(0, 0);
                      // Display it on the screen
                      oled.display();
                      usleep(50000);
                    }
                }
              void drawFontFaceDemo()
                {
                  // Font Demo1
                  // create more fonts at http://oleddisplay.squix.ch/
                  STXT(" drawFontFaceDemo ");
                  oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  oled.setFont(ArialMT_Plain_10);
                  oled.drawString(0, 0, "Hello world");  oled.display();
                  oled.setFont(ArialMT_Plain_16);
                  oled.drawString(0, 10, "Hello world"); oled.display();
                  oled.setFont(ArialMT_Plain_24);
                  oled.drawString(0, 26, "Hello world"); oled.display();
                }
              void drawTextFlowDemo()
                {
                  oled.setFont(ArialMT_Plain_10);
                  oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  oled.drawStringMaxWidth(0, 0, 128,
                                             "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore." );
                  oled.display();
                }
              void drawTextAlignmentDemo()
                {
                  // Text alignment demo
                  oled.setFont(ArialMT_Plain_10);
                  // The coordinates define the left starting point of the text
                  oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  oled.drawString(0, 10, "Left aligned (0,10)");      oled.display();
                  // The coordinates define the center of the text
                  oled.setTextAlignment(TEXT_ALIGN_CENTER);
                  oled.drawString(64, 22, "Center aligned (64,22)");  oled.display();
                  // The coordinates define the right end of the text
                  oled.setTextAlignment(TEXT_ALIGN_RIGHT);
                  oled.drawString(128, 33, "Right aligned (128,33)"); oled.display();
                }
              void drawRectDemo()
                {
                  // Draw a pixel at given position
                  for (int i = 0; i < 10; i++)
                    {
                      oled.setPixel(i, i);
                      oled.setPixel(10 - i, i);
                    }
                  oled.drawRect(12, 12, 20, 20);
                  // Fill the rectangle
                  oled.fillRect(14, 14, 17, 17);
                  // Draw a line horizontally
                  oled.drawHorizontalLine(0, 40, 20);
                  // Draw a line horizontally
                  oled.drawVerticalLine(40, 0, 20);
                }
              void drawCircleDemo()
                {
                  for (int i = 1; i < 8; i++)
                    {
                      oled.setColor(WHITE);
                      oled.drawCircle(32, 32, i * 3);      oled.display();
                      if (i % 2 == 0)
                        {
                          oled.setColor(BLACK);
                        }
                      oled.fillCircle(96, 32, 32 - i * 3); oled.display();
                    }
                }
              void drawProgressBarDemo()
                {
                  uint8_t progress = 0;
                  uint8_t lin0     = oled.getHeight() - 10;
                  uint8_t widthbar = oled.getWidth()  - 30;
                  //(oledCnt * 50) % 100;
                  // draw the progress bar
                  //oled.setFont(ssd1306xled_font6x8_AB);
                  oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  while (progress <= 100)
                    {
                      oled.setColor(BLACK);
                      oled.fillRect(0, lin0, 26, 12);
                      oled.setColor(WHITE);
                      oled.drawProgressBar(28, lin0+3, widthbar, 4, progress); //oled.display();
                      oled.drawString(0, lin0, String(progress) + "%"); oled.display();
                      SVAL(" progress ", progress);
                      progress+=5;
                      usleep(500000);
                    }
                  // draw the percentage as String
                  //oled.setTextAlignment(TEXT_ALIGN_LEFT);
                  //oled.drawString(0, 15, String(progress) + "%"); oled.display();
                }
              //void drawImageDemo()
                //{
                //  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
                //  // on how to create xbm files
                //  oled.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
                //}
              void test_oled()
                {
                    drawLines();
                    sleep(1);
                    oled.clear();
                    drawRect();
                    sleep(1);
                    oled.clear();
                    fillRect();
                    sleep(1);
                    oled.clear();
                    drawCircle();
                    sleep(1);
                    oled.clear();
                    printBuffer();
                    sleep(1);
                    oled.clear();
                    drawFontFaceDemo();
                    sleep(1);
                    oled.clear();
                    drawTextFlowDemo();
                    sleep(1);
                    oled.clear();
                    drawTextAlignmentDemo();
                    sleep(1);
                    oled.clear();
                    drawRectDemo();
                    sleep(1);
                    oled.clear();
                    drawCircleDemo();
                    sleep(1);
                    oled.clear();
                    drawProgressBarDemo();
                    sleep(1);
                    oled.clear();
                }
            #endif // TEST_OLED
        #endif // USE_DISP

#endif // ESP32_TEST_MD_LIB

