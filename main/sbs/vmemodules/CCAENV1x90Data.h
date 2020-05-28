#ifndef CCAENV1X90DATA_H
#define CCAENV1X90DATA_H
/*!
  This file contains a namespace that has several static inline
  functions for peeling bits and pieces of the data buffer and
  making sense of what we get.
*/
#include <string>
#include <stdint.h>


namespace CCAENV1x90Data {

  // Definitions of bits that appear in all data words:

  static const uint32_t TYPE_MASK    (0xf8000000);	// Data type field
  static const uint32_t GLOBAL_HEADER(0x40000000);	// GBL header type fld
  static const uint32_t TDC_HEADER   (0x08000000); // TDC header
  static const uint32_t TDC_TRAILER  (0x18000000); // TDC Trailer
  static const uint32_t MEASUREMENT  (0x00000000); // TDC Measurement
  static const uint32_t TDC_ERROR    (0x20000000); // TDC Error flag.
  static const uint32_t TRIGGER_TIME (0x88000000); // Trigger time tag.
  static const uint32_t GLOBAL_TRAILER(0x80000000);// Global trailer.
  static const uint32_t FILLER_LONG   (0xc0000000); // Filler longs.

  // Definitions in the global header only:

  static const uint32_t EVENTCOUNT_MASK (0x07ffffe0);
  static const uint32_t  EVENT_RSHIFT    (0x5);
  static const uint32_t  GEO_MASK        (0x1f);

  // Definitions in the TDC header longword:

  static const uint32_t TDC_MASK        (0x03000000);
  static const uint32_t  TDC_RSHIFT      (24);
  static const uint32_t EVENTID_MASK    (0x007ff800);
  static const uint32_t  EVENTID_RSHIFT  (12);
  static const uint32_t  BUNCHID_MASK    (0x000007ff);
  static const uint32_t  TDCWORDCOUNT_MASK  (0x000007ff);

  // Definitions in the TDC measurement word:

  static const uint32_t  TRAILING_BIT     (0x04000000);
  static const uint32_t V1190CHANNEL_MASK(0x03f80000);
  static const uint32_t V1290CHANNEL_MASK(0x03e00000);
  static const uint32_t  V1190CHANNEL_RSHIFT(19);
  static const uint32_t  V1290CHANNEL_RSHIFT(21);
  static const uint32_t V1190DATA_MASK     (0x0007ffff);
  static const uint32_t V1290DATA_MASK     (0x001fffff);

  /*!
    Mask bits in the TDC Error word.  There are no functions
    to check individual errors. Call TDCErrorbits to extract
    the bits and then and against the masks below.
  */
  static const uint32_t HITLOST_0_FIFO (0x0001); // hits lost group 0 FIFO overflow
  static const uint32_t HITLOST_0_L1   (0x0002); // hits lost group 0 L1 overflow
  static const uint32_t HITERROR_0     (0x0004); // hit error in group 0.'
  static const uint32_t HITLOST_1_FIFO (0x0008); // hits lost group 1 FIFO overflow
  static const uint32_t HITLOST_1_L1   (0x0010); // hits lost group 1 L1 overflow
  static const uint32_t HITERROR_1     (0x0020); // hit error in group 1.
  static const uint32_t HITLOST_2_FIFO (0x0040); // hits lost group 2 FIFO overflow
  static const uint32_t HITLOST_2_L1   (0x0080); // hits lost group 2 L1 overflow
  static const uint32_t HITERROR_2     (0x0100); // hit error in group 2.
  static const uint32_t HITLOST_3_FIFO (0x0200); // hits lost group 3 FIFO overflow
  static const uint32_t HITLOST_3_L1   (0x0400); // hits lost group 3 L1 overflow
  static const uint32_t HITERROR_3     (0x0800); // hit error in group 3.
  static const uint32_t HITS_EXCEEDED  (0x1000); // Hits lost, size limit exceeded.
  static const uint32_t EVENTLOST_FIFO (0x2000); // Event lost due to trigger fifo overflow.
  static const uint32_t FATALCHIP_ERROR(0x4000); // fatal chip error detected.
  static const uint32_t ERROR_MASK     (0x7fff); // All bits.

