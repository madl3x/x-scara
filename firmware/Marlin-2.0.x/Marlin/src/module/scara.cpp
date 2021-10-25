/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * scara.cpp
 */

#include "../inc/MarlinConfig.h"

#if IS_SCARA

#include "scara.h"
#include "motion.h"
#include "planner.h"

float delta_segments_per_second = SCARA_SEGMENTS_PER_SECOND;

#if ENABLED(X_SCARA)
float delta_min_segment_length = SCARA_MIN_SEGMENT_LENGTH;
#endif//X_SCARA

xyz_pos_t scara_home_offset;

#if ENABLED(X_SCARA)
  xy_pos_t scara_offset = { SCARA_OFFSET_X, SCARA_OFFSET_Y };
#else
  static constexpr xy_pos_t scara_offset = { SCARA_OFFSET_X, SCARA_OFFSET_Y };
#endif

/*
 * X-SCARA definitions 
 */
#if ENABLED(X_SCARA)
#include "../gcode/parser.h"
#include "endstops.h"

float constexpr 
      PK_SumLenSq = sq(L1) + sq(L2),
      PK_ProdOfLen = 2 * L1 * L2,
      PK_DifLenSqrd = sq(L1) - sq(L2);

  #if ENABLED(X_SCARA_DEBUG)
    #ifndef X_SCARA_DEBUG_START_VERBOSE
    #define X_SCARA_DEBUG_START_VERBOSE false
    #endif
    bool x_scara_debug = X_SCARA_DEBUG_START_VERBOSE;
  #endif //X_SCARA_DEBUG

uint8_t x_scara_coordinates_mode = X_SCARA_COORDINATES_CARTESIAN;

static FORCE_INLINE void x_scara_fwdk(const float &a, const float &b);
static FORCE_INLINE void x_scara_invk(const xyz_pos_t &raw);

#endif

#if EITHER(MP_SCARA, MORGAN_SCARA)
// Float constants for SCARA calculations
float constexpr L1 = SCARA_LINKAGE_1, L2 = SCARA_LINKAGE_2,
                L1_2 = sq(float(L1)), L1_2_2 = 2.0 * L1_2,
                L2_2 = sq(float(L2));
#endif


void scara_set_axis_is_at_home(const AxisEnum axis) {

  if (axis == Z_AXIS) {
    current_position.z = Z_HOME_POS;
    return;
  }

  #if ENABLED(MORGAN_SCARA)
    /**
     * SCARA homes XY at the same time
     */
    xyz_pos_t homeposition;
    LOOP_XYZ(i) homeposition[i] = base_home_pos((AxisEnum)i);
    // MORGAN_SCARA uses arm angles for AB home position
    // SERIAL_ECHOLNPAIR("homeposition A:", homeposition.a, " B:", homeposition.b);
    inverse_kinematics(homeposition);
    forward_kinematics_SCARA(delta.a, delta.b);
    current_position[axis] = cartes[axis];
  #endif
  #if ENABLED(MORGAN_SCARA)
    // MP_SCARA uses a Cartesian XY home position
    // SERIAL_ECHOPGM("homeposition");
    // SERIAL_ECHOLNPAIR_P(SP_X_LBL, homeposition.x, SP_Y_LBL, homeposition.y);
    current_position[axis] = homeposition[axis];
    // SERIAL_ECHOPGM("Cartesian");
    // SERIAL_ECHOLNPAIR_P(SP_X_LBL, current_position.x, SP_Y_LBL, current_position.y);
  #endif
  #if ENABLED(X_SCARA)

    // switch to angular positioning
    uint8_t old_mode = x_scara_coordinates_mode;
    x_scara_change_coordinates_mode(X_SCARA_COORDINATES_RELATIVE_ANGLES);

    if (axis == X_AXIS) {
      X_SCARA_DEBUG_LNPAIR("[home] shoulder hit endstop, moving it to home");

      current_position.x = -90 - scara_home_offset.a;
      sync_plan_position();

      do_blocking_move_to_x(0, homing_feedrate(axis) * 3/2);

    } else if (axis == Y_AXIS) {
      X_SCARA_DEBUG_LNPAIR("[home] elbow hit endstop, moving it to home");

      current_position.y = -90 - scara_home_offset.b;
      sync_plan_position();

      do_blocking_move_to_y(0, homing_feedrate(axis) * 3/2);

      if (old_mode != X_SCARA_COORDINATES_LOCK_XY) {
        X_SCARA_DEBUG_LNPAIR("[home] moving to origin");
        x_scara_change_coordinates_mode(X_SCARA_COORDINATES_CARTESIAN);
        do_blocking_move_to_xy(0, 0, homing_feedrate(axis) * 3/2);
      }
    }

    // switch back to the previous mode
    x_scara_change_coordinates_mode(old_mode);
  #endif

  update_software_endstops(axis);
}
/**
 * SCARA Forward Kinematics. Results in 'cartes'.
 */
