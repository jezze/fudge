/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#include "common.h"

// fudge: no SDLPoP.ini, mods or PRINCE.EXE modifications, only the defaults.

void turn_fixes_and_enhancements_on_off(byte new_state) {
	use_fixes_and_enhancements = new_state;
	fixes = (new_state) ? &fixes_saved : &fixes_disabled_state;
}

void turn_custom_options_on_off(byte new_state) {
	use_custom_options = new_state;
	custom = (new_state) ? &custom_saved : &custom_defaults;
}

void set_options_to_default() {
#ifdef USE_MENU
	enable_pause_menu = 1;
#endif
	enable_copyprot = 0;
	enable_music = 1;
	enable_fade = 1;
	enable_flash = 1;
	enable_text = 1;
	enable_info_screen = 1;
	start_fullscreen = 0;
	use_hardware_acceleration = 2;
	use_correct_aspect_ratio = 0;
	use_integer_scaling = 0;
	scaling_type = 0;
	enable_controller_rumble = 1;
	joystick_only_horizontal = 1;
	joystick_threshold = 8000;
	enable_quicksave = 1;
	enable_quicksave_penalty = 1;
	enable_replay = 1;
#ifdef USE_LIGHTING
	enable_lighting = 0;
#endif
	// By default, all the fixes are used, unless otherwise specified.
	// So, if one of these options is omitted from the INI file, they default to true.
	memset(&fixes_saved, 1, sizeof(fixes_saved));
	custom_saved = custom_defaults;
	turn_fixes_and_enhancements_on_off(0);
	turn_custom_options_on_off(0);
}

void load_global_options() {
	set_options_to_default();
}

void check_mod_param() {
}

void load_mod_options() {
	turn_fixes_and_enhancements_on_off(use_fixes_and_enhancements);
	turn_custom_options_on_off(use_custom_options);
}
