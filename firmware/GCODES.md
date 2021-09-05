GCodes
===
The following document documents the **Gcodes** implemented or modified by **X-SCARA** implementation

X-SCARA Specific GCodes
===

M360 - X-SCARA - Change Coordinates Transformation
----

```
 Parameters:
 
 M<int>  - transformation mode (0 or 1)
 
            0 - cartesian (default) 
                current X and Y coords. are cartesian coords. pointing to extruder's nozzle
 
            1 - angular (used by G28, when homing) 
                current X and Y coords. are angular coords. of shoulder and elbow position)

            1 - probe (used by G29, when leveling bed) 
                current X and Y are cartesian cords pointing to probe's center
```

This command will instruct the **X-SCARA** machine to switch the entire coordinate mapping system to cartesian, angular or probe mode.

> Use this explicitly only for testing and calibration!

#### `Cartesian mode`

In cartesian mode **X-SCARA** will automatically transform the `X` and `Y` coordinates to angular positions for the shoulder and elbow joints, using inverse kinematics. This is the normal mode, using during printing.

#### `Angular mode`

The `M360 M1` command allows you to move either the shoulder, or the elbow to an angle you expect and observe the result, usign standard GCode commands (e.g. `G0`, `G1` or `G92`). For example, after invoking `M360 M1` you can move the arm's shoulder to 0º using `G0 X0` or the elbow to 90º using `G0 Y90`. 

Used internally when homing with `G28`.

#### `Probe mode`

The `M360 M2` command will switch to a transformation mode, similar to `M360 M0`, but this time pointing to the probe's center, instead of the extruder's nozzle. Used internally by bed-leveling `G29` commands. 

Used internally when running bed-leveling commands with `G29`.

M361 - X-SCARA - Move joints to angular position
----

```
 Parameters:
 
 S<float>  - Move to given shoulder angle (in degrees)
 E<float>  - Move to given elbow angle (in degrees)
 I<booL>   - Add the given to the current position instead of setting them
```

This command is useful to move the machine's joints to a specific angular position. 
When invoked, it will report back the angular position of each joint and the coresponding cartesian coordinates, even if run without arguments.

M362 - X-SCARA - Set current joints position
----
```
 Parameters:
 
 S<float>  - Set the shoulder angle to this value (in degrees)
 E<float>  - Set the elbow angle to this value (in degrees)
```

This command is the complement of `G92` command for angular positioning. `G92` sets the current absolute position for X,Y,Z axis and even E(xtruder), to a given value while `M362` sets the current position of joints, to a given value.

For example, to reset the machine back to its correct position, you could move the arm perfectly straight and then run:

```
M362 E0 S0
```
This command tells the machine the current position is 0º angle for both shoulder and elbow joints. After that, you can move the machine safely in cartesian space.

Please note that the same effect can be obtained using `G92`, after changing the coordinate system to angular positioning using `M360 M1`. 

> Soon to become deprecated! Same as `G92` when using `M360 M1`.

M364 - X-SCARA - Get X-SCARA version
----
```
 Parameters:
 
 D<bool>  - if `X_SCARA_DEBUG` is configured, enable or disabled verbosity
```

Prints the current version of **X-SCARA** implementation.

If `X_SCARA_DEBUG` is enabled in `Configuration.h` enables detailed log tracing using `M364 D1`.

Modified Marlin GCodes
===

G0/G1 - Move to destination
---

**X-SCARA** uses the `G0` command to move directly to destination (without segmented moves) and `G1` command to move to destination using multiple segmented moves.

To configure the number of segments for each move, use `M665 D` command to set the number of segments per second and `M665 M` command to set the minimum segment length. 

> If the `G1` response is choppy and the linear movement is not fluent, lower the number of segments and/or increase the minimum segment length (e.g. `M665 D30 M0.5`).