void forward_kinematics_SCARA(const float &a, const float &b) {

  #if ENABLED(X_SCARA)

  /*
   * X_SCARA_COORDINATES_CARTESIAN - default transformation mode
   * This is the default transformation mode to be used when printing.
   */
  if(x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN) {
    x_scara_fwdk(a,b);
  }
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER) {
    x_scara_fwdk(a,b); // do nothing special on fwdk
  } 
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_LOCK_XY) {
    // do nothing
  }
  /*
   * X_SCARA_COORDINATES_RELATIVE_ANGLES - angle transformation mode
   * To be used for testing the machine, coordinates are representing 
   * joint relative angles.
   */
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_RELATIVE_ANGLES) {
    cartes.set(a, b-a/X_SCARA_ELBOW_CROSSTALK_RATIO);
  }
      
  #else
  /* MORGAN_SCARA
   * Maths and first version by QHARLEY.
   * Integrated into Marlin and slightly restructured by Joachim Cerny.
   */
 
  const float a_sin = sin(RADIANS(a)) * L1,
              a_cos = cos(RADIANS(a)) * L1,
              b_sin = sin(RADIANS(b)) * L2,
              b_cos = cos(RADIANS(b)) * L2;

  cartes.set(a_cos + b_cos + scara_offset.x,  // theta
             a_sin + b_sin + scara_offset.y); // theta+phi

  /*
    SERIAL_ECHOLNPAIR(
      "SCARA FK Angle a=", a,
      " b=", b,
      " a_sin=", a_sin,
      " a_cos=", a_cos,
      " b_sin=", b_sin,
      " b_cos=", b_cos
    );
    SERIAL_ECHOLNPAIR(" cartes (X,Y) = "(cartes.x, ", ", cartes.y, ")");
  */
  #endif
}

