/* Support program for 900 series emulator to convert 900 telecode */
/* to ASCII                                                        */
/* from900text [-i inFile] [-a  outFile]                           */
/* inFile defaults to .punch                                       */
/* outFile defaults to .ascii                                      */
/*                                                                 */
/* Andrew Herbert 20 February 2021                                 */
  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define INFILE  ".punch"    // input file
#define OUTFILE ".ascii"    // output file

#define ERR_FOPEN_INPUT  "Cannot open input file"
#define ERR_FOPEN_OUTPUT "Cannot open output file"
#define ERR_STRDUP       "Unexpected error in strdup in main"
#define ERR_FILE_IN      "Unexpected error with input file"
#define ERR_FILE_OUT     "Unexpected error with output file"

#define TRUE  1
#define FALSE 0

#define OPTSTR "i:a:"
#define USAGE_FMT  "%s [-i inputfile] [-a asciifile]"

extern int errno;
extern char *optarg;
extern int opterr, optind;

void convert (FILE *inFile, FILE *outFile);

int main (int argc, char *argv[]) {
  int opt;
  char *inPath  = INFILE;
  char *outPath = OUTFILE;
  FILE *inFile, *outFile;

  // decode arguments
  opterr = 0;
  while ( (opt = getopt(argc, argv, OPTSTR)) != EOF )
    
     switch ( opt ) {
       case 'i':
	 if ( !(inPath = strdup(optarg)) ) {
	   perror(ERR_STRDUP);
	   exit(EXIT_FAILURE);
	   /* NOTREACHED */
	 }
         break;
       case 'o':
	 if ( !(outPath = strdup(optarg)) ) {
	   perror(ERR_STRDUP);
	   exit(EXIT_FAILURE);
	   /* NOTREACHED */;
	 }
	 break;
       }

  //open files
  if ( !(inFile = fopen(inPath, "rb")) ){
       perror(ERR_FOPEN_INPUT);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }
  if ( !(outFile = fopen(outPath, "wb")) ){ // write as binary to force ASCII encoding
       perror(ERR_FOPEN_OUTPUT);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }    

  // do conversion
  convert(inFile, outFile);
  
  return EXIT_SUCCESS;
}

void convert (FILE *inFile, FILE *outFile) {
  int i, ch, nlFlag, count = 0;
  nlFlag = FALSE; // tracks if input ends with a newline
  while ( (ch = fgetc(inFile)) != EOF ) {
      ch = ch&127; // strip off parity bit
      if ( (ch==10) || (32<=ch && ch<=122) ) { // filter out non-printing characters
	  fputc(ch,outFile);
	  count++;
	  nlFlag = (ch==10);
	}
    }
  if ( (count > 0) && !nlFlag ) fputc('\n',outFile); // force newline at end of file
  return;
}   
    
	
      
  
