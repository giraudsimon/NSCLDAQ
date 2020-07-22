/**
#******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

##
# @file CompassEventSegment.h
# @brief CAEN PHA event segment whose configuration comes from Compass.
# @author Ron Fox (rfoxkendo@gmail.com)

*/
#include "CompassEventSegment.h"
#include "CAENPha.h"
#include "CAENPhaParameters.h"
#include "CAENPhaChannelParameters.h"
#include "DPPConfig.h"
#include <CAENDigitizer.h>
#include <CAENDigitizerType.h>
#include "CompassProject.h"
#include "CAENPhaBuffer.h"

#include <sstream>
#include <iostream>
#include <stdlib.h>

/**
 * constructor
 *    For now just initialize the data.  The real action is in
 *    the initialize method.  Actually at this point we don't even
 *    require the existence or readability of the configuration file.
 *
 *  @param filename - Name of the compass configuration file.
 *  @param sourceId  - Event builder source id.
 *  @param linkType - Type of connection with the digitizer.
 *  @param linknum  - If CONNET, the link number of the interface.
 *  @param node     - If CONNET, the node on the daisy chain.
 *  @param base     - If necessary the module base addresss.
 *  @param pCheatFile - if not nullptr - register cheat file name.
 */
CompassEventSegment::CompassEventSegment(
        std::string filename, int sourceId,
        CAEN_DGTZ_ConnectionType linkType, int linkNum, int node, int base,
				const char* pCheatFile
	) : m_filename(filename), m_board(nullptr), m_id(sourceId),
    m_linkType(linkType), m_nLinkNum(linkNum), m_nNode(node), 
    m_nBase(base) , m_pCheatFile(pCheatFile)
{
    
}
/**
 * destructor
 */
CompassEventSegment::~CompassEventSegment()
{
    delete m_board;
}
/**
 * initialize
 *    Prepare the board for data taking.
 *    - Parse the Compass file.
 *    - Instantiate the m_board object
 *    - Setup the board from the parsed/processed configuration
 *      file.
 */
void
CompassEventSegment::initialize()
{
    try {
        CompassProject project(m_filename.c_str());
        project();
        
        // We need to locate the board that matches our parameters.
        
        CAENPhaParameters* ourBoard(nullptr);
        
        for (int i = 0; i < project.m_connections.size(); i++) {
            if (
                (m_linkType == project.m_connections[i].s_linkType)  &&
                (m_nLinkNum  == project.m_connections[i].s_linkNum)   &&
                (m_nNode    == project.m_connections[i].s_node)      &&
                (m_nBase    == project.m_connections[i].s_base) 
            ) {
                ourBoard = (project.m_boards[i]);
                break;
            }
        }
        if (!ourBoard) {                    // no matching board.
					std::string msg = "No board in ";
					msg +=  m_filename;
					msg +=  " matches our connection parameters";
					throw msg;   
        }
        // Now we can setup the board.
				
				for (int i =0; i < 16; i++) {
					m_lastTriggerCount[i] = 0;
					m_lastMissedTriggerCount[i] = 0;
					
					m_triggerCount[i] = 0;
					m_missedTriggers[i] =0;
					m_countersActive = ourBoard->m_includeCounters;
				}
        setupBoard(*ourBoard);
    } catch (std::string msg) {
        std::cerr << "Initialization failed - " << msg << std::endl;
        throw;
    }
    catch (std::pair<std::string, int> phaErr) {
       std::cerr << "Initialization caught a error: "
        << phaErr.first << "(" << phaErr.second << ")\n";
       throw;
    }
}
/**
 * clear
 *    Clear the digitizer.
 */
void
CompassEventSegment::clear()
{
    // This is a no-op as reads are destructive.
}
/**
 *  disable()
 *     Diable the digitizer.  Our code assumes we have a valid
 *     digitizer driver.
 */
void
CompassEventSegment::disable()
{
    try {
        m_board->shutdown();
    }
    catch (std::pair<std::string, int> err) {
        std::cerr << "Board shutdown failed: "
            << err.first << " (" << err.second << ")\n";
        throw;
    }
}
/**
 * read
 *    Read an event from the digitizer and pass it up along the
 *    chain of control.
 *
 * @param pBufer - Pointer to the buffer into which to put the data.
 * @param maxwords - Largest event we can fit into th event buffer.
 * @return size_t  - Number of words read.
 */
