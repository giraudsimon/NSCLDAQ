#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file  mg_authedit.tcl
# @brief Editor for authorization database.
# @author Ron Fox <fox@nscl.msu.edu>
#

set libs [file normalize [file join  [file dirname [info script]] .. TclLibs]]
lappend auto_path $libs      

package require Tk
package require auth
package require snit
package require selectablelist 
package require dialogwrapper
package require sqlite3


#-----------------------------------------------------------------------------
#  Widgets

##
# @class UsersAndRoles
#    This is a megawidget that is a treeview that shows a list of the users and,
#    if a user is expanded, the roles that user currently has.
#
# OPTIONS
#    -data   - Results of auth::listall  that sets the contents of the treeview
#   -onuserb3    - Script for right button click in a user.
#   -ongrantb3   - Script for right button in a granted role for a user.
#
# METHODS:
#    addUser   - Add a new user to the tree.
#    rmUser    - Remove a user.
#    grant     - Grant a role to a user.
#    revoke    - Revoke a role from a user.
#
snit::widgetadaptor UsersAndRoles {
    component tree
    
    option -data -default [list] -configuremethod _loadData -cgetmethod _cgetData
    option -onuserb3
    option -ongrantb3
    
    constructor args {
        installhull using ttk::frame
        
        install tree using ttk::treeview $win.tree  \
            -columns [list role]  -show [list tree headings] \
            -displaycolumns #all -selectmode browse \
            -yscrollcommand [list $win.ysb set]
        ttk::scrollbar $win.ysb -command [list $tree yview] -orient vertical
        foreach title [list "User name" "Role"] column [list #0  role] {
            $tree heading $column -text $title
        }
    
        
        grid $tree $win.ysb -sticky nsew
        
        # Make user and role tags and bind b3 to the dispatchers
        # for the -onxxxxb3 scripts.
        #
        
        $tree tag add user [list]
        $tree tag add role [list]
        
        $tree tag bind user <Button-3> [mymethod _dispatchUserMb3 %x %y]
        $tree tag bind role <Button-3> [mymethod _dispatchRoleMb3 %x %y]
        
        $self configurelist $args
    }
    #----------------------------------------------------------------------
    # Configuration management methods
    
    ##
    # _loadData
    #   Loads new data into the tree.
    #
    # @param optname - option name (ignored).
    # @param value   - dict from e.g. auth::listAll.
    #
    method _loadData {optname value} {
        $tree delete [$tree children {}];    # Clear the tree.
        
        # We'll alphabetize both the keys (users) and the roles.
        
        foreach user [lsort -increasing -dictionary [dict keys $value]] {
            set userId [$tree insert {} end -text $user -tags user]
            set roles [dict get $value $user]
            foreach role [lsort -dictionary -increasing $roles] {
                $tree insert $userId  end -values [list $role] -tags role
            }
        }        
    }
    ##
    # _cgetData
    #   Returns the data from the treeview in the form of the same dict
    #   that created it.
    #
    # @param optname  - option name - ignored.
    method _cgetData {optname} {
        set result [dict create]
        set userIds [$tree children {}];     # These are entry ids of users.
        foreach id $userIds {
            set username [$tree item $id -text]
            dict set result $username [list]
            set roleIds [$tree children $id];   # Ids of granted roles
            foreach rid $roleIds {
                dict lappend result $username [$tree item $rid -values]
            }
        }
        return $result
    }
    #---------------------------------------------------------------------
    # Event handling
    
    ##
    # _dispatchUserMb3
    #    Dispatches a context menu callback on a right click on a user.
    #
    # @param x,y - window coordinates of the click.
    method _dispatchUserMb3 {x y} {
        
        set userScript $options(-onuserb3)
        if {$userScript ne ""} {
            set id [$tree identify item $x $y]
            set user [$tree item $id -text]
            lappend userScript $user
            uplevel #0 $userScript
        }
    }
    ##
    # _dispatchRoleMb3
    #    Dispatch a mb3 click on a role line.  We pull together
    #    the role and user it's been granted to and pass that to any
    #    -ongrantb3 script.
    #
    # @param x,y  - window coordinates of the click.
    #
    method _dispatchRoleMb3 {x y} {
        set userScript $options(-ongrantb3)
        
        if {$userScript ne ""} {
            set roleId [$tree identify item $x $y]
            set userId [$tree parent $roleId]
            
            set username [$tree item $userId -text]
            set rolename [$tree item $roleId -values]
            lappend userScript $username $rolename
            uplevel #0 $userScript
        }
    }
    #------------------------------------------------------------------
    # Public methods:
    
    ##
    # addUser
    #   Add a user to the set. We'll do this using cget and configure
    #   so that users remain sorted.
    #
    # @param username - new user.
    # @note username must not already exist.
    #
    method addUser {username} {
        set current [$self cget -data]
        if {[dict exists $current $username]} {
            tk_messageBox -parent $win -icon error -type ok \
                -title "Existing user" \
                -message "$username is an existing user in the list"           
        }
        dict set current $username [list]
        $self configure -data $current;   # resorts the users.
    }
    ##
    # rmUser
    #   Remove a user from the list of users (naturally any children
    #   are also removed).
    #
    # @param username  - user to remove.
    #
    method rmUser {username} {
        set current [$self cget -data]
        if {$username in [dict keys $current]} {
            set current [dict remove $current $username]
            $self configure -data $current
        } else {
            tk_messageBox -parent $win -type ok -icon error \
                -title "No user" \
                -message "There is no user named $username"
        }
    }
    ##
    # grant
    #    Grant a new role to a user:
    #     -   The user must exist.
    #     -   We take the caller's word on the existence of the role as there
    #         may be roles not yet granted to anybody.
    #     -   The user must not already have that role granted.
    #
    # @param user   - User to grant role to.
    # @param role   - Role to grant to the user.
    #
    method grant {user role} {
        set current [$self cget -data]
        if {$user ni [dict keys $current]} {
            tk_messageBox -parent $win -type ok -icon error -title "No user" \
                -message "There is no user named $user"
            return
        } else {
            if {$role in [dict get $current $user]} {
                tk_messageBox -parent $win -type ok -icon error -title "Duplicate role" \
                    -message "$user has already been granted $role"
                return
            }
            # Grant the role:
            
            dict lappend current $user $role
            $self configure -data $current
        }
    }
    ##
    # revoke
    #    Revoke a role from a user:
    #    -   User must exist.
    #    -   User must have been granted the role.
    #
    # @param user - name of the user to modify.
    # @param role - Role to remove.
    #
    method revoke {user role} {
        set current [$self cget -data]
        if {$user ni [dict keys $current]} {
            tk_messageBox -parent $win -title "No user" -icon error -type ok \
               -message "There is no such user $user"
            return
        }
        set currentGrants [dict get $current $user]
        if {$role ni $currentGrants} {
            tk_messageBox -parent $win -title "Not granted" -icon error -type ok \
               -message "$user has not been granted $role"
            return
        }
        set which [lsearch -exact $currentGrants $role]
        if {$which == -1} {
            error "Search of role in current user failed"
        }
        set currentGrants [lreplace $currentGrants $which $which];  #revoke
        dict set current $user $currentGrants;  # Replace existing grants.
        
        $self configure -data $current
    }
}
#-------------------------------------------------------------------
# Global data
#
#
#  The roles must be maintained because it's possible for a role to exist
#  but not be granted to anybody.  The people will all be in the
#  UsersAndRoles widget, however, even if they have never
#  been granted any roles.
#  
#
variable existingRoles  [list];       # All known roles.
variable userlist       "";           #  The UsersAndRoles list.
variable newuser        "";           #  New user entry box.
variable newrole        "";            # New role entry box.

