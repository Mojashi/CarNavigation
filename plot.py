import run
import sys

result = []

for i in range(5):
    result.append(run.exec(sys.argv[1], sys.argv[2], i*8 + 5))

print(result)
