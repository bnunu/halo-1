/*
DIRECTOR.H

header included in hcex build.
*/

#ifndef __DIRECTOR_H
#define __DIRECTOR_H
#pragma once

/* ---------- headers */

#include "cseries/cseries.h"
#include "camera/observer.h"
#include "math/real_math.h"
#include "networking/network_connection.h"

/* ---------- constants */

enum director_game_mode
{
	_director_mode_game = 0,
	_director_mode_netgame,
	_director_mode_editor,
	_director_mode_scripted,
	_director_mode_script_camera_record,
	NUMBER_OF_DIRECTOR_GAME_MODES = 5
};

enum director_camera_mode
{
	_camera_following = 0,
	_camera_orbiting,
	_camera_flying,
	_camera_editor,
	_camera_first_person,
	NUMBER_OF_DIRECTOR_CAMERA_MODES
};

enum director_perspective
{
	_director_perspective_first_person = 0,
	_director_perspective_third_person,
	_director_perspective_scripted,
	_director_perspective_neutral,
	NUMBER_OF_DIRECTOR_PERSPECTIVE_MODES,
};

enum director_variable
{
	_variable_height = 0,
	_variable_roll,
	_variable_forward,
	_variable_right,
	NUMBER_OF_DIRECTOR_VARIABLES
};

/* ---------- macros */

/* ---------- structures */

typedef short director_perspective;

typedef void (*director_camera_update_proc)(
	void *camera,
	void *command,
	void *result);

struct director_variable_instance
{
	real value;
	real velocity;
	real delta;
};

struct director_variable_definition
{
	short negative_bit;
	short positive_bit;
	short reset_bit;
	byte pad6[2];
	real scale;
	real initial_value;
	real minimum;
	real maximum;
	boolean has_hyper_scale;
	byte pad19[3];
};

struct director
{
	short camera_mode_index;
	byte pad2[2];
	real camera_change_pause;
	director_camera_update_proc camera_proc;
	byte camera_data[0x40];
	long bored_time;
	boolean bored;
	boolean inhibited_facing;
	boolean inhibited_input;
	byte pad53;
	short seat_state;
	director_perspective perspective;
	struct observer_command command;
	boolean debug_controls;
	byte padC1[3];
	real debug_input_scale;
	struct director_variable_instance debug_variables[NUMBER_OF_DIRECTOR_VARIABLES];
};

struct director_globals
{
	real dtime;
	short game_mode;
	boolean initialize_camera;
	byte pad7;
	struct director local_players[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
};

/* ---------- prototypes/DIRECTOR.C */

void director_initialize(
	void);
void director_dispose(
	void);
void director_dispose_from_old_map(
	void);
void director_inhibit_facing(
	short local_player_index);
void director_inhibit_input(
	short local_player_index);
boolean director_inhibited_facing(
	short local_player_index);
boolean director_inhibited_input(
	short local_player_index);
/* port: for the sounds' Doppler effect (sound_manager.c) */
long director_get_followed_unit(
	short local_player_index);
director_perspective director_get_perspective(
	short local_player_index);
short director_desired_perspective(
	long unit_index,
	director_perspective *perspective);
short director_camera_deterministic(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward);
void director_set_mode(
	short mode);
void director_save_camera(
	void);
void director_load_camera(
	void);
void director_initialize_for_new_map(
	void);
void director_script_camera(
	boolean scripted);
void director_initialize_for_saved_game(
	void);
void director_update(
	real time_delta_sec);

/* ---------- globals */

extern boolean *director_camera_scripted;
extern boolean director_camera_switch_fast;

/* ---------- public code */

#endif // __DIRECTOR_H