void inverse_kinematics(const xyz_pos_t &raw) {

  /*
   * X-SCARA Inverse Kinematics
   */
  #if ENABLED(X_SCARA)

  /* X_SCARA_COORDINATES_CARTESIAN - default transformation*/
  if(x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN) {
    x_scara_invk(raw);
  } 
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER) {
    x_scara_invk(raw); // nothing special
  }
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_LOCK_XY) {
    // update only the Z position, keep XY the same
    delta.z = raw.z;

    X_SCARA_DEBUG_LNPAIR(
      "INVK X:",raw.x," Y:", raw.y, " -> S:", DEGREES(delta.a)," E:", DEGREES(delta.b), " (locked)");
  }
  /* X_SCARA_COORDINATES_RELATIVE_ANGLES - X,Y are now angles */
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_RELATIVE_ANGLES) {
    delta.set(raw.x, raw.y + raw.x/X_SCARA_ELBOW_CROSSTALK_RATIO, raw.z);

    X_SCARA_DEBUG_LNPAIR(
      "INVK X:",raw.x," Y:", raw.y, " -> S:", DEGREES(delta.a)," E:", DEGREES(delta.b));
  }

  #endif

  #if ENABLED(MORGAN_SCARA)
    /**
     * Morgan SCARA Inverse Kinematics. Results in 'delta'.
     *
     * See http://forums.reprap.org/read.php?185,283327
     *
     * Maths and first version by QHARLEY.
     * Integrated into Marlin and slightly restructured by Joachim Cerny.
     */
    float C2, S2, SK1, SK2, THETA, PSI;

    // Translate SCARA to standard XY with scaling factor
    const xy_pos_t spos = raw - scara_offset;

    const float H2 = HYPOT2(spos.x, spos.y);
    if (L1 == L2)
      C2 = H2 / L1_2_2 - 1;
    else
      C2 = (H2 - (L1_2 + L2_2)) / (2.0f * L1 * L2);

    S2 = SQRT(1.0f - sq(C2));

    // Unrotated Arm1 plus rotated Arm2 gives the distance from Center to End
    SK1 = L1 + L2 * C2;

    // Rotated Arm2 gives the distance from Arm1 to Arm2
    SK2 = L2 * S2;

    // Angle of Arm1 is the difference between Center-to-End angle and the Center-to-Elbow
    THETA = ATAN2(SK1, SK2) - ATAN2(spos.x, spos.y);

    // Angle of Arm2
    PSI = ATAN2(S2, C2);

    delta.set(DEGREES(THETA), DEGREES(THETA + PSI), raw.z);

    /*
      DEBUG_POS("SCARA IK", raw);
      DEBUG_POS("SCARA IK", delta);
      SERIAL_ECHOLNPAIR("  SCARA (x,y) ", sx, ",", sy, " C2=", C2, " S2=", S2, " Theta=", THETA, " Phi=", PHI);
    //*/

  #endif //MORGAN_SCARA

  #if ENABLED(MP_SCARA)

    const float x = raw.x, y = raw.y, c = HYPOT(x, y),
                THETA3 = ATAN2(y, x),
                THETA1 = THETA3 + ACOS((sq(c) + sq(L1) - sq(L2)) / (2.0f * c * L1)),
                THETA2 = THETA3 - ACOS((sq(c) + sq(L2) - sq(L1)) / (2.0f * c * L2));

    delta.set(DEGREES(THETA1), DEGREES(THETA2), raw.z);

    /*
      DEBUG_POS("SCARA IK", raw);
      DEBUG_POS("SCARA IK", delta);
      SERIAL_ECHOLNPAIR("  SCARA (x,y) ", x, ",", y," Theta1=", THETA1, " Theta2=", THETA2);
    //*/

  #endif // MP_SCARA
}

/*
 * X_SCARA implementation
 */
#ifdef X_SCARA

const char * x_scara_coordinates_mode_str [] = {
  "cartesian",  // X_SCARA_COORDINATES_CARTESIAN
  "angular",    // X_SCARA_COORDINATES_RELATIVE_ANGLES
  "probe",      // X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER
  "lock"        // X_SCARA_COORDINATES_LOCK_XY
};

/* called upon machine initialization */
void x_scara_begin() {
  SERIAL_ECHOPGM("\n===\nX-SCARA ");
  SERIAL_ECHOPGM(X_SCARA_VERSION_STR);
  SERIAL_ECHOPGM("\n===\n");
}

void x_scara_run_gcode(_gcode_fn_t gcode_run){

  if (parser.command_letter != 'G') return; // should never happen

  uint8_t previous_mode = x_scara_coordinates_mode;

  X_SCARA_DEBUG_LNPAIR("GCD  ", parser.command_letter, "", parser.codenum);

  switch(parser.codenum) {
    case 29:
      // G29 must be run in probe mode
      x_scara_change_coordinates_mode(X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER);
      gcode_run();
      x_scara_change_coordinates_mode(previous_mode);
      break;
    case 28:
      // G28 must be run in angular mode
      x_scara_change_coordinates_mode(X_SCARA_COORDINATES_RELATIVE_ANGLES);
      gcode_run();
      x_scara_change_coordinates_mode(previous_mode);
      break;
  }
}


/* forward kinematics transformation */
static  FORCE_INLINE void x_scara_fwdk(const float &a, const float &b) {
    const float S = a;
    const float E = b-a/X_SCARA_ELBOW_CROSSTALK_RATIO;

    const float a_sin = - sin(RADIANS(S)) * L1,
                a_cos =   cos(RADIANS(S)) * L1,
                b_sin = - sin(RADIANS(S+E)) * L2,
                b_cos =   cos(RADIANS(S+E)) * L2;

    cartes.set(a_sin + b_sin + scara_offset.x,
               a_cos + b_cos + scara_offset.y); 

    X_SCARA_DEBUG_LNPAIR(
      "FWDK S:", S," E:", E," -> X:", cartes.x," Y:", cartes.y);
}