  // Extended trigger time:

  static const uint32_t TRIGGERTIME_MASK (0x07ffffff);

  // Global trailer:

  static const uint32_t TRIGGERLOST_MASK (0x04000000);
  static const uint32_t TDCERROR_MASK    (0x02000000);
  static const uint32_t OVERFLOW_MASK    (0x01000000);
  static const uint32_t WORDCOUNT_MASK   (0x001fffe0);
  static const uint32_t  WORDCOUNT_RSHIFT (5);

  /*!
	Determines if the parameter is a global event header.

	\param Data	(uint32_t	In)
	The data word to test.

	\return bool
	\retval true - if  word is a global event header
	\retval false - if word is not a global event header.

   */
  static inline bool 
  isGlobalHeader(uint32_t Data) {
    return ((Data & TYPE_MASK) == GLOBAL_HEADER);
  }
  /*!
    Returns true if the longword passed in is a global trailer.  
    Global trailers mark the end of an event taken when the 
    TDC is in trigger matching mode.

    \param Datum (uint32_t	In)
    The datum to analyze.

    \return Bool
    \retval true if datum is a global trailer longword.
    \retval False if datum is not a global trailer longword.

   */
  static inline bool
  isGlobalTrailer(uint32_t datum)
  {
    return ((datum & TYPE_MASK) == GLOBAL_TRAILER);
  }
  /*!
	Extracts the trigger number from a global header word.  The trigger 
	number is the same as the event number.
	\param Header	(uint32_t	In)
	Global header from which to extract the trigger number.

	\return Unsigned The value of the global header's trigger number.
        \throw std::string - If the longword passed in is not a global header 
	word a string indicating this fact is thrown.

  */
  static inline int
  TriggerNumber(uint32_t Header) 
  {
    if(!isGlobalHeader(Header)) {
      throw std::string("CCAENV1x90Data::TriggerNumber - given non global header");
    }
    else {
      return ((Header & EVENTCOUNT_MASK) >> EVENT_RSHIFT);
    }
  }
  /*!
	Extract the board number from a global header or 
	trailer word.  The board number is also known as the 
	GEO field.
	\param Header	(uint32_t	In)
	Global header or trailer from which to extract 
	the GEO field.

	\return unsigned The GEO field of header
	\throw std::string - if the longword passed in is not a 
	global header word, a string indicating this fact is 
	thrown.

   */
  static inline uint32_t
  BoardNumber(uint32_t Header) 
  {
    if((!isGlobalHeader(Header)) && (!isGlobalTrailer(Header))) {
      throw std::string("CCAENV1x90Data::BoardNumber - given non global header");
    }
    else {
      return (Header & GEO_MASK);
    }
  }

  /*!
    Determines if the longword passed in is a TDC header.  
    The TDC header words must be explicitly enabled in the 
    TDC, otherwise they will not appear in the event stream.

    \param Header	(uint32_t	In)	
    The longword to check.

    \return bool
    \retval	true If the longword Header is a TDC Chip 
        header word.
    \retval False If the longword Header is not a TDC Chip 
        header word.
  */
  static inline bool
  isTDCHeader(uint32_t Header) 
  {
    return ((Header & TYPE_MASK) == TDC_HEADER);
  }

