#ifndef DATAAPLIKACE_H
#define DATAAPLIKACE_H

#include <stdio.h>
#include <stdlib.h>
#include "src/ra/ratatoskr.h"

#define BARVA_NADPISU GxEPD_RED
#define BARVA_TEXTU GxEPD_BLACK
#define BARVA_VAROVANI GxEPD_BLACK

#define BARVA_IKONA_VAROVANI_H GxEPD_RED
#define BARVA_IKONA_VAROVANI_L GxEPD_BLACK 


class DataAplikace
{
  public:
    DataAplikace( raLogger * logger, raConfig * config );
    
    double pozice_lat;
    double pozice_lon;
    int pozice_altitude;

    /** ID obce s rozsirenou pusobnosti pro varovani CHMU - https://github.com/petrbrouzda/ChmiWarnings */
    int misto_idorp;

    /** jmeno obce - jen pro zobraceni */
    char misto_jmeno[50];
    
    /** jmeno mista pro Alojze - https://pebrou.wordpress.com/2021/09/14/stavite-si-meteostanici-tipy-na-software/ */
    char misto_alojz[50];

    /** timezona v sekundach */
    int timeZoneDiffSec = 0;

    /** Mame synchronni cas? */
    bool timeSynced = false;
};

#endif   