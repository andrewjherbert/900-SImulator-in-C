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
for each of languages.

The script x3.sh runs the Elliott 900 functional test program X3.

