from nscldaq.boot import ssh

s = ssh.sshPrimitive('localhost', 'ls')

out = s.stdout()
err = s.stderr()

print('stdout-----------')
for l in out:
    print(l)
print('stderr ------------')
for l in err:
    print(l)
