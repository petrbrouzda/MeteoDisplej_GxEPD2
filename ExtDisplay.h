#ifndef _DISP_COORDINATES___H_
#define _DISP_COORDINATES___H_

#include <Arduino.h>
#include <GxEPD2_GFX.h>
#include "src/ra/raLogger.h"

class ExtDisplay {
    public:
        // ---------------------- obecna cast ---------------------------

        /** sirka displeje, nemeni se */
        int displayWidth;
        /** vyska displeje, nemeni se */
        int displayHeight;

        /** aktualni pozice pro vykreslovani - X */
        int posX;
        /** aktualni pozice pro vykreslovani - Y */
        int posY;
        /** nastavi pozici a pro danou pozici nastavi bounding box az do okraje displeje */
        void setPos( int x, int y );

        /** sirka obdelniku, do ktereho se smi vypsat text */
        int boundingBoxWidth;
        /** nastavi maximalni sirku vykreslovaciho obdelniku pro aktualni pozici X */
        void setBbFullWidth();
        /** nastavi sirku vykreslovaciho obdelniku pro aktualni pozici X a urceny pravy okraj */
        void setBbRightMargin( int right );

        // ---------------------- GxEPD2 specific ---------------------------

        /** display */
        GxEPD2_GFX* display;

        void init( GxEPD2_GFX* display, raLogger * logger );

        int printUTF8( const char * text, int x_offset = 0 );
        void setFont( const GFXfont * font );

        /** sirka mezery, nastavi se automaticky z fontu, ale je mozne zmenit */
        int sirkaMezery;

        /** vyska radku, nastavi se automaticky z fontu, ale je mozne zmenit */
        int vyskaRadku;

    private:
        raLogger * logger;


};

#endif