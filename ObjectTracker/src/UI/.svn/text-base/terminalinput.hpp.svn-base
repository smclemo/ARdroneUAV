#ifndef TERMINALINPUT_HPP
#define TERMINALINPUT_HPP

#include <pthread.h>
#include <termios.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

struct termios orig_termios;

void reset_terminal_mode()
{
  tcsetattr(0, TCSANOW, &orig_termios);
}

void *terminal_thread(void *parm);

class TerminalInput
{
  private:
    
    void set_conio_terminal_mode()
    {
      struct termios new_termios;
      /* take two copies - one for now, one for later */
      tcgetattr(0, &orig_termios);
      memcpy(&new_termios, &orig_termios, sizeof(new_termios));
      
      /* register cleanup handler, and set the new terminal mode */
      atexit(reset_terminal_mode);
      //cfmakeraw(&new_termios);
      new_termios.c_lflag &= ~ICANON;
      tcsetattr(0, TCSANOW, &new_termios);
    }
    
  public:
    char last;
    int newChar;
    
    TerminalInput()
    {
      newChar = 0;
      set_conio_terminal_mode();
		  pthread_t thread;
      pthread_create( &thread, NULL, terminal_thread, (void*) this);
    }
    
    char getCh()
    {
      newChar = 0;
      return last;
    }
    
    int ready()
    {
      return newChar;
    }
};

void *terminal_thread(void *parm)
{
  TerminalInput *parent = (TerminalInput *)parm;
  while(1)
  {
    char c = 0;
    c = getchar();
    printf("woot\n");
    if(c != '\n')
    {
      parent->last = c;
      parent->newChar = 1;
    }
    /*
    c = getc(stdin);
    printf("terminal loop! char: (%c,%d) %d\n", c, (int)c, EOF);
    if(c)
    {
      parent->last = c;
      parent->newChar = 1;
    }
    if(c == EOF)
    {
      switch(errno)
      {
        case EACCES:
        printf("Another process has the file locked.\n"); break;

        case EBADF :
        printf("stdin is not a valid stream opened for reading.\n"); break;

        case EINTR :
        printf("A signal interrupted the call.\n"); break;

        case EIO :
        printf("An input error occurred.\n"); break;

        case EISDIR :
        printf("The open object is a directory, rather than a file.\n"); break;

        case ENOMEM :
        printf("Memory could not be allocated for internal buffers.\n"); break;

        case ENXIO :
        printf("A device error occurred.\n"); break;

        case EOVERFLOW :
        printf("The file is a regular file and an attempt was made to read at or beyond the offset maximum associated with the corresponding stream.\n"); break;

        case EWOULDBLOCK :
        printf("The underlying file descriptor is a non-blocking socket and no data is ready to be read.\n"); break;
        
        default:
          printf("an unknown error occured\n");
          break;
      }
    }*/
  }
  return 0;
}

#endif
