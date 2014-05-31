#include <stdio.h>

#include <gsasl.h>

#include "sasl.h"

int
sasl_auth(const char *authid,
          const char *password,
          char **obuf)
{
  Gsasl *ctx;
  Gsasl_session *session;
  int err;

  {
    err = gsasl_init(&ctx);
    if (err != GSASL_OK) {
      fprintf(stderr, "gsasl_init(): %s\n", gsasl_strerror(err));
      return err;
    }
  } /* ... */

  {
    err = gsasl_client_start(ctx, "PLAIN", &session);
    if (err != GSASL_OK) {
      fprintf(stderr, "gsasl_client_start(): %s\n", gsasl_strerror(err));
      return err;
    }

    gsasl_property_set(session, GSASL_AUTHID, authid);
    gsasl_property_set(session, GSASL_PASSWORD, password);
  } /* ... */

  {
    err = gsasl_step64(session, "", obuf);
    if (err != GSASL_OK) {
      fprintf(stderr, "gsasl_step64(): %s\n", gsasl_strerror(err));
      return err;
    }
  }

  gsasl_finish(session);
  gsasl_done(ctx);

  return 0;
}
