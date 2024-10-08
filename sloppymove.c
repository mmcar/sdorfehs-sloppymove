/* Sloppy Move
 *
 * An improvement on the original "Sloppy Focus" packaged with ratpoison
 * Accepts a compromise between "rat" and "no rat" -- changes focus based on
 * rat movement but allows for keyboard-based focus changes too
 *
 * Based on the original Sloppy Focus by Shawn Betts <sabetts@vcn.bc.ca>
 *
 * sloppymove is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * sloppymove is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int (*defaulthandler) (Display *, XErrorEvent *);

int
errorhandler (Display *display, XErrorEvent *error)
{
  if (error->error_code != BadWindow)
    (*defaulthandler) (display, error);
  return 0;
}

int
spawn (char *cmd)
{
  int pid;

  pid = fork ();
  if (pid == 0)
    {
      execl ("/bin/sh", "sh", "-c", cmd, (char *)NULL);
      _exit (EXIT_FAILURE);
    }
  return pid;
}

int
main (void)
{
  Display *display;
  int i, numscreens;
  int lastX = -1, lastY = -1;
Window lastPointerWindow = None;

  display = XOpenDisplay (NULL);
  if (!display)
    {
      fprintf (stderr, "sloppy: could not open display\n");
      exit (1);
    }

  /* Ensure child shell will have $RATPOISON set. */
  if (!getenv ("SDORFEHS"))
    setenv ("SDORFEHS", "sdorfehs", 0);

  defaulthandler = XSetErrorHandler (errorhandler);
  numscreens = ScreenCount (display);

  for (i = 0; i < numscreens; i++)
    {
      unsigned int j, nwins;
      Window dw1, dw2, *wins;

      XSelectInput (display, RootWindow (display, i),
                    SubstructureNotifyMask);
      XQueryTree (display, RootWindow (display, i),
                  &dw1, &dw2, &wins, &nwins);
      for (j = 0; j < nwins; j++)
	XSelectInput (display, wins[j], EnterWindowMask | PointerMotionMask);
    }


XEvent event;

while (1) {

	 XNextEvent (display, &event);	
	  
	 switch(event.type) {

		case CreateNotify:
			XSelectInput (display, event.xcreatewindow.window, EnterWindowMask | PointerMotionMask);
		break;

		case MotionNotify:
			lastPointerWindow = event.xmotion.window;
		break;

		case EnterNotify:
			if (event.xcrossing.window != lastPointerWindow) {
			        char shell[256];

			        snprintf (shell, sizeof(shell),
		                  "$SDORFEHS -c \"select $($SDORFEHS -c 'windows %%i %%n %%f' | grep '%ld' | awk '$3 != '$($SDORFEHS -c curframe)' && $3 != \"\" {print $2}')\" 2>/dev/null",
                		  event.xcrossing.window);
			        spawn (shell);
			        wait (NULL);

			        lastPointerWindow = event.xcrossing.window;
			}
		break;

	}	

}

  XCloseDisplay (display);

  return 0;
}

