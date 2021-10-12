set print asm-demangle on
set pagination off
set disassembly-flavor intel

define hook-stop
x/6i $pc
end

b respond
c
