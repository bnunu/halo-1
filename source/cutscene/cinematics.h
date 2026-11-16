/*
CINEMATICS.H

header included in hcex build.
*/

#ifndef __CINEMATICS_H
#define __CINEMATICS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct cinematic_title
{
	short title_index;
	short time;
};

struct cinematic_global_data
{
	real letterbox_amount;
	long letterbox_last_game_time;
	boolean show_letterbox;
	boolean in_progress;
	boolean can_be_skipped;
	boolean suppress_bsp_object_creation;
	struct cinematic_title queued_titles[4];
};

typedef char verify_cinematic_title_size[
	sizeof(struct cinematic_title) == 0x4 ? 1 : -1];
typedef char verify_cinematic_global_data_size[
	sizeof(struct cinematic_global_data) == 0x1C ? 1 : -1];

/* ---------- prototypes/CINEMATICS.C */

void cinematic_initialize(
	void);
void cinematic_dispose(
	void);
void cinematic_initialize_for_new_map(
	void);
void cinematic_skip_start(
	void);
void cinematic_skip_stop(
	void);
/* port: the checkpoint a skip goes back to, which no other replaces */
void cinematic_port_checkpoint(
	void);
boolean cinematic_port_holds_checkpoints(
	void);
void cinematic_dispose_from_old_map(
	void);
void cinematic_start(
	void);
boolean cinematic_can_be_skipped(
	void);
void cinematic_show_letterbox(
	boolean show);
void cinematic_force_title(
	unsigned short title_index);
void cinematic_suppress_bsp_object_creation(
	boolean suppress);
boolean cinematic_in_progress(
	void);
void cinematic_stop(
	void);
void cinematic_set_title_delayed(
	short title_index,
	real delay);
void cinematic_render(
	void);
void draw_quad(
	rectangle2d *rectangle,
	pixel32 color);
void cinematic_set_title(
	unsigned short title_index);

/* ---------- globals */

extern struct cinematic_global_data *cinematic_globals;

/* ---------- public code */

#endif // __CINEMATICS_H