Check [Marlin documentation](https://marlinfw.org/docs/gcode/G000-G001.html) for more details. 

G28 - Home axis
---

**X-SCARA** will home the `Z` axis using bed-probe and the `X` and `Y` using magnetic endstops. 

The endstops for `X` (shoulder) and `Y` (elbow) axis must be positioned at `-90` degrees.

> When the axis is at `-90` degrees, it point towards machine's `right`. </br>
> When the axis is at `90` degrees, it point  towards machine's  `left`. </br>
> When the axis is at `0` degrees, it point  towards `forward`.

#### XY Homing sequence

The XY homing sequence is designed to avoid crashing the machine as much as possible.

The two axis are marked as `codependent` and can't be homed independently. Running `G28 X` or `G28 Y` or `G28 XY` will have the same effect.

The homing sequence is more elaborate for **X-SCARA** machine, and it happens as follows:

* (1) - Coordinate transformation is set to `angular` (`M360 M1`)
* (2) - X axis (shoulder) moves clockwise until it hits the endstop positioned at `-90º`
* (3) - X axis applies configured home offset (via `M665 X`)
* (4) - X axis moves back to 0º position
* (5) - Y axis (elbow) moves clockwise until it hits the endstop position at `-90º`
* (6) - Y axis applies hconfigured home offset ( via `M665 Y`)
* (7) - Y axis moves back to 0º position
* (8) - Coordinate transformation is set back to `cartesian` (`M360 M0`)
* (9) - Axis are marked as homed
* (8) - Moving to bed center (`X`0 and `Y`0 in cartesian space)


M665 - Set X-SCARA settings
----
```
 Parameters:
 
 D<seg-secs>  - sets the number of segments per seconds for linear moves (default is SCARA_SEGMENTS_PER_SECOND)
 M<seg-min>   - sets the minimum segment length in mm for linear moves (default is SCARA_MIN_SEGMENT_LENGTH)
 S<offset>    - sets the shoulder home offset in degrees (to calibrate homing)
 E<offset>    - sets the elbow home offset in degrees (to calibrate homing)
 X<origin>    - sets the offset on the X axis for bed center position (usually 0, default SCARA_OFFSET_X)
 Y<origin>    - sets the offset on the Y axis for bed center position (default SCARA_OFFSET_Y)
```

This command is used to configure *Delta* printers and also *Scara* machine in *Marlin* Firmware. 
Any of these values can be saved and restored to/from EEPROM using `M50x` G-Code commands.

#### `Tuning segmented moves`
Tune linear movements using `M665 D` and `M665 M` options. If the movement is choppy, lower the number of segments to a more reasonable value (e.g.: `30`) and/or increase the minimum segment (e.g.: to `0.5`).

#### `Aligning bed's center`

If the machine doesn't point at the bed's center correctly, use `M665 X Y` to setup the offset the current known center and save the value in EEPROM using `M500`. For example, running `M665 X-5` will move the bed's center to the right by `5` mm.

#### `Calibrating home position`

This is the one of the most important configurations for **X-SCARA**: **calibrating the home position**.

When setting the endstops for the `X` (shoulder) and `Y` (elbow) axis, most likely you will not be able to mechanically position those at perfectly `-90º` angles. The `M665 X Y` command will allow you to do fine adjustements and save those to EEPROM.

Follow this steps to calibrate the home position:
* (1) - position the `X` and `Y` endstops (the best you can) at `-90º` position
* (2) - home the machine using `G28 XY`
* (3) - move the arm to point forward by running  `M361 S0 E0` (i.e.: set elbow and shoulder angular position to 0º)
* (4) - use the same `M361` command to adjust the angular position for both axis so that the arm gets perfectly straight
* (5) - get the resulted positions and store them as offsets them using `M665 S E`
* (7) - home again using `G28 XY`
* (8) - repeat (2)-(8) until perfect alignment is achieved
* (9) - save the resulted values to EEPROM using `M500`

> Note: bad alignment of shoulder axis will not result in deformed parts, but **improper positioning of the elbow axis will do**.
> 