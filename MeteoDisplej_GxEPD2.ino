/**
 * Seriovy port jede na 115200 bps.
 * 

80 MHz / 40 MHz 

Konfiguracni promenne:
  lat
  lon
  alt
  idorp
  name
  alojz - jmeno mista v Alojzovi
  url_alojz - https://alojz.cz/api/v1/solution?url_id=/
  
 * Konfiguracni portal se spusti automaticky pokud zarizeni nema konfiguraci.
 * Nebo je mozne jej spustit tlacitkem FLASH (D0) - stisknout a drzet tlacitko >3 sec pote, co se pri startu rychle rozblika indikacni LED. 
 */

/*
 * ESP 32:
 * - V Arduino IDE MUSI byt nastaveno rozdeleni flash tak, aby bylo alespon kousek filesystemu SPIFS !
*/

//+++++ RatatoskrIoT +++++

  #include "AppConfig.h"

  // RA objects
  ratatoskr* ra;
  raLogger* logger;
  raConfig config;
  Tasker tasker;

  // stav WiFi - je pripojena?
  bool wifiOK = false;
  // cas, kdy byla nastartovana wifi
  long wifiStartTime = 0;
  // duvod posledniho startu, naplni se automaticky
  int wakeupReason;
  // je aktualni start probuzenim z deep sleep?
  bool deepSleepStart;

  #ifdef RA_STORAGE_RTC_RAM 
    RTC_DATA_ATTR unsigned char ra_storage[RA_STORAGE_SIZE];
  #endif

  #ifdef USE_BLINKER
    raBlinker blinker( BLINKER_PIN );
    int blinkerPortal[] = BLINKER_MODE_PORTAL;
    int blinkerSearching[]  = BLINKER_MODE_SEARCHING;
    int blinkerRunning[] = BLINKER_MODE_RUNNING;
    int blinkerRunningWifi[] = BLINKER_MODE_RUNNING_WIFI;
  #endif  

//----- RatatoskrIoT ----

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// konfigurace hardware
#include "AppPins.h"
#include "ePaperConfig.h"

// komponenty aplikace
#include "ExtDisplay.h"
#include "DataAplikace.h"
#include "PredpovedAlojz.h"
#include "InfoSlunce.h"
#include "InfoMesic.h"



DataAplikace * dataAplikace;
PredpovedAlojz * predpovedAlojz;
InfoSlunce * infoSlunce;
InfoMesic * infoMesic;

GxEPD2_GFX* display;
ExtDisplay extdisplay;

/*
 * Pokud user code oznaci posledni poslana data znackou 
 *    ra->setAllDataPreparedFlag();
 * automaticky se zavola, jakmile jsou vsechna data odeslana.
 * 
 * Typicke pouziti je ve scenari, kdy chceme po probuzeni odeslat balik dat a zase zarizeni uspat.
 * 
 * Pro pripad, ze se odeslani z nejakeho duvodu nezdari, doporucuji do setup() pridat zhruba toto:
 *    tasker.setTimeout( raAllWasSent, 60000 );
 * s nastavenym maximalnim casem, jak dlouho muze byt zarizeni probuzeno (zde 60 sec).
 * Tim se zajisti, ze dojde k deep sleepu i v pripade, ze z nejakeho duvodu nejde data odeslat.
 */
void raAllWasSent()
{
  // v teto aplikaci se nepouziva
}


/**
 * Vraci jmeno aplikace do alokovaneho bufferu.
 * Jmeno aplikace by nemelo obsahovat strednik.
 */
void raGetAppName( char * target, int size )
{
  snprintf( target, size, "%s, %s %s", 
            __FILE__, __DATE__, __TIME__ 
            );  
  target[size-1] = 0;
}


/** kanal pro pocet prekresleni */
int ch1;  