/*  inverse kinematics transformation */
static FORCE_INLINE void x_scara_invk(const xyz_pos_t &raw) {

    const float x = - (raw.x - scara_offset.x);
    const float y =   (raw.y - scara_offset.y);

    float x2y2 = sq(x) + sq(y);
    float hypot = SQRT(x2y2);

    if(hypot == 0) hypot = 0.000001; // avoid division by zero

    float S = ATAN2(x,y) - ACOS((x2y2 + PK_DifLenSqrd )/(2*L1*hypot));
    float E =              ACOS((x2y2 - PK_SumLenSq   )/(PK_ProdOfLen)) + S/X_SCARA_ELBOW_CROSSTALK_RATIO;

    delta.set(DEGREES(S), DEGREES(E), raw.z);

    X_SCARA_DEBUG_LNPAIR(
      "INVK X:",raw.x," Y:", raw.y, " -> S:", DEGREES(S)," E:", DEGREES(E));
}

/* move joints to the given angular position */
void x_scara_change_coordinates_mode(uint8_t coordinates_mode) {

  if (coordinates_mode == x_scara_coordinates_mode) 
    return; // same mode, ignore

  if (coordinates_mode >= X_SCARA_COORDINATES_MODE_END)
    return; // invalid mode, ignore

  X_SCARA_DEBUG_LNPAIR(
    "MODC ", x_scara_coordinates_mode_str[x_scara_coordinates_mode], 
    "->", x_scara_coordinates_mode_str[coordinates_mode]);

  // position from steppers
  ab_pos_t cur_pos = {
    planner.get_axis_position_degrees(A_AXIS),
    planner.get_axis_position_degrees(B_AXIS)
  };

  #if HAS_BED_PROBE
  /*
   * X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER
   *  move the current position to an elbow position shifted by  
   *  SCARA_PROBE_OFFSET_DEGREES
   */
  if (x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER) {
    cur_pos.b += SCARA_PROBE_OFFSET_DEGREES;
  } else 
  if (coordinates_mode == X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER) {
    cur_pos.b -= SCARA_PROBE_OFFSET_DEGREES;
  }
  #else
    if (coordinates_mode == X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER) {
      SERIAL_ECHOLNPGM("No bed probe, ignoring mode");
      return;
    }
  #endif//HAS_BED_PROBE

  // change coordinates mode
  x_scara_coordinates_mode = coordinates_mode;

  // update transformed position
  forward_kinematics_SCARA(cur_pos.a, cur_pos.b);
  current_position.set(cartes.x, cartes.y);

  // sync and report
  sync_plan_position();
}

/* move joints to the given angular position */
void x_scara_move_joints(ab_pos_t & pos, const feedRate_t &fr_mm_s) {

  x_scara_relative_to_delta(pos);
  X_SCARA_DEBUG_LNPAIR(
    "MOVE S:", pos.a, " E:", pos.b);
    
  forward_kinematics_SCARA(pos.a, pos.b);
  x_scara_delta_to_relative(pos);

  do_blocking_move_to_xy(cartes, fr_mm_s);
}

/* get current relative position of joints in degrees */
void  x_scara_get_joints_position(ab_pos_t & pos) {

  pos.a = planner.get_axis_position_degrees(A_AXIS);
  pos.b = planner.get_axis_position_degrees(B_AXIS);

  x_scara_delta_to_relative(pos);
}


#endif // X_SCARA

void scara_report_positions() {

  #ifdef X_SCARA
    ab_pos_t angles = {
      planner.get_axis_position_degrees(A_AXIS),
      planner.get_axis_position_degrees(B_AXIS)
    };

    x_scara_delta_to_relative(angles);
    SERIAL_ECHOLNPAIR("Joints S:", angles.a, " E:", angles.b);
  SERIAL_EOL();
  #else 
  SERIAL_ECHOLNPAIR("SCARA Theta:", 
    planner.get_axis_position_degrees(A_AXIS), "  Psi+Theta:", 
    planner.get_axis_position_degrees(B_AXIS));
  SERIAL_EOL();
  #endif
}

#endif // IS_SCARA
