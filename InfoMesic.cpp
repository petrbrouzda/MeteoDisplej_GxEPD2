/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#include "InfoMesic.h"

// https://github.com/signetica/MoonRise - knihovna MoonRise 2.0.1 v library manageru
#include <MoonRise.h>

// https://github.com/CelliesProjects/moonPhase-esp32 - MoonPhase od Cellies 1.0.3
#include <moonPhase.h>

#include "src/fonts/fonts.h"

InfoMesic::InfoMesic( raLogger * logger, DataAplikace * dataAplikace )
{
    this->logger = logger;
    this->dataAplikace = dataAplikace;
}

//TODO: nebude fungovat 31.12. na data, co jsou zitra/pozitri
void formatTime( char * target, int dnes, int hour, struct tm * cas )
{
    if( cas->tm_yday == dnes ) {
        sprintf( target, "%02d:%02d", cas->tm_hour, cas->tm_min );
    } else if( cas->tm_yday == dnes + 1 ) {
        // if( cas->tm_hour < hour ) {
        //    sprintf( target, "%02d:%02d", cas->tm_hour, cas->tm_min );
        //} else {
            sprintf( target, "z%02d:%02d", cas->tm_hour, cas->tm_min );
        // }
    } else if( cas->tm_yday == dnes + 2 ) {
        sprintf( target, "p%02d:%02d", cas->tm_hour, cas->tm_min );
    } else {
        sprintf( target, "%d.%d.", cas->tm_mday, cas->tm_mon+1 );
    }
}

MoonRise mr;

void InfoMesic::compute() {
    time_t nyni = time(NULL);
    struct tm * cas = localtime( &nyni );
    int dnes = cas->tm_yday;
    int hour = cas->tm_hour;

    mr.calculate(this->dataAplikace->pozice_lat, this->dataAplikace->pozice_lon, time(NULL) );
    MoonRise mr2;
    mr2.calculate(this->dataAplikace->pozice_lat, this->dataAplikace->pozice_lon, time(NULL)+86400 );

    this->vychod[0] =0;
    this->zapad[0] = 0;
    this->zapad2[0] = 0;
    this->isVisible = mr.isVisible;

    if( this->isVisible ) {    
        // If moon is visible at *time*.
        this->logger->log( "mesic: viditelny" );
        
    }	
    if( mr.hasRise ) {
        // There was a moonrise event found in the search
			// interval (default 48 hours, 24 hours before and
			// after *time*).
        cas = localtime( &(mr.riseTime) );
        this->logger->log( "mesic: vychod za %d , %d.%d. %02d:%02d", mr.riseTime - mr.queryTime, cas->tm_mday, cas->tm_mon+1, cas->tm_hour, cas->tm_min );

        if( (mr.riseTime - mr.queryTime) < 0 ) {
            if( mr2.hasRise ) {
                cas = localtime( &(mr2.riseTime) );
                this->logger->log( "mesic: dalsi vychod za %d , %d.%d. %02d:%02d", mr2.riseTime - mr.queryTime, cas->tm_mday, cas->tm_mon+1, cas->tm_hour, cas->tm_min );
                formatTime( this->vychod, dnes, hour, cas );
            }
        } else {
            formatTime( this->vychod, dnes, hour, cas );
        }
    }
    if( mr.hasSet ) {
        // There was a moonset event found in the search interval.
        cas = localtime( &(mr.setTime) );
        this->logger->log( "mesic: zapad za %d, %d.%d. %02d:%02d", mr.setTime - mr.queryTime, cas->tm_mday, cas->tm_mon+1, cas->tm_hour, cas->tm_min );
 
        if( (mr.setTime - mr.queryTime) < 0 ) {
            if( mr2.hasSet ) {
                cas = localtime( &(mr2.setTime) );
                this->logger->log( "mesic: dalsi zapad za %d, %d.%d. %02d:%02d", mr2.setTime - mr.queryTime, cas->tm_mday, cas->tm_mon+1, cas->tm_hour, cas->tm_min );
                formatTime( this->zapad, dnes, hour, cas );
            }
        } else {
            formatTime( zapad, dnes, hour, cas );
            if( mr2.hasSet ) {
                cas = localtime( &(mr2.setTime) );
                this->logger->log( "mesic: dalsi zapad za %d, %d.%d. %02d:%02d", mr2.setTime - mr.queryTime, cas->tm_mday, cas->tm_mon+1, cas->tm_hour, cas->tm_min );
                formatTime( this->zapad2, dnes, hour, cas );
            }
        }
    }

    this->logger->log( "mesic: %s, %s - %s", (mr.isVisible?"sviti":"neni"), this->vychod, this->zapad );

    moonData_t moon; 
    moonPhase moonPhase;
    moon = moonPhase.getPhase(); 

    // faze je 0-360, 180 je uplnek
    this->faze = moon.angle;
    this->logger->log( "mesic: faze %d, osvit %d %%", this->faze, (int)(moon.percentLit * 100) );

}

