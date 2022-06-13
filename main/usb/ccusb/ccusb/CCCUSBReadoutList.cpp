/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CCCUSBReadoutList.h"
#include <string>
#include <stdio.h>

using namespace std;

/*
  The following constant definitions are bits in the mode word that can follow
  the NAF word in the stack:
*/
static const uint16_t MODE_HITDATA    =  0x0001;
static const uint16_t MODE_SUPRESSS2  =  0x0002;
static const uint16_t MODE_NUMBERDATA =  0x0004;
static const uint16_t MODE_HITMODE    =  0x0008;
static const uint16_t MODE_QSTOP      =  0x0010;
static const uint16_t MODE_QSCAN      =  0x0020;
static const uint16_t MODE_COUNT      =  0x0040;
static const uint16_t MODE_LAMWAIT    =  0x0080;
static const uint16_t MODE_FASTCAMAC  =  0x0100;
static const uint16_t MODE_ADDRPTN    =  0x0200;
static const uint16_t MODE_NTMASK     =  0x3000; // Mask of the NT bits.
static const uint16_t MODE_NTSHIFT    =      12; // Shift count to/from the NT bits.
static const uint16_t CONTINUATION    =  0x8000; // More follows in the stack.

// Bits in the NAF word:

static const uint16_t NAFIsLong(0x4000);


/*********************************************************************************/
/*!
   Copy construction: we need to explicitly copy construct the list or else
   the compiler bit by bit copy will cause weird things to happen

   \param rhs - The object that is being used as a template to construct us.

*/
CCCUSBReadoutList::CCCUSBReadoutList(const CCCUSBReadoutList& rhs) :
  m_list(rhs.m_list)
{
}

/**********************************************************************************/
/*!
   Assignment operator.: We need to explicitly assign the list else the compiler
   bit by bit copy will cause weird things to happen. I'm going to assume that
   std::vector is sane in the presence of self-assignment.

   \param rhs - the object we are assigning to ourself.
   \return CCCUSBReadoutList&
   \retval *this
*/
CCCUSBReadoutList&
CCCUSBReadoutList::operator=(const CCCUSBReadoutList& rhs)
{
  m_list = rhs.m_list;
  return *this;
}

/*********************************************************************************/
/*!
  Return the current contents of the readout list.  This is a copy 
  so you could in theory mash it about any way you want.

  \return std::vector<uint16_t>
  \retval contents of this readout list so far.

*/

vector<uint16_t>
CCCUSBReadoutList::get() const
{
  return m_list;
}
/********************************************************************************/
/*!
   Return the number of uint16_t elements in the list.  This is not the same
   as returning the number of list items as those are of varying size.
   Normally this is used by CCCUSB in order to know how to set the list sizes
   when downloading or executing a list.  

   You can also use this to determine if your lists are getting too large
   for the list storage in the CCUSB which is:
   - 768 16 bit words of Event readout list.
   - 256 words of Scaler readout list.

   \return size_t
   \retval Number of 16 bit entries in the list.
*/
size_t
CCCUSBReadoutList::size() const
{
  return m_list.size();
}

/*******************************************************************************/
/*!
   Clear the list. When done, the list will have no elements, that is,
   size() will return 0, and get() will return an empty vector.
*/
void
CCCUSBReadoutList::clear()
{
  m_list.clear();
}


void 
CCCUSBReadoutList::append(const CCCUSBReadoutList& list)
{
  const std::vector<uint16_t>& other = list.get();
  m_list.insert(m_list.end(), other.begin(), other.end());
}

/******************************************************************************/
/*!
   Add a 16 bit write to the list.
   \param n     Slot to which the operation is directed.
   \param a     Subaddress to which the operation is directed.
   \param f     Function code to perform, must be in the range 16-23, see exceptions.
   \param data  The data to write
   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid write function code.

*/
void 
CCCUSBReadoutList::addWrite16(int n, int a, int f, uint16_t data)
{
  string msg;
  if (!validWrite(n,a,f, msg)) {
    throw msg;
  }
 
  m_list.push_back(NAF(n,a,f));

  // My understanding of the stack is that regardless of the width of the write
  // I need to supply 32 bits of data presumably in little endian form.

  m_list.push_back(data);

}

