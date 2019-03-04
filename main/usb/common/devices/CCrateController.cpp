
#include "CCrateController.h"


int CCrateController::write16(const CNAF& cnaf, uint16_t data, uint16_t& qx)
{
    int retval = simpleWrite16(cnaf.n(), cnaf.a(), cnaf.f(), data, qx);
    return retval;
}

int CCrateController::write24(const CNAF& cnaf, uint32_t data, uint16_t& qx)
{
    int retval = simpleWrite24(cnaf.n(), cnaf.a(), cnaf.f(), data, qx);
    return retval;
}

int CCrateController::read16(const CNAF& cnaf, uint16_t& data, uint16_t& qx)
{
    int retval = simpleRead16(cnaf.n(), cnaf.a(), cnaf.f(), data, qx);
    return retval;
}

int CCrateController::read24(const CNAF& cnaf, uint32_t& data, uint16_t& qx)
{
    int retval = simpleRead24(cnaf.n(), cnaf.a(), cnaf.f(), data, qx);
    return retval;
}

int CCrateController::control(const CNAF& cnaf, uint16_t& qx)
{
    int retval = simpleControl(cnaf.n(), cnaf.a(), cnaf.f(), qx);
    return retval;
}
