Versioning scheme
===
Being work in progress, a versioning scheme will is necessary to identify which parts are part of a working set and which not. 
The versioning scheme used is:

    partname_major_minor_revision.STL

  - all parts with the same major version are compatible
  - increasing minor version indicates a change in the part's geometry
  - any estetical changes will not increase the minor version, just the revision
  - revision is incremented within each update on any X-SCARA part

Revision 0
===
- Initial release of X-SCARA frame
- Initial release of X-SCARA mechanics

Revision 1
===
- Typo in part `Frame_Plate_Side_RodClamp_Left`
- Added new frame part mirror `Frame_Plate_Side_RodClamp_Right` which is the mirror version of `Frame_Plate_Side_RodClamp_Left` but without mirrored text
- Corrected extruder mount height

Revision 2
===
- Extruder mount redesigned, probe is now on the left side of the extruder at 20º offset (required by the next version of firmware)
- Updated arm documentation to describe the assembly for the new extruder mount
- Corrected the width of M3 nut holes to be same for all parts, arm and frame (some parts had larger holes for fitting M3 nuts)
- Renamed `Frame_Plate_Side_RodClamp_Left` to `Frame_Plate_Side_Left`
- Renamed `Frame_Plate_Side_RodClamp_Right` to `Frame_Plate_Side_Right`

Revision 3
===
- Added endstop part for holding 5x5 magnets

Revision 4
===
- Added accessory test part [here](accessories/test-part/README.md)