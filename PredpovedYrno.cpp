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


#define VAROVANI_NEMAM_IKONU "a"

char * getIconYrno( const char * icon )
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

int PredpovedYrno::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    if( this->jsonData==NULL ) return false;

    time_t now = time(NULL);
    struct tm * cas = localtime( &now );
    bool wasCurHour = false;

    char buffer[100];

    extdisplay->posY += 5;

    for( int i = 0; i<10; i++ ) {
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
    }
    
    return 0;
}