![X-SCARA](images/repository-logo.png)

Objective
===
**X-SCARA** is a project that intends to bring the SCARA mechanical model to the world of open-source 3D printing machines, with the same functionality and ease of use as the other popular mechanical models (e.g.: Cartesian, CoreXY, Delta, etc.).

    Building, configuring and calibrating the X-SCARA machine must be painless, even for the unexperienced.
    That is our goal!

Hardware
===

    Updates: 
    - X-SCARA hardware is now at revision 5!
    - Added test-part accessory to calibrate your printer before you start printing X-SCARA
    - Added heated bed support directly connected to the X-SCARA frame

The hardware section will contain everything you need to build **X-SCARA**, from the *B.O.M.* to STL files and detailed assembly instructions. 

Check the [hardware section here](hardware/README.md) and if you want to check for updates and/or new accessories take a look at the [changelog](hardware/CHANGELOG.md).

Firmware
===
    Updates:
    - X-SCARA firmware version is now 0.4 BETA!
    - All basic functionality (homing, bed-leveling, tuning segmented movements) implemented & working
    - Added coordinate transformation modes to support bed-leveling (`M360 M2` - probe mode)
    - Implemented homing sequence `G28` 
    - Modified UBL (Unified Bed Leveling) to support **X-SCARA**'s kinematic model
    - Integrated bed-leveling support `G29`
    - Applying bed leveling for planar moves when bed-leveling is activated
    - Modified `M665` for configuring machine's settings
    - EEPROM support to save most important configuration options

For firmware build and configuration follow [these instructions](firmware/README.md).

 if you are already using it check the [changelog](firmware/CHANGELOG.md) for any updates.

Calibration procedure
===
Read [here](CALIBRATION.md) more about you can calibrate **X-SCARA** and start printing.

Credits
===
Author and maintainer of this project is *Alex Mircescu*.

X-SCARA parts and assembly were fully designed in *Autodesk Fusion 360*.

