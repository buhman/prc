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
  while ((unsigned char)*(buf + len - (offset + 1)) > 127)
    offset++;

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
