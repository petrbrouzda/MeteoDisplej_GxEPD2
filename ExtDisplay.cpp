#include "ExtDisplay.h"

#include <stdlib.h>
#include <string.h>

#include "src/gfxlatin2/gfxlatin2.h"


void ExtDisplay::setBbFullWidth() {
    this->boundingBoxWidth = this->displayWidth - this->posX - 1;
}

void ExtDisplay::setBbRightMargin( int right ) {
    this->boundingBoxWidth = this->displayWidth - this->posX - 1 - right;
    if( this->boundingBoxWidth < 10 ) {
        this->boundingBoxWidth = 10;
    }
}

void ExtDisplay::setPos( int x, int y ) {
    this->posX = x;
    this->posY = y;
    this->setBbFullWidth();
}

void ExtDisplay::init( GxEPD2_GFX* display, raLogger * logger )
{
    this->display = display;
    this->displayWidth = display->width();
    this->displayHeight = display->height();
    this->logger = logger;
}


#define BUFFER_SIZE 512
#define MAX_WORD_SIZE 20

char * curPos;
char oneWord[MAX_WORD_SIZE+2];
char delimiter;

void initParser( char * text ) {
    curPos = text;
    delimiter = 0;
}

boolean getNextWord() {
    int outPos = 0;

    while(true) {
        char c = *curPos;

        if( c==0 ) {
            return outPos>0;
        }

        curPos++;

        if( c==' ' || c==',' || c=='.' ) {
            oneWord[outPos] = c;
            oneWord[++outPos] = 0;
            delimiter = c;
            break;
        } 

        oneWord[outPos] = c;
        oneWord[++outPos] = 0;

        if( outPos==MAX_WORD_SIZE) {
            oneWord[outPos] = '-';
            oneWord[++outPos] = 0;
            delimiter = '-';
            break;
        }
    }

    return true;
}

#define DUMP_DEBUG_INFO 0

/**
 * Vytiskne UTF-8 text na displej vcetne korektniho word-wrapu.
 * Tiskne se na nastavenou pozici X,Y, ktera je LEVY DOLNI roh prvniho pismene.
 * x_offset se pouzije jen pro prvni radek.
 * 
 * Tisk je omezeny nastavenym bounding boxem.
 * 
 * Vraci aktualni pozici X.
 * necha posX = puvodni posX; nastavi posY = posledni radek s textem
 */ 
int ExtDisplay::printUTF8( const char * text, int x_offset  ) {
    char buffer[BUFFER_SIZE];
    strncpy( buffer, text, BUFFER_SIZE );
    buffer[BUFFER_SIZE-1] = 0;
    utf8tocp( buffer );  
    if( DUMP_DEBUG_INFO ) this->logger->log( "[%s]", buffer );
    if( DUMP_DEBUG_INFO ) this->logger->log( "  pos=%d,%d bbW=%d xo=%d", this->posX, this->posY, this->boundingBoxWidth, x_offset );

    int x,y;
    x = this->posX + x_offset;
    y = this->posY;

    initParser( buffer );
    while( true ) {
        if( ! getNextWord() ) break;
        if( DUMP_DEBUG_INFO ) this->logger->log( "  # '%s'", oneWord );

        int16_t x1, y1;
        uint16_t w, h;
        this->display->getTextBounds( (const char*)oneWord, x, y, &x1, &y1, &w, &h );
        if( DUMP_DEBUG_INFO )  this->logger->log( "  max pos %d,%d size %d,%d", x1,y1, w,h );

        if( ( x + w ) <= ( this->posX + this->boundingBoxWidth ) ) {
            this->display->setCursor( x, y );     
            this->display->print( oneWord );
            x += w;
            if( DUMP_DEBUG_INFO )  this->logger->log( "  pokracuju, nova souradnice %d,%d", x,y );
        } else {
            if( DUMP_DEBUG_INFO )  this->logger->log( "  @ %d,%d, w=%d", x,y, w );
            
            x = this->posX;
            y += this->vyskaRadku;

            // musime znovu, protoze ve vyjimecnem pripade to zalomi text samo a vrati to w treba 392 bodu
            this->display->getTextBounds( (const char*)oneWord, x, y, &x1, &y1, &w, &h );
            if( DUMP_DEBUG_INFO )  this->logger->log( "  @ %d,%d, w=%d", x,y, w );

            this->display->setCursor( x, y );     
            this->display->print( oneWord );
            x += w;
            if( DUMP_DEBUG_INFO ) this->logger->log( "  novy radek, nova souradnice %d,%d", x,y );
        }
        if( delimiter==' ' ) {
            x += this->sirkaMezery;
        }
    }

    this->posY = y;
    return x - this->posX;
}


void ExtDisplay::setFont( const GFXfont * font ) {
    this->display->setFont( font );
    this->vyskaRadku = font->yAdvance;
    // predpoklada, ze mezera je prvni znak
    this->sirkaMezery = font->glyph[0].xAdvance;
    //D/ this->logger->log( "setFont: mezera %d, radek %d", this->sirkaMezery, this->vyskaRadku );
}

