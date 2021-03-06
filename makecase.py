import subprocess
from subprocess import Popen, PIPE
import random
import sys


Ncar = 500

Vmax = 13.8888889
Vmin = 1.66666667
Pmut = 0.1
Psht = 0.1
Pbps = 0.5
Pind0 = 0.1
Pind1 = 0.5
Tstep = 300
Tpred = 1200
Npop = 40
Ngen = 250
Tsim = 10

Kjam = 0.144
length = 200

directed = False

def makeGraph(mode):
    edges = []
    depCand = []
    N = 0
    if mode == "NET":
        radius = 3
        dense = 8
        for i in range(radius):
            for j in range(dense):
                edges.append((i * dense + j,i * dense + (j + 1) % dense,length * (i + 1), Kjam))
                if i > 0:
                    edges.append((i * dense + j,(i-1) * dense + j % dense,length , Kjam))


        edges.append((radius * dense + 0,(radius-1) * dense,length*2 , Kjam))
        edges.append((radius * dense + 1,(radius-1) * dense + 2 ,length*2 , Kjam))
        edges.append((radius * dense + 2,(radius-1) * dense + 4 ,length*2 , Kjam))
        edges.append((radius * dense + 3,(radius-1) * dense + 6 ,length*2 , Kjam))
        N = radius * dense + 4
        depCand.append((radius * dense + 0, radius * dense + 2))
        depCand.append((radius * dense + 1, radius * dense + 3))
        depCand.append((radius * dense + 2, radius * dense + 0))
        depCand.append((radius * dense + 3, radius * dense + 1))
    if mode == "GRID":
        W = 5
        H = 5
        for i in range(H):
            for j in range(W):
                if j < W-1:
                    edges.append((i * W + j , i * W + j + 1,length, Kjam))
                if i < H-1:
                    edges.append((i * W + j , (i + 1) * W + j ,length, Kjam))
                if j < W-1 and i < H-1:
                    edges.append((i * W + j, (i + 1) * W + j + 1,length * 2, Kjam))
                if j > 0 and i < H-1:
                    edges.append((i * W + j, (i + 1) * W + j - 1,length * 2, Kjam))

        depCand.append((0,W*H-1))
        depCand.append((W*H-1,0))
        depCand.append((W-1, W*(H-1)))
        depCand.append((W*(H-1),W-1))
        N = H * W

    return N,edges,depCand

def output(Nrate, GRAPH, fname):

    Nv,edges,depCand = makeGraph(GRAPH)
    
    instr = ""

    instr += str(Vmax) + " " + str(Vmin) + " " + str(Pmut) + " " + str(Psht) + " " + str(Pbps) + " " + str(Pind0) + " " + str(Pind1) +"\n"
    instr += str(Tstep)+ " " + str(Tpred)+ " " + str(Npop)+ " " + str(Ngen)+ " " + str(Tsim) +"\n"

    instr += str(Nv) + " " + str(len(edges)*2) +"\n"
    for edge in edges:
        if directed == False:
            instr +=str(edge[1]) + " " +str(edge[0]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"
        instr +=str(edge[0]) + " " +str(edge[1]) + " " +str(edge[2]) + " " +str(edge[3]) +"\n"

    instr +=str(Ncar) +"\n"
    for i in range(Ncar):
        fr,to = depCand[random.randint(0, len(depCand) - 1)]
        instr +=str(fr) +" " +  str(to) + " " + str(i // Nrate) +"\n"


    file = open(fname, 'w')
    file.write(instr)
    file.close()

if __name__ == "__main__":
    for i in range(8):
        output((i+1)*5, sys.argv[1], "case/net/test"+str(i)+".txt")
