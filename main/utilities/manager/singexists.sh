#!/bin/bash

if [ -x "$(command -v singularity)" ]
then
    echo yes
else
    echo no

fi
