#include <sys/sysinfo.h>

#include "prc.h"

static prc_plugin_cmd_t sysinfo_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"sysinfo", sysinfo_cmd},
  {NULL},
};

static void
sysinfo_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  int ret;
  struct sysinfo si;

  ret = sysinfo(&si);
  if (ret < 0) {
    perror("sysinfo");
    return;
  }

  dll_enq(wq, prc_msg2("PRIVMSG", target, "up: %f  load: %f %f %f  ram: %f/%f",
                       (float)si.uptime / 86400,
                       (float)si.loads[0] / (1 << SI_LOAD_SHIFT),
                       (float)si.loads[1] / (1 << SI_LOAD_SHIFT),
                       (float)si.loads[2] / (1 << SI_LOAD_SHIFT),
                       (float)si.totalram / (1 << 20),
                       (float)si.freeram / (1 << 20)));
}
