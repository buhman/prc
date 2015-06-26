#pragma once

typedef int (*method_fptr_t)(event_handler *eh, bb_message *msg);

typedef struct {
  const char *name;
  method_fptr_t fptr;
} method_map_t;

void
prc_method_map_init(void);

int
prc_method_connect(event_handler *eh, bb_message *msg);

int
prc_method_subscribe_cmd(event_handler *eh, bb_message *msg);

int
prc_method_read_cb(event_handler *eh, void *buf, size_t len);
