; find out hardware map
HWN I
SUB I, 1
:query_loop
HWQ I
IFE B, 0x7349
  IFE A, 0xf615
    SET [monitor_n], I
SUB I, 1
IFE EX, 0
  SET PC, query_loop

; set up monitor
SET A, 0
SET B, 0x8000
HWI [monitor_n]


HWN Z

; Print Title:
SET I, 0
:l1
SET A, [title+I]
IFE A, 0
  SET PC, l2
BOR A, 0x0f00
SET [0x8000+I], A
ADD I, 1
SET PC, l1
:l2

SET I, 0x20
SET J, 0
:ident_loop
IFE J, Z
  SET PC, ident_end
HWQ J
SET PUSH, A
SET A, J
JSR printhex
ADD I, 6
SET A, B
JSR printhex
ADD I, 5
SET A, POP
JSR printhex
ADD I, 6
SET A, C
JSR printhex
ADD I, 6
SET A, Y
JSR printhex
ADD I, 5
SET A, X
JSR printhex
ADD I, 4
ADD J, 1
SET PC, ident_loop
:ident_end

SUB PC, 1


; Print A in hex to 0x8000+I
:printhex
SET PUSH, A
SET PUSH, B
SET PUSH, I
SET PUSH, J
SET J, I
SUB J, 1
ADD I, 3
:printhex_l1
SET B, A
AND B, 0x0f
IFG B, 9
  ADD B, 7
ADD B, 0xf030
SET [0x8000+I], B
SHR A, 4
SUB I, 1
IFN I, J
  SET PC, printhex_l1
SET J, POP
SET I, POP
SET B, POP
SET A, POP
SET PC, POP


:title
DAT "Idx.  Hardw. ID  Rev.  Manuf. ID", 0

:monitor_n
DAT 0

