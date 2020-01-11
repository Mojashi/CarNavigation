import subprocess
from subprocess import Popen, PIPE
import sys

result = []
modes = ["MINDIST", "GA", "GREEDY"]
for mode in modes:
    rb = []
    for i in range(8):
        f = open(sys.argv[2] + "/test" + str(i)  +".txt", "r")
        p = subprocess.run([sys.argv[1], mode],input=f.read().encode(), stdout = PIPE)
        out = float(p.stdout.decode())
        
        rb.append(out)
        print(out)
    result.append(rb)

for i in range(3):
    print(modes[i] + ":", end="")
    print(result[i])      