/*******************************************************************************/
/*!
   Add a 32 bit write to the list.
   \param n     Slot to which the operation is directed.
   \param a     Subaddress to which the operation is directed.
   \param f     Function code to perform, must be in the range 16-23, see exceptions.
   \param data  The data to write
   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid write function code.

*/
void 
CCCUSBReadoutList::addWrite24(int n, int a, int f, uint32_t data)
{
  string msg;
  if (!validWrite(n,a,f, msg)) {
    throw msg;
  }

  m_list.push_back(NAF(n,a,f, true));
  
  // I believe the data are pushed in low endian order

  uint16_t low = data & 0xffff;
  uint16_t hi  = (data >> 16) & 0xffff;	// should not really need the and but...
  m_list.push_back(low);
  m_list.push_back(hi);
  
}
/****************************************************************************/
/*!
   Add a 16 bit read to the stack.
   \param n     Slot to which the operation is directed.
   \param a     Subaddress to which the operation is directed.
   \param f     Function code to perform, must be in the range 16-23, see exceptions.
   \param lamWait  - if true, the CCUSB is asked to wait for a LAM from the
                     module before doing the read.
   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid read function code.
*/
void
CCCUSBReadoutList::addRead16(int n, int a, int f, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }
  uint16_t naf = NAF(n,a,f);
  if (lamWait) {
    naf |= CONTINUATION;
    m_list.push_back(naf);
    m_list.push_back(MODE_LAMWAIT);
  }
  else {
    m_list.push_back(naf);
  }
}
/****************************************************************************/
/*!
  Add a 24 bit read to the stack.  This is identical to the addRead16 except
  the Long bit is set.
   \param n     Slot to which the operation is directed.
   \param a     Subaddress to which the operation is directed.
   \param f     Function code to perform, must be in the range 16-23, see exceptions.
   \param lamWait - true if the CCUSB will be asked to wait for lam
                    prior to the read.
   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid read function code.
*/
void
CCCUSBReadoutList::addRead24(int n, int a, int f, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }
  uint16_t naf = NAF(n,a,f, true);
  if (lamWait) {
    naf |= CONTINUATION;
    m_list.push_back(naf);
    m_list.push_back(MODE_LAMWAIT);
  }
  else {
    m_list.push_back(naf);
  }
}
/****************************************************************************/
/*!
   Add a control transfer to the stack.
   \param n     Slot to which the operation is directed.
   \param a     Subaddress to which the operation is directed.
   \param f     Function code to perform, must be in the range 16-23, see exceptions.
   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid control function code.
*/
void
CCCUSBReadoutList::addControl(int n, int a, int f)
{
  string msg;
  if (!validControl(n,a,f, msg)) {
    throw msg;
  }
  m_list.push_back(NAF(n,a,f));

}
/****************************************************************************/
/*!
   Add a Q-stop operation to the stack.  For now we only support 16 bit read
   Q-stop operations. Q-Stop operations repeat the same FNA until either
   the dataway Q disappears or the maximum repeat count is exhausted.
   \param n    - Slot to which the operation is directed.
   \param a    - Subaddress for the read.
   \param f    - Function code for the read.
   \param max  - Maximum number of transfers.
   \param lamWait - True if shoud wait for a lam.

   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid control function code.
*/
void
CCCUSBReadoutList::addQStop(int n, int a, int f, uint16_t max, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }

  // build up the elements of the stack (three words, naf, mode, max-count).

  uint16_t naf  = NAF(n,a,f) | CONTINUATION;
  uint16_t mode = MODE_QSTOP | CONTINUATION;
  if (lamWait) mode |= MODE_LAMWAIT;
  
  m_list.push_back(naf);
  m_list.push_back(mode);
  m_list.push_back(max);

}

