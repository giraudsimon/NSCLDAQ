
#ifndef CAMACSLOTLIMITS_h
#define CAMACSLOTLIMITS_h

#include "XXUSBConfigurableObject.h"

static XXUSB::CConfigurableObject::limit minslot(1);
static XXUSB::CConfigurableObject::limit maxslot(23);
static XXUSB::CConfigurableObject::Limits SlotLimits(minslot,maxslot);

#endif
