Marlin configuration
===
> This is the configuration tutorial for the `0.4 - Beta` firmware!

The two template configuration files you can start from are in `Marlin-2.0.x/config` folder: 
* [Configuration.h](Marlin-2.0.x/config/SCARA/X-SCARA/Configuration.h) 
* [Configuration_Adv.h](Marlin-2.0.x/config/SCARA/X-SCARA/Configuration_Adv.h)

Coordinate mapping
===
To get a glympse of how the cartesian coordinate system is mapped, we have some examples:

* (X0,Y0) - represents the location where the center of the shoulder axis is;
* (X0,Y186) - the location where the center of the extruder nozzle is, when the arm is perfectly straight (shoulder & elbow angle are 0º);
* (X98,Y98) - -"- when shoulder angle is 0º and elbow angle is 90º;
* (x-98,Y98) - -"- when shoudler angle is -90º and elbow angle is 0º;


Configuration.h
===

#### `X_SCARA`
To enable **X-SCARA** support in **Marlin** you need to define `X-SCARA` and `SCARA` compile time options.

#### `X_SCARA_DEBUG`
Activate extensive tracing to the serial console for debugging the implemention(not needed in production).

#### `SCARA_LINKAGE_1`
Length of shoulder to elbow linkage (for **X_SCARA** this is currently `98` mm).

#### `SCARA_LINKAGE_2`
Length of elbow to tip linkage (for **X_SCARA** this is currently `98` mm).

