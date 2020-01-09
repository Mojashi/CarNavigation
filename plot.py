import run
import sys

result = []

for i in range(8):
    result.append(run.exec(sys.argv[1], sys.argv[2], i*5 + 5))

print(result)
