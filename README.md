DCPU VM
=======

Another DCPU-16 VM

Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)

COMPILING
---------
    cd build
    cmake ..
    make
    
To doing a release build (compiling with optimizations):
    cmake -DCMAKE_BUILD_TYPE=Release ..

Run make install to copy the executable to the root of the project

RUN
---

    ./dcpu_vm dcpuBinFileProgram

The program will you ask if you like to run the threaded benchmark, step execution mode, single VM benchmark and run 100K cycles.

In step execution mode, each time that you press enter, except 'f' or 'q' followed by enter,
you will execute a CPU clock cycle. If you press 'q' followed by enter , you will end the VM
, and if you press 'f' followed by enter, you will execute directly 100 cycles.

The benchmark setup is coded in some constants at the begin of main.cpp.

The "run 100K cycles" exeutes the VM at realtime speed. At the end of the emulation, the program will wait to press any character to finish the execution.

Actually the "step" and "run 100k" modes VM have attached a LEM1802 and LEM1803 monitor that are showed in a window. It refresh at 30Hz (~833 cpu cycles), so you will need press a few times any key to see anything in the terminal, in step mode.


TODO
----

- Implement more hardware devices
- A more precise benchmarks
- make tester.dasm more exhaustive, covering all possibly cases and putting comments about 
  expected results and wait cycle

BENCHMARKS
----------

In a AMD FX-4100 quad-core with 8GiB DDR3 with Kubuntu 13.04 x64

Note : With 1000000 cycles, a "real" dcpu will take 10000ms to execute, so
       trying to get bit less minor time, say us how many DCPUs can run at
       same time

    Threads 1        CPU PerThread 3400     N cpus 3400
    Cycles 1000000
    Measured time: 62518ms


    Threads 4        CPU PerThread 850      N cpus 3400
    Cycles 1000000
    Measured time: 13871ms

    SpeedUp = x4.5

    Threads 200      CPU PerThread 17       N cpus 3400
    Cycles 1000000
    Measured time: 9837ms

    SpeedUp = x6.35








