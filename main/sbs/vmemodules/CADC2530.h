/*
#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2008.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#            Eric Kasten
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
*/

#ifndef CADC2530_H
#define CADC2530_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string>
#include <stdint.h>

#include <CVMEInterface.h>

#include <VmeModule.h>		// Needed to access registers via peek/poke.

/**
* @class CADC2530 CADC2530.h
* @brief Driver for the Hytec 2530 ADC.
*
* This is a support class for the Hytec 2530 ADC.
*
* Note that for now, copy construction will involve the creation of a
* new memory map and is therefore discouraged.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class CADC2530 {
  // ------------------------------------------------------------
  // Public members
  public:
    CADC2530(int crateNum = 0, uint32_t nBase = 0); //!< 'normal' constructor.
    CADC2530(const CADC2530& card); //!< Copy Constructor.
    ~CADC2530();		//!< Destructor.

    static bool checkCard(int,uint32_t,uint16_t&,uint16_t&);
    static uint16_t volt2lld(double);
    static uint16_t volt2uld(double);

    CADC2530& operator=(const CADC2530&); //!< Assignment operator

    const std::string& toString();      //!< Stringify

    int readListEvents(void*,int&);      //!< Read multiple events into buffer.
    int readHistogramChannel(void*,int); //!< Read a histogram channel.

    uint16_t cardType();		//!< Return module device type.
    uint16_t manufacturerId();    //!< Return module Id of card.

    void clearHistogramMemory();        //!< Zero only histogram memory.
    void clearMemory();                 //!< Zero all histo/list memory.
    void resetCard();                   //!< Reset card to initial settings.

    void clearListAddress();            //!< Clear list memory address counter.
    unsigned int getListAddress();      //!< Get list memory address counter.

    void arm();                         //!< Arm the module.
    void disarm();                      //!< Disarm the module.
    bool isArmed();                     //!< Check if module is armed.

    void modeHistogram(bool);           //!< Set to histogram mode.
    void modeGate();                    //!< Set to list mode.

    void resetCSR();                    //!< Reset the CSR to zero.
    void fastClear();                   //!< Force fast clear all channels.

    bool isBusy();                      //!< Check if modules is busy.
    bool dataReady();                   //!< Check if data ready.
    bool hasInterrupt();                //!< Check CSR interrupt status.

    void enableInterrupt();             //!< Enable interrupt.
    void disableInterrupt();            //!< Disable interrupt.

    void enableGate();                  //!< Enable gate mode.
    void disableGate();                 //!< Disable gate mode.

    void enableZeroCnv();               //!< Enable zero conversion.
    void disableZeroCnv();              //!< Disable zero conversion.

    void enableCalibration();           //!< Enable calibration.
    void disableCalibration();          //!< Disable calibration.

    void enableSlidingScale();          //!< Enable sliding scale.
    void disableSlidingScale();         //!< Disable sliding scale.

    void setIPL(uint16_t);        //!< Set the interrupt priority level
    uint16_t getIPL();            //!< Get the interrupt priority level

    uint16_t getCSR();            //!< Get the CSR value.

    void dataReadyOnEvent();            //!< Data ready when at least 1 event.
    void dataReadyOnFullness();         //!< Data ready on fullness flags.

    void setLLD(uint16_t);        //!< Set the LLD value.
    uint16_t getLLD();            //!< Get the LLD value.
    void setULD(uint16_t);        //!< Set the ULD value.
    uint16_t getULD();            //!< Get the ULD value.

    void setInterruptVector(uint16_t); //!< Set the interrupt vector. 
    uint16_t getInterruptVector();     //!< Get the interrupt vector.
    void setInterruptMask(uint16_t);   //!< Set the interrupt mask.
    uint16_t getInterruptMask();       //!< Get the interrupt mask.

    void clearFullnessFlags();          //!< Clear the fullness flags to 0.
    uint16_t getFullnessFlags();  //!< Get the fullness flags.
    bool isChannelFull(uint16_t); //!< Check if histo. chan. is full.
    bool channelHasData(uint16_t); //!< Check if histo. channel has data.
    bool isListFull();                  //!< Check if list full.
    bool isListHalfFull();              //!< Check if list is half full.

    unsigned int getEventCounter();     //!< Get the event counter.
    void clearEventCounter();           //!< Clear the event counter. 

    // Calibration and test methods
    uint16_t getCTR();            //!< Get the CTR value.
    void resetCTR();                    //!< Reset the CTR to zero.
    void setCTRchannel(uint16_t); //!< Set the CTR channel.
    uint16_t getCTRchannel();     //!< Get the CTR channel.
    void enableAutoFastClear();         //!< Enable CTR auto fast clear. 
    void disableAutoFastClear();        //!< Disable CTR auto fast clear. 
    void enableMUXswitch();             //!< Enable CTR MUX switch.
    void disableMUXswitch();            //!< Disable CTR MUX switch.
    void enableCompensation();          //!< Enable CTR compensation.
    void disableCompensation();         //!< Disable CTR compensation.
    void setSSTR(uint16_t);       //!< Set the sliding scale test reg. 
    uint16_t getSSTR();           //!< Get the sliding scale test reg. 

  // ------------------------------------------------------------
  // Protected members
  protected:
    int    my_nCrate;		  //!< VME crate number housing the module.
    uint32_t my_nBase;	  //!< Base physical VME address in crate.
    uint32_t my_nMemOffset;  //!< Register vale for list/histogram offset.
    uint16_t my_nCardId;    //!< Card Id (filled in at MapCard)
    uint16_t my_nCardType;  //!< Type of card (filled in at MapCard)
    void*  my_nModFd;		  //!< File desc. for registers open on VME.
    void*  my_nMemFd;		  //!< File desc. for memory open on VME.
    bool my_eventmode;            //!< True when in event mode.
    unsigned int my_cureventpos;  //!< The current event uint32_t position.


    volatile uint16_t *my_pModule; //!< Pointer to modulre registers etc.
    volatile unsigned int *my_pMemory; //!< Pointer to list/histogram memory.


    void mapModule();           //!< Map register memory
    void mapMemory();           //!< Map list/histogram memory
    void destroyModule();       //!< Destroy register memory map
    void destroyMemory();       //!< Destroy list/histogram memory map

    void destroyCard();		//!< Destroy a card memory maps.
    void mapCard();		//!< Map the card's memory.

    void slotInit();		//!< Initialize a slot

    int readListEvent(void*,int); //!< Read event into a user buffer.

    uint32_t calcMemoryOffset(uint32_t); //!< Compute memory offset
    uint32_t setMemoryOffset(uint32_t); //!< Set the memory offset

    void setCSR(uint16_t);              //!< Set the CSR to a value.
    void setCSRbits(uint16_t,uint16_t); //!< Set bits in the CSR.
    void resetCSRbits(uint16_t);        //!< Reset bits in the CSR.

    void setCTR(uint16_t);              //!< Set the CTR to a value.
    void setCTRbits(uint16_t,uint16_t); //!< Set bits in the CTR.
    void resetCTRbits(uint16_t);        //!< Reset bits in the CTR.
};

#endif
