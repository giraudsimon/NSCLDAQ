##
#
# Sample daqconfig.tcl for reading out a 
# CAEN V775, V785, and struck sis 3820.
#
# Set up the V775 TDC
set tdcThresh [list 10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 ]
adc create v775 0x11110000
adc config v775 -geo 3 -thresholds $tdcThresh -timescale 100

## Set up the V785 ADC
set adcThresh [list 10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 \
                    10 10 10 10  10 10 10 10 ]

adc create v785 0x22220000
adc config v785 -geo 2 -thresholds $adcThresh

# Set up the chained readout
caenchain create chain
caenchain config chain -base 0x12000000 -modules [list v775 v785] 


# Define an event readout stack to execute everytime a signal arrives
# on NIM input 1
stack create evt 
stack config evt -modules [list chain] -trigger nim1



# Configure the SIS3820 Scaler device for 32-channel readout
sis3820 create sisSclr 0x38000000

# Define a scaler readout stack to execute every 2 seconds
stack create sclr
stack config sclr -modules [list sisSclr] -trigger scaler -period 2
