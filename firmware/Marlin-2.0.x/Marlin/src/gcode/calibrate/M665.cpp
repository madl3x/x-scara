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

#include "../../inc/MarlinConfig.h"

#if IS_KINEMATIC

#include "../gcode.h"
#include "../../module/motion.h"

#if ENABLED(DELTA)

  #include "../../module/delta.h"
  /**
   * M665: Set delta configurations
   *
   *    H = delta height
   *    L = diagonal rod
   *    R = delta radius
   *    S = segments per second
   *    X = Alpha (Tower 1) angle trim
   *    Y = Beta (Tower 2) angle trim
   *    Z = Gamma (Tower 3) angle trim
   */
  void GcodeSuite::M665() {
    if (parser.seen('H')) delta_height              = parser.value_linear_units();
    if (parser.seen('L')) delta_diagonal_rod        = parser.value_linear_units();
    if (parser.seen('R')) delta_radius              = parser.value_linear_units();
    if (parser.seen('S')) delta_segments_per_second = parser.value_float();
    if (parser.seen('X')) delta_tower_angle_trim.a  = parser.value_float();
    if (parser.seen('Y')) delta_tower_angle_trim.b  = parser.value_float();
    if (parser.seen('Z')) delta_tower_angle_trim.c  = parser.value_float();
    recalc_delta_settings();
  }

#elif IS_SCARA

  #if ENABLED(X_SCARA)
  #include "../../module/scara.h"

  /**
   * M665: Set SCARA settings
   *
   * Parameters:
   *   
   * D<bool> - debug on or off (if compiled with X_SCARA_DEBUG)
   * S<int>  - number of segments per second
   */
  void GcodeSuite::M665() {
 
    // update delta segments per second
    if (parser.seenval('D')) delta_segments_per_second = parser.value_float();

    // update delta min segment length
    if (parser.seenval('M')) delta_min_segment_length  = parser.value_float();

    // update new origin offset
    if (parser.seen_test('X') || parser.seen_test('Y')) {
      xy_pos_t new_origin_offset = scara_offset;
      xy_pos_t diff;

      // read new offset from parser
      if (parser.seenval('X')) new_origin_offset.x = parser.value_float();
      if (parser.seenval('Y')) new_origin_offset.y = parser.value_float();

      // update new origin offset
      if (new_origin_offset != scara_offset) {
        diff = new_origin_offset - scara_offset;
        scara_offset = new_origin_offset;

        current_position.x += diff.x;
        current_position.y += diff.y;
        sync_plan_position();
      }
    }

    // update new home offset
    if (parser.seen_test('S') || parser.seen_test('E')) {
        xy_pos_t new_home_offset = scara_home_offset;

      // read new offset from parser
      if (parser.seenval('S')) new_home_offset.x = parser.value_float();
      if (parser.seenval('E')) new_home_offset.y = parser.value_float();

      // update new home offset
      scara_home_offset = new_home_offset;
    }

    SERIAL_ECHOLNPAIR_P(
            PSTR("  M665 D"), delta_segments_per_second
          , PSTR(" M"), delta_min_segment_length
          , PSTR(" S"), LINEAR_UNIT(scara_home_offset.a)
          , PSTR(" E"), LINEAR_UNIT(scara_home_offset.b)
          , PSTR(" X"), LINEAR_UNIT(scara_offset.x)
          , PSTR(" Y"), LINEAR_UNIT(scara_offset.y)
        );

  }

  #else // X_SCARA

  #include "../../module/scara.h"

  /**
   * M665: Set SCARA settings
   *
   * Parameters:
   *
   *   S[segments-per-second] - Segments-per-second
   *   P[theta-psi-offset]    - Theta-Psi offset, added to the shoulder (A/X) angle
   *   T[theta-offset]        - Theta     offset, added to the elbow    (B/Y) angle
   *   Z[z-offset]            - Z offset, added to Z
   *
   *   A, P, and X are all aliases for the shoulder angle
   *   B, T, and Y are all aliases for the elbow angle
   */
  void GcodeSuite::M665() {
    if (parser.seenval('S')) delta_segments_per_second = parser.value_float();

    #if HAS_SCARA_OFFSET

      if (parser.seenval('Z')) scara_home_offset.z = parser.value_linear_units();

      const bool hasA = parser.seenval('A'), hasP = parser.seenval('P'), hasX = parser.seenval('X');
      const uint8_t sumAPX = hasA + hasP + hasX;
      if (sumAPX) {
        if (sumAPX == 1)
          scara_home_offset.a = parser.value_float();
        else {
          SERIAL_ERROR_MSG("Only one of A, P, or X is allowed.");
          return;
        }
      }

      const bool hasB = parser.seenval('B'), hasT = parser.seenval('T'), hasY = parser.seenval('Y');
      const uint8_t sumBTY = hasB + hasT + hasY;
      if (sumBTY) {
        if (sumBTY == 1)
          scara_home_offset.b = parser.value_float();
        else {
          SERIAL_ERROR_MSG("Only one of B, T, or Y is allowed.");
          return;
        }
      }

    #endif // HAS_SCARA_OFFSET
  }

#endif // MORGAN_SCARA or MP_SCARA
#endif // IS_SCARA
#endif // IS_KINEMATIC