/**
 * Mapovani faze mesice na obrazky v pouzitem fontu MoonPhases
 * https://www.dafont.com/moon-phases.font
 * fonts/moon_phases.png
 * 
 * 0 - nov
 * G - 1/4
 * 1 - uplnek
 * T - 3/4
 *            1         2
 *  01234567890123456789012345678
 * "0ABCDEFGHIJKLM1NOPQRSTUVWXYZ0";
 */ 
char mesicMapa[] = "0ABCDEFGHIJKLM1NOPQRSTUVWXYZ0";

char znakMesice( int faze ) {
    if( faze < 7 ) {
        return '0';
    } else if( faze > 353 ) {
        return '0';
    }
    int faze2 = faze - 7;
    // 347 stupnu / 26 dilku
    int suplik = (int) (((float)faze2) / (13.346));
    return mesicMapa[suplik+1];
}


#define Y_OFFSET_IKONY 8

int InfoMesic::drawData( ExtDisplay * extdisplay, bool firstRun ) {
    int c = znakMesice( this->faze );
    if( firstRun ) {
        logger->log( "> mesic faze %d, '%c'", this->faze, c );
    }

    char buf[50];

    extdisplay->posY += Y_OFFSET_IKONY;
    extdisplay->setFont( fnt_MoonPhases15() );
    extdisplay->display->setTextColor( BARVA_TEXTU );
    buf[0] = c; buf[1] = 0;
    int x_offset = extdisplay->printUTF8( buf );

    // na me hloupe cenovce je tenka svisla cara skoro neviditelna, takze mesic vytisknu jeste jednou o bod posunuty
    extdisplay->posX--;
    extdisplay->printUTF8( buf );
    extdisplay->posX++;

    extdisplay->posY -= Y_OFFSET_IKONY;
    extdisplay->setFont( fnt_YanoneSB13() );
    extdisplay->display->setTextColor( BARVA_TEXTU );

    strcpy( buf, "  " );

    if( this->isVisible ) {
        // if(firstRun) logger->log( "mesic: zapad [%s], [%s - %s]", this->zapad, this->vychod, this->zapad2 );
        if( this->zapad[0]!=0 ) {
            sprintf( buf+strlen(buf), "}%s  ", this->zapad);
        }
        if( this->vychod[0]!=0 ) {
            sprintf( buf+strlen(buf), "{%s  ", this->vychod);
        }
        /* if( this->zapad2[0]!=0 ) {
            sprintf( buf+strlen(buf), "}%s ", this->zapad2 );
        } */
    } else {
        // if(firstRun) logger->log( "mesic: [%s - %s]", this->vychod, this->zapad );
        if( this->vychod[0]!=0 ) {
            sprintf( buf+strlen(buf), "{%s  ", this->vychod);
        }
        if( this->zapad[0]!=0 ) {
            sprintf( buf+strlen(buf), "}%s", this->zapad);
        }
    }
    if(firstRun) logger->log( "> mesic: %s", buf );
    
    x_offset = extdisplay->printUTF8( buf, x_offset );
    return x_offset;
}
