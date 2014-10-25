#include <stdlib.h>
#include <time.h>

#include "prc.h"
#include "dll.h"
#include "weight.h"

static prc_plugin_cmd_t weight_add_handler;
static prc_plugin_cmd_t weight_select_handler;

prc_plugin_ctor_t prc_ctor;
prc_plugin_dtor_t prc_dtor;

prc_plugin_sym_t prc_sym[] = {
  {"weight.select", weight_select_handler},
  {"weight.add", weight_add_handler},
};

static dll_t *weight_ll;

static void
weight_add_handler(dll_t *wq, char *prefix, char *target, char *tok)
{
  char **args, *endptr;
  unsigned long int sum, weight;

  args = prc_parse_args(tok, 2);
  if (args == NULL) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid nargs]"));
    return;
  }

  weight = strtol(*args, &endptr, 10);
  if (*endptr != '\0') {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[invalid weight]: %s", *args));
    return;
  }
  
  sum = weight_add(weight_ll, weight, *(args + 1));

  dll_enq(wq, prc_msg2("PRIVMSG", target, "[weight sum]: %ld", sum));

  free(args);
}

static void
weight_select_handler(dll_t *wq, char *prefix, char *target, char *tok)
{
  char *fact;

  if (weight_ll->head == NULL) {
    dll_enq(wq, prc_msg2("PRIVMSG", target, "[no weights]"));
    return;
  }

  fact = weight_select(weight_ll);

  dll_enq(wq, prc_msg2("PRIVMSG", target, "%s", fact));
}

int
prc_ctor(bdb_t *bdb, int evfd)
{
  srandom(time(NULL));
  weight_ll = calloc(1, sizeof(dll_t));

  return 0;
}

int
prc_dtor()
{
  free(weight_ll);

  return 0;
}
