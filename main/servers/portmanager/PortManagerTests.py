#!/usr/bin/env python



#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   PortManagerTests.py
# @brief  Tests of the port manager API for python.
# @author <fox@nscl.msu.edu>


import unittest
from nscldaq.portmanager import PortManager
import errno
import socket
import getpass
import warnings




##
# @class PortManagerTests
#
#   Unit tests for the port manager.
#
class PortManagerTests(unittest.TestCase):
    def setUp(self):
        warnings.simplefilter('ignore', ResourceWarning)
        pass
    def tearDown(self):
        pass
    
    #
    # turn port info list into a dict keyed by the service.
    #
    def _keyInfo(self, info):
        keyedData = {}
        for aport in info:
            service = aport['service']
            keyedData[service] = aport
        return keyedData
    
    ##
    # test_connection_fail
    #   Connecting to a nonexistent port manager throws gaierror
    #   with -5 (?) error.
    #
    def test_connection_fail(self):
        host='no.such.host.nscl.msu.edu'
        port=30000
        threw      =  False
        rightThrow = False
        rightErrno = False
        try:
            pm = PortManager.PortManager(host, port)
        except socket.gaierror as e:
            threw = True
            rightThrow = True
            if e.errno in (-2, -5):
                rightErrno = True
            else:
                print("Errno gotten was ", e.errno)
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(rightThrow)
        self.assertTrue(rightErrno)
        
    ##
    # test_connection_ok
    #   This time we should be able to form a connection:
    #
    def test_connection_ok(self):
        host='localhost'
        port=30000
        threw = False
        
        try:
            pm = PortManager.PortManager(host, port)
        except:
            threw = True
        self.assertFalse(threw)
        
    
    ##
    # test_getPort_ok
    #
    #   We should be able to get a port from the server:
    #
    def test_getPort_ok(self):
        pm = PortManager.PortManager('localhost', 30000)
        port = pm.getPort('myport')
        self.assertTrue(isinstance(port, (int)))
    
    ##
    # test_getPort_remotefail -- This test is not viable in some circumstances
    #
    def test_getPort_remotefail(self):
        return
        threw      = False
        rightThrow = False
        pm         = PortManager.PortManager(socket.gethostname(), 30000)
        
        try:
            port = pm.getPort('myport')
        except RuntimeError:
            threw = True
            rightThrow  = True
        except:
            threw = True
            
        self.assertTrue(threw)
        self.assertTrue(rightThrow)
        
    ##
    # test_list_base
    #   Tests that the port manager can give us the list initially.  The
    #   assumption is that nothing else is running in which case there will
    #   be one service, the RingMaster  We don't carea about the
    #   port or the user.
    #
    def test_list_base(self):
        pm    = PortManager.PortManager('localhost', 30000)
        ports = pm.listPorts()
        message = None
        if len(ports) >= 1:
            message = 'Number of ports... found ports {}'.format(ports)
        self.assertTrue( len(ports) >= 1, msg=message)
        
        found = False
        for rm in ports:
            if rm['service'] == 'RingMaster':
                found = True
        
        self.assertTrue(found)
    
    ##
    # test_list_afew
    #   Not content with ensuring this all works for a single port I'll create
    #   a few and ensure the all are listed.
    #
    def test_list_afew(self):

        pm2    = PortManager.PortManager('localhost', 30000)
        pre    = pm2.listPorts()

        pm     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        
        pm1    = PortManager.PortManager('localhost', 30000)
        theport= pm1.getPort('theport')

        pm3    = PortManager.PortManager('localhost', 30000)

        iam    = getpass.getuser()

        info   = pm3.listPorts()
        message = None
        if len(info) != (len(pre) +2):
            message = 'Number of ports... found ports {}'.format(info)

        self.assertEqual(len(pre)+2, len(info), msg=message)
        
        # Toss the data up into a dict keyed by service name.
        
        keyedData = self._keyInfo(info)
    
        # RingMaster is present:
        
        self.assertTrue('RingMaster' in keyedData.keys())
        
        # myport is present and has the right stuff:
        
        self.assertTrue('myport' in keyedData.keys())
        self.assertEqual(myport, keyedData['myport']['port'])
        self.assertEqual(iam,    keyedData['myport']['user'])
        
        # theport is present and has the right stuff:
        
        self.assertTrue('theport' in keyedData.keys())
        self.assertEqual(theport, keyedData['theport']['port'])
        self.assertEqual(iam,     keyedData['theport']['user'])

    ##
    # test_find_byservice
    #   Test find given a service to  look for (exact match).
    #
    def test_find_byservice(self):
        pm     = PortManager.PortManager('localhost', 30000)
        pm1    = PortManager.PortManager('localhost', 30000)
        pm2    = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm1.getPort('theport')
        iam    = getpass.getuser()
        
        info = pm2.find(service = 'theport')
        self.assertEqual(1, len(info))
        pinfo = info[0]
        
        self.assertEqual('theport', pinfo['service'])
        self.assertEqual(theport,  pinfo['port'])
        self.assertEqual(iam,      pinfo['user'])
    
    
    ##
    # test_find_bybeginswith
    #   Test find given the starting value of a service name:
    #
    def test_find_bybeginswith(self):
        pm     = PortManager.PortManager('localhost', 30000)
        pm1     = PortManager.PortManager('localhost', 30000)
        pm2     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm1.getPort('theport')
        iam    = getpass.getuser()
        
        info   = pm2.find(beginswith = 'my')
        self.assertEqual(1, len(info))
        pinfo = info[0]
        
        self.assertEqual('myport', pinfo['service'])
        self.assertEqual(myport,  pinfo['port'])
        self.assertEqual(iam,      pinfo['user'])
    
    ##
    # test_find_byuser
    #   Locate ports with a specific username.
    #   Note that in jenkins we'll get too many ports so we only care about
    #   having the ports we allocated and don't care if we match too many.
    #
    def test_find_byuser(self):
        pm     = PortManager.PortManager('localhost', 30000)
        pm1     = PortManager.PortManager('localhost', 30000)
        pm2     = PortManager.PortManager('localhost', 30000)
        myport = pm.getPort('myport')
        theport= pm1.getPort('theport')
        iam    = getpass.getuser()
        
        info = pm2.find(user=iam)
        info = self._keyInfo(info)
        
        # We can assume that if the keys are there, the data are good.
        
        self.assertTrue('myport' in info.keys())
        self.assertTrue('theport' in info.keys())
        
    
    ##
    # test_find_byillegal
    #  Illegal search spec throws RuntimError:
    #
    def test_find_byillegal(self):
        throws   = False
        rthrows  = False
        
        try:
            pm = PortManager.PortManager('localhost', 30000)
            pm.find(george = 'harry')
        except RuntimeError:
            throws = True
            rthrows = True
        
        self.assertTrue(throws)
        self.assertTrue(rthrows)
    
#
# Run the tests.
#
if __name__ == '__main__':
    unittest.main()

