
#ifndef CSYSTEMCONTROL_H
#define CSYSTEMCONTROL_H

#include <config.h>
#include <string>
#include <TCLObject.h>
#include <memory>


class CTCLInterpreter;
struct Tcl_Interp;
struct Tcl_Event;

class CBeginRun;
class CEndRun;
class CPauseRun;
class CResumeRun;
class CInit;
class CExit;
class CMonvarCommand;
class CRunStateCommand;

/*! \brief Encapsulation of UI control 
 *
 * This class manages the main interpreter of the VMUSBReadout program.
 * It owns all of the commands and provides some static methods that can
 * be called from other threads to schedule exits or signal errors. The bulk
 * of the VMUSBReadout program will live in the CSystemCOntrol::run() method. 
 * This method is just a wrapper around the Tcl_Main function.
 */
class CSystemControl
{
  private:
    CTCLInterpreter*     m_pInterpreter; //!< the main interpreter
    Tcl_ThreadId         m_threadId;     //!< the main thread id

    static std::string   m_initScript;   //!< initialization script

    // various commands that will be registered in the main tcl interpreter
    // we make them unique_ptrs so that their destructors get called at
    // program completion.
    static std::unique_ptr<CBeginRun>  m_pBeginRun;
    static std::unique_ptr<CEndRun>    m_pEndRun;
    static std::unique_ptr<CPauseRun>  m_pPauseRun;
    static std::unique_ptr<CResumeRun> m_pResumeRun;
    static std::unique_ptr<CInit>      m_pInit;
    static std::unique_ptr<CExit>      m_pExit;
    static std::unique_ptr<CMonvarCommand> m_pMonVar;
    static std::unique_ptr<CRunStateCommand> m_pStateCmd;
  
  public:

    /*! \brief Wrapper around Tcl_Main 
     *
     * \param argc  argument count
     * \param argv  cmd line args
     */
    void run(int argc, char** argv);

    void setInitScript(const std::string& path);

    /*! \brief Attempts to execute user-defined handler
     *
     * If the onTriggerFail proc is defined, then it is called with 
     * the message that was passed in as a Tcl_Event.
     * Otherwise, bgerror is called with the same message.
     *
     * Causes the application to exit.
     *
     * \param pEvent   event structure ( struct { Tcl_Event, StringPayload} )
     * \param flags    unused
     */
    static int AcquisitionErrorHandler(Tcl_Event* pEvent, int flags);

    /*! \brief Thread-safe callable for scheduling an exit
     * 
     * If Tcl_Main has not been entered, then this just calls exit. Otherwise
     * it schedules a Tcl_Event to call CSystemControl::tclExit when the
     * main interpreter gets around to it.
     *
     * \param status  the exit status
     */
    static int scheduleExit(int status);

    /*! \brief Evaluates "exit" in the main interpreter
     *
     * A message provided via the first argument is passed with the 
     * exit message.
     * 
     * \param pEvent  event information
     */
    static int tclExit(Tcl_Event* pEvent, int flags);

  private:
    /*! \brief Initialization routine for main interpreter
     * Initialize the interpreter.  This invoves:
     *  - Initializes the interpreter
     *  - Extends it with the run control, init, and exit commands
     *  - Returning TCL_OK so that the interpreter will start running the main loop.
     *
     */
    static int AppInit(Tcl_Interp* interp);

    static CTCLObject makeCommand(CTCLInterpreter* pInterp, 
                                  const char* verb, 
                                  std::string argument);
};
#endif
