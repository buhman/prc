void
uname()
{
  int err;
  struct utsname un;

  err = uname(&un);
  if (err < 0) {
    perror("uname()");
    return err;
  }

  printf("%s %s %s %s %s\n", un.sysname, un.nodename, un.release, un.version, un.machine);
}
