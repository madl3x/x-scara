0.1 Alpha
===
- First public release
- Coordinate transformation system ready
- Homing not supported but machine can be manually homed and calibrated with `M362` GCode
- `M360` command for changing coordinate transformation mode from cartesian to angular positioning
- `M361` command for setting joint angles individually, for shoulder and elbow
- `M362` command for setting current position (similar to `G92`)
- `M364` command for getting current X-SCARA version

0.2 Alpha
===
- Inverting X axis direction, parts were printed in mirror

0.4 Beta
===
- Moved to Beta stage: all basic functionality (homing, bed-leveling, EEPROM configuration) implemented & working
- Added coordinate transformation modes to support bed-leveling (`M360 M2` - probe mode)
- Activating `M1` angular coordinate transformation mode automatically on `G28`
- Activating `M2` probe coordinate transformation mode automatically on `G29`
- Implemented homing sequence `G28` (X,Y axis are set as codependent by default, shoulder homes first)
- Modified UBL (Unified Bed Leveling) to support X-SCARA's kinematic model
- Integrated bed-leveling support `G29` (requires `NOZZLE_AS_PROBE` and `SCARA_PROBE_OFFSET_DEGREES` configurations)
- Applying bed leveling for planar moves when bed-leveling is activated
- Modified `M665` for configuring:
    * `M665 D` - number of segments per second for `G1` segmented moves
    * `M665 M` - minimum segment length in mm for segmented moves
    * `M665 X Y` - setting bed center origin relative to shoulder axis (default `SCARA_OFFSET_X` and `SCARA_OFFSET_X`)
    * `M665 E S` - setting home offset in degrees for shoulder and elbow joints (required for precise calibration)
- `M50x` EEPROM commands supported to save, load and reset the values configured with `M665`
