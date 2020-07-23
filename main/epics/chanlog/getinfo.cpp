#include <cadef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <os.h>
#include <epicslib.h>
#include <string>

const static float TIMEOUT(2.0); // Seconds.


/*!
  Initializes access to EPICS
  \throw fatal exception caught by epics
        on error.
*/
void EpicsInit(int argc, char** argv)
{

  epicslib::startRepeater(argc, argv);
  int status = ca_task_initialize();
  SEVCHK(status, "Initialization failed");
  
}
/*!
    Shutdown access to epics.
    \throw fatal exception caught by epics
       on error.
*/
void EpicsShutdown()
{
  while(ca_test_io() == ECA_IOINPROGRESS) {
    ca_pend_io(1.0);
  }
  int  status     = ca_task_exit();
  SEVCHK(status,"Task exit call failed");
}
/*!
    Get the value of a channel in epics, along with its
    units.  The data are returned in a string of the
    form name:   value units
    \param output (char* [out]):
       String buffer to hold the output
       If there was a failure, this will contain an
       error message.
    \param chan (const char* [in]):
       String containing the channel name.
       

*/
void GetChannel(char* output, const char* chan)
{
  chid channel_id;
  chid units_id;

  char units[100];
  strcpy(units, chan);
  strcat(units,".EGU");
  int status;
  try {
    epicslib::checkChanStatus(
      ca_search(chan, &channel_id), chan, "%s: ca_search failed: %s"
    );
    
    epicslib::checkChanStatus(
      ca_search(units, &units_id), units, "%s: ca_search failed: %s"
    );
    
    epicslib::checkChanStatus(
      ca_pend_io(TIMEOUT), chan, "%s: ca_pend_io failed %s"
    );
    
    char chanvalue[100];
    char unitsvalue[100];
    
    epicslib::checkChanStatus(
      ca_get(DBR_STRING, channel_id, chanvalue),
      chan, "%s: ca_get failed %s"
    );
    
    epicslib::checkChanStatus(
      ca_get(DBR_STRING, units_id, unitsvalue),
      units, "%s: ca_get failed %s"
    );
    
    epicslib::checkChanStatus(
      ca_pend_io(TIMEOUT), chan, "%s: ca_pend_io failed %s"
    );
  } catch(std::string msg) {
    strcpy(output, msg.c_str());   // No way to overwrite check :-()
  }
  
  status    = ca_clear_channel(channel_id);
  SEVCHK(status, "Clear_channel on channel");

  status    = ca_clear_channel(units_id);
  SEVCHK(status, "Clear_Channel on units");

  status    = ca_pend_io(TIMEOUT);
  SEVCHK(status, "ca_pend_io after clear");
}