/*!
  Same as above but the readout is for 24 bit data.
*/
void
CCCUSBReadoutList::addQStop24(int n, int a, int f, uint16_t max, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }

  // build up the elements of the stack (three words, naf, mode, max-count).

  uint16_t naf  = NAF(n,a,f) | CONTINUATION | NAFIsLong;
  uint16_t mode = MODE_QSTOP | CONTINUATION;
  if (lamWait) mode |= MODE_LAMWAIT;
  
  m_list.push_back(naf);
  m_list.push_back(mode);
  m_list.push_back(max);

}
/****************************************************************************/
/*!
   Add a Q-scan operation to the stack.  QScan operations repeat the same
   F code at a slot, incrementing the subaddress each time.  When Q vanishes,
   the slot is incremented and the subaddress set to zero.  The operation
   continues until a function has no X response or until the maximum repeat
   count is exhausted.  This is a way to sequentially read several modules
   in a crate by scanning their subaddresses.
  
   At this point, only 16 bit reads are supported for the f in Q-Scans.
   \param n    - Starting slot to which the operation is directed.
   \param a    - Starting Subaddress for the read.
   \param f    - Function code for the read.
   \param max  - Maximum number of transfers.
   \param lamWait = Wait for lam if true.

   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid control function code.
*/
void
CCCUSBReadoutList::addQScan(int n, int a, int f, uint16_t max, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }

  // build up and add the stack elements: NAF, Mode, Max count.

  uint16_t naf = NAF(n,a,f) | CONTINUATION;
  uint16_t mode= MODE_QSCAN | CONTINUATION;
  if (lamWait) mode |= MODE_LAMWAIT;
  
  m_list.push_back(naf);
  m_list.push_back(mode);
  m_list.push_back(max);

}

/*****************************************************************************/
/*!
  Add a repeat operation to the stack.  At present, only 16 bit camac reads
 can be repeated.  A repeat transaction simply does the requested transaction
 for a fixed repeat count.

   \param n    - Slot to which the operation is directed.
   \param a    - Subaddress for the read.
   \param f    - Function code for the read.
   \param count- Bumber of transfers
   \param lamWait - If true, wait for lam..

   \throws string in the event that any of the following are true:
   - n is not a valid slot (controller pseudo slots are valid slots).
   - a is not a valid subaddress.
   - f is not a valid control function code.
*/
void
CCCUSBReadoutList::addRepeat(int n, int a, int f, uint16_t count, bool lamWait)
{
  string msg;
  if (!validRead(n,a,f,msg)) {
    throw msg;
  }

  // Three elements, NAF, mode,and count get added to the stack:

  uint16_t naf = NAF(n,a,f) | CONTINUATION;
  uint16_t mode= MODE_COUNT | CONTINUATION;
  if (lamWait) mode |= MODE_LAMWAIT;
  
  m_list.push_back(naf);
  m_list.push_back(mode);
  m_list.push_back(count);

}


/*****************************************************************************/
/*!
 * Adds an address pattern read to the stack. The data read by the NAF provided
 * as an argument to this method is interpreted as a list of A values to use
 * when calling the NAF of the subsequent command in the stack. For each nonzero
 * bit in the data returned, the subsequent command's N and F values are used to
 * form and NAF command. This is a setup command for the next NAF operation that
 * is added to the stack. 
 *
 * \param n   - Slot to which the operation is directed
 * \param a   - Subaddress for the read
 * \param f   - Function code for the read
 * \param lamWait - If true, wait for lam
 *
 * \throws string in the event that any of the following are true:
 *  - n is not a valid slot (controller pseudo slots are valid slots).
 *  - a is not a valid subaddress.
 *  - f is not a valid control function code.
 */
void
CCCUSBReadoutList::addAddressPatternRead16(int n, int a, int f, bool lamWait)
{
  uint16_t naf = NAF(n,a,f) | CONTINUATION;
  uint16_t mode = MODE_ADDRPTN;
  if (lamWait) {
    mode |= MODE_LAMWAIT;
  }

  m_list.push_back(naf);
  m_list.push_back(mode);
}
//
///*****************************************************************************/
///*!
// * This is the same function as the former except that it uses a 
// *
// * \param n   - Slot to which the operation is directed
// * \param a   - Subaddress for the read
// * \param f   - Function code for the read
// * \param lamWait - If true, wait for lam
// *
// * \throws string in the event that any of the following are true:
// *  - n is not a valid slot (controller pseudo slots are valid slots).
// *  - a is not a valid subaddress.
// *  - f is not a valid control function code.
// */
//void
//CCCUSBReadoutList::addAddressPatternRead24(int n, int a, int f, bool lamWait)
//{
//  uint16_t naf = NAF(n,a,f,1) | CONTINUATION;
//  uint16_t mode = MODE_ADDRPTN;
//  if (lamWait) {
//    mode |= MODE_LAMWAIT;
//  }
//
//  m_list.push_back(naf);
//  m_list.push_back(mode);
//}

