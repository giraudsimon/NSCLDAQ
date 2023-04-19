#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <Python.h>

#include "CPyHelper.h"

int main(int argc, char *argv[])
{
  CPyHelper pInstance;
  
  try {
    std::string filename(PREFIX"/ddas/qtscope/main.py");
    PyObject *obj = Py_BuildValue("s", filename.c_str());    
    FILE *file = _Py_fopen_obj(obj, "r+");
    if(file != NULL) {
      PyRun_SimpleFile(file, filename.c_str());
    }
  }
  catch (std::exception& e)
    {
      std::cout << e.what() << '\n';
    }

  return 0;
}
