DCPU VM
=======

  - Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)
  

COMPILING
---------
    cd build
    cmake ..
    make
    
To doing a release build (compiling with optimizations):

    cmake -DCMAKE_BUILD_TYPE=Release ..
    
To doing a DEBUG build:

    cmake -DCMAKE_BUILD_TYPE=DEBUG ..

Run make install to copy the executable to the root of the project
This project fork is created to run perfectly on windows without boost

RUN
---

Just type "dcpu-wm --help" to get these infos.

    usage : dcpu-vm [--options] <dcpu16-exe>
    --------------------------------------------------------
    options:
        --debug                  : start in debug mode
            F1  : next step
            F2  : print registers
            F3  : reset (no need debug mode)
            F12 : switch debug/run
        --monitor=<monitor_name> : use the following monitor
            1802 -> Lem1802 (default) [c]
            1803 -> Lem1803 [c]
            cgm -> Colour Graphics Monitor
            [c] : compatible with Lem1802 0x10c programs

There is a debug/step mode activate/desactivate with F12:
 
 - F1 : Single step (print the current instruction on the console)
 - F2 : Print registers states into console
 - F3 : Reset the DCPU


TODO
----

 - Finnish CGM code and create a 8x8 font
 - Several optimisations on rendering monitor loops
 - Rename variables/namespaces with a convention -> Apply code convention of Trillek proyect more strictly.
 - Debug/Step mode with disassembler
 - Better keyboard support
 - DCPU manager and separated graphic rendering threads
 - Correct bad english :P