/**********************************************************************
/*!
    Add a put marker to the stack.  This is just the same as doing
    a 16 bit write to N =0 any subaddress (we'll use zero)
    \param value  - 16 bit value to add to the event.
*/
void
CCCUSBReadoutList::addMarker(uint16_t value)
{
  m_list.push_back(NAF(0,0,16));
  m_list.push_back(value);
}

/****************************************************************************/
/*                      Private utility functions.                          */
/****************************************************************************/

// Format a message with an integer value embedded in it.
// the position of the value is defined/determined by a valid
// printf format specification for an integer (e.g. %d).
// The final string cannot be longer than 1K-1 bytes or else
// it will be silently truncated.
//
string
CCCUSBReadoutList::messageWithValue(const char* format, int value)
{
  char message[1024];		// hence the limit.
  snprintf(message, sizeof(message), format, value);

  return string(message);
}

// Utility to construt a NAF:
//  n,a,f, are assumed to be legal.

uint16_t
CCCUSBReadoutList::NAF(int n, int a, int f, bool isLong)
{
  uint16_t result = n << 9 | a << 5 | f;
  if (isLong) result |= NAFIsLong;

  return result;
}

// This returns true if the slot is a valis slot.  We also recognize
// the special slots that are actually controller slots.  Thus
// we check the slot is in the range: [0, 30) (slot 0 write's write markers).
//
bool
CCCUSBReadoutList::validSlot(int n)
{
  return ((n >= 0) && (n < 30));

}

// returns true if the subaddress provided is a valid subadrress.
// This is the case if the subaddress is in the range [0,16).
//
bool
CCCUSBReadoutList::validSubaddress(int a)
{
  return ((a >= 0) && (a < 16));
}

// returns true if the slot and subaddress are valid.  If not,
// msg will have a nice error message that can be thrown.

bool
CCCUSBReadoutList::validSlotAndSubaddress(int n, int a, string& msg)
{

  if (!validSlot(n)) {
    msg = messageWithValue("Invalid slot: %d", n);
    return false;
  }
  if (!validSubaddress(a)) {
    msg = messageWithValue("Invalid subaddress %d", a);
    return false;
  }
  return true;
}
// Returns true if the n/a/f is a valid write operation.
// This includes validation fo the subaddress and slot.

bool
CCCUSBReadoutList::validWrite(int n, int a, int f, string& msg)
{
  msg = "Valid write operation";

  if (!validSlotAndSubaddress(n,a,msg)) {
    return false;
  }
  // Writes are function codes in the range [16-23].

  if ((f >= 16) && (f <= 23)) {
    return true;
  }
  msg = messageWithValue("Invalid write operation %d", f);
  return false;
}
// Returns true if the n/a/f is a valid read operation.
//
bool
CCCUSBReadoutList::validRead(int n, int a, int f, string& msg)
{
  msg = "Valid read operation";

  if (!validSlotAndSubaddress(n,a,msg)) {
    return false;
  }

  // Valid reads are in the range [0 7]

  if ((f >= 0) && (f <= 7)) {
    return true;
  }
  msg = messageWithValue("Invalid read function %d", f);
  return false;

}
// Returns true if the n/a/f is a valid control operation.
// this includes validation of the subaddress and slot.
// if the return value is false, the msg parameter
// will have a message that could be thrown to the caller.
//
bool
CCCUSBReadoutList::validControl(int n, int a, int f, string& msg)
{
  msg = "Valid Control Operation"; // Catch incorrect throws...

  if(!validSlotAndSubaddress(n,a,msg)) {
    return false;
  }

  // There are two ranges of valid control function codes.
  // f8-15, f24-31:
  
  if ((f >= 8) && (f <= 15)) {
    return true;
  }
  if ((f >= 24) && (f <= 31)) {
    return true;
  }
  // Invalid function code.
  
  msg = messageWithValue("Invalid control operation %d", f);
  return false;

     
}

