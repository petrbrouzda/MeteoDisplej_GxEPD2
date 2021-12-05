/**
 * Varování ČHMÚ
 */ 

#ifndef P_INFO_METEO_H
#define P_INFO_METEO_H

#include <stdio.h>
#include <stdlib.h>

#include <ArduinoJson.h>
#include "ExtDisplay.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class InfoMeteostanice
{
    public:
        InfoMeteostanice( raLogger * logger, raConfig * config , DataAplikace * dataAplikace);
        void loadData();
        bool hasData();
        int drawData( ExtDisplay * extdisplay, bool firstRun );

    private:
        raLogger * logger;
        raConfig * config;
        DataAplikace * dataAplikace;
        
        DynamicJsonDocument * jsonData = NULL;
        time_t casStazeni = 0;
};

#endif