/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#include "InfoLista.h"

#include "src/fonts/fonts.h"

InfoLista::InfoLista( raLogger * logger, DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->dataAplikace = dataAplikace;
}

int InfoLista::drawData( ExtDisplay * extdisplay, bool firstRun, PredpovedAlojz * predpovedAlojz, PredpovedYrno * predpovedYrno, VarovaniChmi * varovaniChmi ) {

    char buffer[100];

    time_t now = time(NULL);
    struct tm * cas = localtime( &now );
    sprintf( buffer, "%s - %02d:%02d", this->dataAplikace->misto_jmeno, cas->tm_hour, cas->tm_min );

    char chybiData[10];
    chybiData[0] = 0;
    if( !predpovedAlojz->hasData() ) {
        strcat( chybiData, "A");
    }
    if( !varovaniChmi->hasData() ) {
        strcat( chybiData, "C");
    }
    if( !predpovedYrno->hasData() ) {
        strcat( chybiData, "Y");
    }

    if( chybiData[0]!=0 ) {
        sprintf( buffer+strlen(buffer), " - Chybí: %s", chybiData );
    }

    if( varovaniChmi->hasData() && !varovaniChmi->jsouNejakaVarovani() ) {
        sprintf( buffer+strlen(buffer), " - Žádná varování" );
    }

    if( firstRun ) {
        this->logger->log( "> %s", buffer );
    }

    extdisplay->posY -= 4;

    extdisplay->setFont( fnt_YanoneSB10() );
    extdisplay->display->setTextColor( BARVA_TEXTU );
    extdisplay->printUTF8( buffer );

    return 0;
}
