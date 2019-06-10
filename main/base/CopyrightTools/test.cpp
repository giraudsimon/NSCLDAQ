#include <config.h>
#include "CopyrightNotice.h"
#include <iostream>

using namespace std;

int
main(int argc, char** argv)
{
  // Should produce a coypright notice..

  CopyrightNotice::Notice(cout, argv[0],
			  "1.0", "2004");
  // Should produce an author credit:

  CopyrightNotice::AuthorCredit(cout, argv[0],
				"Ron Fox", 
				"Jay Kusler", NULL);
}
