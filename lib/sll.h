#pragma once

typedef struct sll sll_t;
typedef struct sll_link sll_link_t;

struct sll {
  sll_link_t *head;
  sll_link_t *tail;
};

struct sll_link {
  void *buf;
  sll_link_t *next;
};

void
sll_push(sll_t *ol,
         void *buf);

void
sll_pop(sll_t *ol,
        void **obuf);
