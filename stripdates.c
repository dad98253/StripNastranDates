/*
 * stripdates.c
 *
 *  Created on: May 27, 2019
 *      Author: dad
 *
 *      The header of each page of Nastran output looks similar t6o this:
 *
 *      1     JET TRANSPORT WING DYNAMIC ANALYSIS                                   /    95 SUN SOLARIS NASTRAN    / MAY 17, 95 / PAGE    28
 *
 *
 *      This program reads a Nastran output file from stdin, and attemps to strip all header and date fields from the file so that it can be
 *      redily compared to a similar file run on another system or on another date.
 *      First, it skips the entire first page of output. Whenever it finds a page feed, it strips off both the system title ("95 SUN SOLARIS NASTRAN"
 *      above}, as well as the date in the field following it. Both of these fields will be blanked out and reduced to "///". It can attempt to
 *      recognize USER information messages and strip them as well. Finally, it deletes the last three lines of the file (which contain date and
 *      run time).
 *      The modified file is output to stdout.
 *
 *      A sample USER info message looks like this:
 *
 *      0*** USER INFORMATION MESSAGE 225, GINO TIME CONSTANTS ARE BEING COMPUTED
 *           (SEE NASINFO FILE FOR ELIMINATION OF THESE COMPUTATIONS)
 *
 *		Note that it will search for "0*** USER INFORMATION MESSAGE" because error messages (which we want to keep) can start with variants of
 *		"0*** USER FATAL MESSAGE"
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>

char * fixline( char * line );
void removecr(char *strtmp);

#define BUFFLEN 5000
char line[BUFFLEN];
int endme = 0;
int startme = 1;
char blankline[] = "\0";
char userinfomessage[] = "0*** USER INFORMATION MESSAGE";
char restart[] = "0   RESTART";
char banditopencore[] = "                                        BANDIT OPEN CORE";
char usermsg3028[] = "0*** USER INFORMATION MESSAGE 3028";
int lenusemess;
int lenrestart;
int lenbandit;
int lenmsg3028;
int skipline = 0;
int killuserinfo = 0;
char version[] = "1.0";
char cdate[] = __DATE__;


int main(int argc, char *argv[])
 {
	 if ( argc > 1 ) {
		 if ( strcmp(argv[1],"-k") == 0 ) {
			 killuserinfo = 1;
		 } else {
			 fprintf(stderr,"Usage: stripdates [OPTION]\n\nVersion %s -- compiled %s\n"
					 "Strip date and other information from a Nastran-95 output (print) file\n\n"
					 "With no file redirection, read standard input, write standard output.\n\n"
					 "   -k         strip Nastran USER information messages as well\n"
					 "   -h         detailed help information\n\n"
					 "Hint : to generate a sha1 checksum of a Nastran output file enter:\n"
					 "cat <Nastran output> | stripdates | sha1sum\n\n",
					 version,cdate);
			 if (strcmp(argv[1],"-h") == 0 ) fprintf(stderr,
					 "      The header of each page of Nastran output looks similar to this:\n\n"
					 "      1     JET TRANSPORT WING DYNAMIC ANALYSIS                                   /    95 SUN SOLARIS NASTRAN    / MAY 17, 95 / PAGE    28\n\n"
					 "      This program reads a Nastran output file from stdin, and attemps to strip all header and date fields from the file so that it can be\n"
					 "      compared to a similar file run on another system or on another date.\n"
					 "      First, it skips the entire first page of output. Whenever it finds a page feed, it strips off both the system title (\"95 SUN SOLARIS NASTRAN\"\n"
					 "      above}, as well as the date in the field following it. Both of these fields will be blanked out and reduced to \"///\" (so don\'t expect\n"
					 "      the page number to be right justified after). It can attempt to recognize USER information messages and strip them as well. Finally, it\n"
					 "      deletes the last three lines of the file (which contain date and run time).\n"
					 "      The modified file is output to stdout.\n\n"
					 "      A sample USER info message looks like this:\n\n"
					 "      0*** USER INFORMATION MESSAGE 225, GINO TIME CONSTANTS ARE BEING COMPUTED\n"
					 "           (SEE NASINFO FILE FOR ELIMINATION OF THESE COMPUTATIONS)\n\n"
					 "      Note that the program will search for \"0*** USER INFORMATION MESSAGE\" because error messages (which we want to keep) can start with variants of\n"
					 "      \"0*** USER FATAL MESSAGE\"\n" );
			 return 0;
		 }
	}

/*
	    inputfile = fopen(stdin, "r");
	    if (inputfile == NULL)
	    {
	        fprintf(stderr,"Could not open input file\n");
	        return 0;
	    }

	    outputfile = fopen(stdout, "w");
	    if (outputfile == NULL)
	    {
	        fprintf(stderr,"Could not open output file\n");
	        return 0;
	    }
*/
		lenusemess = strlen(userinfomessage);
		lenrestart = strlen(restart);
		lenbandit = strlen(banditopencore);
		lenmsg3028 = strlen(usermsg3028);
		char * tempstr;
	       while ( fgets(line, BUFFLEN, stdin)  ) {
//	        	fprintf(stderr," EOF\n");
	    	   removecr( line );
//	    	   if ( strlen(line) == 0 ) printf(" found zero line\n");
//	    	   if ( strlen(line) == 1 ) printf(" found lne 1 line %u\n",(unsigned char)(line[0]));
	    	   tempstr = fixline(line);
	    	   if ( strlen(tempstr) ) fprintf(stdout, "%s\n", tempstr );
	    	   if ( endme ) return 0;
	        }

	return 0;
}

