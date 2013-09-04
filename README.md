DCPU VM
=======

  - Base code : Zardoz (https://github.com/Zardoz89/dcpu_vm)
  - Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)
  

COMPILING
---------
    cd build
    cmake ..
    make
    
To doing a release build (compiling with optimizations):
    cmake -DCMAKE_BUILD_TYPE=Release ..

Run make install to copy the executable to the root of the project
This project fork is created to run perfectly on windows without boost

RUN
---

Just type "dcpu-wm --help" to get these infos.
There is a debug/step mode activate/desactivate with F12:
 - F1 : Single step (print the current instruction on the console)
 - F2 : Print registers states into console
 - F3 : Reset the DCPU


TODO
----

 - Finnish CGM code
 - Several optimisations on rendering monitor loops
 - Rename variables/namespaces with a convention
 - Debug/Step mode with disassembler
 - Better keyboard support
 - DCPU manager and separated graphic rendering threads
 - Correct bad english










