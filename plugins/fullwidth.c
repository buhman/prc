#include "prc.h"

#include <iconv.h>

#define magic 0xfee0

static prc_plugin_cmd_t full_cmd;

prc_plugin_sym_t prc_sym[] = {
  {"full", full_cmd},
  {NULL},
};

static void
full_cmd(dll_t *wq, char *prefix, char* target, char *args)
{
  iconv_t conv;

  char *iarg, *buf, *ibuf;
  size_t inl, outl;
  wchar_t *in, *iin;

  in = malloc(sizeof(wchar_t));
  iin = in;
  buf = malloc(MSG_SIZE);
  outl = MSG_SIZE - 1;

  conv = iconv_open("utf-8", "ucs-2");

  for (iarg = args, ibuf = buf; iarg < args + strlen(args); iarg++, ibuf--, iin = in) {

    *iin = (wchar_t)(magic + *iarg);
    inl = sizeof(wchar_t);

    iconv(conv, (char**)(&iin), &inl, &ibuf, &outl);
  }

  free(in);
  iconv_close(conv);

  dll_enq(wq, prc_msg2("PRIVMSG", target, buf));

  free(buf);
}