  /*!
     Determines if the longword passed in is a TDC header.  
    The TDC header words must be explicitly enabled in the 
    TDC, otherwise they will not appear in the event stream.

    \param Header	(uint32_t	In)	
    The longword to check.

    \return bool
    \retval	true If the longword Header is a TDC Chip 
        header word.
    \retval False If the longword Header is not a TDC Chip 
        header word.   
   */
  static inline bool
  isTDCTrailer(uint32_t header)
  {
    return ((header & TYPE_MASK) == TDC_TRAILER);
  }
  /*!
    Determine if a longword is a TDC error reporting word.  
    Note that you must explicitly enable the insertion of TDC
    error reporting longwords in the data stream at setup time.
    \param Datum	(uint32_t	In)	
    The data longword to analyze

    \return Bool
    \retval True if datum is a TDC error flag word.
    \retval False if dataum is not a TDC error flag word.
    
   */
  static inline bool
  isTDCError(uint32_t Datum)
  {
    return ((Datum & TYPE_MASK) == TDC_ERROR);
  }
 
  /*!
	Returns the chip number of a TDC within the module from a
	word that is a TDC header or trailer word.
	\param Header	(uint32_t	In)
	The header longword from which the chip number will be 
	extracted.

	\return unsigned -  the chip number within the board of 
	the TDCchip from a TDC header or trailer word.
	\throw std::string - Message is thrown if the input Header is 
	  not a TDC chip header or trailer longword.
  */
  static inline uint32_t
  TDCChip(uint32_t Header) 
  {
    if(! isTDCHeader(Header) && 
       !isTDCTrailer(Header) &&
       !isTDCError(Header)) {
      throw std::string("CCAENV1x90Data::TDCChip - not data with chip no.");
    } else {
      return ((Header & TDC_MASK) >> TDC_RSHIFT);
    }
  }

  /*!
	Returns the Event id from a TDC header or trailer.  I'm 
	not sure if this value will be the same as the event id in 
	the global header.
	\param Header	(uint32_t	In)
	The header longword to extract the event id from.

	\return unsigned - the event id
	\throw	String - a message is thrown in the event that 
	this header does not represent a TDC Header or trailer.
  */
  static inline uint32_t
  EventId(uint32_t Header) 
  {
    if (!isTDCHeader(Header)  && !isTDCTrailer(Header)) {
      throw std::string("CAENV1x90Data::EventId - not a tdc header");
    } 
    else {
      return ((Header & EVENTID_MASK) >> EVENTID_RSHIFT);
    }
  }
  /*!

	Returns the bunch count field from a TDC header or
	trailer.  The bunch id is a counter that is incremented 
	for each hit I think it is reset via the front panel reset 
	or software resets.  Perhaps it represents the event 
	counter within a bunch on an accelerator system that is 
	not CW.
	\param Header	(uint32_t	In)
	The TDC header longword.

	\return	The bunch count extracted from the TDC Header.
	\throw	string - if the Header longword does not 
	represent a TDC header

   */
  static inline uint32_t
  BunchId(uint32_t Header) 
  {
    if(!isTDCHeader(Header)) {
      throw std::string("CCAENV1x90Data::Bundhid - not a tdc header");
    }
    else {
      return (Header & BUNCHID_MASK); // Assumes right justified.
    }
  }

