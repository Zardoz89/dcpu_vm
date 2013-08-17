DCPU VM
=======

Another DCPU-16 VM

Based on Benedek Vartok VM (https://bitbucket.org/benedek/dcpu-16/overview)

COMPILING
---------

  make

RUN
---

  ./Debug/DCPU dcpuBinFileProgram
  

Actually the VM have attached a fake LEM1802 monitor that shows in terminal a 
B&W ASCII representation of the screen. Allow to see text on it, and respond to 
all commands of a real LEM1802, but don't uses palettes, color, border color or font maps.
It refresh at 60Hz (1666,7 cpu cycles), sou will need press a few times any key 
to see anything in the terminal.

TODO
----

- Implement more hardware devices
- Realtime execution at 100KHz

BENCHMARKS
----------

In a AMD FX-4100 with 8GiB DDR3 and with Kubuntu 13.04 x64


Input BIN File : ../tester.bin
Readed 286 bytes - 143 words
Threads 1        CPU PerThread 10000    N cpus 10000
Cycles 1000
Measured time: 841ms

Threads 4        CPU PerThread 2500     N cpus 10000
Cycles 1000
Measured time: 242ms

SpeedUp = 841 / 242 = 3.47

Threads 40       CPU PerThread 250      N cpus 10000
Cycles 1000
Measured time: 84ms

SpeedUp = 841 / 84 = 10.01

Input BIN File : ../tester.bin
Readed 286 bytes - 143 words
Threads 1        CPU PerThread 40000    N cpus 40000
Cycles 2000
Measured time: 14173ms

Threads 4        CPU PerThread 10000    N cpus 40000
Cycles 2000
Measured time: 5115ms

SpeedUp = 14173 / 5115 = 2.77

Threads 40       CPU PerThread 1000     N cpus 40000
Cycles 2000
Measured time: 926ms

SpeedUp = 14173 / 926 = 15.3