void setup() {

  // musi byt driv nez ratatoskr_startup(), protoze spusti serial!
  display1.init(115200); 
  // a udelame si pointer pro predavani do objektu
  display = &display1 ;

  // *** special handling for Waveshare ESP32 Driver board *** //
  // ********************************************************* //
  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  //SPI: void begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);
  SPI.begin(EPAPER_SCLK, SPI_MISO_UNUSED, EPAPER_MOSI, EPAPER_CS); // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  // *** end of special handling for Waveshare ESP32 Driver board *** //

  //+++++ RatatoskrIoT +++++
    // Mel by byt jako prvni v setup().
    // Pokud je parametr true, zapne Serial pro rychlost 115200. Jinak musi byt Serial zapnuty uz drive, nez je tohle zavolano.
    ratatoskr_startup( false );
  //----- RatatoskrIoT ----

  //++++++ user code here +++++
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  // nacteni konfigurace
  dataAplikace = new DataAplikace( logger, &config );
  extdisplay.init( display, logger );

  // kanaly pro posilani dat na server
  // pocet prekresleni displeje
  ch1 = ra->defineChannel( DEVCLASS_IMPULSE_SUM, 7, (char*)"redraw", 4600 ); 
  if( !deepSleepStart ) {
    // pokud to neni probuzeni z deep sleepu, posleme tez informaci o cold startu
    int ch2 = ra->defineChannel( DEVCLASS_IMPULSE_SUM, 7, (char*)"reboot", 4600 );
    ra->postImpulseData( ch2, 15, 1 );
  }

  // konfigurace jednotlivych modulu
  predpovedAlojz = new PredpovedAlojz( logger, &config, dataAplikace );
  infoSlunce = new InfoSlunce( logger, dataAplikace );
  infoMesic = new InfoMesic( logger, dataAplikace );

  startWifi();
  //------ user code here -----
}

void loop() {
  //+++++ RatatoskrIoT +++++
    // do scheduled tasks
    tasker.loop();
  //----- RatatoskrIoT ----

  //++++++ user code here +++++
  //------ user code here ------
}

/**
 * Zajistuje vlastni vykresleni vseho na displej
 */ 
void doDraw()
{
  bool firstRun = true;

  // most e-papers have width < height (portrait) as native orientation, especially the small ones
  // in GxEPD2 rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
  // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
  display->setRotation(0);
  // full window mode is the initial mode, set it anyway
  display->setFullWindow();

  // here we use paged drawing, even if the processor has enough RAM for full buffer
  // so this can be used with any supported processor board.
  // the cost in code overhead and execution time penalty is marginal
  // tell the graphics class to use paged drawing mode
  display->firstPage();
  do
  {
    extdisplay.setPos( 1, 22 );
    infoSlunce->drawData( &extdisplay, firstRun );

    // stejny radek jako mesic, ale v pulce displeje
    extdisplay.posX = 160;
    extdisplay.setBbFullWidth();
    infoMesic->drawData( &extdisplay, firstRun );
    
    extdisplay.setPos( 5, 55 );
    extdisplay.setBbRightMargin( 5 );

    if( predpovedAlojz->hasData() ) {
      predpovedAlojz->drawData( &extdisplay, firstRun );
      // posY je nastavena na posledni radek textu z Aloise, tak ji posuneme na dalsi
      extdisplay.posY += extdisplay.vyskaRadku;
    } else {
      logger->log( "Alojz nema data" );
    }

    // posX/Y uz mame nastavenu

    firstRun = false;
  }
  // tell the graphics class to transfer the buffer content (page) to the controller buffer
  // the graphics class will command the controller to refresh to the screen when the last page has been transferred
  // returns true if more pages need be drawn and transferred
  // returns false if the last page has been transferred and the screen refreshed for panels without fast partial update
  // returns false for panels with fast partial update when the controller buffer has been written once more, to make the differential buffers equal
  // (for full buffered with fast partial update the (full) buffer is just transferred again, and false returned)
  while (display->nextPage());

}



/**
 * stahne a spocte vsechna data 
 * a pak zavola funkci doDraw() na jejich vykresleni
 */ 
void delej()
{
  // nejprve stahneme vsechna data a vse si spocteme

  predpovedAlojz->loadData();
  infoSlunce->compute();
  infoMesic->compute();

  // a pak vse najednou vykreslime
  logger->log( "Vykresluji:");

  #ifdef USE_BLINKER
        blinker.off();
  #endif 

  doDraw();

  // vratime puvodni blikani
  #ifdef USE_BLINKER
        blinker.setCode( blinkerRunningWifi );
  #endif 
}



//------------------------------------ callbacks from WiFi +++


void wifiStatus_StartingConfigPortal(  char * apName, char *apPassword, char * ipAddress   )
{
  // +++ user code here +++
    logger->log( "Starting AP [%s], password [%s]. Server IP [%s].", apName, apPassword, ipAddress );
  // --- user code here ---
}

/**
 * Spocte pauzu do dalsiho vykresleni
 */ 
long computeNextRefreshPause()
{
  // prekreslujeme jednou za 20 minut
  long rc = 1200000;

  // v noci prekreslujeme jen jednou za 40 minut
  time_t now = time(NULL);
  struct tm * cas = localtime( &now );
  if( cas->tm_hour >= 22 || cas->tm_hour <= 5 ) {
    rc = 2400000;
  }
  return rc;
}

