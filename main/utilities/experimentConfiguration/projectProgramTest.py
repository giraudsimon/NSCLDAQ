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
# @file   projectProgramTest.py
# @brief  Tests for project.Programs methods.
# @author <fox@nscl.msu.edu>

import unittest
import project

class projectProgramtest(unittest.TestCase):
    def setUp(self):
        self._project = project.Project(':memory:')
        self._project.create()
        self._hosts   = project.Hosts(self._project)
        self._programs= project.Programs(self._project)
        
    ##
    # test_add_simple
    #    Ok add with no parameters
    #
    def test_add_simple(self):
        path = '/usr/opt/daq/current/bin/dumper'
        wd   = '/user/fox'
        hostid = self._hosts.add('test.nscl.msu.edu')
        id = self._programs.add(path, wd, hostid)

        # Ensure we have an entry with our id and the right stuff.
        conn = self._project.connection
        curr = conn.cursor()
        curr.execute('''
            SELECT * from programs WHERE id=?
                     ''', (id,))
        program = curr.fetchone()
        self.assertNotEquals(None, program)
        self.assertEquals(id, program[0])
        self.assertEquals(path, program[1])
        self.assertEquals(wd, program[2])
        self.assertEquals(id, program[3])
        
        
        self.assertEquals(None, curr.fetchone())     # only one answer.
        
        #  Ensure no args entries exist for us.
    
        curr.execute('''
            SELECT COUNT(*) FROM program_args
                WHERE program_id = ?
                     ''', (id,))
        self.assertEquals((0,), curr.fetchone())

    ##
    # test_add_args
    #    Test add with parameters.
    #
    def test_add_args(self):
        path = '/usr/opt/daq/current/bin/dumper'
        wd   = '/user/fox'
        hostid = self._hosts.add('test.nscl.msu.edu')
        id = self._programs.add(path, wd, hostid, ('--port=managed', '--debug=0'))

        # Ensure we have an entry with our id and the right stuff.
        conn = self._project.connection
        curr = conn.cursor()
        curr.execute('''
            SELECT * from programs WHERE id=?
                     ''', (id,))
        program = curr.fetchone()
        self.assertNotEquals(None, program)
        self.assertEquals(id, program[0])
        self.assertEquals(path, program[1])
        self.assertEquals(wd, program[2])
        self.assertEquals(id, program[3])
        
        
        self.assertEquals(None, curr.fetchone())     # only one answer.
        
        curr.execute('''
            SELECT argument FROM program_args
                WHERE program_id =?
                ORDER BY argument ASC
            
                     ''', (id,))
        arg = curr.fetchone()
        self.assertNotEquals(None, arg)
        self.assertEquals('--debug=0', arg[0])
        
        arg = curr.fetchone()
        self.assertNotEquals(None, arg)
        self.assertEquals('--port=managed', arg[0])
        
        arg  = curr.fetchone()
        self.assertEquals(None, arg)
        
    ##
    # test_add_nohost
    #    Failed add due to no such host.
    #
    def test_add_nohost(self):
        threw = False
        try:
            self._programs.add('/some/path', '/home/here', 1)
        except RuntimeError:
            threw = True
        except:
            pass
            
        self.assertTrue(threw)
    ##
    # test_addbyhname_ok
    #     Add by hostname ok.
    #
    def test_addbyname_ok(self):
        host = 'test.nscl.msu.edu'
        path = '/usr/opt/daq/current/bin/dumper'
        wd   = '/user/fox'
        hostid = self._hosts.add('test.nscl.msu.edu')
        id     = self._programs.add_byHostname(
            path, wd, host 
        )
        
        conn    = self._project.connection
        cursor  = conn.cursor()
        cursor.execute('''
            SELECT host_id FROM programs WHERE id =?
                       ''', (id,))
        row = cursor.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(hostid, row[0])
        
        # Should be only 1:
        
        row = cursor.fetchone()
        self.assertEquals(None, row)
        
        
    ##
    # test_addbyhname_create
    #    add by host name creating.
    #
    def test_addbyname_create(self):
        host = 'test.nscl.msu.edu'
        path = '/usr/opt/daq/current/bin/dumper'
        wd   = '/user/fox'
        id = self._programs.add_byHostname(path, wd, host, list(), True)
        hostid = self._hosts.id(host)
        
        self.assertNotEquals(None, hostid)
        conn    = self._project.connection
        cursor  = conn.cursor()
        cursor.execute('''
            SELECT host_id FROM programs WHERE id =?
                       ''', (id,))
        row = cursor.fetchone()
        self.assertNotEquals(None, row)
        self.assertEquals(hostid, row[0])
        
        # Should be only 1:
        
        row = cursor.fetchone()
        self.assertEquals(None, row)        
    
    ##
    # test_addbyhname_nohost
    #    failed add because bad host.
    def test_addbyname_nohost(self):
        host = 'test.nscl.msu.edu'
        path = '/usr/opt/daq/current/bin/dumper'
        wd   = '/user/fox'
        threw = False
        
        try:
           self._programs.add_byHostname(path, wd, host)
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
        
    ##
    # test_list0
    #    List with no elements
    def test_list0(self):
      self.assertEquals(0, len(self._programs.list()))
      
    ##
    # test_list_n
    #    List with a few elements no args.
    def test_list_n(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        for h,p,wd in zip(hosts, paths, wds):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid)
            pids.append(pid)
            
        listing = self._programs.list()
        for pid, path, wd, hid, hname, row in map(None, pids, paths, wds, hids, hosts, listing):
            self.assertEquals(pid, row['program_id'])
            self.assertEquals(path, row['path'])
            self.assertEquals(wd, row['working_dir'])
            self.assertEquals(hid, row['host_id'])
            self.assertEquals(hname, row['host_name'])
      
    ##
    # test_list_args
    #    Lists with args
    def test_list_args(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.list()
        for pid, path, wd, hid, hname, a, row in map(None, pids, paths, wds, hids, hosts, args, listing):
            self.assertEquals(pid, row['program_id'])
            self.assertEquals(path, row['path'])
            self.assertEquals(wd, row['working_dir'])
            self.assertEquals(hid, row['host_id'])
            self.assertEquals(hname, row['host_name'])
            self.assertEquals(a, row['args'])
        
    ##
    # test_find_noterms
    #
    #   This should match everything
    #
    def test_find_noterms(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)

        listing = self._programs.find()
        for pid, path, wd, hid, hname, a, row in map(None, pids, paths, wds, hids, hosts, args, listing):
            self.assertEquals(pid, row['program_id'])
            self.assertEquals(path, row['path'])
            self.assertEquals(wd, row['working_dir'])
            self.assertEquals(hid, row['host_id'])
            self.assertEquals(hname, row['host_name'])
            self.assertEquals(a, row['args'])        
    
    ##
    # test_find_id
    #
    #  I should be able to find a specific item by id:
    
    def test_find_id(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
    
        listing = self._programs.find(program_id=2)
        self.assertEquals(2, listing[0]['program_id'])
        self.assertEquals(paths[1], listing[0]['path'])
        self.assertEquals(wds[1], listing[0]['working_dir'])
        self.assertEquals(hids[1], listing[0]['host_id'])
        self.assertEquals(hosts[1], listing[0]['host_name'])
        self.assertEquals(args[1], listing[0]['args'])
        
    ##
    # test_find_exact_path
    #  Exact match on path key:
    #
    def test_find_exact_path(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        listing = self._programs.find(path=paths[2])
        self.assertEquals(3, listing[0]['program_id'])
        self.assertEquals(paths[2], listing[0]['path'])
        self.assertEquals(wds[2], listing[0]['working_dir'])
        self.assertEquals(hids[2], listing[0]['host_id'])
        self.assertEquals(hosts[2], listing[0]['host_name'])
        self.assertEquals(args[2], listing[0]['args'])
    
    ##
    # test_find_approx_path
    #   Path with wildcards that will match more than one program.
    #
    def test_find_approx_path(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', 'user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        listing = self._programs.find(path='%eadout')  # match 1,2
        self.assertEquals(2, len(listing))
        
        for i,j in zip([1,2], [0,1]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])    
    
    ##
    # test_find_exact_wd
    #   Find with an exact match on the working directory:
    #
    def test_find_exact_wd(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(working_dir='/user/fox/test')
        self.assertEquals(1, len(listing))
        for i,j in zip([1], [0]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])    
        
        
    ##
    # test_find_approx_wd
    #    Test approximate match on working directory:
    
    def test_find_approx_wd(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(working_dir='%test%')
        self.assertEquals(2, len(listing))
        for i,j in zip([1,2], [0,1]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])            
    ##
    # test_find_host_id
    #   Match on a host id
    
    def test_find_host_id(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(host_id=2)
        self.assertEquals(1, len(listing))
        for i,j in zip([1], [0]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])   
    ##
    # test_find_exact_host
    #   Exact match on host_name
    
    def test_find_exact_host(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(host_name=hosts[1])
        self.assertEquals(1, len(listing))
        for i,j in zip([1], [0]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])   
    
    ##
    # test_find_approx_host
    #    Match on host_name with a wildcard.
    
    def test_find_approx_host(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(host_name='%st.nscl.msu.edu')
        self.assertEquals(2, len(listing))
        for i,j in zip([0,1], [0,1]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])   
    
    
    ##
    # test_find_multiple_keys
    #  Match on multiple keys
    def test_find_multiple_keys(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(host_name='%st.nscl.msu.edu', path='%vm%')
        self.assertEquals(1, len(listing))
        for i,j in zip([1], [0]):
            row = listing[j]
            self.assertEquals(pids[i], row['program_id'])
            self.assertEquals(paths[i], row['path'])
            self.assertEquals(wds[i], row['working_dir'])
            self.assertEquals(hids[i], row['host_id'])
            self.assertEquals(hosts[i], row['host_name'])
            self.assertEquals(args[i], row['args'])        
    
    ##
    # test_find_nomatch
    #
    def test_find_nomatch(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.find(host_name='%st.nscl.msu.edu', path='%cc%')
        self.assertEquals(0, len(listing))
    
    ##
    # test_find_bad_key
    #   error if we search with a bad key:
    #
    def test_find_bad_key(self):
        threw = False
        try:
            self._programs.find(george='takei')
        except RuntimeError:
            threw = True
        except:
            pass
        
        self.assertTrue(threw)
     
    ##
    # test_find_none
    #
    def test_ids_none(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.ids(host_name='%st.nscl.msu.edu', path='%cc%')
        self.assertEquals(0, len(listing))
    
    ##
    # test_ids_some
    #    Test with some rows matching
    
    def test_ids_some(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        listing = self._programs.ids(host_name='%st.nscl.msu.edu')
        self.assertEquals(2, len(listing))
        for i in [0,1]:
            self.assertEquals(pids[i], listing[i])
        
    ##
    # test_modify_base
    #    modify attributes in the base record.
        
    def test_modify_base(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        newpath='/usr/local/bin/dumper'
        newwd  = '/'
        self._programs.modify(1, path=newpath, working_dir=newwd)        
        listing = self._programs.find(program_id=1)
        row = listing[0]

        self.assertEquals(newpath, row['path'])
        self.assertEquals(newwd,   row['working_dir'])
            
    ##
    # test_modify_hostid
    #
    #   Modify hostid to something legal:
    #
    def test_modify_hostid(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        self._programs.modify(2, host_id=1)
        listing = self._programs.find(program_id = 2)
        row = listing[0]
        self.assertEquals(1, row['host_id'])
        self.assertEquals(hosts[0], row['host_name'])
        
        
    ##
    # test_modify_hostname
    #  Can modify the host by name as well:
    
    def test_modify_hostname(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
            
        self._programs.modify(2, host_name=hosts[0])
        listing = self._programs.find(program_id = 2)
        row = listing[0]
        self.assertEquals(1, row['host_id'])
        self.assertEquals(hosts[0], row['host_name'])
    
    
    ##
    # test_modify_both_hosts
    #   It is an error to modify both the host name and id
    #
    def test_modify_both_hosts(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        thrown = False
        try:
            self._programs.modify(2, host_name='test.nscl.msu.edu', host_id=2)
        except RuntimeError:
            thrown = True
        except:
            pass
        
        self.assertTrue(thrown)
        
    ##
    # test_modify_nosuch_hostid
    #   Can't specify a hostid for a host that does not exist:
    #
    def test_modif_nosuch_hostid(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        thrown = False
        try:
            self._programs.modify(2, host_id=1234)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
        

    ##
    # test_modify_nosuch_hostname
    #   host name must exist clearly.
    #
    def test_modify_nosuch_hostname(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        thrown = False
        try:
            self._programs.modify(2, host_name='this.host.does.not.exist')
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)

    
    ##
    # test_modify_invalid_key
    #    An invalid field key should fail too:
    #
    def test_modify_invalid_key(self):
        thrown = False
        try:
            self._programs.modify(2, leonard='nimoy')
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)        
        
    ##
    # test_modify_nosuchid
    #  Not allowed to modify a program that does not exist.
    #
    def test_modify_nosuchid(self):
        thrown = False
        try:
            self._programs.modify(2, path='/some/other/path')
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)        

    
    ##
    # test_modify_args
    #
    #  Should also be able to modify the args of a program.
    #
    def test_modify_args(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        newargs = ['--port=managed', '--controlconfig=/dev/null']
        self._programs.modify(1, args=newargs)
        listing = self._programs.find(host_id=1)
        row = listing[0]
        self.assertEquals(newargs, row['args'])

    ##
    # test_delete_ok
    
    def test_delete_ok(self):
        hids = list()
        pids = list()
        hosts= ('test.nscl.msu.edu', 'host.nscl.msu.edu', 'aaa.nscl.msu.edu')
        paths= ('/usr/opt/daq/current/bin/dumper',
                '/usr/opt/daq/current/bin/vmusbReadout',
                '/usr/opt/daq/current/bin/ccusbReadout')
        wds  = ('/user/fox', '/user/fox/test', '/user/fox/test/nextgenrod')
        args = (['--port=managed', 'a,b,c'],
                ['--port=1234', 'p q r '],
                ['--daqconfig=/user/fox/config/c1.tcl', '--controlconfig=/user/fox/config/c2.tcl']
                )
        for h,p,wd, a in zip(hosts, paths, wds, args):
            hid = self._hosts.add(h)
            hids.append(hid)
            pid  = self._programs.add(p, wd, hid, a)
            pids.append(pid)
        
        self._programs.delete(1)
        ids = self._programs.ids(program_id = 1)
        self.assertEquals(0, len(ids))
    ##
    # test_delete_nosuch
    #
    def test_delete_nosuch(self):
        thrown = False
        try:
            self._programs.delete(1)
        except RuntimeError:
            thrown = True
        except:
            pass
        self.assertTrue(thrown)
    
    ##
    #  may want tests to ensure arguments went away too....
    #
    
if __name__  == '__main__':
    unittest.main()