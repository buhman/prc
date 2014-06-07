#pragma once

#define W_KI "\u2654"
#define W_QU "\u2655"
#define W_RO "\u2656"
#define W_BI "\u2657"
#define W_KN "\u2658"
#define W_PA "\u2659"

#define B_KI "\u265A"
#define B_QU "\u265B"
#define B_RO "\u265C"
#define B_BI "\u265D"
#define B_KN "\u265E"
#define B_PA "\u265F"

#define ELPS "\u2610"

static prc_plugin_cmd_t autoprint_cmd;
static prc_plugin_cmd_t print_cmd;
static prc_plugin_cmd_t move_cmd;
static prc_plugin_cmd_t undo_cmd;
static prc_plugin_cmd_t snap_cmd;

enum position {
  X1 = 0,
  Y1,
  X2,
  Y2,
};

prc_plugin_ctor_t prc_ctor;
prc_plugin_dtor_t prc_dtor;
