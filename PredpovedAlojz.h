/**
 * Lidsky citelna predpoved z https://alojz.cz/praha
 */ 

#ifndef P_ALOJZ_H
#define P_ALOJZ_H

#include <stdio.h>
#include <stdlib.h>

#include <ArduinoJson.h>
#include "ExtDisplay.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class PredpovedAlojz
{
    public:
        PredpovedAlojz( raLogger * logger, raConfig * config , DataAplikace * dataAplikace);
        void loadData();
        bool hasData();
        void drawData( ExtDisplay * extdisplay, bool firstRun );

    private:
        raLogger * logger;
        raConfig * config;
        DataAplikace * dataAplikace;
        
        DynamicJsonDocument * jsonData = NULL;
        time_t casStazeni = 0;
};

#endif