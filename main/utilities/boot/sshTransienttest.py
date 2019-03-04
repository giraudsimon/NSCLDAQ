import sys

from nscldaq.boot import ssh

s = ssh.Transient('localhost', 'ls -l *.ctbl')

print('stdout -------------')

for l in s.output():
    sys.stdout.write(l)

print('stderr ----------')
for l in s.error():
    sys.stdout.write(l)