variable usercontext    "";            # Stores user context  menu.
variable rolecontext    "";          # Storees role context menu.

#  Store stuff handed to our context menu posters:

variable user          "";           # User in user and role contexts
variable role          "";           # Role in role contexts.

#-------------------------------------------------------------------
#   Utility procs.
#

##
# _usage
#    Outputs an error message to stderr, then usage text and
#    finally exits with an error status return code.
#
# @param msg - the user message to output.
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "\Usage:\n"
    puts stderr "  \$DAQBIN/mg_authedit  configuration-database\n"
    puts stderr "Editor to define users and roles and to grant "
    puts stderr "Roles to users."
    puts stderr "\nWhere:"
    puts stderr "     configuration-database is the sqlite3 database"
    puts stderr "     file containing the experiment configuration"
    exit -1
}
##
# _addUser
#   Adds the user in the new user entry to the set of users in the
#   listing widget.
#
# @param e - entry widget.
# @note userlist containse the user/roles listbox.
#
proc _addUser {e} {
    set newUser [$e get]
    $::userlist addUser $newUser
}
##
# _addRole
#   Adds a new role to the list of possible roles.  Note that since
#   this role is not yet granted to anyone all this does it add it to
#   the ::existingRoles list.
#
# @param e - the entry in which the name of the new role is held.
# @note it is an error to try to add a role that's already in the list.
#
proc _addRole {e} {
    set newRole [$e get]
    if {$newRole in $::existingRoles} {
        tk_messageBox -parent $::newrole -type ok -icon error \
            -message "$newRole is already a defined role."   
    } else {
        lappend ::existingRoles $newRole
    }
}
##
# _postUserContext
#    Posts the user context menu after first saving the name of the user
#    the context menu is being posted for.
#
# @param username - name of user over which the pointer is when the menu
#               is posted.
#
proc _postUserContext {username} {
    set ::user $username
    set x [winfo pointerx $::userlist]
    set y [winfo pointery $::userlist]
    
    $::usercontext post $x $y
    focus $::usercontext
    
}
##
# _rmvUser
#   Remove the current user (handles the user context menu entry for
#   remove...)
#
proc _rmvUser { } {
    if {[tk_messageBox -parent $::userlist -icon question -type yesno \
         -title "Really?" -message "Really delete $::user?"] eq "yes"} {
        $::userlist rmUser $::user
    }
}

