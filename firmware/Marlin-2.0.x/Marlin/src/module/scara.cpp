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
static constexpr xy_pos_t scara_offset = { SCARA_OFFSET_X, SCARA_OFFSET_Y };

/*
 * X-SCARA definitions 
 */
#if ENABLED(X_SCARA)
float constexpr 
      PK_SumLenSq = sq(L1) + sq(L2),
      PK_ProdOfLen = 2 * L1 * L2,
      PK_DifLenSqrd = sq(L1) - sq(L2);

  #if ENABLED(X_SCARA_DEBUG)
    bool x_scara_debug = false;
  #endif //X_SCARA_DEBUG

uint8_t x_scara_coordinates_mode = X_SCARA_COORDINATES_CARTESIAN;

#include "../gcode/parser.h"
#endif

#if EITHER(MP_SCARA, MORGAN_SCARA)
// Float constants for SCARA calculations
float constexpr L1 = SCARA_LINKAGE_1, L2 = SCARA_LINKAGE_2,
                L1_2 = sq(float(L1)), L1_2_2 = 2.0 * L1_2,
                L2_2 = sq(float(L2));
#endif


void scara_set_axis_is_at_home(const AxisEnum axis) {
  if (axis == Z_AXIS)
    current_position.z = Z_HOME_POS;
  else {

    /**
     * SCARA homes XY at the same time
     */
    xyz_pos_t homeposition;
    LOOP_XYZ(i) homeposition[i] = base_home_pos((AxisEnum)i);

    #if ENABLED(MORGAN_SCARA) || ENABLED(X_SCARA)
      // MORGAN_SCARA uses arm angles for AB home position
      // SERIAL_ECHOLNPAIR("homeposition A:", homeposition.a, " B:", homeposition.b);
      inverse_kinematics(homeposition);
      forward_kinematics_SCARA(delta.a, delta.b);
      current_position[axis] = cartes[axis];
    #else
      // MP_SCARA uses a Cartesian XY home position
      // SERIAL_ECHOPGM("homeposition");
      // SERIAL_ECHOLNPAIR_P(SP_X_LBL, homeposition.x, SP_Y_LBL, homeposition.y);
      current_position[axis] = homeposition[axis];
    #endif

    // SERIAL_ECHOPGM("Cartesian");
    // SERIAL_ECHOLNPAIR_P(SP_X_LBL, current_position.x, SP_Y_LBL, current_position.y);
    update_software_endstops(axis);
  }
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

    const float S = a;
    const float E = b-a/3;

    const float a_sin = sin(RADIANS(S)) * L1,
                a_cos = cos(RADIANS(S)) * L1,
                b_sin = sin(RADIANS(S+E)) * L2,
                b_cos = cos(RADIANS(S+E)) * L2;

    cartes.set(a_sin + b_sin + scara_offset.x,
               a_cos + b_cos + scara_offset.y); 

    X_SCARA_DEBUG_LNPAIR(
      "FWDK S:", S," E:", E," -> X:", cartes.x," Y:", cartes.y);
  }
  /*
   * X_SCARA_COORDINATES_RELATIVE_ANGLES - angle transformation mode
   * To be used for testing the machine, coordinates are representing 
   * joint relative angles.
   */
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_RELATIVE_ANGLES) {
    cartes.set(a, b-a/3);
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

  /*
   * X_SCARA_COORDINATES_CARTESIAN - default transformation mode
   * This is the default transformation mode to be used when printing.
   */
  if(x_scara_coordinates_mode == X_SCARA_COORDINATES_CARTESIAN) {

    const float x = raw.x - scara_offset.x;
    const float y = raw.y - scara_offset.y;

    float x2y2 = sq(x) + sq(y);
    float hypot = SQRT(x2y2);

    if(hypot == 0) hypot = 0.000001; // avoid division by zero

    float S = ATAN2(x,y) - ACOS((x2y2 + PK_DifLenSqrd )/(2*L1*hypot));
    float E =              ACOS((x2y2 - PK_SumLenSq   )/(PK_ProdOfLen)) + S/3;

    delta.set(DEGREES(S), DEGREES(E), raw.z);

    X_SCARA_DEBUG_LNPAIR(
      "INVK X:",raw.x," Y:", raw.y, " -> S:", DEGREES(S)," E:", DEGREES(E));
  } 
  /*
   * X_SCARA_COORDINATES_RELATIVE_ANGLES - angle transformation mode
   * To be used for testing the machine, coordinates are representing 
   * joint relative angles.
   */
  else if (x_scara_coordinates_mode == X_SCARA_COORDINATES_RELATIVE_ANGLES) {
    delta.set(raw.x, raw.y + raw.x/3, raw.z);
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
 * X_SCARA specifics
 */
#ifdef X_SCARA

bool x_scara_move_joints(ab_pos_t & pos) {
  if (IsRunning()) {

    x_scara_relative_to_delta(pos);
    forward_kinematics_SCARA(pos.a, pos.b);
    x_scara_delta_to_relative(pos);

    do_blocking_move_to_xy(cartes);
    return true;
  }
  return false;
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
