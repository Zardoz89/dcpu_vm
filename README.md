DCPU VM
=======

Another DCPU-16 VM

Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)

COMPILING
---------

    make
    
To doing a relese build (compiling with optimizations):
    make release

RUN
---

    ./dcpu_vm dcpuBinFileProgram

The program will you ask if you like to run the benchmark or step execution mode.

In step execution mode, each time that you press enter, except 'f' or 'q' followed by enter,
you will exceute a CPU clock cycle. If you press 'q' followed by enter , you will end the VM
, and if you press 'f' followed by enter, you will exceute directly 100 cycles.

The benchmark setup is coded in some contants at the begin of main.cpp.  

Actually the VM have attached a fake LEM1802 monitor that shows in terminal a 
B&W ASCII representation of the screen. Allow to see text on it, and respond to 
all commands of a real LEM1802, but don't uses palettes, color, border color or font maps.
It refresh at 60Hz (1666,7 cpu cycles), so you will need press a few times any key 
to see anything in the terminal, in step mode.


TODO
----

- Implement more hardware devices
- Realtime execution at 100KHz
- A more precise benchmarks
- make tesetes.dasm more exaustive, covering all posible cases and putting comments about 
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








