#pragma once

typedef struct dll dll_t;
typedef struct dll_link dll_link_t;

struct dll {
  dll_link_t *head;
  dll_link_t *tail;
};

struct dll_link {
  void *buf;
  dll_link_t *next;
  dll_link_t *prev;
};

void
dll_enq(dll_t *ol,
        void *buf);

void
dll_push(dll_t *ol,
         void *buf);

void*
dll_pop(dll_t *ol);

void
dll_remove(dll_t *ol,
           dll_link_t *li);

void
dll_insert(dll_t *ol,
           dll_link_t *li,
           void *buf);
