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

float constexpr 
      L1 = SCARA_LINKAGE_1, 
      L2 = SCARA_LINKAGE_2;
      
void scara_set_axis_is_at_home(const AxisEnum axis);
void inverse_kinematics(const xyz_pos_t &raw);
void forward_kinematics_SCARA(const float &a, const float &b);
void scara_report_positions();

/*
 * X_SCARA specifics
 */
#ifdef X_SCARA 

  #if ENABLED(X_SCARA_DEBUG)
    extern bool x_scara_debug;
    #define X_SCARA_DEBUG_LNPAIR(V...) if (x_scara_debug) {SERIAL_ECHOLNPAIR(V);}
  #else //X_SCARA_DEBUG
    #define X_SCARA_DEBUG_LNPAIR(V...) 
  #endif

  #define X_SCARA_VERSION_STR "0.2 Alpha"

  extern xy_pos_t scara_offset;
  
  enum X_SCARA_COORDINATES_MODE {
        X_SCARA_COORDINATES_CARTESIAN,
        X_SCARA_COORDINATES_RELATIVE_ANGLES,
        X_SCARA_COORDINATES_MODE_END
  };
  extern uint8_t x_scara_coordinates_mode;


  FORCE_INLINE void x_scara_relative_to_delta(ab_pos_t & angles) {
    angles.b += angles.a/3;
  }
  FORCE_INLINE void x_scara_delta_to_relative(ab_pos_t & angles) {
    angles.b -= angles.a/3;
  }

  void x_scara_move_joints(ab_pos_t & pos);


#endif // X_SCARA