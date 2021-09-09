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
#pragma once

/**
 * scara.h - SCARA-specific functions
 */

#include "../core/macros.h"

extern float delta_segments_per_second;
extern xyz_pos_t scara_home_offset;

/*
 * X-SCARA compiler directives
 */
#if ENABLED(X_SCARA)

  extern float delta_min_segment_length;

  #if ENABLED(X_SCARA_DEBUG)
    extern bool x_scara_debug;
    #define X_SCARA_DEBUG_LNPAIR(V...) if (x_scara_debug) {SERIAL_ECHOLNPAIR(V);}
  #else //X_SCARA_DEBUG
    #define X_SCARA_DEBUG_LNPAIR(V...) 
  #endif

  #define X_SCARA_VERSION_STR "0.4.1 Beta"
#endif//X_SCARA

float constexpr L1 = SCARA_LINKAGE_1, L2 = SCARA_LINKAGE_2;
      
void scara_set_axis_is_at_home(const AxisEnum axis);
void inverse_kinematics(const xyz_pos_t &raw);
void forward_kinematics_SCARA(const float &a, const float &b);
void scara_report_positions();

inline bool scara_position_is_reachable(const float &rx, const float &ry, const float inset=0) {

  #if EITHER(MORGAN_SCARA, MP_SCARA)
      const float R2 = HYPOT2(rx - SCARA_OFFSET_X, ry - SCARA_OFFSET_Y);
      return (
        R2 <= sq(L1 + L2) - inset
        #if MIDDLE_DEAD_ZONE_R > 0
          && R2 >= sq(float(MIDDLE_DEAD_ZONE_R))
        #endif
      );
  #elif ENABLED(X_SCARA)
    X_SCARA_DEBUG_LNPAIR("CHCK X:",rx," Y:",ry, " I:", inset);
    return true; ///TBD: software endstops or this, not clear yet
  #endif
}

/*
 * X_SCARA definitions
 */
#ifdef X_SCARA 

  enum X_SCARA_COORDINATES_MODE {
        X_SCARA_COORDINATES_CARTESIAN         = 0,
        X_SCARA_COORDINATES_RELATIVE_ANGLES   = 1,
        X_SCARA_COORDINATES_CARTESIAN_PROBE_AT_CENTER = 2,
        X_SCARA_COORDINATES_LOCK_XY = 3,
        X_SCARA_COORDINATES_MODE_END
  };

  typedef void (*_gcode_fn_t)();

  /* X-SCARA global variables */
  extern xy_pos_t scara_offset;
  extern uint8_t x_scara_coordinates_mode;
  extern const char * x_scara_coordinates_mode_str [];

  /* X-SCARA  inline methods */
  FORCE_INLINE void x_scara_relative_to_delta(ab_pos_t & angles) {
    angles.b += angles.a/3;
  }

  FORCE_INLINE void x_scara_delta_to_relative(ab_pos_t & angles) {
    angles.b -= angles.a/3;
  }

  /* X-SCARA functions */
  void x_scara_begin();

  void x_scara_run_gcode(_gcode_fn_t gcode_fn);

  void x_scara_move_joints(ab_pos_t & pos, const feedRate_t &fr_mm_s=0.0f);

  void x_scara_change_coordinates_mode(uint8_t coordinates_mode);

#endif // X_SCARA