Marlin configuration
===
The two template configuration files you can start from are in `Marlin-2.0.x/config` folder: 
* [Configuration.h](Marlin-2.0.x/config/SCARA/X-SCARA/Configuration.h) 
* [Configuration_Adv.h](Marlin-2.0.x/config/SCARA/X-SCARA/Configuration_Adv.h)

Configuration.h
===
Enable X-SCARA implementation
***
To enable **X-SCARA** support in **Marlin** you need to define `X-SCARA` and `SCARA` compile time options.

#### `X_SCARA_DEBUG`
Activate extensive tracing to the serial console for debugging the implemention(not needed in production)

#### `SCARA_LINKAGE_1`
Length of shoulder to elbow linkage (for **X_SCARA** this is 98mm)

#### `SCARA_LINKAGE_2`
Length of elbow to tip linkage (for **X_SCARA** this is 98mm)

#### `SCARA_OFFSET_X` and `SCARA_OFFSET_Y`
Bed center offset from the real coordinate mapping (if bed is at center, X will alwayse be 0).

To get a glympse of how the cartesian coordinate system is mapped, we have some examples:

* (X0,Y0) - represents the location where the center of the shoulder axis is
* (X0,Y186) - represents the location where the center of the extruder nozzle is, when the arm is perfectly straight (shoulder angle is 0º, elbow is 0º)
* (X98,Y98) - represents the location where the center of the extruder nozzle is, when shoulder angle is 0º and elbow angle is 90º
* (x-98,Y98) - represents the location where the center of the extruder nozzle is, when shoudler angle is -90º and elbow angle is 0º


#### `SCARA_SEGMENTS_PER_SECOND`
The number of segments per second that SCARA will use to execute an actual move.


```C
#define SCARA
#define X_SCARA
//#define X_SCARA_DEBUG // activates debug features

#if ENABLED(X_SCARA)
  #define SCARA_SEGMENTS_PER_SECOND 100
  #define SCARA_LINKAGE_1  98.0    // (mm)
  #define SCARA_LINKAGE_2  98.0    // (mm)
  
  // SCARA tower offset (position of Tower relative to bed zero position)
  #define SCARA_OFFSET_X     0       // (mm)
  #define SCARA_OFFSET_Y     -110     // (mm)

#endif
```
Enable saving configuration to EEPROM 
***

I strongly recommend using the EEPROM to save and restore configuration settings.

Currently, X-SCARA doesn't use the EEPROM to save any configuration values, but it will surely do in the future for configuring homing offsets.

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
#define X_BED_SIZE 240
#define Y_BED_SIZE 80
```

Homing 
***
Current version of **X-SCARA** implementation does not use Endstops on X and Y axis. 
Disable `VALIDATE_HOMING_ENDSTOPS` so the machine does not panic when `G28` is run.

```C
// Validate that endstops are triggered on homing moves
//#define VALIDATE_HOMING_ENDSTOPS
```

Steps per unit
***

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