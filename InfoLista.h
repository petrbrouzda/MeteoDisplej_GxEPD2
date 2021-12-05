/**
 * Informace o casu vychodu a zapadu slunce pro danou lokalitu (lat/lon) a casovou zonu.
 */ 

#ifndef P_INFOLISTA_H
#define P_INFOLISTA_H

#include <stdio.h>
#include <stdlib.h>

#include "ExtDisplay.h"

#include "PredpovedAlojz.h"
#include "PredpovedYrno.h"
#include "VarovaniChmi.h"

#include "src/ra/ratatoskr.h"
#include "DataAplikace.h"

class InfoLista
{
    public:
        InfoLista( raLogger * logger, DataAplikace * dataAplikace);
        int drawData( ExtDisplay * extdisplay, bool firstRun, PredpovedAlojz * predpovedAlojz, PredpovedYrno * predpovedYrno, VarovaniChmi * varovaniChmi );

    private:
        raLogger * logger;
        DataAplikace * dataAplikace;
};

#endif