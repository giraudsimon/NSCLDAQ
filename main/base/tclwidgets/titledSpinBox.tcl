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
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321
#

package provide titledSpinBox 2.0

package require Tk
package require snit

#   A titled spin box is just a frame containing a spin box with a
#   label.
# Options:
#     -label     - Provides the label text for the spinbox's label.
#     -labelside - Determines the positioning of the label relative to
#                  the spinbox:
#                    above  - The title is placed above the spinbox.
#                    below  - The title is placed below the spinbox.
#                    left   - The title is placed to the left of the spinbox.
#                    right  - The title is placed to the right of the spinbox (default).
# Additional bindings:
#    <enter> and focus out are bound to invoke the spinbox's command substituting
#    as documented in the spinbox manpage ...with the direction 'set'.
#

snit::widget titledSpinBox {
    delegate option -label to label as -text
    delegate option   *    to  spinbox except {-label -labelside}
    delegate method   *    to  spinbox

    option -labelside {right}

    constructor args {
        install label   using label   $win.label
        install spinbox using spinbox $win.spinbox

        $self configurelist $args

        switch -exact $options(-labelside) {
            above {
                set packside top
            }
            below {
                set packside bottom
            }
            left  {
                set packside left
            }
            right {
                set packside right
            }
            default {
                error "-labelside $options(-labelside) must be above, below, left or right"
            }
        }
        pack $win.label $win.spinbox -side $packside

        # Establish the bindings:

        bind $win.spinbox <Return>    [mymethod setValue]
        bind $win.spinbox <FocusOut>  [mymethod setValue]

    }
    #-----------------------
    #   setValue
    #      This function is invoked in response to the bindings placed on the
    #      spinbox component that ensures that typed in changes get reported.
    #      We invoke the spinbox -command script doing substitutions as follows:
    #      %W%  The path to the spinbox.
    #      %s%  The value in the spinbox.
    #      %d%  'set' indicating that a value set action took place rather than an up/down
    #            arrow click.
    #
    method setValue {} {
        set value  [$win.spinbox get]
        set script [$win.spinbox cget -command]
        if {$script ne ""} {
            regsub -all {%W}  $script $win.spinbox script
            regsub -all {%s}  $script $value script
            regsub -all {%d}  $script {set}  script

            eval $script
        }
    }
}
