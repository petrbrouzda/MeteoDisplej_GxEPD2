#ifndef RA_CONNECTION_H
#define RA_CONNECTION_H

#if defined(ESP8266)

    #include <ESP8266HTTPClient.h>
    #include <ESP8266WiFi.h>

#elif defined(ESP32)

    #include <WiFi.h>
    #include <HTTPClient.h>

#endif

#include "raConfig.h"
#include "raLogger.h"
#include "TextCircularBuffer.h"

#include "../aes-sha/sha256.h"

#include "../../AppFeatures.h"

#define RACONN_MAX_DATA 448
#define RACONN_MSG_LEN 1024

/**
 * Jak dlouho se ceka na odpoved po poslani hlavicky
 */ 
#define RA_BLOB_TIMEOUT 15000
#define RA_UPDATE_TIMEOUT 15000

#define RA_APP_NAME_SIZE 256

class raConnection 
{
  public:
    raConnection( raConfig * cfg, raLogger* logger );
    int send( unsigned char * dataKOdeslani, int dataLen );
    int sendBlob( unsigned char * blob, int blobSize, int startTime, char * desc, char * extension );
    bool isConnected();
    void setAppName( const char * appName );
    void setConfigVersion( int version );
    void setRssi( int rssi );
    
    #ifdef LOG_SHIPPING
      bool sendLogData( char * data );  
    #endif
    long updateReqId;
    bool shouldRestart;

    #ifdef OTA_UPDATE
      bool doUpdate( long updateId );
    #endif
    
  private:
    void login();
    void createLoginToken( BYTE *target, BYTE * ecdh_public );
    void createDataPayload( BYTE *target, BYTE *source, int sourceLen );
    // long readServerTime();
    int sendInt( unsigned char * dataKOdeslani, int dataLen );
    int sendBlobInt( unsigned char * blob, int blobSize, int startTime, char * desc, char * extension );
    void log_keys( unsigned char * key, unsigned char * iv );
    bool decryptBlock( char * encryptedData, char * plainText, int plainTextBuffSize );
    void parseConfigData( char * encryptedData, char * plainText, int plainTextBuffSize, char * oneLine, int oneLineSize );
    void parseUpdateRequest( char * encryptedData, char * plainText, int plainTextBuffSize );
    void decryptPublicKeyBlock( char * encryptedData, char * plainText, int plainTextBuffSize );
    int doRequest( char * url, unsigned char * postData, int postDataLen, char * auth1 = NULL, char * auth2 = NULL );
    
    raLogger* logger;
    raConfig * cfg;
    bool connected;
    char session[32];
    BYTE sessionKey[32];
    Sha256* sha256;
    BYTE msg[RACONN_MSG_LEN+10];
    char httpBuffer[RACONN_MSG_LEN+10];
    char appName[RA_APP_NAME_SIZE];
    int localConfigVersion = 0;
    int wifiSignalStrength = 0;

    char * identity = (char*)"CONN";    
};

#endif

