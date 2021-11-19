/* Support program for 900 series emulator to convert ASCII to     */
/* 900 Telecode/                                                   */
/* from900text inFile [outFile]                                    */
/* outFile defaults to .reader                                     */
/*                                                                 */
/* Andrew Herbert 18 November 2021                                 */
  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <locale.h>
#include <wchar.h>

#define OUTFILE ".reader"    // output file

#define ERR_FOPEN_INPUT  "Cannot open input file"
#define ERR_FOPEN_OUTPUT "Cannot open output file"
#define ERR_STRDUP       "Unexpected error in strdup in main"
#define ERR_FILE_IN      "Unexpected error with input file"
#define ERR_FILE_OUT     "Unexpected error with output file"

#define HALTCODE "<! HALT !>"
#define HALTCODELEN 9

#define OPTSTR "i:a:"
#define USAGE_FMT  "to900text inputfile [outputfile]"

void convert (FILE *inFile, FILE *outFile);
int addParity (int code);

int main (int argc, char *argv[]) {
  char *inPath, *outPath = OUTFILE;
  FILE *inFile, *outFile;

  setlocale(LC_ALL, ""); // set system locale

  // decode arguments
  if ( argc < 2 )
    { fprintf(stderr, USAGE_FMT);
      exit(EXIT_FAILURE);
      /* NOTREACHED */
    }
  else
    { inPath = argv[1];
    }
  if ( argc>=3 )
    { outPath = argv[2];
    };

  // open files
  if (!(inFile = fopen(inPath, "r")) ){
       perror(ERR_FOPEN_INPUT);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }
 if (!(outFile = fopen(outPath, "wb")) ){
       perror(ERR_FOPEN_OUTPUT);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
     }    

  // perform conversion
  convert(inFile, outFile);
  
  return EXIT_SUCCESS;
}

void convert (FILE *inFile, FILE *outFile) {
  static char haltCode[] = HALTCODE;
  int ptr = -1;
  wchar_t wideCh;
  fwide (inFile, 1); /* enable wide characters */
  while ( (wideCh = fgetwc(inFile) ) != WEOF)
    { if ( wideCh > 128 ) // ignore non-ASCII codes, e.g, if input is in UTF-8
	  continue; 
      if ( wideCh == haltCode[++ptr] )
	{ // matching against HALTCODE
          if ( ptr == HALTCODELEN )
	      { // matched to end
	        ptr =- 1; // reset pointer
	        fputc(20, outFile);
	      }
	}
      else
	{ // match failed, empty buffer and then output character
	  for ( int i = 0 ; i < ptr; i++ )
	    fputc(addParity(haltCode[i]), outFile);
	  fputc(addParity(wideCh), outFile);
	  ptr =- 1; // reset pointer
	};
    };
  return;
}

int addParity (int code) {
  int p = 0, c = code;
  while ( c != 0 )
    { if ( (c & 1) > 0) p++;
      c >>= 1;
    }
  if ( (p & 1) > 0)
    return (code + 128); // odd parity, make even
  else
    return code;
}
    
	
      
  
