/**
 * Varování ČHMÚ
 */ 

#include <Arduino.h>

#include "VarovaniChmi.h"
#include "DataAplikace.h"

#include "src/fonts/fonts.h"

// maximalni stari dat v sekundach
#define DATA_MAX_AGE 14400

// maximalni pocet varovani
#define MAX_VAROVANI 3

VarovaniChmi::VarovaniChmi( raLogger * logger, raConfig * config,  DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->config = config;
    this->dataAplikace = dataAplikace;
}


void VarovaniChmi::loadData()
{
    char url[255];
    sprintf( url, "%s/chmi/vystrahy/%d?kratke=1&odhackuj=0",
        this->config->getString( "url_varovani", "http://lovecka.info/ChmiWarnings1" ),
        this->dataAplikace->misto_idorp );        

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

bool VarovaniChmi::jsouNejakaVarovani() {
    if( this->jsonData==NULL ) return false;

    for( int i = 0; i<10; i++ ) {
        const char* name = (* this->jsonData)["events"][i]["text"];
        if( name==NULL ) break;     // konec dat

        const char* type = (* this->jsonData)["events"][i]["type"];
        if( strcasecmp( type, "forest-fire" ) == 0 ) {
            // varovani pred pozary mne nezajima
            continue;
        }

        // alespon jedno varovani je
        return true;
    }
    return false;
}

boolean VarovaniChmi::hasData()
{
    if( this->jsonData!=NULL 
        && ( time(NULL) - this->casStazeni ) < DATA_MAX_AGE ) {
          return true;
    } else {
      return false;
    }
}


#define VAROVANI_NEMAM_IKONU "a"

char * getIcon( const char * icon )
{
    if( icon==NULL ) {
        return (char*)VAROVANI_NEMAM_IKONU;
    }

    if( strcasecmp( icon, "Wind" )==0) {
        return (char*)"F";
    } else if( strcasecmp( icon, "snow-ice" )==0) {
        return (char*)"W";
    } else if( strcasecmp( icon, "Thunderstorm" )==0) {
        return (char*)"6";
    } else if( strcasecmp( icon, "Fog" )==0) {
        return (char*)"M";
    } else if( strcasecmp( icon, "high-temperature" )==0) {
        return (char*)"'";
    } else if( strcasecmp( icon, "low-temperature" )==0) {
        return (char*)"'";
    } else if( strcasecmp( icon, "coastalevent" )==0) {
        return (char*)VAROVANI_NEMAM_IKONU;
    } else if( strcasecmp( icon, "forest-fire" )==0) {
        return (char*)VAROVANI_NEMAM_IKONU;
    } else if( strcasecmp( icon, "avalanches" )==0) {
        return (char*)VAROVANI_NEMAM_IKONU;
    } else if( strcasecmp( icon, "Rain" )==0) {
        return (char*)"8";
    } else if( strcasecmp( icon, "unknown" )==0) {
        return (char*)VAROVANI_NEMAM_IKONU;
    } else if( strcasecmp( icon, "flooding" )==0) {
        return (char*)VAROVANI_NEMAM_IKONU;
    } else if( strcasecmp( icon, "rain-flood" )==0) {
        return (char*)"8";
    } else {
        return (char*)VAROVANI_NEMAM_IKONU;
    }
}


#define BUFFER_SIZE 100

#define OFFSET_IKONY_Y 5

int VarovaniChmi::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    if( this->jsonData==NULL ) return false;

    int radku = 0;
    bool ledVarovaniChmi = false;
    int x_offset = 0;

    for( int i = 0; i<10; i++ ) {
        const char* name = (* this->jsonData)["events"][i]["text"];
        if( name==NULL ) break;     // konec dat

        const char* type = (* this->jsonData)["events"][i]["type"];
        if( strcasecmp( type, "forest-fire" ) == 0 ) {
            // varovani pred pozary mne nezajima
            continue;
        }

        // alespon jedno varovani je
        ledVarovaniChmi = true;
        
        char iconColor[2] = "y";
        const char* color = (* this->jsonData)["events"][i]["color"];
        if( color!=NULL ) {
            iconColor[0] = color[0];
        }

        if( iconColor[0]=='o' || iconColor[0]=='r' ) {   // stupnice je Yellow Orange Red 
            extdisplay->display->setTextColor( BARVA_IKONA_VAROVANI_H );
        } else {
            extdisplay->display->setTextColor( BARVA_IKONA_VAROVANI_L );
        }
        extdisplay->setFont( fnt_Meteocons15() );
        extdisplay->posY += OFFSET_IKONY_Y;
        x_offset = extdisplay->printUTF8( getIcon(type) ); 
        extdisplay->posY -= OFFSET_IKONY_Y;

        logger->log( "> [%s %s] %s ", iconColor, type, name );

        int prevX = extdisplay->posX;
        extdisplay->setFont( fnt_YanoneSB11() );
        extdisplay->posX += x_offset + 2*extdisplay->sirkaMezery;
        extdisplay->setBbFullWidth();

        extdisplay->display->setTextColor( BARVA_TEXTU );
        x_offset = extdisplay->printUTF8( name  );

        // extdisplay->setFont( fnt_YanoneSB11() );
        // extdisplay->posY += extdisplay->vyskaRadku;

        char casInfo[50];
        strcpy( casInfo, " " );

        const char* stav = (* this->jsonData)["events"][i]["in_progress"];
        if( stav!=NULL && stav[0]=='Y' ) {
            // probiha, nepiseme startovni cas
        } else {
            const char* cas_od = (* this->jsonData)["events"][i]["time_start_t"];
            if( cas_od!=NULL ) {
                strcat( casInfo, "od ");
                strcat( casInfo, cas_od );
                strcat( casInfo, "  ");
            }
        }
        const char* cas_do = (* this->jsonData)["events"][i]["time_end_t"];
        if( cas_do!=NULL ) {
            strcat( casInfo, "do ");
            strcat( casInfo, cas_do );
        }
        if( casInfo[0]==0 ) {
            strcpy( casInfo, " " );
        }

        extdisplay->display->setTextColor( BARVA_TEXTU );
        extdisplay->printUTF8( casInfo, x_offset );

        radku++;
        extdisplay->posY += extdisplay->vyskaRadku;
        extdisplay->posX = prevX;

        if( radku==MAX_VAROVANI ) {
            // konec, uz je jich dost
            break;
        }
    }

    return radku;
}