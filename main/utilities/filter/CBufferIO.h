
#ifndef CBUFFERIO_H
#define CBUFFERIO_H

class CBuffer;
class CDataSource;
class CDataSink;

#include <iosfwd>


extern std::istream& operator>>(std::istream& str, CBuffer& buf);
extern std::ostream& operator<<(std::ostream& str, const CBuffer& buf);

extern CDataSource& operator>>(CDataSource& str, CBuffer& buf);
extern CDataSink&   operator<<(CDataSink& str, const CBuffer& buf);

#endif
