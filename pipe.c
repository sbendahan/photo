/* lesson 11-1 file name pipe.c, this program illustrates the usage of pipes

Description
-----------
pipes are used to communicate between two threads in a single process or
between parent and child processes.
this is an example of the later.
the main function creates another processes (using fork)
the parent process writes to the fds[1] 
the child process reads from fds[0] .




To compile me for Linux, use gcc -ggdb pipe.c -o pipe 
To execute, type:  ./pipe
 */




/*************  includes     *****************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>             //used for exit
#include <string.h> 

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <error.h>


/*************  definitions     *****************/


/*************  Prototypes   *****************/
void writer (const char* message, int count, FILE* stream);
void reader (FILE* stream);



/*************  Globals   *****************/



/*************  main() function ****************/ 
int main() 
{
	
	int fds[2];
  	int rtn;
  	pid_t pid;

  	// Create a pipe. File descriptors for the two ends of the pipe are
    // placed in fds. 
	rtn=pipe(fds);
	if(0!=rtn)
	{
		perror("pipe failed");
	}
	printf("%d %d",fds[0],fds[1]);

  	/* Fork a child process. */
  	pid = fork();
	if (pid == (pid_t) 0) 
	{
		//Child => Read End
		FILE* stream;
		// This is the child process. Close our copy of the write end of
		// the file descriptor. 
		   
		close (fds[1]);
		// Convert the read file descriptor to a FILE object, and read
		//   from it. */
		stream = fdopen (fds[0], "r");
		reader (stream);
		close (fds[0]);
	}
    else
    {
		 // This is the parent process => Write End 
		 FILE* stream;
		 /* Close our copy of the read end of the file descriptor. */
		 close (fds[0]);
		 /* Convert the write file descriptor to a FILE object, and write
			 to it. */
		 stream = fdopen (fds[1], "w");
		 writer ("Hello, world.", 5, stream);
		 close (fds[1]);
    }
    
    return 0;
 }

/****************************************************************************/
/* Write COUNT copies of MESSAGE to STREAM, pausing for a second
   between each. */
void writer (const char* message, int count, FILE* stream)
{
  for (; count > 0; --count) 
  {
    /* Write the message to the stream, and send it off immediately. */
    fprintf (stream, "%s\n", message);
    fflush (stream);
    /* Snooze a while. */
    sleep (1);
  }
}
/****************************************************************************/
/* Read random strings from the stream as long as possible.  */
void reader (FILE* stream)
{
  char buffer[1024];
  /* Read until we hit the end of the stream. fgets reads until
     either a newline or the end-of-file. */
  while (!feof (stream)
         && !ferror (stream)
         && fgets (buffer, sizeof (buffer), stream) != NULL)
    fputs (buffer, stdout);
}
/****************************************************************************/


