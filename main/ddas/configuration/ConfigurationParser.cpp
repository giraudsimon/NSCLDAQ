#include "ConfigurationParser.h"
#include "Configuration.h"
#include "FirmwareVersionFileParser.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <unistd.h>

using namespace std;
#define  FILENAME_STR_MAXLEN     256


namespace DAQ {
namespace DDAS {

ConfigurationParser::ConfigurationParser()
 : m_matchExpr(R"(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)")
{}

/*!
 * \brief Parse the contents of the cfgPixie16.txt file
 *
 * \param input     the input stream associated with the cfgPixie16 content (likely an std::ifstream)
 * \param config    a configuration to store the parsed data
 *
 * \throws std::runtime_error if failed to read in sufficient slot map data for number of modules
 * \throws std::runtime_error if settings file does not end in .set
 *
 */
void ConfigurationParser::parse(istream &input, Configuration &config)
{
    int CrateNum;
    std::vector<unsigned short> PXISlotMap;
    int NumModules;

    char temp[FILENAME_STR_MAXLEN];
    std::string line;
    std::string DSPParFile;

    std::string ComFPGAConfigFile_RevBCD;
    std::string SPFPGAConfigFile_RevBCD;
    std::string DSPCodeFile_RevBCD;
    std::string DSPVarFile_RevBCD;

    std::string ComFPGAConfigFile_RevF_250MHz_14Bit;
    std::string SPFPGAConfigFile_RevF_250MHz_14Bit;
    std::string DSPCodeFile_RevF_250MHz_14Bit;
    std::string DSPVarFile_RevF_250MHz_14Bit;

    std::string ComFPGAConfigFile_RevF_500MHz_12Bit;
    std::string SPFPGAConfigFile_RevF_500MHz_12Bit;
    std::string DSPCodeFile_RevF_500MHz_12Bit;
    std::string DSPVarFile_RevF_500MHz_12Bit;
    
    std::map<int, FirmwareMap> perModuleFirmware;    // Maps capture nosuch
    std::map<int, std::string> perModuleSetfiles;    // better than arrays..


    input >> CrateNum;
    input.getline(temp,FILENAME_STR_MAXLEN);  // skip rest of line (comment)
    input >> NumModules;
    input.getline(temp,FILENAME_STR_MAXLEN);  // skip rest of line (comment)
    PXISlotMap.resize(NumModules);
    for(int i = 0; i < NumModules; i++){
        auto slotInfo = parseSlotLine(input);
        PXISlotMap[i] = std::get<0>(slotInfo);
        
        std::string perModuleMap = std::get<1>(slotInfo);
        if (!perModuleMap.empty()) {
            std::ifstream fwMapStream(perModuleMap);
            FirmwareMap aMap;
            FirmwareVersionFileParser fwparser;
            fwparser.parse(fwMapStream, aMap);
            perModuleFirmware[i] = aMap;
            std::string perModuleSetfile = std::get<2>(slotInfo);
            if (!perModuleSetfile.empty()) {
                perModuleSetfiles[i] = perModuleSetfile;
            }
        }
        
    }
    input >> DSPParFile;
    input.getline(temp,FILENAME_STR_MAXLEN);

    //check to make sure that this line contains a set file (.set extension)
    //since the format has changed from previous versions of the code.
    if( DSPParFile.find_last_of(".set") == std::string::npos) {
        std::string errmsg("The file ");
        errmsg += DSPParFile;
        errmsg += " read in from configuration file";
        errmsg += " does not appear to be a *.set file required by DDAS";
        throw std::runtime_error(errmsg);
    }


    // the [100MSPS], [250MSPS], and [500MSPS] tags are still supported but should not be used.
    while (getline(input, line)) {
        if (line == "[100MSPS]") {
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevB_100MHz_12Bit, calibration);

            config.setFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevC_100MHz_12Bit, calibration);

            config.setFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevD_100MHz_12Bit, calibration);

        } else if (line == "[250MSPS]"){
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevF_250MHz_14Bit, calibration);

        } else if (line == "[500MSPS]"){
            FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
            double calibration = extractClockCalibration(input);

            config.setFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit, fwConfig);
            updateClockCalibration(HardwareRegistry::RevF_500MHz_12Bit, calibration);

        } else if (std::regex_match(line , m_matchExpr) ) {
            int revision, adcFreq, adcRes;
            if (parseHardwareTypeTag(line, revision, adcFreq, adcRes)) {

                FirmwareConfiguration fwConfig = extractFirmwareConfiguration(input);
                double calibration = extractClockCalibration(input);
                int type = HardwareRegistry::createHardwareType(revision, adcFreq, adcRes, calibration);
                config.setFirmwareConfiguration(type, fwConfig);

            } else {
                std::string msg("ConfigurationParser::parse() Failed to parse ");
                msg += " the hardware tag '" + line + "'";
                throw std::runtime_error(msg);
            }
        }

    }

    config.setCrateId(CrateNum);
    config.setNumberOfModules(NumModules);
    config.setSlotMap(PXISlotMap);
    config.setSettingsFilePath(DSPParFile);
    
    // Set the per module firmware maps:
    
    for (auto const& p : perModuleFirmware) {
      config.setModuleFirmwareMap(p.first, p.second);
    }
    // Set the per module DSP Parameter maps:
    
    for (auto const& p : perModuleSetfiles) {
     config.setModuleSettingsFilePath(p.first, p.second);
    }
}

