// Elliott 903 emulator - Andrew Herbert - 24/02/2021

// Emulator for Elliott 903 / 920B.
// Does not implement 'undefined' effects.
// Has simplified handling of priority levels and initial orders.
// No support (as yet) for: interactive use of teletype, line printer, 
// graph plotter, card reader or magnetic tape.

// Ths program is written C--, i.e. a strict subset of ANSI C without using
// pointer arithmetic, or struct
//                     but I could not resist the C++ comment convention

// Code assumes int is >= 19 bits and long int >= 38 bits.

// Usage: emu900  [options] [ ptr [ ptp [ tty ]]]
//
// ptr is file containing paper tape reader input.  Defaults to .reader.
// ptp is file to be used for paper tape punch output.  Defaults to .punch.
// tty is file to be used for teletype input. Defaults to .ttyin.

// Verbosity is controlled by the -v option:
//
//    1      -- diagnostic reports, e.g., dynamic stop, etc
//    2      -- jumps taken
//    4      -- each instruction
//    8      -- input/output

// Options are:
//    -vn      set verbosity to n, e.g., v4
//    -tn      turn on diagnostics after n instructions, e.g., t888
//    -sn      when execution first reaches location n turn on diagnostics
//    -rn      when execution first reaches location n turn on -v7 and abandon
//                 after 1000 instructions  
//    -an      abandon execution after n instructions, e.g., a9999
//    -d       write diagnostics to file log.txt rather than stdout
//    -m       monitor word n for changes, e.g., m100

// can also write m^n for word n of (8K) store module m to express value n*8K+m.

// At beginning reads in contents of store from file .store if available,
// otherwise core is set to all zeros. At end, dumps out contents of store
// to .store, unless catastrophic errors. This is to simulate retention of
// data in core store between entry points.

// By default reads paper tape input from file .reader unless overridden by
// reader ptrfile argument on the command line. At end copies any unconsumed
// paper tape input to  .reader overwriting previous content, unless
// catastrophic errors, overwriting existing contents. This is to emulate
// leaving a tape in the reader between successive runs.

// The input file should be raw bytes representing eight bit paper tape
// codes, either binary of one of the Elliott telecodes.  There is a companion
// program "to900text" which converts a UTF-8 character file to its equivalent
// in Elliott 900 telecode. (Be aware that a UTF-8 file which looks like ASCII
// might have an invisible BOM code at the beginning, so generally it is always
// safer to run text input tapes through to900text.

// Teletype input is handled similarly, taken from the file .ttyin unless
// overridden by teleprinter file argument on the command line. Teletype output
// is sent to stdout.  The emulator does not emulate interactive use of the
// teletype very well.

// By default paper tape output is send to file .punch, unless overridden by
// punch file argument on the command line.  There is a companion program
// "from900text" which converts a file containing 900 telecode output to it's
// UTF-8 equivalent. (Note since 900 telecode includes ASCII and cariage returns
// it is generally best to convert to a normal ASCII tool before processing further.

// By default the simulator jumps to 8181 to start execution, unless overriden by
// -j option on the command line.

// A limit on maximum number of instructions to be executed can be set using
// the -limit command line option.

// A trace in file .trace will be written if -trace command line option is
// present.  There is a companion program "traceprint.py" that produces am
// interpreted listing of the trace.

// The program exits with an exit code indicating the reason for completion,
// e.g., 0 = dynamic stop, 1 = run out of paper tape input, etc.2 = run out of
// teletype input, 3 = reached execution limit, 255 = catastropic error.  The
// contents of .store, .reader and  .punch are undefined after a catastrophic
// error.

/**********************************************************/
/*                     HEADER FILES                       */
/**********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>


/**********************************************************/
/*                         DEFINES                        */
/**********************************************************/


// Default file names
#define LOG_FILE   "log.txt"   // diagnostic output file path
#define RDR_FILE   ".reader"   // paper tape reader input file path
#define PUN_FILE   ".punch"    // paper tape punch output file path
#define TTYIN_FILE ".ttyin"    // teletype input file path
#define STORE_FILE ".store"    // store image - n.b., ERR_FOPEN_STORE_FILE