  /*!

	Returns the word count from a TDC trailer longword.
	\param Header	(uint32_t	In)
	The TDC trailer to analyze.

	\return unsigned uint16_t	The word count extracted from the 
	bunch count word.
	\throws	string if the header is not a TDC Trailer.

  */
  static inline uint16_t
  TDCWordCount(uint32_t Header) 
  {
    if(!isTDCTrailer(Header)) {
      throw std::string("CCAENV1x90Data::TDCWordCount not a tdc trailer");
    } 
    else {
      return (Header & TDCWORDCOUNT_MASK);
    }

  }
  /*!

  Returns true if the input longword represents a TDC measurement.
  \param data	(uint32_t	In)
  The data word to analyze.

  \return	Bool
  \retval True Data is a TDC Measurement datum.
  \retval False Data is not a TDC Measurement datum.
  \throw	None
    
   */
  static inline bool
  isMeasurement(uint32_t data)
  {
    return ((data & TYPE_MASK) == MEASUREMENT);
  }
  /*!

  Determine if a measurement is a trailing edge time. 

  \param Datum	(uint32_t	In)
  The data word to analyze.

  \return Bool
  \retval True - datum represents a trailing edge signal timing.
  \retval False - datum represents a non trailing edge signal 
  (trailing edge or width).
  \throw std::string if the datum does not represent a TDC measurement.

   */
  static inline bool
  isTrailing(uint32_t datum)  
  {
    if(!isMeasurement(datum)) {
      throw std::string("CCAENV1x90Data::isTrailing not a measurement word");
    } else {
      return ((datum & TRAILING_BIT) != 0);
    }
  }
  /*!

  Extracts the channel number from a tdc data word.

  \param Datum	(uint32_t	In)
  The datum to analyze.
  \param is1190	(bool	in)	
   - True if the data comes from an 1190. 
   -false otherwise.  
   The channel number is positioned slightly differently in the 
   1190 vs. the 1290. Defaults to true (NSCL needs).

   \return Unsigned the value of the contents of the dataum word.
   \throw std::string if datum is not a TDC measurement.
  */
  static inline int
  ChannelNumber(uint32_t Datum, bool is1190 = true) 
  {
    if(!isMeasurement(Datum)) {
      throw std::string("CCAENV1x90Data::ChannelNumber  is not a measurement");
    } 
    else   {
      uint32_t mask = V1190CHANNEL_MASK;
      uint32_t shift= V1190CHANNEL_RSHIFT;
      
      // If the module is actually a 1290, correct the
      // assumptions above.
      
      if(!is1190) {
	mask = V1290CHANNEL_MASK;
	shift= V1290CHANNEL_RSHIFT;
      }

      return ((Datum & mask) >> shift);
    }
  }
  /*!
    Returns the digitized value of a channel.
    \param Datum	(uint32_t	In)
	The data longword to analyze.
    \param is1190	(bool	in)
    - True if the data comes from an 1190. 
    - false otherwise.  
    The data value is positioned slightly differently in the 1190 
    vs. the 1290. 
    Defaults to true (NSCL needs).

    \return The uint32_t
    value of the conversion represented by this data 
    word.
    \throw std::string if datum is not a measuremenet.

   */
  static inline long
  ChannelValue(uint32_t datum, bool is1190=true) 
  {
    if (!isMeasurement(datum)) {
      throw std::string("CCAENV1sx90Data::ChannelValue - is not a measurement");
    } else {
      uint32_t mask = is1190 ? V1190DATA_MASK : 
	                            V1290DATA_MASK;
      return (datum & mask);
    }
  }
 /*!
    Extract the error bits from a TDC error reporting longword.

    \param Datum   (uint32_t  In)
    The data longword to analyze.

    \return Returns the error flag bit fields from TDCErrorBits. 
    Note that there are definitions in this class that describe 
    each of the bits in this bitfield.
    \throw std::string if datum is not a TDC error reporting longword.
    
   */
  static inline uint32_t
  TDCErrorBits(uint32_t datum) 
  {
    if(!isTDCError(datum)) {
      throw std::string("CCAENV1x90DATA:: not an error flag word");
    }
    else {
      return datum & ERROR_MASK;
    }
    
  }
  /*!
    Determine if the input longword is a trigger time tag word.  
    Trigger time tag words can be put in to denote the time at 
    which a trigger occurred (relative to reset?)  They must be 
    explicitly enabled and only have a meaning when the TDC is in 
    Trigger Matching mode.

    \param Datum       (uint32_t	In)
    The data word to analyze.

    \return Boolean
    \retval true if the input longword is a trigger time tag word.
    \retval False if the input longword is not a trigger time 
    tagword.

   */
  static  inline bool 
  isTriggerTimeTag(uint32_t datum)
  {
    return ((datum & TYPE_MASK) == TRIGGER_TIME);
  }
  /*!
    Return the extended trigger time field from a trigger tag 
    longword.

    \param Datum (uint32_t	In)
    The datum to anlyze.

    \return uint32_t 
    \retval The extended timestamp of the trigger tag longword.
    \throw std::string - if datum is not a trigger tag longword.

   */
  static inline uint32_t 
  ExtendedTriggerTime(uint32_t datum) 
  {
    if(!isTriggerTimeTag(datum)) {
      throw std::string("CCAENV1x90DATA::ExtendedTriggerTime not a trigger time word");
    } 
    else {
      return (datum & TRIGGERTIME_MASK);
    }
  }

