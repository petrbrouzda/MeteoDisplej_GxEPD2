/**
 * Lidsky citelna predpoved z https://alojz.cz/praha
 */ 

#include <Arduino.h>

#include "PredpovedAlojz.h"
#include "DataAplikace.h"

#include "src/fonts/fonts.h"

// maximalni stari dat v sekundach
#define DATA_MAX_AGE 14400

PredpovedAlojz::PredpovedAlojz( raLogger * logger, raConfig * config,  DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->config = config;
    this->dataAplikace = dataAplikace;
}


void PredpovedAlojz::loadData()
{
    char url[255];
    sprintf( url, "%s%s",
        this->config->getString( "url_alojz", "https://alojz.cz/api/v1/solution?url_id=/" ),
        this->dataAplikace->misto_alojz );

    logger->log( "Stahuji varovani: %s", url );

    HTTPClient http;

    // Send request
    http.useHTTP10(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.begin(url);
    int httpResponseCode = http.GET();

    if( httpResponseCode==200 ) {
        // Parse response
        DynamicJsonDocument * doc = new DynamicJsonDocument(4000);
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


boolean PredpovedAlojz::hasData()
{
    if( this->jsonData!=NULL 
        && ( time(NULL) - this->casStazeni ) < DATA_MAX_AGE ) {
          return true;
    } else {
      return false;
    }
}


#define BUFFER_SIZE 100

void PredpovedAlojz::drawData( ExtDisplay * extdisplay, bool firstRun )
{
    char buffer[BUFFER_SIZE];
    const char * nadpis;
    const char * text;
    const char * created;
    const char * prefer;

    prefer = (*this->jsonData)["prefer"];

    extdisplay->setFont( fnt_YanoneSB13() );

    if( firstRun ) {
        this->logger->log( "Alojz:" );
    }

    // vetsinu dne ukazujeme jen jednodenni predpoved; ale 16-20 ukazujeme na dnes i zitra
    time_t now = time(NULL);
    struct tm * cas = localtime( &now );
    if( cas->tm_hour >= 16 && cas->tm_hour < 20 ) {

        int x_offset = 0;

        nadpis = (*this->jsonData)["day1"]["today_tomorrow"];
        text = (*this->jsonData)["day1"]["string"];
        created = (*this->jsonData)["day1"]["created"];
        if( nadpis!=NULL && text!=NULL ) {
          if( firstRun ) {
              this->logger->log( "created: %s", created );
              this->logger->log( "> '%s': '%s'", nadpis, text );
          }

          extdisplay->display->setTextColor( BARVA_NADPISU );
          snprintf( buffer, BUFFER_SIZE, "%s:", nadpis );
          buffer[BUFFER_SIZE-1]=0;
          buffer[0] = toupper(buffer[0]);
          x_offset = extdisplay->printUTF8( buffer );

          extdisplay->display->setTextColor( BARVA_TEXTU );
          x_offset = extdisplay->printUTF8( text, x_offset );
        }

        nadpis = (*this->jsonData)["day2"]["today_tomorrow"];
        text = (*this->jsonData)["day2"]["string"];
        created = (*this->jsonData)["day2"]["created"];
        if( nadpis!=NULL && text!=NULL ) {
          if( firstRun ) {
              this->logger->log( "created: %s", created );
              this->logger->log( "> '%s': '%s'", nadpis, text );
          }

          extdisplay->display->setTextColor( BARVA_NADPISU );
          snprintf( buffer, BUFFER_SIZE, "%s:", nadpis );
          buffer[BUFFER_SIZE-1]=0;
          buffer[0] = toupper(buffer[0]);
          x_offset = extdisplay->printUTF8( buffer, x_offset );

          extdisplay->display->setTextColor( BARVA_TEXTU );
          x_offset = extdisplay->printUTF8( text, x_offset );
        }

    } else {

        nadpis = (*this->jsonData)[prefer]["today_tomorrow"];
        text = (*this->jsonData)[prefer]["string"];
        created = (*this->jsonData)[prefer]["created"];

        if( nadpis!=NULL && text!=NULL ) {
          if( firstRun ) {
              this->logger->log( "prefer: %s", prefer );
              this->logger->log( "created: %s", created );
              this->logger->log( "> '%s': '%s'", nadpis, text );
          }

          extdisplay->display->setTextColor( BARVA_NADPISU );
          snprintf( buffer, BUFFER_SIZE, "%s:", nadpis );
          buffer[BUFFER_SIZE-1]=0;
          buffer[0] = toupper(buffer[0]);
          int x_offset = extdisplay->printUTF8( buffer );

          extdisplay->display->setTextColor( BARVA_TEXTU );
          extdisplay->printUTF8( text, x_offset );
        }
    }

}