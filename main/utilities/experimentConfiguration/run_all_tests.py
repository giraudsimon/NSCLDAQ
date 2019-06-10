#!/usr/bin/env python

## Run all tests:

import glob
import unittest
import os

##
# load_tests - Loads all tests in a directory.
#              * *Test.py are considered tests that don't require a DISPLAY
#              * *TestDisplay.py are considered tests that require a DISPLAY
#                and are only loaded if DISPLAY is set.
#
def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for all_test_suite in unittest.defaultTestLoader.discover('.', pattern='*Test.py'):
        for test_suite in all_test_suite:
            suite.addTests(test_suite)
    try:
        display = os.environ['DISPLAY']
        for all_test_suite in unittest.defaultTestLoader.discover('.', pattern='*TestDisplay.py'):
            for test_suite in all_test_suite:
                suite.addTests(test_suite)
    except:
        pass
    return suite

if __name__ == '__main__':
    unittest.main()
