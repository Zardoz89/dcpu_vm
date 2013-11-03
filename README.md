DCPU VM
=======
Multi-platform DCPU-16 Virtual Machine
-----------------------------------

Note that this VM is aimed to implement and test some of the proposed hardware devices of the [Trillek project](http://trillek.org/).
In addition, includes some extra tools.


Based on [Benedek Vartok VM](https://bitbucket.org/benedek/dcpu-16/overview)


### REQUISITES

DCPU VM needs SFML 2.1 installed in your system with the include files. 
CMake handles the job of find the include files if your setup your system ain the apropiated way.

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
Tested compiler :
  - GCC 4.7 Linux
  - MinGW 4.8 Windows
  - Visual Studio 2012
  - Add yours here !

RUNNING
-------

Just type **dcpu-vm --help** to get these infos.

    dcpu-vm 0.1
    usage : dcpu-vm [-options] <dcpu16-exe>
    --------------------------------------------------------
      options:
        -assemble (-a) : assemble before load (experimental)
        -output <filename> (-o) : output assembled filename
        -debug (-d) : start in debug mode
                F1  : next step
                F2  : print CPU status
                F10 : switch debug/run
        -time (-t) : use timed emulation (else refresh based)
        -vsync (-v) : use vertical synchronisation
                        (more accurate but may bug)
      Hardware loadout options:
        -loadout (-l) : XML file that describes hardware loadout. If isn't using
                a loadout file, then a default loadout will be used. The default
                loadout contains a generic clock, a generic keyboard, a monitor,
                a floppy drive and a sound speaker.
                The next options will be used to change the default loadout of
                the virtual machine.
        --monitor=<monitor_name> : use the following monitor
                1802 -> Lem1802 (default) [c] (-1802)
                1803 -> Lem1803 [c] (-1803)
                cgm -> Colour Graphics Monitor (-cgm)
                [c] : compatible with Lem1802 0x10c programs
        --no-sound : disable the sound speaker device
        -floppy <filenames...> (-fd) : floppy image files



There is a debug/step mode that can be activated and deactivated with **System+F12**:
 
 - **F1** : Single step (print the current instruction on the console)
 - **F2** : Print registers states into console

Available shortcuts all-time: 

 - **F3** : Reset the DCPU
 - **F4**  : Dump the ram into file "dump_ram"
 - **F9** : Ejects/Inserts all floppies!
 - **F10** : Activate/Deactivate debug mode.

LOAD-OUT DESCRIPTION FILES
--------------------------
A custom device load-out can be set using a XML file describing what devices will be used. The XML file follow this structure :

    <?xml version="1.0" encoding="utf-8"?>
    <loadout>
      <hardware>
        <device type="GenericClock" />
        <device type="GenericKeyboard" />
        <device type="M35FD" />
        <device type="LEM1803" />
        <device type="CGM1084" />
        <device type="SimpleSpeaker" />
      </hardware>
    </loadout>

The devices will be loaded in the same order that is in the XML file. Also can read the load-out from a **.10csln** solution file from **DevKit**.
Actually admit this device list:

- **GenericClock** : Generic Clock
- **GenericKeyboard** : Generic Keyboard
- **M35FD** : Mackpar 3.5" Floppy drive
- **LEM1802** : LEM 1802 monitor
- **LEM1803** : LEM 1803 monitor
- **CGM1084** : CGM 1084 monitor
- **SimpleSpeaker** : Basic buzzer / IBM PC like speaker device

TODO
----

 - Manager of floppy disks in use.
 - A tool to create floppies and fill/copy data to it.
 - GUI of the floppy drive to eject and insert different floppies.
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

![Creating a font with The Gimp](https://raw.github.com/Zardoz89/dcpu_vm/gh-pages/images/gimp_font.png)


BIN2DSK
--------
BIN2DSK is a small tool to convert bin dcpu programs into bootables floppy disk images (.dsk).

### RUN

    usage : bin2dsk <input.bin> <output.dsk> (-f)
    
Actually there are 2 way to boot from a floppy with the mrboot program : 
 - the program is small so it can be stored on the MBR (sector 0) and loaded at the address 0x7C00
 - the program is too big and it is stored into many sectors so only mrboot can boot it with a signature on the sector 0
 
You can force the program to create mrboot signed floppy by using the option -f  after the filenames 






