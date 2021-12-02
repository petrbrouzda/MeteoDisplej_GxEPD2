/**
 * Varování ČHMÚ
 */ 

#ifndef P_CHMI_WARNINGS_H
#define P_CHMI_WARNINGS_H

#include <stdio.h>
#include <stdlib.h>

#include <ArduinoJson.h>
#include "ExtDisplay.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class VarovaniChmi
{
    public:
        VarovaniChmi( raLogger * logger, raConfig * config , DataAplikace * dataAplikace);
        void loadData();
        bool hasData();
        int drawData( ExtDisplay * extdisplay, bool firstRun );
        bool jsouNejakaVarovani();

    private:
        raLogger * logger;
        raConfig * config;
        DataAplikace * dataAplikace;
        
        DynamicJsonDocument * jsonData = NULL;
        time_t casStazeni = 0;
};

#endif