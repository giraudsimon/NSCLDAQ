#include "CBufferIO.h"
#include "CBuffer.h"
#include <CDataSource.h>
#include <CDataSink.h>
#include <iostream>

std::istream& operator>>(std::istream& str, CBuffer& buf) 
{
	str.read(buf.begin(),buf.size());
	return str;
}

std::ostream& operator<<(std::ostream& str, const CBuffer& buf) 
{
	str.write(buf.begin(),buf.size());
	return str;
}

CDataSource& operator>>(CDataSource& str, CBuffer& buf) 
{
	str.read(buf.begin(), buf.size());
	return str;
}

CDataSink& operator<<(CDataSink& str, const CBuffer& buf)
{
	str.put(buf.begin(), buf.size());
	return str;
}
