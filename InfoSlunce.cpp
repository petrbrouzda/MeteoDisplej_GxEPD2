/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#include "InfoSlunce.h"

#include "src/sunset/sunset.h"
#include "src/fonts/fonts.h"

InfoSlunce::InfoSlunce( raLogger * logger, DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->dataAplikace = dataAplikace;
}

void InfoSlunce::compute() {
    int sunrise;
    int sunset;

    SunSet sun;
    sun.setPosition( this->dataAplikace->pozice_lat, this->dataAplikace->pozice_lon, 0);
    sun.setTZOffset( (double)(this->dataAplikace->timeZoneDiffSec/3600) );

    time_t now = time(NULL);
    struct tm * cas;
    cas = localtime( &now );
    sun.setCurrentDate( 1900+cas->tm_year, 1+cas->tm_mon, cas->tm_mday );
    int currentTime = 60*cas->tm_hour + cas->tm_min;

    sunrise = static_cast<int>(sun.calcSunrise());
    sunset = static_cast<int>(sun.calcSunset());
    sprintf( this->sunRise, "%02d:%02d", (sunrise/60), (sunrise%60) );
    sprintf( this->sunSet, "%02d:%02d", (sunset/60), (sunset%60) );

    if( currentTime < sunrise ) {
        this->sunState = 1;
        this->logger->log( "slunce: %s - %s (pred vychodem)", this->sunRise, this->sunSet );
    } else if( currentTime < sunset ) {
        this->sunState = 2;
        this->logger->log( "slunce: %s - %s (den)", this->sunRise, this->sunSet );
    } else {
        this->sunState = 3;
        now = now + 86400;
        cas = localtime( &now );
        sun.setCurrentDate( 1900+cas->tm_year, 1+cas->tm_mon, cas->tm_mday );
        sunrise = static_cast<int>(sun.calcSunrise());
        sunset = static_cast<int>(sun.calcSunset());
        sprintf( this->sunRise, "%02d:%02d", (sunrise/60), (sunrise%60) );
        sprintf( this->sunSet, "%02d:%02d", (sunset/60), (sunset%60) );
        this->logger->log( "slunce: %s - %s (noc)", this->sunRise, this->sunSet );
    }
}

#define Y_OFFSET_IKONY 10

int InfoSlunce::drawData( ExtDisplay * extdisplay, bool firstRun ) {
    if( firstRun ) {
        logger->log( "> slunce %s %s", this->sunRise, this->sunSet );
    }

    extdisplay->posY += Y_OFFSET_IKONY;
    extdisplay->setFont( fnt_Meteocons30() );
    extdisplay->display->setTextColor( BARVA_NADPISU );
    int x_offset = extdisplay->printUTF8( "1" );

    extdisplay->posY -= Y_OFFSET_IKONY;
    extdisplay->setFont( fnt_YanoneSB13() );
    extdisplay->display->setTextColor( BARVA_TEXTU );
    char buffer[50];
    sprintf( buffer, "%s - %s", this->sunRise, this->sunSet );
    x_offset = extdisplay->printUTF8( buffer, x_offset );
    return x_offset;
}
