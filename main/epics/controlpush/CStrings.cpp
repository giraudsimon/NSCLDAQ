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
#include <CStrings.h>
#include <string.h>

using namespace std;

/*!
   Escapes a set of 'forbidden' string characters in an input string
   producing an output string:

   \param in  (const char*)
      An input string that may have some forbidden characters in it that
      need to be escaped.
   \param charset  (const char*)
      The set of characters that need escaping.
   \param how  (const char*)
      A one or two character string.  
      - If a one character string, any character
      in in that is a member of charset is escaped in the output by prefixing
      it with the character in this string.
      - If a two character string, any character that is a member of charset
        is escaped by prefixing it with the first character and suffixing
      it with the second.
      - If 0 characters a string exception is thrown.
      - If > 2 characters, the additional characters are ignored.
*/
string 
CStrings::EscapeString(const char* in, 
		       const char* charset, 
		       const char* how) throw(string)
{
  string output;

  // The how string must be at least one character long:

  if(how[0] == '\0') {
    throw string("CStrings::EscapeString - quoting string 0 characters");
  }

  int    len = strlen(in);
  for(int i = 0; i < len; i++) {
    char c = in[i];
    if(strchr(charset, c)) {	// must escape..
      output += how[0];
      output  += c;
      if(how[1] != '\0') {	// If 2 characters, infix.
	output+= how[1];
      }
      
    } else {
      output   += c;
    }
  }
  return output;
}
  
