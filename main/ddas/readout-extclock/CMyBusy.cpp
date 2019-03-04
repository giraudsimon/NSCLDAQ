#include <config.h>
#include <iostream>
#include "CMyBusy.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

CMyBusy::CMyBusy() 
{  
}

void CMyBusy::GoBusy() 
{
  //   cout << "Going busy "<< endl << flush;
}

void CMyBusy::GoClear() 
{  
  // cout << "going clear "<< endl << flush;
}

// void CMyStatus::ModuleClear() 
// {  
//   // cout << "clearing module "<< endl << flush;
// }

