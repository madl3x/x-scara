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