#### `SCARA_OFFSET_X` and `SCARA_OFFSET_Y`
Bed center offset from the real coordinate mapping (if bed is aligned with machine's center, X will be 0).

Default value for **98mm** linkage lengths is `X`:`0` mm and `Y`:`-110`mm.

> Starting with Firmwre **V0.4** `SCARA_OFFSET_X` and `SCARA_OFFSET_Y` are configurable at run-time using `M665 X Y` G-Code and can be saved/restored to/from EEPROM using `M50x` commands.

#### `SCARA_SEGMENTS_PER_SECOND`
The default number of segments per second that **X-SCARA** will use to segment a linear move.

Tune this value according to your Mainboard's processor. For an 8-bit AVR this can range from `30` to `200`, for a 32-bit ARM processor this value can go even higher. If the movement is choppy when you run `G1` gcode, set this configuration to a lower value (e.g.: for 30 segments per second `M665 D30`).

The higher this value is, the more precise the movements will be, with the cost of more CPU usage.

> Starting with Firmware **V0.4** you can configure this setting at run-time using `M665 D` G-Code, and save it to EEPROM using `M500`.

#### `SCARA_MIN_SEGMENT_LENGTH`
The minimum segment width the **X-SCARA** will use for segmented moved. Default is `0.5` mm.

Setting this value higher, will reduce the kinematic overhead and use less CPU for linear movements, but the prints will have more jagged lines. For better precission, reduce this value to the lowest possible and also raise the number of segments using `M665 D` G-Code command.

> Starting with Firmware **V0.4** you can configure this setting at run-time using `M665 M` G-Code, and save it to EEPROM using `M500`.

#### `SCARA_PROBE_OFFSET_DEGREES`
The angular offset the bed probe is positioned relative to the extruder's nozzle, in degrees. Default is `20` degrees.

**X-SCARA** cant't use the already existing cartesian probe offset configuration in Marlin, because the probe changes orientation when either of the two axis are moving, therefore it also changes the relative position in cartesian space against the extruder's nozzle. This problem only exists in SCARA mechanical models, DELTA, CoreXY and Cartesian printers keep the same X,Y offset of the probe when print head is moving.

To solve this, **X-SCARA** enters in a special `probe` coordinate transformation mode (i.e. `M360 M2`) when required to use the bed probe, applying this offset to the current elbow angle, and instructing the coordinate mapping system to point to the probe, instead of the extruder's nozzle.

Sample configuration that is X-SCARA specific
===
```C
#define SCARA
#define X_SCARA
//#define X_SCARA_DEBUG // activates debug features

#if ENABLED(X_SCARA)

#if ENABLED(X_SCARA)
  // If movement is choppy try lowering this value
  #define SCARA_SEGMENTS_PER_SECOND 30

  // Minimum segment length in mm
  #define SCARA_MIN_SEGMENT_LENGTH  0.5f
  //#define SCARA_FEEDRATE_SCALING

  // Length of inner and outer support arms. Measure arm lengths precisely.
  #define SCARA_LINKAGE_1  98.0    // (mm)
  #define SCARA_LINKAGE_2  98.0    // (mm)
  
  // SCARA tower offset (position of Tower relative to bed zero position)
  #define SCARA_OFFSET_X     0      // (mm)
  #define SCARA_OFFSET_Y    -110    // (mm)

  // Relative probe offset in angles, required for bed leveling (G29)
  #define SCARA_PROBE_OFFSET_DEGREES 20

  // For X-SCARA these are actually degrees not mmm
  // It is useful for homing, to know when the movement exceeds
  // maximum axis length.
  #define X_MAX_LENGTH 300
  #define Y_MAX_LENGTH 300

#endif
#endif
```

Other Marlin configurations
===

Enable saving configuration to EEPROM 
***

I strongly recommend using the EEPROM to save and restore configuration settings.

Otherwise, tuning your machine with `M665` will not allow you to store the resulted values persistently.

```C
/**
 * EEPROM
 *
 * Persistent storage to preserve configurable settings across reboots.
 *
 *   M500 - Store settings to EEPROM.
 *   M501 - Read settings from EEPROM. (i.e., Throw away unsaved changes)
 *   M502 - Revert settings to "factory" defaults. (Follow with M500 to init the EEPROM.)
 */
#define EEPROM_SETTINGS     // Persistent storage with M500 and M501
//#define DISABLE_M503        // Saves ~2700 bytes of PROGMEM. Disable for release!
#define EEPROM_CHITCHAT       // Give feedback on EEPROM commands. Disable to save PROGMEM.
#if ENABLED(EEPROM_SETTINGS)
  //#define EEPROM_AUTO_INIT  // Init EEPROM automatically on any errors.
#endif
```

Centered bed
***

To simplify coordinate mapping **X-SCARA** uses the center of the bed as coordinate origins.
To activate that, enable `BED_CENTER_AT_0_0` in `Configuration.h`.

```C
// The center of the bed is at (X=0, Y=0)
#define BED_CENTER_AT_0_0
```

Bed size
***

In regards of bed size, you can use these settings for safe operation. 

```C
// The size of the print bed
#define X_BED_SIZE 100
#define Y_BED_SIZE 100
```

Steps per unit
===

## X and Y axis
On the **X-SCARA** machine the *X axis* represents the *shoulder joint*, while the *Y axis* represents the *elbow joint*. Therefore, the number of steps per unit describes the number of steps required to move the joint for an angle of 1 degree (unlike the cartesian printers where the unit is 1mm). Depending on the type motors and microstepping configuration you have, the number of steps per degree may differ. 

To speed up your configuration, start from the sample configuration below, wich sets the number of steps for motors having 200 steps/rev and drivers set to 16 microstepping, which is the default for most configurations.

```C
/**
 * X-SCARA: 
 * X - uses 26.6667 steps/degree for moving shoulder (with 16 microsteps)
 * Y - uses 80 steps/degree for moving elbow (with 16 microsteps)
 * Z - uses 400 steps/mm for moving Z (with 16 microsteps)
 * E - adjust according to your extruder (93 is for MK8 extruder)
 */
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 26.6667, 80, 400, 93 }
```

If you increase your microstepping to (let's say) 32, you must double the number of steps for X and Y axis. Same goes if your motors move 400 steps/revolution (0.9 degree per step).

## Generic calculation of steps per unit

If you need the generic formula for calculating the number of steps, this is:

```
shoulder steps  = (shoulder_pulley_teeth / shoulder_motor_pulley_teeth) * number_of_steps_per_revolution / 360

elbow steps     = (elbow_pulley_teeth / elbow_motor_pulley_teeth) * elbow_reduction * number_of_steps_per_revolution / 360

elbow_reduction = elbow_front_pulley_teeth / elbow_back_pulley_teeth
```

In the original design of **X-SCARA** these values are:

Shoulder drive:
* shoulder_pulley_teeth = 60
* shoulder_motor_pulley_teeth = 20

Elbow drive:
* elbow_pulley_teeth = 60
* elbow_motor_pulley_teeth = 20

Elbow reduction:
* elbow_front_pulley_teeth = 60
* elbow_back_pulley_teeth = 20
* elbow_reduction = 3 (60 teeth/20 teeth)

## Let's test the formula:

For a default configuration, having motors with 200 steps per revolution, and 16 microstepping:

```
number_of_steps_per_revolution = 3200 (200 steps per revolution * 16 microstepping)

shoulder steps per unit = 60 / 20 * 3200 / 360      = 9600 / 360 = 26,(6) steps/deg
elbow steps per unit    = 60 / 20 * 3 * 3200 / 360  = 28800 / 360 = 80    steps/deg
```

Or, for example, if you are using 16 teeth pulleys for motors and elbow reduction (instead of 20), motors with 400 steps per revolution, and 32 microstepping, the number of steps will become:

```
number_of_steps_per_revolution = 12800 (400 steps per revolution * 32 microstepping)

shoulder steps per unit = 60 / 16 * 12800 / 360      = 48000 / 360 = 133,(3) steps/deg
elbow steps per unit    = 60 / 16 * 3 * 12800 / 360  = 144000 / 360 = 400    steps/deg
```

## Z and E axis

The Z and E axis are configured similar to any other printer. 

For the Z axis, depending on the lead screw configuration, the formula is:

``` 
Z steps per unit = number_of_steps_per_revolution / lead_screw_mm_per_revolution
```

For the E axis, you need to check your extruder documentation. 

Acceleration and feedrate
***
For the moment, there was no extensive tuning made for this machine to improve print time and travel speed.

The values I use momentarly are to ensure the machine doesn't skip steps and can be increased considerably.

```C

#define DEFAULT_MAX_FEEDRATE          { 300, 300, 15, 30 }
#define DEFAULT_MAX_ACCELERATION      { 1000, 1000, 10, 20000 }

#define DEFAULT_ACCELERATION          1000    // X, Y, Z and E acceleration for printing moves
#define DEFAULT_RETRACT_ACCELERATION  1000    // E acceleration for retracts
#define DEFAULT_TRAVEL_ACCELERATION   1000    // X, Y, Z acceleration for travel (non printing) moves
```

You don't need to modify these values in the configuration file, you may use these as safe settings and tune the machine with `M203` to increase feedrate and `M201` and `M204` to increase acceleration. If the configuration works, save it with `M500` in EEPROM.

Configuration_Adv.h
===
They are no configuration values specific for **X-SCARA** in this file for the moment.