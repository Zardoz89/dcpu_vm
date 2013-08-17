; Basic DCPU compilance test

; set and literals
set a, 1
set b, 2
set c, 3
set x, 4
set y, 64
set z, -1

; Add and Sub
add a, 512  ; A must be 513
sub a, 1    ; A must be 512
add j, a    ; J must be 512
sub j, a    ; J must be 0
sub j, z    ; J must be 1 = 0 - (-1)

; Mul and Div
mul j, z    ; J must be 0xFFFF = -1
set j, 10
mul j, x    ; J must be 40
mul y, b    ; Y must be 128
div a, x    ; A must be 128

; Mod
mod c, 2    ; C must be 1

; MLI and DVI
set j, 10
mli j, z    ; J must be -10 -> 0xFFF6
dvi j, b    ; J must be -5 ->  0xFFFB

; Signed Mod
mdi j, 3    ; J must be -2 -> 0xFFFE

; AND, BOR and XOR
and z, 0xAA95 ; Z must be 0xAA95
bor z, 0x00FF ; Z must be 0xAAFF
and z, 0x5595 ; Z must be 0x0095
xor z, 0xAA95 ; Z must be 0xAA00

;Shifts
set z, 0x00F1
shr z, 4      ; Z must be 0x000F and EX = 0x1000
shl z, 4      ; Z must be 0x00F0 and EX = 0
set z, -128
asr z, 4      ; Z must be -32

; 32 bit add and sub
set a, 0x0FFF ; MSB 1
set b, 0xFF0F ; LSB 1

set x, 0x0111 ; MSB 2
set y, 0x1111 ; LSB 1
add b, y   ; add LSBs
adx a, x   ; add MSBs + EX
; B must be = 0x1020
; A must be = 0x1111

sub b, y
sbx a, x
; B must be 0xFF0F
; A must be 0x0FFF

; STI and STD test
set i, datos
set j, datos +1

sti [j], [i]
sti [j], [i]
; Must be at 0x800 write 0xCAFE 3 times
std [i], [j]
; Erase the last 0xCAFE

; Test of JSR and stack
set push, 0xCAFE
jsr subrut
set y, pop
; Z must be 0xBEBE and Y must be 0xCAFE

; Test of branchs
ifb 0x00AA, 0x0055
set pc, crash

ifc 0x00AA, 0x005A
set pc, crash

ife a, z
set pc, crash

ifn a, a
set pc, crash

ifg 1, 60000
set pc, crash

ifa -128, 1
set pc, crash

ifl 60000, 1
set pc, crash

ifu 1, -128
set pc, crash

; Chaining
ifu 1, -128
ife a, a
ifg 10, 1
set pc, crash

; Other stuff
hwn a
ifg a, 0
sub a, 1
hwq a

:crash
set pc, crash

:subrut  ; Subrutine that sets z to 0xBEBE
set z, 0xBEBE
set pc, pop


:str   dat "Hello world"
:datos dat 0xcafe
