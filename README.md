DCPU VM
=======
Multi-platform DCPU-16 Virtual Machine
-----------------------------------

Note that this VM is aimed to implement and test some of the proposed hardware devices of the [Trillek project](http://trillek.org/).
In addition, includes some extra tools.


Based on [Benedek Vartok VM](https://bitbucket.org/benedek/dcpu-16/overview)


COMPILATION
-----------

    mkdir build
    cd build
    cmake ..
    make
    
To make a Release build (compiling with optimizations):

    cmake -DCMAKE_BUILD_TYPE=Release ..
    
To make a Debug build:

    cmake -DCMAKE_BUILD_TYPE=Debug ..

Run **make install** to copy the executable to the root of the project
This project fork is made to run perfectly on windows without boost

RUNNING
-------

Just type **dcpu-vm --help** to get these infos.

    dcpu-vm x.x.x
    usage : dcpu-vm [-options] <dcpu16-exe>
    --------------------------------------------------------
      options:
        -assemble (-a) : assemble before load (experimental)
        -debug (-d) : start in debug mode
                F1  : next step
                F2  : print CPU status
                F3  : reset (no need debug mode)
                F9  : ejects/insert floppy (no need debug mode)
                System + F12 : switch debug/run
        --monitor=<monitor_name> : use the following monitor
                1802 -> Lem1802 (default) [c] (-1802)
                1803 -> Lem1803 [c] (-1803)
                cgm -> Colour Graphics Monitor (-cgm)
                [c] : compatible with Lem1802 0x10c programs
        -output <filename> (-o) : output assembled filename
        -floppy <filename> (-fd) : floppy image file
        -time (-t) : use timed emulation (else refresh based)
        -vsync (-v) : use vertical synchronization
                        (more accurate but may bug)


There is a debug/step mode that can be activated and deactivated with **System+F12**:
 
 - **F1** : Single step (print the current instruction on the console)
 - **F2** : Print registers states into console

Avaliable shortcuts alltime: 

 - **F3** : Reset the DCPU
 - **F9** : Ejects/Inserts a floppy
 - **System+F12** : Activate/Deactivate debug mode. Note that System is Windows/Apple/Super key

TODO
----

 - <s>Finnish CGM code and create a 8x8 font</s>
 - Set a way of how configure what devices will be used in the machine
 - Several optimizations on rendering monitor loops
 - Rename variables/namespaces with a convention -> Apply code convention of Trillek proyect more strictly.
 - More options to Debug mode
 - DCPU manager and separated graphic rendering threads
 - Correct bad English :P
 
 
TOOLS
=====

PBM2FONT
--------
PBM2FONT is a small tool to create LEM180x and CGM 1084 fonts. It uses a ASCII **PBM** image file to create the HEX data values that represents the font.

### RUN

Just type **pbm2font --help** to get these infos.

    usage : pbm2font [-options] <input-file>
    --------------------------------------------------------
      options:
        -output <filename> (-o) : output filename
        -charset=<charset_type> : use the following charset type
                4x8 -> Generates 4x8 font charset
                8x8 -> Generates 8x8 font charset
        -format=<output_format> : use the following format
                dat -> Uses universal .dat output format
                hex_dump -> Generates a hexadecimal dump
                By default, the ouput format is "dat" and the charset is 4x8
                
If no output file has been chosen, then it will write to standard out.

### HOW IT WORKS

PBM2FONT reads the PBM file, and makes a grid of 4x8, or 8x8 cells. Each cell is converted to the appropriate format for 4x8 or 8x8 fonts. So to generate a font, you only need to use a graphics editor program that outputs ASCII PBM files (for example The Gimp), and set a grid of 4x8 or 8x8. Then you only need to draw each character in black and white and save the file.

![Creating a font with The Gimp](images/gimp_font.png)










