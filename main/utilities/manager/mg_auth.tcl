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

package provide auth 1.0
package require sqlite3
##
# @file    mg_auth.tcl
# @brief   Provide authorization support (not really secure)
# @author Ron Fox <fox@nscl.msu.edu>
#
namespace eval ::auth {
    
}
#
#  The public API consist of:
#
#  ::auth::adduser   - adds a new user.
#  ::auth::rmuser    - Remove a user.
#  ::auth::addrole   - Adds a new role.
#  ::auth::rmrole    - Remove role.
#  ::auth::grant     - Grant a role to a user.
#  ::auth::revoke    - Revoke a role from a user.
#  ::auth::users     - List the users.
#  ::auth::listRoles     - List the roles
#  ::auth::listAll   - List the users and the roles they've been granted.
#
#------------------------------------------------------------------------------
# Private utilities.

##
# ::auth::_userExists
#   Does a user exist?
#
# @param db  - database command.
# @param user- Name of user to check
# @return bool - True if the user exists.
#
proc ::auth::_userExists {db user} {
    return [$db eval {
        SELECT COUNT(*) FROM users WHERE username = $user
    }]
}
##
# ::auth::_existingRole
#   Check for the existence of a role.
#
# @param db   - database command.
# @param role - name of role.
# @return bool - true if the role exists.
#
proc ::auth::_existingRole {db role} {
    return [$db eval {
        SELECT COUNT(*) FROM roles WHERE role = $role
    }]
}
    


#-----------------------------------------------------------------------------
# API entries.

##
# auth::adduser
#    Add a new user to the system.
#
# @param db   - database command.
# @param name - Username of the new user - must be unique.
#
proc ::auth::adduser {db name} {
    if {[::auth::_userExists $db $name]} {
        error "There is already a user named: $name"
    }
    $db eval {
        INSERT INTO users (username) VALUES ($name)
    }
}
##
# ::auth::rmuser - remove an existing user.
#
# @param db       - database command.
# @param user     - name of usr to remove.
#
proc ::auth::rmuser {db user} {
    if {![::auth::_userExists $db $user]} {
        error "$user is not an existing user and therefore cannot be removed"
    }
    
    # We also need to get rid of grants to this user.. A transaction makes this
    # neatly amtomic
    
    $db transaction {
        $db eval {
            DELETE FROM user_roles WHERE user_id IN (
                select id FROM users WHERE username = $user
            )
        }
        $db eval {
            DELETE FROM users WHERE username = $user
        }
        
    }
}
##
# ::auth::addrole
#    Add a new entry to the role table.
#
# @param db     - database command.
# @param role   - name of new role.
#
proc ::auth::addrole {db role} {
    if {[::auth::_existingRole $db $role]} {
        error "The role $role already exists"
    }
    $db eval {
        INSERT INTO roles (role) VALUES ($role)
    }
    
}
##
# ::auth::rmrole
#    Remove a role.   This implies a revocation of the role from all holders.
#
# @param db    - database command.
# @param role  - Role to remove (must exist).
#
proc ::auth::rmrole {db role} {
    if {[::auth::_existingRole $db $role]} {
        # Make the revocation and removal atomic:
        
        $db transaction {
            # revoke everywhere:
            
            $db eval {
                DELETE FROM user_roles WHERE role_id IN (
                    SELECT id FROM roles WHERE role = $role
                )
            }
            # Destroy the role itself.
            
            $db eval {
                DELETE from roles WHERE role = $role
            }
        }
    } else {
        error "There is no role named '$role' to remove"
    }
}
##
# ::auth::grant
#   Grant a user a role.
#
# @param db   - database command.
# @param user - username.
# @param role - rolename.
#
proc ::auth::grant {db user role} {
    if {![::auth::_userExists $db $user]} {
        error "Cannot grant $role to nonexistent user $user"
    }
    if {![::auth::_existingRole $db $role]} {
        error "Cannot grant nonexistent role: $role to user $user"
    }
    
    # Maybe that works...
    
    $db eval {
        INSERT INTO user_roles (user_id, role_id)
        SELECT users.id AS user_id, roles.id AS role_id FROM
        users, roles
        WHERE  users.username = $user AND roles.role = $role
    }
}
##
# ::auth::revoke
#    Revoke a role from the  user.  Not only must the role exist but
#    the usr must have been granted that role.
#
# @param db    - Database command.
# @param user  - name of the user.
# @param role  - Role to revoke.
#
proc ::auth::revoke {db user role} {
    if {![::auth::_userExists $db $user]} {
        error "There is no user '$user' from which roles can be revoked"
    }
    if {![::auth::_existingRole $db $role]} {
        error "There is no such role '$role' that can be revoked."
    }
    #  See if the user has been granted that role:
    
    set hasRole [$db  eval {
        SELECT COUNT(*) FROM user_roles
        INNER JOIN users ON users.id = user_roles.user_id
        INNER JOIN roles ON roles.id = user_roles.role_id
        WHERE username = $user AND role = $role
    }]
    if {$hasRole} {
        #
        #  This is a tricky bit of SQL.  Subqueries allow a single statement
        #  to do the delete; Specifically we make use of the fact there will only be
        #  a single element in each of the IN  subqueries as users and roles are
        #  unique.
        #
        $db eval {
            DELETE FROM user_roles
            WHERE user_id IN (
                SELECT users.id FROM users WHERE username = $user
            ) AND role_id IN (
                SELECT roles.id FROM roles WHERE role = $role
            )
        }
    } else {
        error "$user has never been granted $role"
    }
}
##
# ::auth::users
#    Return a lis tof the users.
#
# @param db - database command
# @return list of textual items - each item a username.
#
proc ::auth::users {db} {
    return [$db eval {
        SELECT username FROM users
    }]
}
##
#  ::auth::listRoles
#     Lists all role names.
#
# @param db
# @return list of role names.
#
proc ::auth::listRoles {db} {
    return [$db eval {
        SELECT role FROM roles
    }]
}
##
# ::auth::listAll
#
#  Lists the users and the roles they hold.
#
# @param db - database command.
# @return dict - the keys are usernames the values are lists of roles held
#                by that user.
#
proc ::auth::listAll {db} {
    set result [dict create]
    
    $db eval {
        SELECT username, role FROM user_roles
        INNER JOIN users ON users.id = user_roles.user_id
        INNER JOIN roles ON roles.id = user_roles.role_id
    } userrole {
        dict lappend result $userrole(username) $userrole(role)
    }
    #  This has gotten all people that have roles...but there can be people
    #  that don't have roles... This adds an empty list to the dict for each of those.
    
    set usersWithRoles [dict keys $result]
    set allUsers [::auth::users $db]
    
    foreach user $allUsers {
        if {$user ni $usersWithRoles} {
            dict set result $user [list];        
        }
    }
    return $result
}