  /*!
    Determines if a global trailer longword has the overflow bit 
    set.
    
    \param Datum	(uint32_t	In)
    A global trailer longword.

    \return Bool 
    \retval 	true if the global trailer longword has the 
                overflow bit set.
    \retval	False if not.
    \throw    String if datum is not a global trailer.

   */
  static inline bool
  Overflow(uint32_t datum) 
  {
    if(!isGlobalTrailer(datum)) {
      throw std::string("CCAENV1x90DATA::Overflow - not a global trailer");
    }
    else {
      return ((datum & OVERFLOW_MASK) == OVERFLOW_MASK);
    }
  }

  /*!
    Determines if a global trailer longword has the error bit set.
  This would indicate a global error in one of the TDC chips that
  must be further diagnosed.  My guess is that if insertion of 
  error flag longs is enabled, this bit is only set when there is 
  at least one of these in the event.

  \param dataum (uint32_t	In)
  The global trailer longword.

  \return Bool
  \retval 	true if there was an error this last event
  \retval 	false if there were no errors this last event.
  \throw   String if the datum parameter is not a global trailer 
  longword.
  */
  static inline bool 
  Error(uint32_t datum) 
  {
    if(!isGlobalTrailer(datum)) {
      throw std::string("CCAENV1x90DATA::Error - not a global trailer");
    } 
    else {
      return ((datum & TDCERROR_MASK) == TDCERROR_MASK);
    }
  }
  /*!
    Determines if there were triggers that did not get digitized 
    due to pileup.

    \param Datum	(uint32_t	In)
	The Global trailer longword to analyze.

    \return Bool
    \retval true if the TriggersLost bit was set in datum
    \retval false if the Triggerslost bit is not set in datum.
    \throw std::string - if datum is not a global trailer longword.

   */
  static inline bool 
  Lost(uint32_t datum) 
  {
    if(!isGlobalTrailer(datum)) {
      throw std::string("CAENV1x90DATA::Lost - not a trailer long");
    } 
    else {
      return ((datum & TRIGGERLOST_MASK) & TRIGGERLOST_MASK);
    }
  }
  /*!
    Extracts the size of an event from a global trailer longword

    \param Datum	(uint32_t	In)
	A global trailer longword being analyzed

    \return uint32_t 
    \retval The number of words in the events for which this is a trailer.
    \throw  String if datum is not a global trailer longword.

   */
  static inline uint32_t
  EventSize(uint32_t datum)
  {
    if(!isGlobalTrailer(datum)) {
      throw std::string("CAENV1x90DATA::EventSize - not a trailer long");
    }
    else {
      return ((datum & WORDCOUNT_MASK) >> WORDCOUNT_RSHIFT);
    }
  }
  /*!
    Determines if a longword is a filler word. Filler words are
    returned if the device is read when it has no data and bus
    errors are disabled.  This is how the device should be run 
    as the bus interfaces don't do a good job of reporting bus 
    errors to the code in a synchronous manner.

    \param Datum	(uint32_t	In)
    The longword to analyze.

    \return Bool
    \retval true if datum is a filler.
    \retval False if datum is not a filler.

   */
  static inline bool isFiller(uint32_t datum)
  {
    return ((datum & TYPE_MASK) == FILLER_LONG);
  }
  /*!
    
   */
}



#endif
