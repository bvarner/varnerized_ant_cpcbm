# Varnerized ANT Compact PCB Maker

Since first coming across the ANT project, I've wanted to make one. However there are some esoteric hardware choices in the base project that seem to prioritize form over function and cost.
As a hobbiest, my primary concerns are not asthetics as much as cost and component availability.

In making my decisions about what to 'Varnerize' vs. purchase and stick to the plans I'll be weighing things based upon:

* What's already in my component drawers or easily obtainable given current supply chain and production limitations. Prioritize functional available parts over form.
* What I can reasonably 3d-print and combine with something else already in my component drawers. Prioritize self fabrication over purchasing produced parts.
* Functional Accuracy. Modifications will prioritize accuracy over workable size. I expect some modifications to reduce the work area.
* Build cost. If I can cut the cost to build by 1/3rd but still retain 80% of the work area and feature set -- then that's a very Varner thing to do.

### Things I planned to investigate:

* NEMA17 steppers in place of the NEMA11s, and in the case of the head, in place of the NEMA 8. I've seen [this top-mounted NEMA 11 modification](https://jplattel.nl/post/2021-03-07-improving-the-ant-head/) which has supplied a surprising amount of 'idea fuel'.
* Bridge segmenting and joinery.
* Use of an Arduino Uno R3 + GRBL board instead of the STM32 based Arm board + GRBL board.
* 8mm Rods and LM8UU bearings instead of the 6mm rods and LM6UU bearings.
* 3d printable pulleys based around 683ZZ bearings. As of 10/2021 I was able to obtain a 20 pack of these bearings for $10. Sourcing the appropirate number of 16-tooth pulleys would have cost more than the 20 pack of bearings.
* Overhangs in some parts (which seem to be dimensionally important) should have 'snap-off' supports.
* Use of a TR8x8 leadscrew, along with a GT2 closed loop belt and pulley system for Z-Axis motion. I have a few of these belts laying around from converting my prusa-style printer to a [Skelestruder](https://jltxplore.dozuki.com/c/Skelestruder_for_Prusa_MK3) extruder.

## Finalized Varnerizations
1. Created a parametric (parameterized) reimplemenation of the bridge & head in Fusion 360. Fusion Archive and STEP export included.
2. 8MM Linear Rods & LM8UU linear bearings. 
    * Sacrificed 4mm of Y axis travel to allow for wall thicknesses to be reasonable with 15mm bores.
    * Added 1mm to the height to the left-right bridge and center bridge, added 1mm separation betweeen centers of the x-axis rails.
3. Bridge Core is now One Piece.
    * Captive M3 Hex Nuts allow the use of (3) M3x30 and (3) M3x40 screws instead of the original BOM spec for (3) M3 threaded rods cut to length.
    * Linear bearing holes are _barely_ oversized, allowing for the M3x30 screws to act as compression mechanisms to hold the bearings.
    * Expansion / Compression cut for linear bearings. M3 screws holding together the bridge act as compression points to hold the bearings snug.
4. Use of sub-mini microswitches for limit-switches. No need to purchase specific orientation limit switch modules, especially when the connections to the grbl board aren't making use of a pull resistors.
5. NEMA17 Stepper Motors
    * Reinforced COREXY Mounts based upon [this design on Thingiverse](https://www.thingiverse.com/thing:3590519) eliminate the need for extra retainers from the thingiverse part.
    * GT2 belt-driven Z-Axis. [140 Tooth 2GT-6](https://www.amazon.com/140-2GT-6-Timing-Belt-Closed-Loop/dp/B014QJBVOY/ref=sr_1_2?dchild=1&keywords=GT2-140+belt&qid=1634881957&qsid=135-9090533-9887601&sr=8-2&sres=B014QJBVOY%2CB014SLWP68%2CB07B5ZQY4W%2CB096D4NTVR%2CB07MWDMBWK%2CB01HM6DIA8%2CB07X9CHY23%2CB07KK86NYX%2CB0897CJKS1%2CB01IPYNQT4%2CB01E91K4N8%2CB08BJ2G2X6%2CB00XR0YJIO%2CB08974S1CC%2CB07JKT5BZQ%2CB00OZDJTKK) allows for swapping out gear ratios to adjust for different leadscrew pitches.
6. TR8 Leadscrew (with 8mm pitch) for Z-Axis
7. Remodeled and removed unnecessary holes from the bridge left and right.
8. Belt holder holds a captive nut to secure it to the bridge.
9. Easily Obtainable [D2826 1000kv](https://www.amazon.com/dp/B089YPSYD6?psc=1&ref=ppx_yo2_dt_b_product_details) BLDC motor.
    * 3d-printed TPU (shore 95A) drive belt.
    * 3d-printed PolyCarbonate drive pulleys. (Eliminated the stop-collar on the spindle)
10. Use of an open cable chain, and necessary mounts to the bridge & electronics case.
    * Use of 40mm screws on the electronics case, further customizations to make it easier to fit panel-mount through-hole jacks.
    * modified the electronics panel to use a rectangular switch and a XT-60H connector for power.
11. 3d printed GT2 pulleys.

### Things I did not 'skimp' on:
* MakerBeam! I bought the starter kit, and the tee nuts. I _did_ investigate printing the tee nuts and had workable facimilies printed with a 0.4mm nozzle but was not confident about the long-term use of the plastic tee-nuts.
* Linear rails. Experience from building 3d printers told me this was worth it.
* LM8UU Drylin Bushings. Because LM8UU ball bearings that are affordable are rubbish. I've been using drylin on my 3d printer for a year or so now, and it's so much quieter, and has so much less lash that it's just worth it.

### Z-Axis Analysis
#### NEMA 8 ANT Head
The "Beta" NEMA 8 stepper head design uses a 1.8-degree, 18mN/m (0.018N/cm) holding torque stepper paired with a 4mm leadscrew with a 0.635mm pitch, equating to a 0.003175mm linear resolution per step.
(0.635mm / 200 steps = 0.003175mm). 

The default configuration is to set this to half-stepping, doubling that resolution to:
(0.635 / 400) = 0.0015875mm

#### NEMA 17 Varnerized Head
I have on hand several 17HS4401 steppers, which are 1.8-degree, 42N/cm holding torque, and TR8x8 leadscrews with 8mm pitch and anti-backlash nuts.
Un-reduced, this equates to a linear resolution of 0.04mm.
(8 / 200 = 0.04mm) 

In order to increase the resolution, I've selected a 16:52 gear ratio and 140 tooth GT2 timing belt (which I had on hand from building a Skelestruder extruder for my printer).
This brings the effective resolution to:
(8 / (200 * 3.25)) = 0.012307692mm

Enabling 1/4 stepping on the Z-axis motor brings this to:
(8 / (200 * 3.25 * 4)) = 0.003076923mm -- slighty better than the half-stepped NEMA 8.

Enabling 1/8 stepping on the Z-axis will yield:
(8 / (200 * 3.25 * 8)) = 0.001538462mm resolution -- on par with the half-stepped NEMA 8 head.

This equates to a theoretical steps/mm setting of: ~650 steps/mm

Assuming a loss of torque of 29% at the motor shaft, the motor will have 29.82N/cm of holding torque.
(42 - (42 * .29))
At the lead-screw, this would be 3.25 times higher.
(29.82 * 3.25) = 89.46N/cm

#### Conclusion
With 1/8th microstepping, a 1:3.25 gear ratio, a 8mm pitch leadscrew, and a stepper driver set to 1/8 stepping, we should exceed the resolution capability and torque output of the ANT's NEMA8 design.


## Original ANT Project resources
* Bitbucket: [CPCBM](https://bitbucket.org/compactpcbmaker/cpcbm/src/master/)
* Youtube Channel: [The Ant PCB Maker](https://www.youtube.com/channel/UCX44z-SSL7LzcB4xxgUdHHA)
* Instagram : [The Ant Team](https://www.instagram.com/the_ant_team/)
* Reddit: [r/TheANT](https://www.reddit.com/r/TheANT/)

### Contact us at: the.ant.pcb@gmail.com or compact.pcb.maker.team@gmail.com

[![alternativetext](https://bitbucket.org/compactpcbmaker/cpcbm/raw/6ffa7937e43088e09a562490447c1323f4919ad3/resources/the_ant_logo/the_ant_logo_small.png =50x20)](https://www.youtube.com/channel/UCX44z-SSL7LzcB4xxgUdHHA)

The Ant is a project to develop a CNC machine able to realize single and double-layered printed circuit boards.

[Here is a video presentation](https://youtu.be/nVkbG-CYaAA)

The CNC machine is designed to achieve well-determined characteristics:

- Compact
- Low-cost
- Robust
- Scalable
- Easy to build
- Open License

To achieve these properties the following design principle have been adopted:

- Electrical, mechanical and electronics parts have been chosen to be small, low-cost and easily available on the open market or 3D printable
- Mechanical design has been oriented to minimize the space occupied by the components
- Mechanics has been designed to be robust and stress resistant
- Mechanics has been designed so that the external dimensions of the machine may be changed with a minimal change of pieces and no re-design needed.
- Mechanical and electrical designs have taken in account the assembling and disassembling operations, allowing to use easily available tools as screwdrivers, Allen wrenches and a solder for electrical contacts.
- The microcontroller board used to control the CNC machine is inexpensive (~15 $) but is equipped with a powerful STM32 processor and many peripherals.
- The machine control firmware can be loaded with almost no effort, since the microcontroller board has been chosen to have an on-board programmer.
- The control firmware is derived from GRBL v.0.9j, but it is also customized to achieve better performance, taking advantage of the powerful processor, and to be more flexible, since more configuration parameters and features have been added respect to the original firmware


The Ant is an open project, but if you feel like donating a small sum, you are very welcome. Just use the paypal button below.

[![paypal](https://bitbucket.org/compactpcbmaker/cpcbm/raw/4311b6ad335d86206ed62cc0bc5e36fd7de749bf/resources/button/pp_button_small.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=BTRCVPZUZYW2E)
