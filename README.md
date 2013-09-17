DCPU VM
=======
Multi-platform DCPU-16 Virtual Machine
-----------------------------------

Note that this VM is aimed to implement and test some of the proposed hardware devices of the <a href="http://trillek.org/" target="_blank">Trillek proyect</a>.


Based on <a href="https://bitbucket.org/benedek/dcpu-16/overview" target="_blank">Benedek Vartok VM</a>
  

COMPILING
---------
    mkdir build
    cd build
    cmake ..
    make
    
To doing a Release build (compiling with optimizations):

    cmake -DCMAKE_BUILD_TYPE=Release ..
    
To doing a Debug build:

    cmake -DCMAKE_BUILD_TYPE=Debug ..

Run make install to copy the executable to the root of the project
This project fork is created to run perfectly on windows without boost

RUN
---

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


There is a debug/step mode activate/deactivate with **System+F12**:
 
 - **F1** : Single step (print the current instruction on the console)
 - **F2** : Print registers states into console

Avaliable shortcuts alltime: 

 - **F3** : Reset the DCPU
 - **F9** : Ejects/Inserts a floppy
 - **System+F12** : Activate/Deactivate debug mode. Note that System is Windows/Apple/Super key

TODO
----

 - Finnish CGM code and create a 8x8 font
 - Set a way of how configure what devices will be used in the machine
 - Several optimizations on rendering monitor loops
 - Rename variables/namespaces with a convention -> Apply code convention of Trillek proyect more strictly.
 - More options to Debug mode
 - DCPU manager and separated graphic rendering threads
 - Correct bad English :P











