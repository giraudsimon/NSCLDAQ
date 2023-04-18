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

/** @file:  tinyxmlutil.cpp
 *  @brief: Implement useful tinyxml2 utility methods.
 */
 
 
 #include <tinyxmlutil.h>
 #include <sstream>
 #include <stdexcept>
 
 
/**
 *  Various overloads for getAttribute that call the appropriate
 *   QueryxxxAttribute.
 * @param result -- where to put the reulst, this type varies.
 * @param container  - the container to query.
 * @param att    - attribute name (e.g. "value").
 */
void
getAttribute(              // unit32_t
    uint32_t& result, tinyxml2::XMLElement& container, const char* att
)
{
    unsigned i;
    checkXmlError(
        container.QueryUnsignedAttribute(att, &i), *container.GetDocument()
    );
    result = i;                // in case uint32_t isn't unsigned.
}
void
getAttribute(                        // double
    double& result, tinyxml2::XMLElement& container, const char* att
)
{
    checkXmlError(
        container.QueryDoubleAttribute(att, &result),
        *container.GetDocument()
    );
}
void
getAttribute(                      // bool.
    bool& result, tinyxml2::XMLElement& container, const char* att 
)
{
    checkXmlError(
        container.QueryBoolAttribute(att, &result), *container.GetDocument()
    );
}
/**
 * haveChild
 *    Checks for and returns a required child element
 * @param parent - the parent that's required to have the child.
 * @param element - the name of the element.
 * @return tinyxml2::XMLElement* - pointer to the element.
 * @throw std::invalid_argument if the elmeent does not exist.
 */
tinyxml2::XMLElement*
haveChild(tinyxml2::XMLElement& parent, const char* element)
{
    tinyxml2::XMLElement* result =
        parent.FirstChildElement(element);
    if (result == nullptr) {
        std::stringstream msg;
        msg << "The " << parent.Name() << " tag is required to enclose a "
            << element << " tag but I can't find it";
        throw std::invalid_argument(msg.str());
    } else {
      return result;
    }
}
/**
 * checkXmlError
 *    IF the XML status passed in is not normal,
 *    throw an std::invalid_argument with as detailed a message
 *    as possible.
 *
 * @param err - XMLError which, hopefully is success.
 * @param doc - Reference to the document.
 */
void
checkXmlError(
    tinyxml2::XMLError err, tinyxml2::XMLDocument& doc
)
{
    if (err != tinyxml2::XML_SUCCESS) {
        // an error  fi we don't have a node, we just can get the string
        // from the public but undocumented ErrorIDToName

        std::string msg;
        msg = doc.ErrorStr();
        throw std::invalid_argument(msg.c_str());
    }
    
}
