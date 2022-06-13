
#ifndef CCAMACCOMPAT_H
#define CCAMACCOMPAT_H

#include <CReadoutHardware.h>
#include <CReadoutModule.h>


/**! \brief A compatibility class for Camac hardware
*   
*   This class wraps an arbitrary object
*   in a ReadoutHardware object. This facilitates the ability to use the same
*   source code in both a VMUSBReadout and CCUSBReadout environment. This 
*   compatibility layer however differs from the VMUSBReadout version becuase
*   it is sensible for a call to Initialize or addReadoutList be forwarded
*   directly to the wrapped object. In the VMUSBReadout world, you cannot pass
*   a CVMUSB or CVMUSBREadoutList to the objectm, because it just does not make
*   sense. So these Initialize and addReadoutList methods do not throw.
*
*   This requires that the wrapped object of type T must define the following
*   methods:
*
*   T* T::clone() const
*   void T::onAttach(CreadoutModule&)
*
*   Memory management: This wrapper object owns its wrapped object.
*/
template <class T>
class CCamacCompat : public CReadoutHardware
{
    private:
        T* m_obj;    ///!< A pointer to the wrapped object

    public:
        /**! \brief The constructor
        * 
        *   This calls the clone operation of the object to be 
        *   wrapped. Compilation will fail if this method is not 
        *   privided.
        *
        *   \param obj the object whose clone will be wrapped  
        */ 
        CCamacCompat(const T& obj) 
         : m_obj(obj.clone())
        {} 

        /**! \brief Copy constructor
        *   
        *   Performs a deep copy of the original object. In other
        *   words this clones the original objects wrappped object.
        *
        *   \param rhs the object to copy
        */
        CCamacCompat(const CCamacCompat& rhs) 
           : m_obj(0) 
        {
            if (rhs.m_obj) {
                m_obj = rhs.m_obj->clone();
            }
        }


        /**! \brief Assignment operator
        *
        *   Performs a deep copy of the object. The previously 
        *   wrapped object is deleted. 
        *
        *   \param rhs the object whose state will be copied
        *
        *   \return a reference to this object
        */
        CCamacCompat& operator=(const CCamacCompat& rhs) 
        {
            if (this!=&rhs) {
                if (rhs.m_obj) {
                    T* tmp = rhs.m_obj->clone();
                    delete m_obj;
                    m_obj = tmp;
                }
            }
            return *this;
        }
 
        /**! \brief Destructor
        *
        *   Because this owns the wrapped object, it
        *   deletes the wrapped object.
        */
        ~CCamacCompat()
        {
            if (m_obj) { delete m_obj; m_obj=0;}
        }


        /**! \brief Access the wrapped object
        *   
        *   \return a pointer to the wrapped object
        */
        T* getWrappedObj() const
        {
            return m_obj;
        }

        /**! \brief onAttach relay
        *   
        *   This merely delegates the functionality to
        *   the wrapped object.
        *
        *   \param config the configuration associated with the wrapped object
        */
        void onAttach(CReadoutModule& config) 
        { 
            m_obj->onAttach(config);
        }

        /**! \brief Off limits Initialization
        *
        *   This ensures that no one actually tries to use this.
        *
        *   \throws unnamed exception
        */
        void Initialize(CCCUSB& controller) { m_obj->Initialize(controller);}

        /**! \brief Off limits addReadoutList 
        *
        *   This ensures that no one actually tries to use this.
        *
        *   \throws unnamed exception
        */
        void addReadoutList(CCCUSBReadoutList& list) { m_obj->addReadoutList(list);}
        

        /**! Polymorphic copy constructor
        *
        *   Returns a covariant type. This provides the concrete implementation
        *   of the CReadoutHardware* CReadoutHardware::clone() const method. The ownership
        *   of the returned object transfers to the caller.
        *
        *   \returns a dynamically allocated copy of the obect
        */
        CCamacCompat* clone() const {
            // clone the wrapped obj
            return new CCamacCompat(*m_obj);
        }
};

/**! \fn CCamacCompat<T*> compat_clone(const T&)
* \brief A convenience function for wrapping an object
*
*   \param the object whose clone will be wrapped
*    
*   \return a wrapped clone
*/
template<class T> CCamacCompat<T>* compat_clone( const T& obj) {
    return new CCamacCompat<T>(obj);
} 

#endif 
