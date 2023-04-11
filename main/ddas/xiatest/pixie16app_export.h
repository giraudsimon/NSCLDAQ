

#define PIXIE16APP_EXPORT
#define PIXIE16APP_API 

#include <HardwareRegistry.h>
#include <vector>
#include <string>

extern "C" {

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16InitSystem (
	unsigned short NumModules,    // total number of Pixie16 modules in the system
	unsigned short *PXISlotMap,   // an array containing the PXI slot number for each pixie16 module
	unsigned short OfflineMode ); // specify if the system is in offline mode

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ExitSystem (
	unsigned short ModNum );      // module number

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ReadModuleInfo (
	unsigned short ModNum,        // module number
	unsigned short *ModRev,       // returned module revision
	unsigned int   *ModSerNum,    // returned module serial number
	unsigned short *ModADCBits,   // returned module ADC bits
	unsigned short *ModADCMSPS ); // returned module ADC sampling rate

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16BootModule (
	char *ComFPGAConfigFile,      // name of communications FPGA configuration file
	char *SPFPGAConfigFile,       // name of signal processing FPGA configuration file
	char *TrigFPGAConfigFile,     // name of trigger FPGA configuration file
	char *DSPCodeFile,            // name of executable code file for digital signal processor (DSP)
	char *DSPParFile,             // name of DSP parameter file
	char *DSPVarFile,             // name of DSP variable names file
	unsigned short ModNum,        // pixie module number
	unsigned short BootPattern ); // boot pattern bit mask

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16StartListModeRun (
	unsigned short ModNum,        // module number
	unsigned short RunType,       // run type
	unsigned short mode );        // run mode

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16CheckRunStatus (
	unsigned short ModNum );      // Pixie module number  

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16EndRun (
	unsigned short ModNum );      // Pixie module number  


PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16CheckExternalFIFOStatus (
	unsigned int   *nFIFOWords,
	unsigned short ModNum );

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ReadDataFromExternalFIFO (
	unsigned int   *ExtFIFO_Data, // To receive the external FIFO data
	unsigned int   nFIFOWords,    // number of words to read from external FIFO
	unsigned short ModNum );      // module number


PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16WriteSglModPar (
    char *ModParName,             // the name of the module parameter
    unsigned int   ModParData,    // the module parameter value to be written to the module
    unsigned short ModNum );      // module number


}


namespace DAQ {
namespace DDAS {

/*! \namespace Test
 *
 * The Test namespace for DDAS provides some tools useful for
 * testing the DDAS system. At the moment, the testing version of the
 * Pixie16 API logs the interactions with the API and also allows the
 * user the ability to set up a fake system using Pixie16SetModuleType().
 *
 */
namespace Test {


void Pixie16SetModuleType(int modNum, int hdwrType);
std::vector<std::string> Pixie16GetActivityLog();
void Pixie16ResetActivityLog();

}
}
}