##
# _grantableRoles
#   Given a user determines the roles that can be granted to that user.
#
# @param user -  name of the user.
# @note       - we use the $::userlist to figure out what the user has.
# @note       - we user $::existingRoles to get the exhaustive set of roles.
# @return list - List of role names that the user does not have.
#
proc _grantableRoles {user} {
    set current [$::userlist cget -data]
    if {$user ni [dict keys $current]} {
        error "$user is not a valid user."
    }
    set grantedRoles [dict get $current $user];    # Roles already granted.
    set grantableRoles [list]
    foreach role $::existingRoles {
        if {$role ni $grantedRoles} {
            lappend grantableRoles $role
        }
    }
    
    return $grantableRoles
}

##
# _PromptRoles
#    Produce a listbox prompts from among some set of roles.
#
# @param roles - roles to prompt among
# @return list of strings - emptyy if Ok not used.
#
proc _promptRoles {roles} {
    
    
    toplevel .roleprompt
    set dlg [DialogWrapper .roleprompt.dialog]
    set parent [$dlg controlarea]
    set form [ScrollableList $parent.roles -selectmode extended]
    foreach role [lsort -increasing -dictionary $roles] {
        $form insert end $role
    }
    $dlg configure -form $form
    pack $dlg -fill both -expand 1
    set choice [$dlg modal]
    set result [list]
    if {$choice eq "Ok"} {
        set selectedIndices [$form curselection]
        foreach index $selectedIndices {
            set roleName [$form get $index]
            lappend result $roleName
        }
    }
    destroy .roleprompt
    return $result
}


