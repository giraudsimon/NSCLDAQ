
#include <pixie16app_export.h>
//#include <ChannelDataGenerator.h>
//#include <Channel.h>
#include <Configuration.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>


//////////////////////////////////////////////////////////////////////////////
// static data members

static DAQ::DDAS::Configuration *gTestConfig = nullptr;
static std::vector<std::string> *gActivityLog = nullptr;

// avoid static initialization order fiasco with the construct on first use
// idiom. This is safe because there are no resources that need to be
// freed at the termination of the program besides our memory.
static DAQ::DDAS::Configuration& getTestConfig () {
    if (gTestConfig == nullptr) {
        gTestConfig = new DAQ::DDAS::Configuration;
    }

    return *gTestConfig;
}

// avoid static initialization order fiasco with the construct on first use
// idiom. For the same reasons given for the test configuration, this is safe.
static std::vector<std::string>& getTestActivityLog() {
    if (gActivityLog == nullptr) {
        gActivityLog = new std::vector<std::string>();
    }

    return *gActivityLog;
}


extern "C" {


PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16InitSystem (
	unsigned short NumModules,    // total number of Pixie16 modules in the system
	unsigned short *PXISlotMap,   // an array containing the PXI slot number for each pixie16 module
	unsigned short OfflineMode ) 
{
    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ExitSystem (
    unsigned short ModNum )
{
    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ReadModuleInfo (
	unsigned short ModNum,        // module number
	unsigned short *ModRev,       // returned module revision
	unsigned int   *ModSerNum,    // returned module serial number
	unsigned short *ModADCBits,   // returned module ADC bits
    unsigned short *ModADCMSPS )
{


    auto hdwrMap = getTestConfig().getHardwareMap();

    auto type = hdwrMap.at(ModNum);

    auto specs = DAQ::DDAS::HardwareRegistry::getSpecification(type);
    *ModRev = specs.s_hdwrRevision;
    *ModSerNum = 0;
    *ModADCBits = specs.s_adcResolution;
    *ModADCMSPS = specs.s_adcFrequency;

    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16BootModule (
	char *ComFPGAConfigFile,      // name of communications FPGA configuration file
	char *SPFPGAConfigFile,       // name of signal processing FPGA configuration file
	char *TrigFPGAConfigFile,     // name of trigger FPGA configuration file
	char *DSPCodeFile,            // name of executable code file for digital signal processor (DSP)
	char *DSPParFile,             // name of DSP parameter file
	char *DSPVarFile,             // name of DSP variable names file
	unsigned short ModNum,        // pixie module number
    unsigned short BootPattern )  // boot pattern bit mask
{
    std::stringstream msg;
    msg << "f0:" << ComFPGAConfigFile;
    msg << ",f1:" << SPFPGAConfigFile;
    msg << ",f2:" << DSPCodeFile;
    msg << ",f3:" << DSPVarFile;
    msg << ",index:" << ModNum;
    msg << ",pattern:" << std::hex << BootPattern << std::dec;

    getTestActivityLog().push_back(msg.str());

    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16StartListModeRun (
	unsigned short ModNum,        // module number
	unsigned short RunType,       // run type
	unsigned short mode )        // run mode
{
    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16CheckRunStatus (
	unsigned short ModNum )      // Pixie module number  
{
    return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16EndRun (
	unsigned short ModNum )      // Pixie module number  
{
    return 0;
}


PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16CheckExternalFIFOStatus (
	unsigned int   *nFIFOWords,
	unsigned short ModNum )
{
  *nFIFOWords = 60;
  return 0;
}

PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16ReadDataFromExternalFIFO (
	unsigned int   *ExtFIFO_Data, // To receive the external FIFO data
	unsigned int   nFIFOWords,    // number of words to read from external FIFO
	unsigned short ModNum )      // module number
{
//  using namespace DAQ::DDAS;

//    unsigned int *pCursor = ExtFIFO_Data;
//    std::vector<std::shared_ptr<channel> > data;
//    for(int i=0; i<10; ++i) {
//        std::shared_ptr<channel> pChan = Test::channelGenerator();
//        data.push_back(pChan);
//    }

//    std::random_device rd;
//    std::mt19937 randomizer(rd());

//    std::shuffle(data.begin(), data.end(), randomizer);

//    for (auto& pChan : data) {
//        for (int j=0; j<6; ++j) {
//            *pCursor = pChan->data[j];
//            ++pCursor;
//        }
//    }

    return 0;
}


PIXIE16APP_EXPORT int PIXIE16APP_API Pixie16WriteSglModPar (
    char *ModParName,             // the name of the module parameter
    unsigned int   ModParData,    // the module parameter value to be written to the module
    unsigned short ModNum )      // module number
{
    return 0;
}



} // end extern

namespace DAQ {
namespace DDAS {
namespace Test {

/*!
 * \brief Setup a fake module
 *
 * \param modNum    index of module in system (starts at 0)
 * \param hdwrType      hardware type
 *
 * Using this method, a fake system can be set up from which hardware
 * information can be queried. The Pixie16ReadModuleInfo() function returns
 * information using what was set up using this function.
 */
void Pixie16SetModuleType(int modNum, int hdwrType) {

    if (modNum >= getTestConfig().getNumberOfModules()) {
        getTestConfig().setNumberOfModules(modNum+1);
    }

    auto hdwrMap = getTestConfig().getHardwareMap();
    hdwrMap[modNum] = hdwrType;

    getTestConfig().setHardwareMap(hdwrMap);
}


/*!
 * \brief Retrieve the activity log for the test Pixie16App API
 *
 * \return log of all activities sinces last clear or program startup
 */
std::vector<std::string> Pixie16GetActivityLog() {
    return getTestActivityLog();
}

/*!
 * \brief Clear out the activity log
 */
void Pixie16ResetActivityLog() {
    getTestActivityLog().clear();
}

}
}
}
