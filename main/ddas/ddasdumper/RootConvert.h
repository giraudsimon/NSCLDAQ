/*!
  Root converter SNL 01/01/2012
  Conversion of NSCL daq evt data files from Pixie16 modules
  to ROOT format

 */
//root converter

#ifndef ROOTCONVERTER_H
#define ROOTCONVERTER_H

#include <string>

#ifndef CONVERTER_H
#include "Converter.h"
#endif

class ddaschannel;
class CRingItem;

/// \class RootConverter
/** \brief Converter for data formats < NSCLDAQ 10.2
 *
 * A concrete implementation of the Converter base class to 
 * support the conversion of data built with NSCLDAQ versions 
 * prior to 10.2. This class expects a simple physics data 
 * structure defined as follows:
 *
 *  struct data_structure {
 *      uint32_t size_in_bytes
 *      uint32_t* pointer to Pixie16 data format
 *  }
 *
 * The Pixie-16 data format is defined by the Pixie-16 digitizer
 * module. It consists of a minimum of 4 32-bit words for the 
 * Pixie16 data header followed by any subsequent optional data.
 * The optional data may include trace data, energy sum data, or QDC
 * data. 
 *
 * The RootConverter object creates a branch in the tree referenced
 * by Converter::m_treepointer named dchan. It holds the ddaschannel objects
 * produced by every call to the RootConverter::DumpData(const CRingItem&) 
 * method.
 * 
*/
class RootConverter : public Converter 
{

 private:
  ddaschannel *dchan;  /**< point to current object (channel information from pixie16) */

 public:
  /** \brief Default constructor
   */
  RootConverter();

  /** \brief Default destructor
   * 
   * Constructs the ddaschannel object pointed to by this class's dchan pointer with its
   * default constructor.
   *
   */
  virtual ~RootConverter();

  /** \brief Adds dchan branch to tree and associates it with the local ddaschannel object
    * 
    * First calls the Converter::Initialize(std::string,std::string) method to create
    * the output file and the tree, then adds the dchan branch to the tree. This branch
    * holds the ddaschannel objects produced by the DumpData(const CRingItem&) method.
    *
    * @param filein currently not implemented
    * @param fileout name of the root file to store the outputted data tree 
   */
  virtual void Initialize(std::string filein, std::string fileout);

  /** \brief Conversion of CRingItem data and filling of the tree
    *
    * Responsible for converting the data stored in the CRingItem into the ddaschannel
    * object. The majority of the conversion is actually performed by the ddaschannel::UnpackData
    * method, so that this merely strips away the nscldaq information that encloses the 
    * Pixie-16 data format (which is what is understood by the ddaschannel::UnpackData method). 
    * In this implementation, only the ring item size is stripped away before delegating the 
    * remainder of the unpacking task to the ddaschannel object. Following the unpacking,
    * the ddaschannel object holds the data it extracted and this same object is used to fill
    * the tree.
   */
  virtual void DumpData(const CPhysicsEventItem& item);

};

#endif