##
# _grant
#  Let the user select roles to grant to the user.
#  $::user is the user who will be granted.  We'll have the user choose
#  from a listbox that contains the roles in ::existingRoles that are not
#  held by the user.
#
proc _grant { } {
    set grantableRoles [_grantableRoles $::user]
    if {[llength $grantableRoles] == 0} {
        tk_messageBox -parent $::userlist -title "All granted" -icon info -type ok \
            -message "There are no roles that have not already been granted to $::user"
        return
    }
    set roles [_promptRoles $grantableRoles]
    foreach role $roles {
        $::userlist grant $::user $role;    
    }
}
##
# _revoke
#   Let the user select a role to revoke from the selected user.
#
# @note the user in question is in ::user.
#
proc _revoke { } {
    set current [$::userlist cget -data]
    if {$::user ni [dict keys $current]} {
        error "$::user is not a user in the userlist somehow"
    }
    
    set currentRoles [dict get $current $::user]
    if {[llength $currentRoles] == 0} {
        tk_messageBox -parent $::userlist -title "None granted" -icon info \
            -type ok \
            -message "$::user has no roles to revoke."
        return
    }
    set revocations [_promptRoles $currentRoles]
    foreach role $revocations {
        $::userlist revoke $::user $role
    }
}
    
###
# _postRoleContext
#   Post the role context menu.
#   - the user and roles are saved in global data.
#   - The pointer position is retreived and
#   - $::role is posted at that position.
#    - the menu is given keyboard focus so <Esc> will unpost it.
#
# @param user - the user involved.
# @param role - The role over which the user is placed.
#
proc _postRoleContext {user role} {
    set ::user $user
    set ::role $role
    
    $::rolecontext post  [winfo pointerx $::userlist] [winfo pointery $::userlist]
    focus $::rolecontext
}
##
# _revokeRole
#     Hooked to the role context Revoke menu.
#
#  -  ::user -has the user
#  -  ::role -has the role
#  -  ::userlist has the list of users.
#
proc _revokeRole { } {
    $::userlist revoke $::user $::role
}
##
# _reconcileRoles
#    Given a user and the roles they have in the editor and database,
#    Removes from the database roles that were revoked in the editor and
#    grants new roles.
#
# @param db    - database command.
# @param user  - Name of the user
# @param editRoles - roles the user has in the editor.
# @param dbRoles - Roles they have in the database.
#
proc _reconcileRoles {db user editRoles dbRoles} {
    # Revoke the roles they no longer have:
    
    foreach role $dbRoles {
        if {$role ni $editRoles} {
            ::auth::revoke $db $user $role
        }
    }
    # Grant roles they now have:
    
    foreach role $editRoles {
        if {$role ni $dbRoles} {
            ::auth::grant $db $user $role
        }
    }
}
##
# _Save
#    Save the current configuration.
#    - Remove current roles that are not in ::existingRoles
#    - Add ::existingRoles that are not in current roles.
#    - Remove users that no longer exist.
#    - Add users that came into being.
#    - For each user revoke roles they no longer have and grant new roles.
#
# @param db   - database command.
proc _Save {db} {
    set currentRoles [::auth::listRoles $db]
    
    # Remove deleted roles.
    
    foreach role $currentRoles {
        if {$role ni $::existingRoles} {
            ::auth::rmrole $db $role
        }
    }
    #  Add new roles
    
    foreach role $::existingRoles {
        if {$role ni $currentRoles} {
            ::auth::addrole $db $role
        }
    }
    #   Now add/remove users as needed:
    
    set userInfo [::auth::listAll $db]
    set editInfo [$::userlist cget -data]
    
    set existingUsers [dict keys $userInfo]
    set editUsers     [dict keys $editInfo]
    
    foreach user $existingUsers {
        if {$user ni $editUsers} {
            ::auth::rmuser $db $user;      # Removed
        }
    }
    foreach user $editUsers {
        if {$user ni $existingUsers} {
            auth::adduser $db $user;      # Added.
        }
    }
    #  Now we get to the roles.
    #  -  If a user doesn't exist in the existingUsers all of his/her roles
    #     get added.
    #  -  If a user does exist we need to do something pretty similar
    #     to what we just did with role names and usernames:
    #     *  Revoke roles they no longer hold
    #     *  grant new roles they do have.
    #
    foreach user $editUsers {
        set editRoles [dict get $editInfo $user]
        if {$user ni $existingUsers} {
            # New user - grant all roles.
            
            foreach role $editRoles {
                ::auth::grant $db $user $role
            }
            
        } else {
            # existing user reconcile roles:
            
            set existingRoles [dict get $userInfo $user]
            _reconcileRoles $db $user $editRoles $existingRoles
            
        }
    }
    
}
##
# _removeRolesFromList
#    Given a list of roles to remove, removes those roles from a list of roles
#   passed in.  Roles not in the target list are ignored.
#
# @param delete   - Roles to delete.
# @param existing - List of roles to delete them from.
# @return the modified list.
#
proc _removeRolesFromList {delete existing} {
    set result [list]
    foreach role $existing {
        if {$role ni $delete} {
            lappend result $role
        }
    }
    return $result
}
    

