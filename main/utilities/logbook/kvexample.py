import sys
from  LogBook import LogBook

if len(sys.argv) != 3 :
    print("kvexample.py logbook key")
    exit()

bookfile = sys.argv[1]
key      = sys.argv[2]

book = LogBook.LogBook(bookfile)
if book.kv_exists(key) :
    print(key + " : " + book.kv_get(key))
else :
    print(key + " does not exist")

exit()