#define USAGE "Usage: emu900[-adjmrstv] <reader file> <punch file> <teletype file>\n"
#define ERR_FOPEN_DIAG_LOGFILE  "Cannot open log file"
#define ERR_FOPEN_RDR_FILE      "Cannot open paper tape input file"
#define ERR_FOPEN_PUN_FILE      "Cannot open paper tape punch file"
#define ERR_FOPEN_TTYIN_FILE    "Cannot open teletype input file"
#define ERR_FOPEN_STORE_FILE    "Cannot open .store file for writing"

// Booleans
#define TRUE  1
#define FALSE 0

// Exit codes from emulation
#define EXIT_DYNSTOP       0 // = EXIT_SUCCESS
//      EXIT_FAILURE       1
#define EXIT_RDRSTOP       2
#define EXIT_TTYSTOP       4
#define EXIT_LIMITSTOP     8

/* Useful constants */
#define BIT19       01000000
#define MASK18       0777777
#define MASK18L      0777777L
#define BIT18       00400000
#define MASK16      00177777
#define ADDR_MASK       8191
#define MOD_MASK    00160000
#define MOD_SHIFT         13
#define FN_MASK           15
#define FN_SHIFT          13

// Locations of B register and SCR for priority levels 1 and 4
#define SCRLEVEL1  0
#define SCRLEVEL4  6
#define BREGLEVEL1 1
#define BREGLEVEL4 7

#define STORE_SIZE 16384 // 16K


/**********************************************************/
/*                         GLOBALS                        */
/**********************************************************/


/* these next two declarations will blow up if int < 19 bits */
int           bits18 =      262144;  // modulus for 18 bit operations
long long int bits36 = 68719476736L; // modulus for 64 bit operations

/* Diagnostics related variables */
FILE *diag;              // diagnostics output - se to either  stderr or .log

/* File handles for peripherals */
FILE *ptr  = NULL;       // paper tape reader
FILE *pun  = NULL;       // paper tape punch
FILE *ttyi = NULL;       // teleprinter input
FILE *ttyo = NULL;       // teleprinter output

int verbose   = 0;       // no diagnostics by default
int diagCount = -1;      // turn diagnostics on at this instruction count
int abandon   = -1;      // abandon on this instruction count INT_MAX
int diagFrom  = -1;      // turn on diagnostics when first reach this address
int diagLimit = -1;      // stop after this number of instructions executed
int monLoc    = -1;      // report if this location changes
int monLast   = -1;

/* Input output streams */
char *ptrPath   = RDR_FILE;    // path for reader input file
char *ptpPath   = PUN_FILE;    // path for punch output file
char *ttyInPath = TTYIN_FILE;  // path for teletype input file

int lastttych   = -1; // last tty character punched

/* Emulated store */
int store [STORE_SIZE];
int storeValid = FALSE; // set TRUE when a store image loaded

/* Machine state */
int opKeys = 8181; // setting of keys on operator's control panel, overidden by
                   // -j option
