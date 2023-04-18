/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  tinyxmlutil.h
 *  @brief: Provide common utilities for working with Tinyxml2
 */
 
 #ifndef TINYXMLUTIL_H
 #define TINYXMLUTIL_H
 #include <tinyxml2.h>
 
 
 template <class T>
 void readValue(T& result, tinyxml2::XMLElement& container);
 void getAttribute(
     uint32_t& result, tinyxml2::XMLElement& container,
     const char* att
 );
 void getAttribute(
     double&   result, tinyxml2::XMLElement& container, const char* att
 );
 void getAttribute(
     bool&     result, tinyxml2::XMLElement& container, const char* att
 );
 
 tinyxml2::XMLElement*
 haveChild(tinyxml2::XMLElement& parent, const char* element);


void
checkXmlError(
     tinyxml2::XMLError err, tinyxml2::XMLDocument& doc
 );

// Template implementations must be accessible at compile time.
/**
 * readValue
 *   Gets the contents of a "value" attribute in an element/tag.
 * @param dest  - templated type reference to where this is stored.
 * @param container - Element that contains the "value" attribute.
 */
template <class T>
void
readValue(
    T& result, tinyxml2::XMLElement& container
)
{
    getAttribute(result, container, "value");
}

 #endif