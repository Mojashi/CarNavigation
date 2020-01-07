import subprocess
from subprocess import Popen, PIPE
import random
import sys


#入力形式
#Vmax[m/sec] Vmin[m/sec] Pmut Psht Pind0 Pind1
#Tstep[sec] Tpred[sec] Npop, Ngen Tsim

#|V| |E|
#from to length Kjam[台/m]
#...

#|Car|
#from to startT
#...

programname = sys.argv[1]
Nrate = int(sys.argv[2])
Ncar = 5000

radius = 3

Vmax = 13.8888889
Vmin = 1.66666667
Pmut = 0.1
Psht = 0.1
Pind0 = 0.1
Pind1 = 0.5
Tstep = 30
Tpred = 1200
Npop = 20
Ngen = 25
Tsim = 10

length = 783
dense = 8
Kjam = 0.144

edges = []

directed = False

for i in range(radius):
    for j in range(dense):
        edges.append((i * dense + j,i * dense + (j + 1) % dense,length , Kjam))
        if i > 0:
            edges.append((i * dense + j,(i-1) * dense + j % dense,length , Kjam))

            
edges.append((radius * dense + 0,(radius-1) * dense,length , Kjam))
edges.append((radius * dense + 1,(radius-1) * dense + 2 ,length , Kjam))
edges.append((radius * dense + 2,(radius-1) * dense + 4 ,length , Kjam))
edges.append((radius * dense + 3,(radius-1) * dense + 6 ,length , Kjam))

with Popen(programname,stdin = subprocess.PIPE) as p:
    instr = ""
    
    instr += str(Vmax) + " " + str(Vmin) + " " + str(Pmut) + " " + str(Psht) + " " + str(Pind0) + " " + str(Pind1) +"\n"
    instr += str(Tstep)+ " " + str(Tpred)+ " " + str(Npop)+ " " + str(Ngen)+ " " + str(Tsim) +"\n"
    
    instr += str(radius * dense + 4) + " " + str(len(edges)*2) +"\n"
    for edge in edges:
        if directed == False:
            instr +=str(edge[1]) + " " +str(edge[0]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"
        instr +=str(edge[0]) + " " +str(edge[1]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"
    
    instr +=str(Ncar) +"\n"
    for i in range(Ncar):
        fr = random.randint(radius * dense,radius * dense + 3)
        to = fr
        while to == fr:
            to = random.randint(radius * dense,radius * dense + 3)
        instr +=str(fr) +" " +  str(to) + " " + str(i // Nrate) +"\n"
    
    print(instr)
    p.communicate(input=instr.encode())