int redrawCount = 0;

/**
 * Je spusteno automaticky, pokud mame pripojene WiFi.
 * (z callbacku wifiStatus_Connected() via Tasker )
 */ 
void doActions()
{
  // mame wifi -> sync casu
  delay( 500 );
  timeSync( dataAplikace );
  
  long nextStart = computeNextRefreshPause();
  if( dataAplikace->timeSynced ) {
    // vykreslit
    delej();
    redrawCount++;
    ra->postImpulseData( ch1, 1, redrawCount );
  } else {
    // kdyz neni sync, zkusime to za chvili znovu = zkratime cas
    logger->log( "Nemame sync casu" );
    nextStart = 30000;
  }
  logger->log( "Dalsi beh za %d min; prekresleno %d krat", (nextStart/60000), redrawCount );

  raShipLogs();
  
  // nechci vypnout wifi rovnou odsud, chci aby nejprve dobehlo odeslani dat
  tasker.setTimeout( stopWifi, 5000 );
  // a nastavime cas dalsiho startu
  tasker.setTimeout( startWifi, nextStart );
}


void wifiStatus_Connected(  int status, int waitTime, char * ipAddress  )
{
  // +++ user code here +++
  logger->log("* wifi [%s], %d dBm, %d s", ipAddress, WiFi.RSSI(), waitTime );
  // neni dobre to tady rovnou volat, je lepsi si jen zaregistrovat callback
  tasker.setTimeout( doActions, 1000 );
  // --- user code here ---
}

void wifiStatus_NotConnected( int status, long msecNoConnTime )
{
  // +++ user code here +++
    char desc[32];
    getWifiStatusText( status, desc );
    logger->log("* no wifi (%s), %d s", desc, (msecNoConnTime / 1000L) );
  // --- user code here ---
}

void wifiStatus_Starting()
{
  // +++ user code here +++
  // --- user code here ---
}

/**
   Je zavolano pri startu.
   - Pokud vrati TRUE, startuje se config portal.
   - Pokud FALSE, pripojuje se wifi.
   Pokud nejsou v poradku konfiguracni data, otevira se rovnou config portal a tato funkce se nezavola.
*/
bool wifiStatus_ShouldStartConfig()
{
  // +++ user code here +++
    pinMode(CONFIG_BUTTON, INPUT_PULLUP);

    // Pokud se pouziva FLASH button na NodeMCU, je treba zde dat pauzu, 
    // aby ho uzivatel stihl zmacknout (protoze ho nemuze drzet pri resetu),
    // jinak je mozne usporit cas a energii tim, ze se rovnou pojede.
    logger->log( "Sepni pin %d pro config portal", CONFIG_BUTTON );
    delay(3000);

    if ( digitalRead(CONFIG_BUTTON) == CONFIG_BUTTON_START_CONFIG ) {
      logger->log( "Budu spoustet config portal!" );
      return true;
    } else {
      return false;
    }
  // --- user code here ---
}
//------------------------------------ callbacks from WiFi ---

/*
ESP32 1.0.6
Using library FS at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\FS 
Using library WiFi at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\WiFi 
Using library HTTPClient at version 1.2 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\HTTPClient 
Using library WiFiClientSecure at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\WiFiClientSecure 
Using library WebServer at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\WebServer 
Using library DNSServer at version 1.1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\DNSServer 
Using library Tasker at version 2.0 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Tasker 
Using library Ticker at version 1.1 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Ticker 
Using library GxEPD2 at version 1.3.1 in folder: C:\Users\brouzda\Documents\Arduino\libraries\GxEPD2 
Using library Adafruit_GFX_Library at version 1.10.0 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Adafruit_GFX_Library 
Using library SPI at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\SPI 
Using library ArduinoJson at version 6.18.0 in folder: C:\Users\brouzda\Documents\Arduino\libraries\ArduinoJson 
Using library SPIFFS at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\SPIFFS 
Using library MoonRise at version 2.0.1 in folder: C:\Users\brouzda\Documents\Arduino\libraries\MoonRise 
Using library MoonPhase at version 1.0.3 in folder: C:\Users\brouzda\Documents\Arduino\libraries\MoonPhase 
Using library Update at version 1.0 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Update 
Using library Adafruit_BusIO at version 1.4.1 in folder: C:\Users\brouzda\Documents\Arduino\libraries\Adafruit_BusIO 
Using library Wire at version 1.0.1 in folder: C:\Users\brouzda\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Wire 
*/
