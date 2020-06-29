#ifndef COPYRIGHTNOTICE_H
#define COPYRIGHTNOTICE_H

/// Requires include of config.h by parent.

#include <ostream>


/*!
  Generate simple copyright and authorship notices.
  Copyright notices are intended for interactive output.
  Authorship notices are intended to acknowledge and ego boost.
 */  
class CopyrightNotice
{
public:
  static void  Notice(std::ostream& out,  const char* program, 
		      const char* version,  const char* year);
  static void  AuthorCredit(std::ostream& out,const  char* program, ...);

}; 

#endif