void ConfigurationParser::updateClockCalibration(int type, double calibration)
{
    HardwareRegistry::HardwareSpecification& hdwrSpec
            = HardwareRegistry::getSpecification(type);
    hdwrSpec.s_clockCalibration = calibration;
}



/*!
 * \brief parseHardwareTypeTag
 *
 * \param line          the tag to parse
 * \param revision      integer variable to store X into
 * \param freq          integer variable to store Y into
 * \param resolution    integer variable to store Z into
 *
 * Parses the values of X, Y, and Z from a tag of the form [RevX-YBit-ZMSPS].
 *
 * \retval false if line is not in the format [RevX-YBit-ZMSPS]
 * \retval true otherwise
 */
bool ConfigurationParser::parseHardwareTypeTag(const std::string& line,
                                               int &revision, int &freq, int &resolution)
{
    bool result = false;
    std::smatch color_match;
    std::regex_search(line, color_match, m_matchExpr);

    if (color_match.size() == 4) {
      std::string revStr(color_match[1].first, color_match[1].second);
      revision = std::stoi(revStr, 0, 0); // auto detect base
      resolution = std::stoi(std::string(color_match[2].first, color_match[2].second));
      freq = std::stoi(std::string(color_match[3].first, color_match[3].second));
      result = true;
    }
    return result;
}

/*!
 * \brief ConfigurationParser::extractFirmwareConfiguration
 *
 * \param input     the stream to read from
 *
 * The current implementation does not support reading firmware paths with whitespace in them
 *
 * \return a firmware configuration encapsulating the data read from the file.
 *
 * \throw std::runtimer_error if an error occurs while processing next 4 lines
 */
FirmwareConfiguration ConfigurationParser::extractFirmwareConfiguration(std::istream &input)
{
    FirmwareConfiguration fwConfig;
    // load in files to overide defaults

    //load syspixie
    input >> fwConfig.s_ComFPGAConfigFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load fippipixe
    input >> fwConfig.s_SPFPGAConfigFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load ldr file
    input >> fwConfig.s_DSPCodeFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    //load var file
    input >> fwConfig.s_DSPVarFile;
    if (!input.good())
        throw std::runtime_error("Configuration file contains incomplete hardware specification!");

    return fwConfig;
}

/*!
 * \brief ConfigurationParser::extractClockCalibration
 *
 * \param input the stream to read from
 *
 * \return the clock calibration integer that was read from the file
 *
 * \throw std::runtime_error if an error occurs while processing the next line
 */
double ConfigurationParser::extractClockCalibration(std::istream& input)
{
    double calibration;
    input >> calibration;
    if (!input.good()) {
        throw std::runtime_error("ConfigurationParser attempted to parse an incomplete hardware specification!");
    }
    return calibration;
}

/**
 * parseSlotLine
 *    Parses a slot line.  Slot lines consist of a mandatory slot number,
 *    and optional substitute firmware mapping file and an optional .set file
 *    for that module.  Care must be taken since any populated field (other than
 *    the slot number) might actually be a comment.  Requirements:
 *      - Filenames cannot have spaces in their paths.
 *      - Files must be readable by the user.
 *      - #'s must be spaced from the last file e.g.:
 *         1 firmwaremap#  this is an error but.
 *         2 firmwaremap  # This is ok.
 *         3 firmwaremap setfile.set # as is this.
 *   
 * @param input - input stream from which the line is parsed.
 * @return ConfigurationParser::SlotSpecification
 *        - a tuple that contains the slot number and
 *         and file paths.  The filepaths will be empty strings if omitted.
 * @throw std::runtime_error if there are errors processing this line.
 *       -  Slot cannot be decoded.
 *       -  A file is not readable.
 */
ConfigurationParser::SlotSpecification
ConfigurationParser::parseSlotLine(std::istream& input)
{
    
    std::string       line;
    std::getline(input, line);
    
    if (!input.good()) {                       // Maybe eof?
       throw std::runtime_error(
           "Unable to read a line from the input file when parsing a slot line"
       );
    }
    std::stringstream lineStream(line);
    
    int slot;
    std::string firmwareMap;
    std::string setFile;
    
    // do the slot separately so that we can indicate we can't parse:
    
    if (!(lineStream >> slot)) {
    
       std::string msg("Unable to parse a slot number from : ");
       msg += line;
       
       throw std::runtime_error(msg);
    }
    
    // Now the files:
    
    lineStream >> firmwareMap >> setFile;
    
    // Handle leading #'s which imply a comment.
    
    if (firmwareMap[0] == '#') {                 // both are comments.
        firmwareMap.clear();
        setFile.clear();
    } else if (setFile[0] == '#') {             // setfile is a comments
        setFile.clear();               
    }
    // Check readability of any files:
    
    if (firmwareMap != "") {
         if (access(firmwareMap.c_str(), R_OK)) {
            std::string msg("Unable to read firmware mapping file ");
            msg += firmwareMap;
            msg += " from ";
            msg += line;
            
            throw std::runtime_error(msg);
         }
         if (setFile != "") {
             if (access(setFile.c_str(), R_OK)) {
                 std::string msg("Unable to read DSP Parameter file ");
                 msg += setFile;
                 msg += " from ";
                 msg += line;
                 
                 throw std::runtime_error(msg); 
             }
         }
    }
    // Ok everything is good so:
    
    return std::make_tuple(slot, firmwareMap, setFile);
}

} // end DDAS namespace
} // end DAQ namespace