char * fixline( char * line ) {
	int linelength;
	char * start;
	char * end;
	char * endtemp;

	if ( skipline && line[0] != '1' ) {
		skipline--;
		strcpy(line,blankline);
		return (line);
	}
	if ( strncmp ( usermsg3028, line, lenmsg3028) == 0  && killuserinfo ) {
		skipline = 2;
		strcpy(line,blankline);
		return (line);
	}
	if ( strncmp ( userinfomessage, line, lenusemess) == 0  && killuserinfo ) {
		skipline = 1;
		strcpy(line,blankline);
		return (line);
	}
	if ( strncmp ( restart, line, lenrestart) == 0  && killuserinfo ) {
		strcpy(line,blankline);
		return (line);
	}
	if ( strncmp ( banditopencore, line, lenbandit) == 0  && killuserinfo ) {
		strcpy(line,blankline);
		return (line);
	}

	if ( line[0] != '1' ) {
		if ( startme ) {
//			fprintf(stderr,"skipping another line\n");
			strcpy(line,blankline);
			return (line);
		} else {
			return(line);
		}
	}
	startme = 0;
	if ( ( linelength = strlen(line) ) < 10 ) {
		endme = 1;
		return(line);
	}
	if ( ( start = strchr( line, '/' ) ) == NULL ) return (line);
	if ( ( end = strchr( (start+1), '/' ) ) == NULL ) return (line);
	if ( ( end = strchr( (end+1), '/' ) ) == NULL ) return (line);
	endtemp = end;
	while ( ( endtemp = strchr( (endtemp+1), '/' ) ) ) end = endtemp;
	*(start+1) = '/';
	strcpy((start+2),end);

	return(line);
}

void removecr(char *strtmp) {
  if (strtmp == NULL) return;
  int length = strlen(strtmp);
  if (length == 0 ) return;
  if (strtmp[length-1] == '\n') {
    strtmp[length-1]  = '\000';
  }
  length = strlen(strtmp);
  if (length == 0 ) return;
  if (strtmp[length-1] == '\012') {
     strtmp[length-1]  = '\000';
   }
  length = strlen(strtmp);
  if (length == 0 ) return;
  if (strtmp[length-1] == '\015') {
     strtmp[length-1]  = '\000';
   }
  length = strlen(strtmp);
  if (length == 0 ) return;
  int i;
  int last = -1;
  for (i=0;i<length;i++) {
	  if ( !isspace(strtmp[i]) ) last =i;
  }
  strtmp[last+1] = '\000';
}

