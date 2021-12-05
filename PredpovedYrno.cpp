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





#define SIRKA_SLOUPCE 50
#define POCET_SLOUPCU 8
#define ZUZENI_RADKU -2

int PredpovedYrno::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    if( this->jsonData==NULL ) return false;

    time_t now = time(NULL);
    struct tm * cas = localtime( &now );
    bool wasCurHour = false;

    char buffer[100];

    extdisplay->posY += 10;

    int x_offset, x_offset2;
    int x = extdisplay->posX;

    extdisplay->display->setTextColor( BARVA_TEXTU );

    for( int i = 0; i<POCET_SLOUPCU; i++ ) {
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

        extdisplay->posX = x;
        extdisplay->setBbFullWidth();

        extdisplay->setFont( fnt_YanoneSB11() );
        sprintf( buffer, "%s h", hour );
        extdisplay->printUTF8( buffer  );

        extdisplay->posY += extdisplay->vyskaRadku + ZUZENI_RADKU;
        sprintf( buffer, "%.0f", temp );
        x_offset = extdisplay->printUTF8( buffer  );
        
        extdisplay->setFont( fnt_YanoneSB9() );
        sprintf( buffer, " °C" );
        extdisplay->printUTF8( buffer, x_offset );
        extdisplay->setFont( fnt_YanoneSB11() );
        
        if( rain > 0 ) {
            sprintf( buffer, "%.1f", rain );
            extdisplay->posY += extdisplay->vyskaRadku + ZUZENI_RADKU;
            x_offset = extdisplay->printUTF8( buffer );
            
            extdisplay->setFont( fnt_YanoneSB9() );
            sprintf( buffer, " mm" );
            extdisplay->printUTF8( buffer, x_offset );
            extdisplay->setFont( fnt_YanoneSB11() );

            extdisplay->posY -= extdisplay->vyskaRadku + ZUZENI_RADKU;
        } 

        extdisplay->posY -= extdisplay->vyskaRadku + ZUZENI_RADKU;
        x += SIRKA_SLOUPCE;
    }
    
    return 0;
}