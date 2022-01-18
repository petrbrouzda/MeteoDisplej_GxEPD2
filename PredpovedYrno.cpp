/**
 * Varování ČHMÚ
 */ 

#include <Arduino.h>

#include "PredpovedYrno.h"
#include "DataAplikace.h"

#include "src/fonts/fonts.h"

// maximalni stari dat v sekundach
#define DATA_MAX_AGE 14400


PredpovedYrno::PredpovedYrno( raLogger * logger, raConfig * config,  DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->config = config;
    this->dataAplikace = dataAplikace;
}


void PredpovedYrno::loadData()
{
    char url[255];
    sprintf( url, "%s/yrno/forecast?lat=%f&lon=%f&alt=%d",
        this->config->getString( "url_predpoved", "http://lovecka.info/YrNoProvider1" ),
        dataAplikace->pozice_lat,
        dataAplikace->pozice_lon,
        dataAplikace->pozice_altitude );       

    logger->log( "Stahuji: %s", url );

    HTTPClient http;

    // Send request
    http.useHTTP10(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.begin(url);
    int httpResponseCode = http.GET();

    if( httpResponseCode==200 ) {
        // Parse response
        DynamicJsonDocument * doc = new DynamicJsonDocument(10000);
        DeserializationError de = deserializeJson( *doc, http.getStream());

        if( !de ) {
            logger->log( "OK parsed");
            if( this->jsonData!=NULL ) {
                delete this->jsonData;
            }
            this->jsonData = doc;
            this->casStazeni = time(NULL);
        } else {
            logger->log( "JsonParse: error %s", de.c_str() );
            delete doc;
        }
    } else {
        logger->log( "http: error %d", httpResponseCode );
    }

    // Disconnect
    http.end();   
}


boolean PredpovedYrno::hasData()
{
    if( this->jsonData!=NULL 
        && ( time(NULL) - this->casStazeni ) < DATA_MAX_AGE ) {
          return true;
    } else {
      return false;
    }
}





#define SIRKA_PRVNIHO_SLOUPCE 60
#define SIRKA_SLOUPCE 34
#define POCET_SLOUPCU 10
#define ZMENA_VYSKY_RADKU 1
#define OFFSET_LINKY_Y 4
#define OFFSET_LINKY_X -4

int PredpovedYrno::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    if( this->jsonData==NULL ) return false;

    time_t now = time(NULL);
    struct tm * cas = localtime( &now );
    bool wasCurHour = false;

    char buffer[100];

    extdisplay->posY += 10;

    int x_offset;
    int x = 5;

    extdisplay->display->setTextColor( BARVA_TEXTU );
    extdisplay->setFont( fnt_YanoneSB11() );
    int vyskaRadku = extdisplay->vyskaRadku + ZMENA_VYSKY_RADKU;

    
    extdisplay->display->drawLine( x, extdisplay->posY + OFFSET_LINKY_Y, 
                                    extdisplay->display->width() - 1, extdisplay->posY + OFFSET_LINKY_Y , 
                                    GxEPD_BLACK);

    extdisplay->display->drawLine( x, extdisplay->posY + OFFSET_LINKY_Y + vyskaRadku, 
                                    extdisplay->display->width() - 1,  extdisplay->posY + OFFSET_LINKY_Y + vyskaRadku, 
                                    GxEPD_BLACK);

    
    extdisplay->posX = x;
    extdisplay->printUTF8( "Čas" );
    extdisplay->posY += vyskaRadku;
    extdisplay->printUTF8( "Teplota" );
    extdisplay->posY += vyskaRadku;
    extdisplay->printUTF8( "Srážky" );
    extdisplay->posY -= vyskaRadku + vyskaRadku;

    x = x + SIRKA_PRVNIHO_SLOUPCE;

    for( int i = 0; i<POCET_SLOUPCU; i++ ) {

        extdisplay->display->drawLine( x + OFFSET_LINKY_X, extdisplay->posY - vyskaRadku + OFFSET_LINKY_Y, 
                                       x + OFFSET_LINKY_X, extdisplay->posY + vyskaRadku + OFFSET_LINKY_Y + vyskaRadku, 
                                    GxEPD_BLACK);

        const char* hour = (* this->jsonData)["hours"][i]["hour"];
        if( hour==NULL ) break;

        int hr = atoi( hour );
        if( hr == cas->tm_hour ) {
            wasCurHour = true;
        }

        if( !wasCurHour ) {
            continue;
        }

        double temp = (* this->jsonData)["hours"][i]["temp"];
        double rain = (* this->jsonData)["hours"][i]["rain"];
        const char* icon = (* this->jsonData)["hours"][i]["icon"];

        sprintf( buffer, "[%s %s %.0f", hour, icon, temp );
        if( rain > 0 ) {
            sprintf( buffer+strlen(buffer), " %.1f mm", rain );
        } 
        strcat( buffer, "] " );

        if( firstRun) this->logger->log( "## %s", buffer );

        extdisplay->setFont( fnt_YanoneSB11() );

        extdisplay->posX = x;
        extdisplay->setBbFullWidth();
        sprintf( buffer, "%s", hour );
        extdisplay->printUTF8( buffer );

        extdisplay->posY += vyskaRadku;
        sprintf( buffer, "%.0f", temp );
        extdisplay->printUTF8( buffer  );
        
        if( rain > 0 ) {
            sprintf( buffer, "%.1f", rain );
            extdisplay->posY += vyskaRadku;
            extdisplay->printUTF8( buffer );
            extdisplay->posY -= vyskaRadku;
        } 

        extdisplay->posY -= vyskaRadku;
        x += SIRKA_SLOUPCE;
    }

    extdisplay->posY += 3*vyskaRadku;
    
    return 0;
}