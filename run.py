import subprocess
from subprocess import Popen, PIPE
import random
import sys


Ncar = 5000

Vmax = 13.8888889
Vmin = 1.66666667
Pmut = 0.1
Psht = 0.1
Pind0 = 0.1
Pind1 = 0.5
Tstep = 300
Tpred = 1200
Npop = 40
Ngen = 250
Tsim = 10

Kjam = 0.144

def exec(programname, mode, Nrate):
    radius = 3
    length = 200
    dense = 8


    edges = []

    directed = False

    for i in range(radius):
        for j in range(dense):
            edges.append((i * dense + j,i * dense + (j + 1) % dense,length * (i + 1), Kjam))
            if i > 0:
                edges.append((i * dense + j,(i-1) * dense + j % dense,length , Kjam))


    edges.append((radius * dense + 0,(radius-1) * dense,length*2 , Kjam))
    edges.append((radius * dense + 1,(radius-1) * dense + 2 ,length*2 , Kjam))
    edges.append((radius * dense + 2,(radius-1) * dense + 4 ,length*2 , Kjam))
    edges.append((radius * dense + 3,(radius-1) * dense + 6 ,length*2 , Kjam))

    with Popen(programname,stdout = subprocess.PIPE,stdin = subprocess.PIPE) as p:
        instr = mode + "\n"

        instr += str(Vmax) + " " + str(Vmin) + " " + str(Pmut) + " " + str(Psht) + " " + str(Pind0) + " " + str(Pind1) +"\n"
        instr += str(Tstep)+ " " + str(Tpred)+ " " + str(Npop)+ " " + str(Ngen)+ " " + str(Tsim) +"\n"

        instr += str(radius * dense + 4) + " " + str(len(edges)*2) +"\n"
        for edge in edges:
            if directed == False:
                instr +=str(edge[1]) + " " +str(edge[0]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"
            instr +=str(edge[0]) + " " +str(edge[1]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"

        instr +=str(Ncar) +"\n"
        for i in range(Ncar):
            fr = random.randint(0,3)
            to = (fr+2)%4
            instr +=str(radius * dense + fr) +" " +  str(radius * dense + to) + " " + str(i // Nrate) +"\n"

        print(instr)
        out,err = p.communicate(input=instr.encode())
        return float(out.decode())

if __name__ == "__main__":
    res = exec(sys.argv[1], sys.argv[2], int(sys.argv[3]))
    print(res)