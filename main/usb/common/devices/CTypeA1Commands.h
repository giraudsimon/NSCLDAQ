
#ifndef CTYPEA1DEF_H
#define CTYPEA1DEF_H

#include "CNAF.h"

namespace TypeA1Command
{
    const CNAF GenDatawayZ (28,8,26);
    const CNAF GenDatawayC (28,9,26);

    const CNAF ReadGL0 (30,0,0);
    const CNAF ReadGL1 (30,1,0);
    const CNAF ReadGL2 (30,2,0);
    const CNAF ReadGL3 (30,3,0);
    const CNAF ReadGL4 (30,4,0);
    const CNAF ReadGL5 (30,5,0);
    const CNAF ReadGL6 (30,6,0);
    const CNAF ReadGL7 (30,7,0);

    const CNAF LoadSNR       (30,8,16);
    const CNAF RemoveDatawayI    (30,9,24);
    const CNAF SetDatawayI   (30,9,26);
    const CNAF TestDatawayI  (30,9,27);

    const CNAF DisableBD           (30,10,24);
    const CNAF EnableBD            (30,10,26);
    const CNAF TestBDEnabled       (30,10,27);

    const CNAF TestDemandsPresent  (30,11,27);

}

#endif
