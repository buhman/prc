#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "prc.h"

char*
prc_prefix_parse(char *prefix, enum prefix_cp comp)
{
  char *sp, *tok;

  // first character will always be ':' and should be discarded
  prefix++;

  tok = strtok_r(prefix, "!@", &sp);
  if (!tok)
    return prefix;

  for (int i = NICK; i < (int)comp; i++) {
    tok = strtok_r(NULL, "!@", &sp);
    if (!tok)
      return prefix;
  }

  return tok;
}

char*
prc_msg(const char *cmd, ...)
{
  va_list ap;
  char *buf = calloc(1, MSG_SIZE);
  int len, offset;

  va_start(ap, cmd);

  while (cmd != NULL) {

    len = strlen(buf);
    if ((*cmd == ':' || *cmd == '\001') && *(cmd + 1) == '\0')
      snprintf(buf + len, MSG_SIZE - (3 + len), cmd);
    else
      snprintf(buf + len, MSG_SIZE - (3 + len), "%s ", cmd);

    cmd = va_arg(ap, char*);
  }

  va_end(ap);

  len = strlen(buf);
  offset = 1;
  /*while ((unsigned char)*(buf + len - (offset + 1)) > 127)
    offset++;*/

  sprintf(buf + len - offset, "\r\n");

  return buf;
}

prc_plugin_msg_t*
prc_msg2(const char *cmd, const char *target, const char *format, ...)
{
  char *buf = malloc(MSG_SIZE);
  prc_plugin_msg_t *msg = malloc(sizeof(prc_plugin_msg_t));

  {
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MSG_SIZE - 1, format, ap);
    va_end(ap);
  }

  msg->cmd = cmd;
  msg->buf = buf;
  msg->target = target; // copy will happen in prc_msg

  return msg;
}

char*
prc_msg3(const char *format, ...)
{
  char *buf = malloc(MSG_SIZE);

  {
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, MSG_SIZE - 1, format, ap);
    va_end(ap);
  }

  return buf;
}

char **
prc_parse_args(char *tok, int nargs)
{
  char *ptr;
  char **args, **argi;

  args = malloc(sizeof(char*) * nargs);

  if (!tok)
    goto invalid;

  for (argi = args; argi < args + nargs; argi++) {

    ptr = strchr(tok, ' ');
    if (!ptr) {
      if (argi - args < nargs - 1)
        goto invalid;
    }
    else
      *ptr = '\0';

    *argi = tok;

    tok = ptr + 1;
  }

  return args;

 invalid:
  free(args);
  return NULL;
}

unsigned long int 
prc_rand_lim(unsigned long int limit)
{  
  unsigned long int divisor, ret;

  divisor = RAND_MAX / limit;

  do {
    ret = random() / divisor;
  } while (ret > limit);

  return ret;
}
