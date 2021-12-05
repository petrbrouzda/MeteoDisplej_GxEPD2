/**
 * Varování ČHMÚ
 */ 

#include <Arduino.h>

#include "InfoMeteostanice.h"
#include "DataAplikace.h"

#include "src/fonts/fonts.h"

// maximalni stari dat v sekundach
#define DATA_MAX_AGE 14400


InfoMeteostanice::InfoMeteostanice( raLogger * logger, raConfig * config,  DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->config = config;
    this->dataAplikace = dataAplikace;
}


void InfoMeteostanice::loadData()
{
    char url[255];
    sprintf( url, "%s/%s/%s/?temp=%s&rain=%s",
        this->config->getString( "meteo_url", "http://lovecka.info/ra/json/meteo" ),
        this->config->getString( "meteo_token", "628hx2r7rn6s0u7z8amivkawtuq08vzom13mf2p0" ),
        this->config->getString( "meteo_id", "23" ),
        this->config->getString( "meteo_temp", "bd358d05" ),
        this->config->getString( "meteo_rain", "rain" )
        );  

    logger->log( "Stahuji: %s", url );

    HTTPClient http;

    // Send request
    http.useHTTP10(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.begin(url);
    int httpResponseCode = http.GET();

    if( httpResponseCode==200 ) {
        // Parse response
        DynamicJsonDocument * doc = new DynamicJsonDocument(2500);
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


boolean InfoMeteostanice::hasData()
{
    if( this->jsonData!=NULL 
        && ( time(NULL) - this->casStazeni ) < DATA_MAX_AGE ) {
          return true;
    } else {
      return false;
    }
}




#define TEXT_ROW_SPACING 20
#define BOX_WIDTH 80
#define ROW_OFFSET 18
#define BOX_HEIGHT 58


void printOneBox( ExtDisplay * extdisplay, int posX, int posY, char * nadpis, char * teplota, char * srazky ) {

    extdisplay->setFont( fnt_YanoneSB11() );
    extdisplay->display->setTextColor( BARVA_NADPISU );
    extdisplay->setPos( posX, posY + ROW_OFFSET ); 
    extdisplay->printUTF8(nadpis ); 

    extdisplay->display->setTextColor( BARVA_TEXTU );
    extdisplay->setPos( posX, posY+TEXT_ROW_SPACING + ROW_OFFSET ); 
    extdisplay->printUTF8( teplota ); 

    extdisplay->setPos( posX, posY+TEXT_ROW_SPACING+TEXT_ROW_SPACING + ROW_OFFSET -4 ); 
    extdisplay->printUTF8( srazky ); 
}


int InfoMeteostanice::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    if( this->jsonData==NULL ) return false;

    int posX = 5;
    int posY = extdisplay->display->height() - BOX_HEIGHT - 1;

    double tyden_srazky = (* this->jsonData)["tyden"]["rain"];
    double tyden_temp_min = (* this->jsonData)["tyden"]["temp_min"];
    double tyden_temp_max = (* this->jsonData)["tyden"]["temp_max"];

    double vcera_srazky = (* this->jsonData)["vcera"]["rain"];
    double vcera_temp_min = (* this->jsonData)["vcera"]["temp_min"];
    double vcera_temp_max = (* this->jsonData)["vcera"]["temp_max"];

    double noc_srazky = (* this->jsonData)["noc"]["rain"];
    double noc_temp_min = (* this->jsonData)["noc"]["temp_min"];
    double noc_temp_max = (* this->jsonData)["noc"]["temp_max"];

    double dnes_srazky = (* this->jsonData)["dnes"]["rain"];
    double dnes_temp_min = (* this->jsonData)["dnes"]["temp_min"];
    double dnes_temp_max = (* this->jsonData)["dnes"]["temp_max"];

    double temp = (* this->jsonData)["nyni"]["temp"];
    const char* stav = (* this->jsonData)["nyni"]["valid"];

    char temp_str[15];
    if( stav!=NULL && stav[0]=='Y' ) {
        sprintf( temp_str, "%.1f°C", temp );
    } else {
        // nemame platna data - ze senzoru neprisly uz dlouho
        sprintf( temp_str, "---", temp );
    }

    if( firstRun ) {
        this->logger->log( "## ted: %s", temp_str );
        this->logger->log( "## dnes: %.1f mm, %.0f .. %.0f C", dnes_srazky, dnes_temp_min, dnes_temp_max );
        this->logger->log( "## noc: %.1f mm, %.0f .. %.0f C", noc_srazky, noc_temp_min, noc_temp_max );
        this->logger->log( "## vcera: %.1f mm, %.0f .. %.0f C", vcera_srazky, vcera_temp_min, vcera_temp_max );
        this->logger->log( "## tyden: %.1f mm, %.0f .. %.0f C", tyden_srazky, tyden_temp_min, tyden_temp_max );
    }

    char buffer[50];
    char buffer2[50];


    // display.drawLine( 0, posY+BOX_HEIGHT, 399, posY+BOX_HEIGHT, GxEPD_BLACK );
    extdisplay->display->drawLine( 0, posY, 399, posY, GxEPD_BLACK );

    // ---------------- 
    extdisplay->setFont( fnt_YanoneSB11() );
    extdisplay->display->setTextColor( BARVA_NADPISU );
    extdisplay->setPos( posX, posY + ROW_OFFSET ); 
    extdisplay->printUTF8( "Nyní" ); 

    extdisplay->display->setTextColor( BARVA_TEXTU );
    extdisplay->setPos( posX, posY+TEXT_ROW_SPACING + ROW_OFFSET ); 
    extdisplay->printUTF8( temp_str ); 

    // ---------------- 
    posX += BOX_WIDTH;
    extdisplay->display->drawLine( posX-3, posY, posX-3, posY+BOX_HEIGHT, GxEPD_BLACK );
    sprintf( buffer, "%.0f/%.0f°C", dnes_temp_min, dnes_temp_max );
    if( dnes_srazky!=0.0 ) {
        sprintf( buffer2, "%.1f mm", dnes_srazky );
    } else {
        sprintf( buffer2, " " );
    }
    printOneBox( extdisplay, posX, posY, "Dnes", buffer, buffer2 );

    // ---------------- 
    posX += BOX_WIDTH;
    extdisplay->display->drawLine( posX-3, posY, posX-3, posY+BOX_HEIGHT, GxEPD_BLACK );
    sprintf( buffer, "%.0f/%.0f°C", noc_temp_min, noc_temp_max );
    if( noc_srazky!=0.0 ) {
        sprintf( buffer2, "%.1f mm", noc_srazky );
    } else {
        sprintf( buffer2, " " );
    }
    printOneBox( extdisplay, posX, posY, "Noc", buffer, buffer2 );

    // ---------------- 
    posX += BOX_WIDTH;
    extdisplay->display->drawLine( posX-3, posY, posX-3, posY+BOX_HEIGHT, GxEPD_BLACK );
    sprintf( buffer, "%.0f/%.0f°C", vcera_temp_min, vcera_temp_max );
    if( vcera_srazky!=0.0 ) {
        sprintf( buffer2, "%.1f mm", vcera_srazky );
    } else {
        sprintf( buffer2, " " );
    }
    printOneBox( extdisplay, posX, posY, "Včera", buffer, buffer2 );

    // ---------------- 
    posX += BOX_WIDTH;
    extdisplay->display->drawLine( posX-3, posY, posX-3, posY+BOX_HEIGHT, GxEPD_BLACK );
    sprintf( buffer, "%.0f/%.0f°C", tyden_temp_min, tyden_temp_max );
    if( tyden_srazky!=0.0 ) {
        sprintf( buffer2, "%.1f mm", tyden_srazky );
    } else {
        sprintf( buffer2, " " );
    }
    printOneBox( extdisplay, posX, posY, "Týden", buffer, buffer2 );

    return 0;
}