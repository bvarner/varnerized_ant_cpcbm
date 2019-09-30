# 28-09-2019
LASER HEAD ADD-ON

[Related video presentation](https://youtu.be/V-mHSj8K_-w)

In this folder you can find all the necessary files to make
the laser head holder: the head holder and two carriage
mods.
The pieces fit a FB03-2500 laser head, an EleksMaker PWM
capable, 2500mW Blue Laser Module and related clones.  

All mechanical pieces are included in the main project BOM
except the 4 screws needed to fix the laser head.
We used 4 M3x8mm DIN 912 bolt, but longer bolts could be
used.  

In the sub-folder gcode, you can find two different demo
codes. The first is a 40x40mm Bud Spencer Clip Art, the second
one is just a 40x40mm rectangle useful to try different
combination of engraving settings, like feedrate (F command)
and laser power (S command) that has a range from 0 to 1000.  

The settings changed respect to the default settings for The Ant 
Compact PCB Maker are the following:  

$28=500 (Spindle pwm period, us)  
$29=499 (Spindle pwm Max time-on, us)  
$30=0 (Spindle pwm min time-on, us)  
$31=1 (Spindle pwm Enabled at startup)  
 
$110=3500.000 (x max rate, mm/min)  
$111=3500.000 (y max rate, mm/min)  
$112=500.000 (z max rate, mm/min)  
$120=30000.000 (x accel, mm/sec^2)  
$121=30000.000 (y accel, mm/sec^2)  
 

