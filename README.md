# 900-Simulator-in-C

A simple simulator in C for the Elliott 900 range of minicomputers.

This repository consists of a simulator for the Elliott 900 series of minicomputers
written in C, along with supporting utilities.  It compliments a similar simulator
written in Python, available at

  https://github.com/andrewjherbert/900-Simulator-in-Python

This version has more diagnostic facilities than the Python version.  The file
formats ued by both programs are compatible.

emu900 is the simulator.

from900 is a utility program to convert Elliott 900 paper tape and teleprinter
code output to equivalent ASCII.

to900text converts a file containing ASCII characters to its equivalent in the
Elliott 900 paper tape and teleprinter code.

emu900 is the principal program to use.  It reads a dump of the machine store from
a file ".store" if present.  paper tape input is read from the file ".reader" which
should be a sequence of bytes binary file containing either an image of an Elliott
paper tape or the result of translating an ASCII file to Elliott 900 Telecode using
to990text.  900sim writes all teleprinter output to stdout.  Teleprinter input
is read from the file "ttyin". Any paper tape punch output is written as a sequence
of bytes to the file ".punch".  If the output was in Elliott 900 Telecode it can be
converted to ASCII using from900text.

At the end of a run of 900sim.py .reader is updated to contain any unconsumed input
and .store is updated with the new contents of the store.

Usage: emu900  [options] [ ptr [ ptp [ tty [ plot ]]]]

   ptr is file containing paper tape reader input.  Defaults to .reader.
   ptp is file to be used for paper tape punch output.  Defaults to .punch.
   tty is file to be used for teletype input. Defaults to .ttyin.
   plot is file to be used for plotter output in PNG format. Defaults to .paper.

   Verbosity is controlled by the -v option:

      1      -- diagnostic reports, e.g., dynamic stop, etc
      2      -- jumps taken
      4      -- each instruction
      8      -- input/output

   Options are:
      -vn      set verbosity to n, e.g., v4
      -tn      turn on diagnostics after n instructions, e.g., t888
      -sn      when execution first reaches location n turn on diagnostics
      -rn      when execution first reaches location n turn on -v7 and abandon
                   after 1000 instructions
      -an      abandon execution after n instructions, e.g., a9999
      -d       write diagnostics to file log.txt rather than stdout
      -m       monitor word n for changes, e.g., m100

  In addresses can also write m^n for word n of (8K) store module m to express
value n*8K+m.

The source code for the programs is in the directory src.

The "binaries" of paper tapes used by the emulator are in the directory bin.
This directory also holds store images for each of the language system tapes
(e.g., fort16klg_iss5_store is an image after loading the binary fort16klg
using initial instructions.)

There are scripts masd903algol.sh, ajh903algol.sh, 903fortran.sh 905fortran.sh
to run example programs which can be found in demos/903algol (Elliott Algol60),
demos/903fortran (Elliott FORTRAN II), demos/905fortran (Elliott FORTRAN IV).

masdalgol is Elliott 16K Load and Go Algol Issue 5 modified to accept { and } as
string quotes.

ajhalgol is a 16K Load and Go Algol system built by Andrew Herbert using Algol
Issue 6 sources and removes a number of restrictions from the implementation. It
also accepts { } as string quotes.


In the docs directory there are .docx / .pdf files containing a short "manual"
for each of languages

The script x3.sh runs the Elliott 900 functional test program X3.

