#include <time.h>

#include "prc.h"

prc_plugin_ctor_t prc_ctor;

prc_plugin_sym_t prc_sym[] = {
  {NULL},
};

int
prc_ctor(bdb_t *b, int evfd)
{
  return *((char*)0x7);
}
