import sys
assert len(sys.argv) == 3

f1 = open(sys.argv[1], 'rb')
f2 = open(sys.argv[2], 'rb')

a1 = f1.read()
a2 = f2.read()

print(a1 == a2)
f1.close()
f2.close()