size_t
CompassEventSegment::read(void* pBuffer, size_t maxwords)
{
  std::tuple<int, const CAEN_DGTZ_DPP_PHA_Event_t*, const CAEN_DGTZ_DPP_PHA_Waveforms_t*>
  event = m_board->Read();
  
  
  int nsPerTick = m_board->Getm_nsPerTick();
  // Figure out the event size.  Note that since CAEN_DGTZ_DPP_PHA_Event_t*
  // and CAEN_)DGTZ_DPP_PHA_Waveforms_t almost certainly have pads,
  // we're going to write the data an item at a time into the buffer.
  
  int chan = std::get<0>(event);
  const CAEN_DGTZ_DPP_PHA_Event_t* dppData = std::get<1>(event);
  const CAEN_DGTZ_DPP_PHA_Waveforms_t* wfData = std::get<2>(event);

  if (!(dppData || wfData)) {
    return 0;                            // No event.
  }
  
  // CAENPha returns a timetag calibrated to ns.
  
  setTimestamp(dppData->TimeTag);        // Event timestamp - in ns from CAENPha
  setSourceId(m_id);                     // Source id from member data.         
  size_t eventSize = CAENPhaBuffer::computeEventSize(*dppData, *wfData);
  //std::cout <<std::dec<< "\tSourceID:"<< m_id << "\tEnergy:" << dppData->Energy<<"\tTs:"<<dppData->TimeTag<<std::endl;
 
  if ((eventSize / sizeof(uint16_t)) > maxwords) {
    throw std::string("PHAEventSegment size exceeds maxwords - expand max event size");
  }
  
  // Header consists of the total inclusive size in bytes and the channel #
  
  pBuffer = CAENPhaBuffer::putLong(pBuffer, eventSize);
  pBuffer = CAENPhaBuffer::putLong(pBuffer, chan);
  
  // Body is dpp data followed by wf data:
  
  pBuffer = CAENPhaBuffer::putDppData(pBuffer, *dppData);
  pBuffer = CAENPhaBuffer::putWfData(pBuffer, *wfData);
	
	// If the counters are enabled, this chanel's counters need to be updated
	// from extras2
  
	if (m_countersActive || true) {      // unconditionally on - for now
		uint32_t trigCount = dppData->Extras2 & 0xffff;
		uint32_t missedTrigs = dppData->Extras2 >> 16;
		
		// There's a per channel multiplier that must be applied:
		
		uint32_t mult = 1;   // m_board->m_incrementsPerLsb[chan]; // multiplier doesn't work.
		
		// Figure out the increments ... allowing for rollover.
	
		unsigned trinc(0);	
		if (trigCount >= m_lastTriggerCount[chan]) {
			trinc = trigCount - m_lastTriggerCount[chan];
		} else {
			trinc = trigCount + 0x10000 - m_lastTriggerCount[chan];
		}
		m_triggerCount[chan] += trinc;
		m_lastTriggerCount[chan] = trigCount;
	
		unsigned misinc(0);
		if (missedTrigs >= m_lastMissedTriggerCount[chan]) {
			misinc = missedTrigs - m_lastMissedTriggerCount[chan];
		} else {
			misinc = missedTrigs + 0x10000 - m_lastMissedTriggerCount[chan];
		}
		m_missedTriggers[chan] += misinc;
		m_lastMissedTriggerCount[chan] = missedTrigs;
	
		
	}
  
  return (eventSize / sizeof(uint16_t));     
}
/**
 * checkTrigger
 *    Return true if our board has data.. used for the compound trigger
 *    event struct.
 * @return bool - true if the board has data.
 */
bool
CompassEventSegment::checkTrigger()
{
    return m_board->haveData();
}
/*-------------------------------------------------------------------
 * Private member functions.
 */

/**
 * setupBoard
 *    Setup the board:
 *    - If there's a Pha object, delete it and set it's pointer to null.
 *    - Create a new board object save it as m_pBoard.
 *    - Invoke the board object's setup mode
 *    @param board - Reference to our board configuration object.
 */
void
CompassEventSegment::setupBoard(CAENPhaParameters& board)
{
    delete m_board;                // Get rid of any prior board driver.
    m_board = nullptr;
    m_board = new CAENPha(
        board, m_linkType, m_nLinkNum,
				m_nNode, m_nBase,
        board.s_startMode, true,                    // TODO get this from board
        board.startDelay,
				m_pCheatFile
    );
    m_board->setup();
    
}

