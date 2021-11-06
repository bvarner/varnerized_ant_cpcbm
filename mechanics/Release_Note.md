# 06-11-2021
New release The_Ant_version_1_1_0 with the new NEMA-8 motor as Z-axis. From now on, this head will be considered the stable version. List of updates:  

	* Total_BOM_Ver_1_1_0.xlsx:  
	    - Revisited version of the BOM that integrates the previous BOM with the new parts.

	* head_4p3 replaces head_4p2(copied in release folder from beta one):  
		- new head_4p3 that uses new Z-axis motor. See the BOM for the model of motor.  
	  
	* bridge_2p4_b replaces bridge_2p3_b(copied in release folder from beta one):  
	    - new bridge needed to accomodate the anti-backlash nut of the new Z-axis motor.
		
	* pcb_lock_2p3 replaces pcb_lock_1p0(copied in release folder from beta one):  
	    - the new pcb lock is more robust and adjustable.  
		- integrates part of probe circuit.  
		
	* pcb_lock_frame_to_bed_connection_1p0 added(copied in release folder from beta one):  
	    - new parts of the bed holder needed to integrate the new parts of probe circuit.  
		
	* cable_chain_1p0(copied in release folder from beta one):  
	    - new parts to use drag chain for the cable, substituting the spiral coil used previously.  
		
	

# 21-07-2019
New parts released, list of updates:

	* head_4p2 copied in release folder from beta one:
		- previous head_4p2 splitted in head_4p2 and pulley_4p2
		- pulley_4p2. It uses new collar for chuck pulley, if hexagon socket screw doesn't protude from the collar, replace it with a longer one. The file collects chuck pulley, turnigy pulley (3mm shaft) and mystery pulley (2.2mm shaft)
		- spindle motor with outer diameter up to 28mm can now be mounted.
	
	* bridge_2p3_a and bridge_2p3_b:
		- New version of the bridge, more rigid and stable.
		- The *_a it is file related at the unchanged parts from previous version. *_b file it's related to the new parts. They are fully compatible with the older version. If you have printed previous version, print only the *_b parts to update.
		- The bridge is compatible both with 4p1 and 4p2 head versions.
		- Added protection cap.
		- Modified camera holder, now more strong with new lock mechanism. 
		- Added lock holes at the bottom for future features.
		- Reinforced and updated z-motor connection.
	
	* z_motor_tool, useful tool to lock the z-motor shaft to the bridge.

Removed parts:

	* head_4p1
	
	* bridge_2p2

All previous parts are available in The_Ant_version_1_0_0 file.
You can find it in the download section under the "tags" tab.
A new version of BOM is added to keep trak of all changes.
The new pieces require a set of new parts:
	
	* 2 M3x10mm DIN 912 bolt
	* 1 M3x12mm DIN 912 bolt
	* 1 Lock collar(ring) internal diameter 6 mm, external diameter 12mm
	* 2 O-ring inner diameter > 17mm, 1.78mm thick. Innerd diameter to be defined.

# 23-04-2019
Added beta_developments directory, where we share parts not yet released and under test.

# 14-04-2019
Released all step files. 
Added tool for tool bit change (both stl and step).

Bug fixes:
bridge_2p2
	* minor bug fix, camera holder screw holes now deeper
	  to improve the clamping force
	  
head_4p1
	* minor bug fix, chuck pulley base now smaller to prevent
	  friction with bearing


# 31-12-2018

## Scope

Release of mechanical drawings of the Compact PCB Maker. 
This release completes the previous one and comprises the mechanical drawings of the bridge, of the head and of the microcontroller cover. 
The suggestions of the previous release apply also to this one.

# 09-12-2018

## Scope

First release of mechanical drawings of the Compact PCB Maker. 
This is a partial release that comprises all the frame related mechanical drawings. Other parts and corrections will follow in other releases.

## Suggestions

We suggest you to print the parts as they are, because the printing position is designed to enhance the mechanical behavior.

We also suggest you to print with a 0.2 mm layer thickness, 25% of infill, 1.5 mm of wall thickness and extrusion width (also called line width) of 0.5 mm. 

We usually print with PLA but it's not a MUST, you may choose different materials taking in account the properties of the material you want to use. The overall rigidity is needed.
 