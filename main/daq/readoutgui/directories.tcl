#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#


package provide ExpFileSystem 2.0
package require ExpFileSystemConfig
package require DAQParameters
package require Diagnostics

namespace eval ExpFileSystem {
 
}


# ExpFileSystem::CreateHierarchy
#
#      Create the entire directory hierarchy.  All Directories
#      are created with mkdir -p so that paths to them will be made
#      if needed and no errors will be reported if the dirs arleady
#      exist
#
#
proc ExpFileSystem::CreateHierarchy {}  {
    # Fetch the location of the stagearea.  In this implementation
    # We truly don't care if this is a link or a dir.. we are just
    # going to create the needed directories inside of it.
    # If the user puts his/her stuff on the same filesystem as
    # their home directory, that may be useful e.g. for tests.
    #
    #   This proc is unix specific.

    set stagearea [ExpFileSystemConfig::getStageArea]

    file mkdir [file join $stagearea complete]
    file mkdir [file join [ExpFileSystem::getMetadataRoot] current]
    file mkdir [file join  $stagearea current]

    #  Now make the link to it, if it does not already exist.

    set expdir [ExpFileSystemConfig::getExperimentDirectory]
    if {[file exists $expdir]} {
        if {[catch {file readlink $expdir}]} {
            set msg "$expdir already exists and is not a link\n"
            Diagnostics::Warning $msg
            error {ExpFileSystem::CreateHierarchy Conflicting existing files in the way}
        }
    } else {
        file link -symbolic $expdir [file join [ExpFileSystem::getMetadataRoot]] 
    }
}
##
# getStageArea
#
# @return string  - Location of the current stage are:
#
proc ExpFileSystem::getStageArea {} {
    return [ExpFileSystemConfig::getStageArea]
}
# getMetadataRoot:
#  
# @return string - top level directory of the metadata tree (the run view).
#
#
proc ExpFileSystem::getMetadataRoot {} {
    return [file join [ExpFileSystemConfig::getStageArea] experiment]
}


##
# getCurrentMetadataDir
#
# @return string The directory that holds the current run's metadata
#
proc ExpFileSystem::getCurrentRunDir {} {
    set expdir [ExpFileSystem::getMetadataRoot]
    return [file join $expdir current]
}
##
# getRunDir
#
# @return string  - The directory that holds the current run metadata
#                   and event data file.
#
proc ExpFileSystem::getRunDir {num} {
    set expdir [ExpFileSystem::getMetadataRoot]
    append subdirname run $num
    return [file join $expdir $subdirname]
}
#  genEventfileBasename.
#
# @param run - Run number for which we are generating the files.
#
#   @return string - Event file base name.
#
# An event filename is of the form:
#    run-<runnum>-<segment>.evt
# Where:
# * <runnum> is the  file runnumber and is encoded in %04d form.
# * <segment> is the segemt of the run and is encoded in %02d form.
#   segments are used to limit file sizes to that which can easily be
#   accessed by NFS (< 2Gbytes/segment).
#
proc ExpFileSystem::genEventfileBasename {num} {
    return [format run-%04d $num]
}
# genEventFilename
#    Generate the full name of a run file.
#
#  @param   num   - Run number.
#  @param  seq   - The sequence number of the file.
#
# @return string - Filename for the event file specified by the parameters.
#
# @note    See GenRunFileBase for a discussion of the form of
#    these files.
#
proc ExpFileSystem::genEventFilename {num {seq 0}} {

    set fname [::ExpFileSystem::genEventfileBasename $num]
    return  [format %s-%02d.evt $fname  $seq]
}
# getEventFilename
#    Return the full path to a selected event file
#    The assumption is that the run has already ended.
#    This implies that the run file is now in the experiment/run
#    directory.
#
# @param  num  - The run number we are interested in.
# @param  seq  - The sequence number of the file within the run.
#
# @return string - a fully absolute path to the event file
#                  for a given run and sequence.
#
proc ExpFileSystem::getEventFilename {num {seq 0}} {
    set dir [::ExpFileSystem::getRundir $num]
    set filename [::ExpFileSystem::genEventFilename $num $seq]
    return [file join $dir $filename]
}

# getCompleteEventfileDir
#   @return string - The name of the directory in links to completed
#                    event files are maintained (event file view).
#
proc ExpFileSystem::getCompleteEventfileDir {} {
    set stage [ExpFileSystemConfig::getStageArea]
    return [file join $stage complete]
}

##
# getCurrentEventFileDir
#
# @return string - path to directory that has the current event files.
#
proc ExpFileSystem::getCurrentEventFileDir {} {
    set stage [ExpFileSystemConfig::getStageArea]
    return  [file join $stage current]
}
