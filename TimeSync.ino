/**
 * Zajistuje synchronizaci realneho casu
 */ 

#define NTP_SERVER "europe.pool.ntp.org"
#define TIMEZONE_DEF "CET-1CEST,M3.5.0/2,M10.5.0/3"

bool timeSyncConfigured = false;

void printTime( char * target, const struct tm *timeptr )
{
  static const char wday_name[][4] = {
    "Ne", "Po", "Ut", "St", "Ct", "Pa", "So"
  };
  static const char mon_name[][4] = {
    "Led", "Uno", "Bre", "Dub", "Kve", "Cer",
    "Cec", "Srp", "Zar", "Rij", "Lis", "Pro"
  };
  sprintf( target, "%s %2d. %s %d %.2d:%.2d:%.2d",
    wday_name[timeptr->tm_wday],
    timeptr->tm_mday, 
    mon_name[timeptr->tm_mon],
    timeptr->tm_year - 100,
    timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec
    );
}

void timeSync( DataAplikace * dataAplikace )
{
  int prevTime;

  if( ! timeSyncConfigured ) {
    // musi byt az zde - pokud se spusti v dobe, kdy neni wifi, svet se zhrouti
    configTime(0, 0, NTP_SERVER );
    // nastaveni casove zony
    setenv("TZ", TIMEZONE_DEF, 3);
    timeSyncConfigured = true;
  }

  struct tm timeinfo;

  for( int i = 0; i<5; i++ ) {
    prevTime = time(NULL);
    if(!getLocalTime(&timeinfo)){
      logger->log("timeSync - error");
    } else {
      dataAplikace->timeSynced = true;
      break;
    }
  }

  if( !dataAplikace->timeSynced ) {
    return;
  }
  
  char buffer[40];
  printTime( buffer, &timeinfo );

  int curTime = time(NULL);

  logger->log( "time=%d (+%d) [%s]", time(NULL), (curTime-prevTime), buffer );

  time_t now = time(NULL);
  struct tm * cas;
  cas = gmtime( &now );
  int gm_daytime = cas->tm_hour*3600 + cas->tm_min*60 + cas->tm_sec;
  cas = localtime( &now );
  int loc_daytime = cas->tm_hour*3600 + cas->tm_min*60 + cas->tm_sec;
  dataAplikace->timeZoneDiffSec = loc_daytime-gm_daytime;
  if(dataAplikace->timeZoneDiffSec < 0) {
    // tesne po pulnoci
    dataAplikace->timeZoneDiffSec += 86400;
  }
  logger->log( "timezone: %d sec", dataAplikace->timeZoneDiffSec );
}
