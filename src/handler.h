#pragma once

typedef struct {
  event_handler *eh;
  const char *target;
} prc_handler_t;

void
prc_handler_subscribe_cmd(event_handler *eh, const char *command, const char *target);

int
prc_handler(event_handler *eh, void *buf, size_t len);
