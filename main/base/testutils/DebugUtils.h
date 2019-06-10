

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace Test
{

// Utility function to print two vectors side by side
template<class T>
void print_vectors(const std::vector<T>& expected, const std::vector<T>& actual) 
{

  using namespace std;


  int columnWidth = sizeof(T)*2+2;

  // create header
  cout << setw(columnWidth) << "Expected"; 
  cout << " ";
  cout << setw(columnWidth) << "Actual" << endl;
  cout << setfill('-') << setw(2*columnWidth+1) << '-' << setfill(' ') << endl;

  // figure out the max size
  size_t sizeExpected = expected.size();
  size_t sizeActual   = actual.size();
  size_t nIters = std::max(sizeExpected, sizeActual);

  for (size_t i=0; i<nIters; ++i) {

    cout << setw(3) << i << " : ";

    // we are interested in hex output and also it showing the x before it
    auto oldFlags = cout.flags( ios::hex | ios::showbase );

    // output either a value or spaces
    if (i < sizeExpected) {
      auto oldFill  = cout.fill('0');
      cout << setw(columnWidth) << static_cast<unsigned int>(expected.at(i));
      cout.fill(oldFill);
    } else {
      cout << setw(columnWidth) << ' ';
    }

    // separate the columns
    cout << " ";

    // output either a value or spaces
    if (i < sizeActual) {
      auto oldFill  = cout.fill('0');
      cout << setw(columnWidth) << static_cast<unsigned int>(actual.at(i));
      cout.fill(oldFill);
    } else {
      cout << setw(columnWidth) << '0';
    }

    // revert the formatting of cout to what it was
    cout.flags(oldFlags);

    cout << endl;
  } // end loop

}

}

//extern std::ostream& operator<<(std::ostream& stream, std::uint8_t value)
//{
//  stream << int(value);
//  return stream;
//}

template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec)
{
  stream << "{ ";
  auto size = vec.size();
  for (size_t i=0; i<size; ++i) {
      stream << vec.at(i);
      if ( i != (size-1) ) {
        stream << ", ";
      }
  }
  stream << "}" << std::flush;
  return stream;
}



#endif
