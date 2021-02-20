/* Support program for 900 series emulator to reverse a paper tape */
/*                                                                 */
/* reverse [-i inFile] [-o  outFile]                               */
/* inFile defaults to .punch                                       */
/* outFile defaults to .reverse                                    */
/*                                                                 */
/* Andrew Herbert 20 February 2021                                 */
  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define INFILE  ".punch"      // input file
#define OUTFILE ".reverse"    // output file

#define ERR_FOPEN_INPUT  "Cannot open input file "
#define ERR_FOPEN_OUTPUT "Cannot open output file "
#define ERR_STRDUP       "Unexpected error in strdup in main"
#define ERR_FILE_IN      "Unexpected error with input file"
#define ERR_FILE_OUT     "Unexpected error with output file"
#define ERR_TOO_LONG     "Input file longer than a reel of paper tape"

#define TAPELEN 1000*12*10    // length of paper tape in characters

#define OPTSTR "i:o:"
#define USAGE_FMT  "%s [-i inputfile] [-o outputfile]"

extern int errno;
extern char *optarg;
extern int opterr, optind;

void reverse (FILE *inFile, FILE *outFile);

int main (int argc, char *argv[]) {
  int opt;
  char *inPath  = INFILE;
  char *outPath = OUTFILE;
  FILE *inFile, *outFile;

  // decode arguments
  opterr = 0;
  while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
    
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

  // open files

  printf("opening input %s output %s\n", inPath, outPath);
  
  if ( !(inFile = fopen(inPath, "rb")) ){
       printf(ERR_FOPEN_INPUT);
       perror(inPath);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }
 if ( !(outFile = fopen(outPath, "wb")) ){
       printf(ERR_FOPEN_OUTPUT);
       perror(outPath);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }    

  reverse(inFile, outFile);

  return EXIT_SUCCESS;
}

void reverse (FILE *inFile, FILE *outFile) {
  int i, size, mid, top;
  char buffer [TAPELEN];
  /* calculate size of file */
  // move file point at the end of file 
  if ( (fseek(inFile, 0, SEEK_END)) == -1 ) {
    perror(ERR_FILE_IN);
    exit(EXIT_FAILURE);
  }
  // get the current position of the file pointer
  size = ftell(inFile);
  if ( size == -1 ) {
    // some file reading error
    perror(ERR_FILE_IN);
    exit(EXIT_FAILURE);
    }
  else if ( size > TAPELEN ) {
    // longer than a reel of tape
    fprintf(stderr, ERR_TOO_LONG);
    exit(EXIT_FAILURE);
    }
  // reset position in file
  fseek(inFile, 0, SEEK_SET);
  // read whole file
  if ( (size = fread(buffer, 1, TAPELEN, inFile)) <= 0 ) {
      perror (ERR_FILE_IN);
      exit (EXIT_FAILURE);
    }

  // reverse the contents
  top = size - 1;     // last index of buffer
  mid = size / 2 - 1; // middle index of buffer
  for (i = 0 ; i <= mid ; ++i) {
    int t = buffer[i];
    buffer[i] = buffer[top-i];
    buffer[top-i] = t;
  }

  // output reversed file
  if ( (fwrite(buffer, 1, size, outFile)) == -1 ) {
	perror (ERR_FILE_OUT);
	exit (EXIT_FAILURE);
      }

  return;
}   
    
	
      
  
