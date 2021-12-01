/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#ifndef P_MESIC_H
#define P_MESIC_H

#include <stdio.h>
#include <stdlib.h>

#include "ExtDisplay.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class InfoMesic
{
    public:
        InfoMesic( raLogger * logger, DataAplikace * dataAplikace);
        void compute();
        int drawData( ExtDisplay * extdisplay, bool firstRun );

        char vychod[20];
        char zapad[20];
        char zapad2[20];

        /** faze je 0-360, 180 je uplnek */
        int faze;
        bool isVisible;

    private:
        raLogger * logger;
        DataAplikace * dataAplikace;
};

#endif