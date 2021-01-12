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
                X and Y are cartesian coordinates
 
            1 - angular (for testing and calibration) 
                X and Y are shoulder and elbow angles respectively)
```

This command will instruct the **X-SCARA** machine to switch the entire coordinate mapping system to cartesian or plain angular.

In cartesian mode **X-SCARA** will transform the X and Y coordinates to angular positions for the shoulder and elbow joints, using inverse kinematics. To test the machine, it is sometimes useful to bypass that and have direct access to that angular position of each individual joint. 

This `M360 M1` command allows you to move either the shoulder, or the elbow to an angle you expect and observe the result, usign nothing but standard GCode commands. For example, after invoking `M360 M1` you can move the arm's shoulder to 0º using `G0 X0` or the elbow to 90º using `G0 Y90`.

It is also useful to use some specialized software (like Repetier, Octoprint, Pronterface, etc.) to move the machine's joints to the desired position, using their own user-interface.

After you invoke `M360 M1` the machine will treat each later Gcode command, using X or Y coordinates, as angular positions. It applies to `G0`, `G1`, `G92` or others. It is not useful for regular printing, so after you test the machine, revert the coordinates back with `M360 M0`.

Examples:
```
# make sure we are in cartesian mode
> M360 M0
Mode:0
ok

# report current position
> G92
X:32.61 Y:11.69 Z:30.00 E:0.00
Motors A:-933 B:7067 Z:12000
Joints S:-34.99 E:100.00

ok

# change to angular mode (not how joints angle and X,Y are now egual)
> M360 M1
X:-34.99 Y:100.00 Z:30.00 E:0.00
Motors A:-933 B:7067 Z:12000
Joints S:-34.99 E:100.00

Mode:1
ok

# G92 will report the same as M360
G92
X:-34.99 Y:100.00 Z:30.00 E:0.00
Motors A:-933 B:7067 Z:12000
Joints S:-34.99 E:100.00

ok

# move shoulder to -90 degrees
> G0 X-90
ok

# change back to cartesian mode
M360 M0
X:-80.98 Y:-13.49 Z:30.00 E:0.00
Motors A:-2400 B:5600 Z:12000
Joints S:-90.00 E:100.00

Mode:0
ok

 # move to 90,0 coordinates
> G0 X90 Y0
ok
# report position
> G92
X:90.00 Y:0.00 Z:30.00 E:0.00
Motors A:-113 B:6850 Z:12000
Joints S:-4.24 E:87.04
```

M361 - X-SCARA - Move joints to angular position
----

```
 Parameters:
 
 S<float>  - Move to given shoulder angle (in degrees)
 E<float>  - Move to given elbow angle (in degrees)
 I<booL> - Add the given to the current position instead of setting them
```

This command is useful to move the machine's joints to a specific angular position. 
When invoked, it will report back the angular position of each joint and the coresponding cartesian coordinates. 

Examples:
```
# report angular position and cartesian coordinates
> M361
S:0.00 E:0.00 X:0.00 Y:86.00
ok

# move elbow to 90º
> M361 E90
S:0.00 E:90.00 X:98.00 Y:-12.00
ok

# move shoulder to -45º
> M361 S-45
S:-45.00 E:90.00 X:-0.00 Y:28.59
ok

# incrementally move shoulder and elbow 10º more
> M361 S10 E10 I1
S:-35.00 E:100.00 X:32.61 Y:11.69
ok
```

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

Same effect can be obtained using `G92` by changing the coordinate system to angular positioning, but implies combining more commands:
```
# X,Y are now angular positions 
> M360 M1
X:-55.87 Y:111.72 Z:0.00 E:0.00
Motors A:-1490 B:7448 Z:0
Joints S:-55.87 E:111.72

Mode:1
ok

# Reset arm position to 0º
> G92 X0 Y0
X:0.00 Y:0.00 Z:0.00 E:0.00
Motors A:0 B:0 Z:0
Joints S:0.00 E:0.00

# Move back to cartesian space
> M360 M0
X:0.00 Y:86.00 Z:0.00 E:0.00
Motors A:0 B:0 Z:0
Joints S:0.00 E:0.00

Mode:0
ok
```

M364 - X-SCARA - Get X-SCARA version
----

Prints the current version of **X-SCARA** implementation.

```
> M364
0.1 Alpha
ok
```

Modified Marlin GCodes
===

G0/G1 - Move to destination
---

**X-SCARA** uses `G0` command to move directly to destination (without intermittent moves) and `G1` command to move to destination using multiple segmented moves.

To configure the number of segments for each move, use `M665` command to set the number of segments per second and the `SCARA_SEGMENTS_PER_SECOND` configuration option to set the default value. 

Check [Marlin documentation](https://marlinfw.org/docs/gcode/G000-G001.html) for more details. 

G28 - Home axis
---

**X-SCARA** will home the *Z axis* just like any other printer. 

For the X and Y axis (currently) the firmware will set the home location in place, to the current machine position, without moving or checking endstops.

M665 - Set X-SCARA settings
----
```
 Parameters:
 
 D<bool>  - debug on or off (requires X_SCARA_DEBUG configuration option)
 S<int>   - number of segments per second
```

This command is used to configure *Delta* printers and also *Scara* machine in *Marlin* Firmware. For **X-SCARA** it is currently use to active debug tracing (when `X_SCARA_DEBUG` is enabled in `Configuration.h`) and set the number of segments per second.

Default value for segments per second is 100, but if the movement is too choppy, you can lower the value. After you set it to the correct value, use `M500` to save the configuration.

Examples:
```
> M665 D1 S100
Debug:1 Seg/Sec:100.00
ok
```