int aReg  = 0, qReg  = 0;
int bReg  = BREGLEVEL1, scReg = SCRLEVEL1; // address in store of B register and SCR
int lastSCR; // used to detect dynamic loops
int level = 1; // priority level
int iCount = 0; // count of instructions executed
int instruction, f, a, m;
int fCount[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // function code counts

/* Tracing */
int traceOne      = FALSE; // TRUE => trace current instruction only



/**********************************************************/
/*                         FUNCTIONS                      */
/**********************************************************/


void decodeArgs(int argc, char **argv); // decode command line
void catchInt();        // interrupt handler
int  argtoi(char* arg); // read numeric part of argument
void usageError();      // report argument error

void emulate();         // run emulation
void clearStore();      // clear main store
void readStore();       // read in a store image
void tidyExit();        // tidy up and exit
void writeStore();      // dump out store image

void printDiagnostics(int i, int f, int a); // print diagnostic information for current instruction
void printTime(long long us);  // print out time counted in microseconds
void printAddr(FILE *f, int addr); // print address in m^nnn format
  
int readTape();         // red from paper tape
void punchTape(int ch); // punch to paper tape
int readTTY();          // read from teletype
void writeTTY(int ch);  // write to teletype
void flushTTY();        //force output of last tty output line
void loadII();          // load initial orders
int makeIns(int m, int f, int a); // help for loadII


/**********************************************************/
/*                           MAIN                         */
/**********************************************************/


void main (int argc, char **argv) {
   signal(SIGINT, catchInt); // allow control-C to end cleanly
   decodeArgs(argc, argv);   // decode command line and set options etc
   emulate();                // run emulation
}

void catchInt(int sig, void (*handler)(int)) {
  fprintf(stderr, "*** Execution terminated by interrupt\n");
  tidyExit(EXIT_FAILURE);
}


/**********************************************************/
/*                  DECODE ARGUMENTS                      */
/**********************************************************/


void decodeArgs (int argc, char **argv) {
   char buffer[3000]; // not checked for overflow
   int i = 0;
   char *s;
   diag = stderr; // default output for diagnostics
   buffer[0] = '\0'; // null string
   while  ( (++i < argc)  &&  (*(s = argv[i]) == '-') ) { // process options
      strcat(buffer, " ");
      strcat(buffer, s);
      if  ( *++s == 'd' ) {
	if  ( !(diag = fopen("log.txt", "w")) ) // diagnostics to file
	  {
	    perror(ERR_FOPEN_DIAG_LOGFILE);
	    exit(EXIT_FAILURE);
	    /* NOTREACHED */
	  }
      }
      else if  ( *s == 'v' )             // set verbosity
        verbose   = argtoi(s+1);
      else if  ( *s == 'a' )             // abandon after n
        abandon   = argtoi(s+1);
      else if  ( *s == 'j' )             // initial jump location
	opKeys    = argtoi(s+1);
      else if  ( *s == 't' )             // turn on diagnostics after n instructions
	diagCount = argtoi(s+1);
      else if  ( *s == 's' )             // turn on diagnostics at address n
	diagFrom  = argtoi(s+1);
      else if  ( *s == 'r' )             // as r but only 1000 diagnostics
        diagLimit = argtoi(s+1);  
      else if  ( *s == 'm' )             // monitor address
        monLoc    = argtoi(s+1);
      else {                             // unknown flag
	usageError();
	/* NOT REACHED */
      } // if
   } // while
   // check arguments are reasonable
   if  ( (monLoc >= STORE_SIZE ) || (diagFrom >= STORE_SIZE)
	 || (diagLimit >= STORE_SIZE) )
     {
        fprintf(stderr, "Diagnostics address outside available store\n");
        exit(EXIT_FAILURE);
        /* NOT REACHED */
     }
   // check for file arguments
   if  ( argc >   i ) ptrPath   = (argv[i]);
   if  ( argc > ++i ) ptpPath   = (argv[i]);
   if  ( argc > ++i ) ttyInPath = (argv[i]);
   if  ( (verbose & 1) > 0 )
     {
       fprintf(diag, "Options are:%s\n", buffer);
       fprintf(diag, "Paper tape will be read from % s\n", ptrPath);
       fprintf(diag, "Paper tape will be punched to % s\n", ptpPath);
       fprintf(diag, "Teletype input will be read from %s\n", ttyInPath);
     };
}

int argtoi (char *s) {
  int value = 0;
  while  ( *s != '\0' )
    {
      int ch = *s++;
      if  ( isdigit(ch) )
        value = value * 10 + ch - (int)'0'; 
      else if ( ch == '^' )
        {
          value = value * 8192 + atoi(s);
          return value; }
      else usageError();
    } // while
  return value;
}

void usageError () {
  fprintf(stderr, USAGE);
  exit(EXIT_FAILURE);
}


/**********************************************************/
/*                         EMULATION                      */
/**********************************************************/


void emulate () {

  int exitCode      = EXIT_SUCCESS; // reason for terminating
  int tracing       = FALSE; // true if tracing enabled
  long long emTime   = 0L; // crude estimate of 900 elapsed time

  // set up machine ready to execute
  clearStore();  // start with a cleared store
  readStore();   // read in store image if available
  loadII();      // load initial orders
  ttyo = stdout; // teletype output to stdout
  store[scReg] = opKeys; // set SCR from operator control panel keys
  
  if   ( (verbose & 1) > 0 )
    {
      fprintf(diag,"Starting execution from location ");
      printAddr(diag, opKeys);
      fputc('\n', diag);
    }
  if   ( monLoc >= 0 ) monLast = store[monLoc]; // set up monitoring

  // instruction fetch and decode loop
  while ( ++iCount )
    {
      
      // increment SCR
      lastSCR = store[scReg];         // remember SCR
      if   ( lastSCR >= STORE_SIZE )
        {
          fprintf(diag, "*** SCR has overflowed the store (SCR = %d)\n", lastSCR);
	  flushTTY();
  	  tidyExit(EXIT_FAILURE);
        }
      store[scReg]++;                 // increment SCR

      // fetch and decode instruction
      instruction = store[lastSCR];
      f = (instruction >> FN_SHIFT) & FN_MASK;
      a = (instruction & ADDR_MASK) | (lastSCR & MOD_MASK);
      fCount[f]+=1; // track number of executions of each function code

      // perform B modification if needed
      if ( instruction >= BIT18 )
        {
  	  m = (a + store[bReg]) & MASK16;
	  emTime += 6;
	}
      else
	  m = a & MASK16;

      // perform function determined by function code f
      switch ( f )
        {

        case 0: // Load B
	    qReg = store[m]; store[bReg] = qReg;
	    emTime += 30;
	    break;

          case 1: // Add
       	    aReg = (aReg + store[m]) & MASK18;
	    emTime += 23;
	    break;

          case 2: // Negate and add
	    aReg = (store[m] - aReg) & MASK18;
	    emTime += 26;
	    break;

          case 3: // Store Q
	    store[m] = qReg >> 1;
	    emTime += 25;
	    break;

          case 4: // Load A
	    aReg = store[m];
	    emTime += 23;
	    break;

          case 5: // Store A
	    if   ( level == 1 && m >= 8180 && m <= 8191 )
	      {
		if ( ((verbose & 1) > 0) )
	            fprintf(diag,
		      "Write to initial instructions ignored in priority level 1");
	      }
	    else
	      store[m] = aReg; 
	    emTime += 25;
	    break;

          case 6: // Collate
	    aReg &= store[m];
	    emTime += 23;
	    break;

          case 7: // Jump if zero
  	    if   ( aReg == 0 )
	      {
	        traceOne = tracing && ((verbose & 2) > 0);
	        store[scReg] = m;
		emTime += 28;
	      }
	    if  ( aReg > 0 )
	      emTime += 21;
	    else
	      emTime += 20;
	    break;

          case 8: // Jump unconditional
	    store[scReg] = m;
	    emTime += 23;
	    break;

          case 9: // Jump if negative
	    if   ( aReg >= BIT18 )
	      {
	        traceOne = tracing && ((verbose & 2) > 0);
		store[scReg] = m;
		emTime += 25;
	      }
	    emTime += 20;
	    break;

          case 10: // increment in store
 	    store[m] = (store[m] + 1) & MASK18;
	    emTime += 24;
	    break;

          case 11:  // Store S
	    {
              int s = store[scReg];
	      qReg = s & MOD_MASK;
	      store[m] = s & ADDR_MASK;
	      emTime += 30;
	      break;
	    }

          case 12:  // Multiply
	    {
	      // extend sign bits for a and store[m]
	      long long al = (long long) ( ( aReg >= BIT18 ) ? aReg - BIT19 : aReg );
	      long long sl = (long long) ( ( store[m] >= BIT18 ) ? store[m] - BIT19 : store[m] );
	      long long  prod = al * sl;
	      qReg = (int) ((prod << 1) & MASK18L );
	      if   ( al < 0 ) qReg |= 1;
	      prod = prod >> 17; // arithmetic shift
 	      aReg = (int) (prod & MASK18L);
	      emTime += 79;
	      break;
	    }

          case 13:  // Divide
	    {
	      // extend sign bit for aq
	      long long al   = (long long) ( ( aReg >= BIT18 ) ? aReg - BIT19 : aReg ); // sign extend
	      long long ql   = (long long) qReg;
	      long long aql  = (al << 18) | ql;
	      long long ml   = (long long) ( ( store[m] >= BIT18 ) ? store[m] - BIT19 : store[m] );
              long long quot = (( aql / ml) >> 1) & MASK18L;
	      int q     = (int) quot;
  	      aReg = q | 1;
	      qReg = q & 0777776;
	      emTime += 79;
	      break;
	    }

          case 14:  // Shift - assumes >> applied to a signed long or int is arithmetic
	    {
              int places = m & ADDR_MASK;
	      long long al  = (long long) ( ( aReg >= BIT18 ) ? aReg - BIT19 : aReg ); // sign extend
	      long long ql  = qReg;
	      long long aql = (al << 18) | ql;
	      int i;
	      
	      if   ( places <= 2047 )
	        {
		  emTime += (24 + 7 * places);
	          if   ( places >= 36 ) places = 36;
	          aql <<= places;
	        }
	      else if ( places >= 6144 )
	        { // right shift is arithmetic
	          places = 8192 - places;
		  emTime += (24 + 7 * places);
	          if ( places >= 36 ) places = 36;
		  aql >>= places;
	        }  
	      else
	        {
	          fprintf(stderr, "*** Unsupported i/o 14 i/o instruction\n");
	          printDiagnostics(instruction, f, a);
	          tidyExit(EXIT_FAILURE);
	          /* NOT REACHED */
	        }

	      qReg = (int) (aql & MASK18L);
	      aReg = (int) ((aql >> 18) & MASK18L);
	      break;
	    }

            case 15:  // Input/output etc
	      {
                int z = m & ADDR_MASK;
	        switch   ( z )
	    	  {

		    case 2048: // read from tape reader
		      { 
	                int ch = readTape(); int a = aReg;
	                aReg = ((aReg << 7) | ch) & MASK18;
			emTime += 4000; // assume 250 ch/s reader
	                break;
	               }

	            case 2052: // read from teletype
		      {
	                int ch = readTTY();
	                aReg = ((aReg << 7) | ch) & MASK18;
			emTime += 100000; // assume 10 ch/s teletype
	                break;
	              } 

	            case 6144: // write to paper tape punch 
	              punchTape(aReg & 255);
		      emTime += 9091; // assume 110 ch/s punch
	              break;

	            case 6148: // write to teletype
	              writeTTY(aReg & 255);
		      emTime += 100000; // assume 10 ch/s teletype
	              break;	      
	  
	            case 7168:  // Level terminate
	              level = 4;
	              scReg = SCRLEVEL4;
		      bReg  = BREGLEVEL4;
		      emTime += 19;
	              break;

	            default:
		      flushTTY();
	              fprintf(stderr, "*** Unsupported 15 i/o instruction\n");
	              printDiagnostics(instruction, f, a);
	              tidyExit(EXIT_FAILURE);
	              /* NOT REACHED */
		  } // end 15 switch
	      } // end case 15
	} // end function switch

        // check for change on monLoc
        if   ( monLoc >= 0 && store[monLoc] != monLast )
	  {
	    fprintf(diag, "Monitored location changed from %d to %d\n",
		monLast, store[monLoc]);
	    monLast = store[monLoc];
	    traceOne = TRUE;
          }

        // check to see if need to start diagnostic tracing
        if   ( (lastSCR == diagFrom) || ( (diagCount != -1) && (iCount >= diagCount)) )
	    tracing = TRUE;
        if   ( iCount == diagLimit )
	  {
	    tracing = TRUE;
	    abandon = iCount + 1000; // trace 1000 instructions
	  }

        // print diagnostics if required
        if   ( traceOne )
	  {
 	    flushTTY();
	    traceOne = FALSE; // dealt with single case
  	    printDiagnostics(instruction, f, a);
          }
	else if ( tracing && ((verbose & 4) > 0) )
	  {
	    flushTTY();
	    printDiagnostics(instruction, f, a);
	  }
	  
	// check for limits
        if   ( (abandon != -1) && (iCount >= abandon) )
        {
	  flushTTY();
          if  ( (verbose & 1) > 0 ) fprintf(diag, "Instruction limit reached\n");
          exitCode = EXIT_LIMITSTOP;
  	  break;
        }

        // check for dynamic stop
        if   ( store[scReg] == lastSCR )
	  {
	    flushTTY();
	    if   ( (verbose & 1) > 0 )
	      {
	        fprintf(diag, "Dynamic stop at ");
	        printAddr(diag, lastSCR);
	         fputc('\n', diag);
	       }
	     exitCode = EXIT_DYNSTOP;
	     break;
	  }
    } // end while fetching and decoding instructions

  // execution complete
  if   ( (verbose & 1) > 0 ) // print statistics
    {
      int i;
      fprintf(diag, "Function code count\n");
      for ( i = 0 ; i <= 15 ; i++ )
	{
	  fprintf(diag, "%4d: %8d (%3d%%)", i, fCount[i], (fCount[i] * 100) / iCount);
	  if  ( ( i % 4) == 3 ) fputc('\n', diag);
	}
       fprintf(diag, "%d instructions executed in ", iCount);
       printTime(emTime);
       fprintf(diag, " of simulated time\n");
     }

  tidyExit(exitCode);
}


/**********************************************************/
/*              STORE DUMP AND RECOVERY                   */
/**********************************************************/

 
void clearStore() {
  int i;
  for ( i = 0 ; i < STORE_SIZE ; i++ ) store[i] = 0;
  if  ( (verbose & 1) > 0 )
    fprintf(diag, "Store (%d words) cleared\n", STORE_SIZE);
}

void readStore () {
  FILE *f  = fopen(STORE_FILE, "r");
  if   ( f != NULL )
    {
      // read store image from file
      int i = 0, n, c;
      while ( (c = fscanf(f, "%d", &n)) == 1 )
	{
	  if  ( i >= STORE_SIZE )
	    {
	      fprintf(stderr, "*** %s exceeds store capacity (%d)\n", STORE_FILE, STORE_SIZE);
	      exit(EXIT_FAILURE);
	      /* NOT REACHED */
	    }
	  else store[i++] = n;
	} // while
      if ( c == 0 )
 	{
	  fprintf(stderr, "*** Format error in file %s\n", STORE_FILE);
	  exit(EXIT_FAILURE);
	  /* NOT REACHED */
        }
      else if ( ferror(f) )
	{
	  fprintf(stderr, "*** Error while reading %s", STORE_FILE);
	  perror(" - ");
	  exit(EXIT_FAILURE);
	  /* NOT REACHED */
        }
      fclose(f); // N.B. STORE_FILE gets re-opening for writing at end of execution
      if   ( (verbose & 1) > 0 )
	fprintf(diag, "%d words read in from %s\n", i, STORE_FILE);
    }
  else if  ( (verbose & 1) > 0 ) 
    fprintf (diag, "No %s file found, store left empty\n", STORE_FILE);

  storeValid = TRUE;
}

void writeStore () {
   FILE *f = fopen(STORE_FILE, "w");
   int i;
   if  ( f == NULL ) {
      perror(ERR_FOPEN_STORE_FILE);
      exit(EXIT_FAILURE);
      /* NOT REACHED */ }
   for ( i = 0 ; i < STORE_SIZE ; ++i )
     {
       fprintf(f, "%7d", store[i]);
       if  ( ((i%10) == 0) && (i!=0) ) fputc('\n', f);
     }
   if  ( (verbose & 1) > 0 )
	 fprintf(diag, "%d words written out to %s\n", STORE_SIZE, STORE_FILE);
   fclose(f);
}


/**********************************************************/
/*                      DIAGNOSTICS                       */
/**********************************************************/


 
 void printDiagnostics(int instruction, int f, int a) {
   // extend sign bit for A, Q and B register values
   int an = ( aReg >= BIT18 ? aReg - BIT19 : aReg); 
   int qn = ( qReg >= BIT18 ? qReg - BIT19 : qReg);
   int bn = ( store[bReg] >= BIT18 ? store[bReg] - BIT19 : store[bReg]);
   fprintf(diag, "%10d   ", iCount); // instruction count
   printAddr(diag, lastSCR);    // SCR and registers
   if   (instruction & BIT18 )
     {
       if   ( f > 9 )
	 fprintf(diag, " /");
      else
	fprintf(diag, "  /"); }
    else if  (f > 9 )
      fprintf(diag, "  ");
    else
      fprintf(diag, "   ");
    fprintf(diag, "%d %4d", f, a);
    fprintf(diag, " A=%+8d (&%06o) Q=%+8d (&%06o) B=%+7d (",
		 an, aReg, qn, qReg, bn);
    printAddr(diag, store[bReg]);
    fprintf(diag, ")\n");
}

void printTime (long long us) { // print out time in us
   int hours, mins; float secs;
   hours = us / 360000000L;
   us -= (hours * 360000000L);
   mins = us / 60000000L;
   secs = ((float) (us - mins * 60000000L)) / 1000000L;
   fprintf(diag, "%d hours, %d minutes and %2.2f seconds", hours, mins, secs);
}

 void printAddr (FILE *f, int addr) { // print out address in module form
   fprintf(f, "%d^%04d", (addr >> MOD_SHIFT) & 7, addr & ADDR_MASK);
}

/* Exit and tidy up */
 
void tidyExit (int reason) {
  if ( storeValid )
    {
      writeStore(); // save store for next run
      if   ( (verbose & 1) > 0 )
	fprintf(diag, "Copying over residual input to %s\n", RDR_FILE);
      if  ( ptr  != NULL )
	{
	  int ch;
	  FILE *ptr2 = fopen(RDR_FILE, "wb");
	  if  ( ptr2 == NULL )
	    {
	      printf("*** Unable to save paper tape to %s", RDR_FILE);
	      perror("");
	      putchar('\n');
	      fclose(ptr);
	      exit(EXIT_FAILURE);
	      /* NOT REACHED */
	    }
	  while ( (ch = fgetc(ptr)) != EOF ) fputc(ch, ptr2);
	  fclose(ptr);
	  fclose(ptr2);
	}
      if  ( pun  != NULL ) fclose(pun);
      if  ( ttyi != NULL ) fclose(ttyi);
    }
  if ( (verbose & 1) > 0) printf("Exiting %d\n", reason);
  exit(reason);
}


/**********************************************************/
/*                    PAPER TAPE SYSTEM                   */
/**********************************************************/


/* Paper tape reader */
int readTape() {
  int ch;
  if   ( ptr == NULL )
    {
      if  ( (ptr = fopen(ptrPath, "rb")) == NULL )
	{
          printf("*** %s ", ERR_FOPEN_RDR_FILE);
          perror(ptrPath);
          putchar('\n');
          tidyExit(EXIT_FAILURE);
	  /* NOT REACHED */
        }
      else if  ( (verbose & 1) > 0 )
	{
	  flushTTY();
	fprintf(diag, "Paper tape reader file %s opened\n", ptrPath);
	}
    }
  if  ( (ch = fgetc(ptr)) != EOF )
      {
	if  (( verbose & 8 ) > 0 )
	  {
	    flushTTY();
	    traceOne = TRUE;
	    fprintf(diag, "Paper tape character %3d read\n", ch);
	  }
        return ch;
      }
    else
      {
	flushTTY();
        if  ( (verbose & 1) > 0 )
	  fprintf(diag, "Run off end of input tape\n");
        tidyExit(EXIT_RDRSTOP);
	/* NOT REACHED */
      }
}

/* paper tape punch */
void punchTape(int ch) {
  if  ( pun == NULL )
    {
      if  ( (pun = fopen(ptpPath, "wb")) == NULL )
	{
	  flushTTY();
	  printf("*** %s ", ERR_FOPEN_PUN_FILE);
	  perror("ptpPath");
	  putchar('\n');
	  tidyExit(EXIT_FAILURE);
	  /* NOT REACHED */
	}
      else if  ( (verbose & 1) > 0 )
	{
	  flushTTY();
	 fprintf(diag, "Paper tape punch file %s opened\n", ptpPath);
	}
    }
  if  ( fputc(ch, pun) != ch )
    {
      flushTTY();
      printf("*** Problem writing to ");
      perror(ptpPath);
      putchar('\n');
      tidyExit(EXIT_FAILURE);
      /* NOT REACHED */
    }
  if  ( (verbose & 8 ) > 0 )
    {
      flushTTY();
      traceOne = TRUE;
      fprintf(diag, "Paper tape character %d punched\n", ch);
    }
}

/* Teletype */
int readTTY() {
  int ch;
  if   ( ttyi == NULL )
    {
      if  ( (ttyi = fopen(ttyInPath, "rb")) == NULL )
	{
	  flushTTY();
          printf("*** %s ", ERR_FOPEN_TTYIN_FILE);
          perror(ttyInPath);
          putchar('\n');
          tidyExit(EXIT_FAILURE);
	  /* NOT REACHED */
        }
      else if ( (verbose & 1) > 0 )
	{
	  flushTTY();
	  fprintf(diag,"Teletype input file %s opened\n", TTYIN_FILE);
	}
    }
    if  ( (ch = fgetc(ttyi)) != EOF )
      {
	if ( (verbose & 8 ) > 0 )
	  {
	    flushTTY();
	    traceOne = TRUE;
	    fprintf(diag, "Read character %d from teletype\n", ch);
	  }
	putchar(ch); // local echoing assumed
        return ch;
      }
    else
      {
        if  ( (verbose & 1) > 0 )
	  {
	    flushTTY();
	    fprintf(diag, "Run off end of teleprinter input\n");
	  }
        tidyExit(EXIT_TTYSTOP);
      }
}

void writeTTY(int ch) {
  int ch2 = ( ((ch &= 127) == 10 ) || ((ch >= 32) && (ch <= 122)) ? ch : -1 );
  if  ( (verbose & 8) > 0 )
    {
      flushTTY();
      traceOne = TRUE;
      fprintf(diag, "Character %d output to teletype", ch);
      if  ( ch2 == -1 )
	fprintf(diag, " - ignored\n");
      else
	fprintf(diag, "(%c)\n", ch2);
    }
    if  ( ch2 != -1 )
      putchar((lastttych = ch2));
}

void flushTTY() {
  if  ( (lastttych != -1) && (lastttych != '\n') )
    {
      putchar('\n');
      lastttych = -1;
    }
}

/**********************************************************/
/*               INITIAL INSTRUCTIONS                     */
/**********************************************************/


void loadII() {
  store[8180] = (-3 & MASK18);
  store[8181] = makeIns(0,  0, 8180);
  store[8182] = makeIns(0,  4, 8189);
  store[8183] = makeIns(0, 15, 2048);
  store[8184] = makeIns(0,  9, 8186);
  store[8185] = makeIns(0,  8, 8183);
  store[8186] = makeIns(0, 15, 2048);
  store[8187] = makeIns(1,  5, 8180);
  store[8188] = makeIns(0, 10,    1);
  store[8189] = makeIns(0,  4,    1);
  store[8190] = makeIns(0,  9, 8182);
  store[8191] = makeIns(0,  8, 8177);
  if  ( (verbose & 1) > 0 )
    fprintf(diag, "Initial orders loaded\n");
}

int makeIns(int m, int f, int a) {
  return ((m << 17) | (f << 13) | a);
}
