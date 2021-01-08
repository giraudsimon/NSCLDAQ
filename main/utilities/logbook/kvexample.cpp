#include <LogBook.h>
#include <iostream>
#include <stdlib.h>
#include <stdexcept>

int main(int argc, char** argv)
{
  if (argc  != 3 ) {
    std::cerr << "kvexmple logbookdatabase key\n";
    exit(EXIT_FAILURE);
  }
  try {
    const char* key = argv[2];
    LogBook book(argv[1]);
    if (book.kvExists(key)) {
      std::cout << key << " : " <<  book.kvGet(key) << std::endl;
    } else {
      std::cerr << key << " does not exist\n";
      exit (EXIT_FAILURE);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
