#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <assert.h>

#include <sys/ioctl.h>

static struct winsize ws;

void
term_read(int fd, sll_t *wq)
{

}

void
term_write(int fd, sll_t *wq)
{

}

int
term_setup()
{
  int err, mode;
  struct termios t;

  {
    err = tcgetattr(STDIN_FILENO, &t);
    if (err < 0) {
      perror("tcgetattr()");
      return err;
    }

    t.c_lflag &= ~(ECHO | ECHONL | ICANON);

    err = tcsetattr(STDIN_FILENO, TCSADRAIN, &t);
    if (err < 0) {
      perror("tcsetattr()");
      return err;
    }
  }

  {
    mode = fcntl(STDIN_FILENO, F_GETFL);
    if (mode < 0) {
      perror("fcntl() GETFL");
      return mode;
    }

    mode |= O_NONBLOCK;

    err = fcntl(STDIN_FILENO, F_SETFL, mode);
    if (err < 0) {
      perror("fcntl() SETFL");
      return err;
    }
  }

  {
    err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (err < 0) {
      perror("TIOCGWINSZ");
      return err;
    }
  }

  return 0;
}
