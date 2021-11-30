/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#ifndef P_SLUNCE_H
#define P_SLUNCE_H

#include <stdio.h>
#include <stdlib.h>

#include "ExtDisplay.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class InfoSlunce
{
    public:
        InfoSlunce( raLogger * logger, DataAplikace * dataAplikace);
        void compute();
        int drawData( ExtDisplay * extdisplay, bool firstRun );

        /** hh:mm */
        char sunRise[10];
        /** hh:mm */
        char sunSet[10];

        /**
         * 1 pred vychodem slunce
         * 2 pres den
         * 3 po zapadu slunce = data jsou pro zitrek
         */ 
        int sunState;

    private:
        raLogger * logger;
        DataAplikace * dataAplikace;
};

#endif