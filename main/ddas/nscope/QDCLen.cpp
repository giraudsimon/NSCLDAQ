/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  QDCLen.cpp
 *  @brief: Implement the dialog that prompts for QDC Lengths.
 */
#include "QDCLen.h"
//#include "pixie16app_export.h"
#include <config.h>
#include <config_pixie16api.h>
#include <stdio.h>

/*  These are the channel parameters involved:
 */
// Each 
static const char*  QdcLenParams[8] = {
    "QDCLen0", "QDCLen1", "QDCLen2", "QDCLen3", "QDCLen4",
    "QDCLen5", "QDCLen6", "QDCLen7"
  };

/**
 * constructor
 * @pararm p  - window pointer.
 * @param main - main window (parent?).
 * @note It's silly to pass more than that, we know there are 9 colummns and
 *       16 rows.  We also know the name of this dialog.
 */
QDCLen::QDCLen(const TGWindow* p, const TGWindow* main) :
    Table(p, main, 9, 16, "QDC Lengths", 13),
    m_module(0),
    m_channel(0),
    m_loaded(false)
{
    // Set all the titles and such.
    
    // Channel (row) labels).
    char number[10];                  // Channel number text string:
    cl0->SetText("Ch #");
    for (int i =0; i < 16; i++) {
        sprintf(number, "%2d", i);
        Labels[i]->SetText(number);
    }
    // Column labels (QDCLenn)
    
    char qdclen[20];
    for (int i =0; i < 8; i++) {
        sprintf(qdclen, "QDCLen %d", i);
        CLabel[i]->SetText(qdclen);
    }
    // Now the copy button:
    
    TGHorizontal3DLine *ln2 = new TGHorizontal3DLine(mn_vert, 200, 2);
  mn_vert->AddFrame(ln2,
		    new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0,
				      0, 10, 10));
  TGHorizontalFrame *CopyButton = new TGHorizontalFrame(mn_vert, 400, 300);
  mn_vert->AddFrame(CopyButton,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0,
				      0));

  TGLabel *Copy = new TGLabel(CopyButton, "Select channel #");

  chanCopy = new TGNumberEntry(CopyButton, 0, 4, MODNUMBER + 1000, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 15);
  chanCopy->SetButtonToNum(0);
  chanCopy->IsEditable();
  chanCopy->SetIntNumber(0);
  CopyButton->AddFrame(Copy,
		       new TGLayoutHints(kLHintsCenterX, 5, 10, 3, 0));
  CopyButton->AddFrame(chanCopy,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));

  chanCopy->Associate(this);

////////////////////Copy button per se///////////////////
  TGTextButton *copyB =
      new TGTextButton(CopyButton, "C&opy", COPYBUTTON + 1000);
  copyB->Associate(this);
  copyB->
      SetToolTipText
      ("Copy the setup of the selected channel to all channels of the module",
       0);
  CopyButton->AddFrame(copyB,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));



///////////////////////////////////////////////////////////////////////
  MapSubwindows();
  auto frameSize = GetSize();
  frameSize.fWidth *= 1.5;
  Resize(frameSize);			// resize to default size


}
/**
 * destructor the base class destructor presumably destroys this window
 * so all else is unecessary.
 */
QDCLen::~QDCLen()
{}
/**
 * ProcessMessage
 *    Processes events from the user interface.  We really only expect
 *    kC_COMMAND messages but those have a plethora of subcommands
 * @param msg - message encoded. in some way.
 * @param parm1, parm2 - parameters of the message.
 * @return Bool_t - I think this is success kTRUE is unconditionally returned.
 */
Bool_t
QDCLen::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
    case kC_COMMAND:
      // It's all in the subcommand and parameters now:
      
      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
        // The parameter parm1 drives the rest of this:
        
          switch (parm1) {
            case MODNUMBER:                 // change module number:
              if(parm2 == 0) {               // up
                if (m_module < numModules-1) {
                  numericMod->SetIntNumber(++m_module);
                }
              } else {                           // down
                if (m_module > 0) {
                  numericMod->SetIntNumber(--m_module);
                }
              }
              load_info(m_channel);
              break;
            case (MODNUMBER + 1000):          // CHange channel number
              if (parm2 == 0) {               // increment
                if (m_channel < 16) ++m_channel;
              } else {                        // decrement
                if (m_channel > 0) --m_channel;
              }
              chanCopy->SetIntNumber(m_channel);
              break;
            case LOAD:                        // Load from module -> values
              m_loaded = true;
              load_info(m_module);
              break;
            case APPLY:                       // Load module from values:
              if(m_loaded) {                  // Require that we were populated once.
                change_values(m_module);
              } else {
                std::cout << "PLease load the values once at least\n";
              }
              break;
            case CANCEL:                    // Dismiss the window.
              DeleteWindow();
              break;
            case (COPYBUTTON + 1000):      // Copy the lengths from the selected chan to all.
              for (int i =0; i < 16; i++) {
                if (i != m_channel) {
                  for (int len = 1; len < 9; len++) {
                    char value[10];
                    double qdclen = NumEntry[len][m_channel]->GetNumber();
                    sprintf(value, "%1.3f", qdclen);
                    NumEntry[len][i]->SetText(value);
                  }
                }
              }
              break;
            default:
              break;
          }
          break;
        default: break;
      }
      
      break;
    default:
      break;
  }
  return kTRUE;
}

/**
 * load_info
 *    Load the QDC Lengths from the module to the dialog.
 * @param mod - module number.
 * @return int
 * @retval 0 - all operations were successful.
 * @retval < 0 - Return value of the first failed operation.
 */
int
QDCLen::load_info(Long_t mod)
{
  int result;
  double value;
  char   text[10];
  for (int ch = 0; ch < 16; ch++) {
      for(int len = 0; len < 8; len++) {
        result = Pixie16ReadSglChanPar(QdcLenParams[len], &value, mod, ch);
        if (result < 0) {
          std::cerr << "Failed to read parameter " << QdcLenParams[len]
              << " from channel " << ch << " of module " << mod
              << " return value from ReadSglChanPar: " << result
              <<  std::endl;
          return result;
        }
        sprintf(text,"%1.3f", value);
        NumEntry[len+1][ch]->SetText(text);
      }
  }
  return result;
}
/**
 * change_values
 *    Load the contents of the array of lengths into the module.
 *    Inner iteration is over the length.
 *  @param mod - module number.
 *  @return int
 *  @retval 0  - everything was written.
 *  @retval < 0 - error value of the first failing WritSglChanPar.
 */
int
QDCLen::change_values(Long_t mod)
{
  double value;
  int    result;
  
  for (int ch = 0; ch < 16; ch++) {
    for (int len =0; len < 8; len++) {
      value = NumEntry[len+1][ch]->GetNumber();
      result = Pixie16WriteSglChanPar(QdcLenParams[len], value, mod, ch);
      if (result < 0) {
        std::cerr << "Failed to write parameter: " << QdcLenParams[len]
          << " to channel " << ch << " of module " << mod
          << ".  Return value from WriteSglChanPar: " << result << std::endl;
        return result;
      }
    }
  }
  return result;
}
/**
 * setModuleNumber
 *   @param mod new module number.
 */
void
QDCLen::setModule(short unsigned mod)
{
    m_module = mod;
    numericMod->SetIntNumber(mod);
}