##
# _deleteRoles
#   Prompts for list of roles to delete.
#   Note that the roles must also be deleted from users they've been granted
#   to.
#
proc _deleteRoles {} {
    set deletedRoles [_promptRoles $::existingRoles]
    
    # Get rid of them from the existing roles list.
    
    set ::existingRoles [_removeRolesFromList $deletedRoles $::existingRoles]

    # Get rid of grants:
     
    set editorData [$::userlist cget -data]
    set editorData [dict map {user grants} $editorData {
        puts "Removing '$deletedRoles' from $grants in $user"
        set results [_removeRolesFromList $deletedRoles $grants]
        puts "Resulting in $results"
        set results;             # result of script.
    }]
    
    
    $::userlist configure -data $editorData
}
    

#-------------------------------------------------------------------
#  Entry point:


if {[llength $argv] != 1} {
    _usage {Incorrect number of command line arguments}
}

sqlite3 db [lindex $argv 0]
set existingRoles [::auth::listRoles db]
set fullInfo      [::auth::listAll db]


#   Create the user interface.  It'll look like this:
#
#   +-----------------------------------------------+
#   |  +--------------------------------------+     |
#   |  |  UsersAndRoles widget                |     |
#   |  +--------------------------------------+     |
#   +---- add user ---------------------------------|
#   |  New user: [               ]  [Add]           |
#   +----- add role --------------------------------|
#   | New role:  [               ] [Add]            |
#   +-----------------------------------------------+
#   |  [Save]                                       |
#   +-----------------------------------------------+
#
#  Right clicking on a user pops up a context menu that allows the
#  user to be:
#    *  Removed
#    *  Have role granted.
#    *  Have a role revoked.
#
#  Right clicking on a role gratned to a user allows:
#    *  That role to be revoked.
#    *  A new role to be granted.
#

# TODO: Must have a way to remove a role!!!


set userlist [UsersAndRoles .listing -data $fullInfo \
    -onuserb3 [list _postUserContext] -ongrantb3 [list _postRoleContext]]

ttk::labelframe .newuserframe -text "Add user"
ttk::label .newuserframe.label -text "Username: "
set newuser [ttk::entry .newuserframe.newuser]
ttk::button .newuserframe.add -text Add -command [list _addUser $newuser]

ttk::labelframe .newroleframe -text "Add role"
ttk::label .newroleframe.label -text Role:
set newrole [ttk::entry .newroleframe.newrole]
ttk::button .newroleframe.add -text Add -command [list _addRole $newrole]
ttk::button .newroleframe.delete -text Delete... -command [list _deleteRoles]

ttk::frame .action
ttk::button .action.save -text Save -command [list _Save db]


grid $userlist -sticky nsew

grid .newuserframe.label $newuser .newuserframe.add -sticky w
grid .newuserframe -sticky nsew

grid .newroleframe.label $newrole .newroleframe.add -sticky w
grid .newroleframe.delete
grid .newroleframe -sticky nsew

grid .action.save -sticky w
grid .action  -sticky nsew

##
#  The following are context menus
# posted on right clicks in the UsersAndRoles widget:

set usercontext [menu .usercontext -tearoff 0]
.usercontext add command -label {Remove user...} -command _rmvUser
.usercontext add separator
.usercontext add command -label {Grant role(s)...} -command _grant
.usercontext add command -label {Revoke role(s)...} -command _revoke

##
#  Role context menu:

set rolecontext [menu .rolecontext -tearoff 0]
.rolecontext add command -label {Revoke} -command [list _revokeRole]
.rolecontext add command -label {Grant role(s)...} -command _grant



    

