#include "DataAplikace.h"

DataAplikace::DataAplikace( raLogger * logger, raConfig * config  ) {
    this->pozice_lat = atof( config->getString( "lat", "49.8921" ) );
    this->pozice_lon = atof(  config->getString( "lon", "14.5716" ) );
    this->pozice_altitude = (int) config->getLong( "alt", 430 );
    this->misto_idorp = (int) config->getLong( "idorp", 2122	 );
    strcpy( this->misto_jmeno , config->getString( "name", "Teptin" ) );
    strcpy( this->misto_alojz , config->getString( "alojz", "jablonec-nad-nisou" ) );
}
