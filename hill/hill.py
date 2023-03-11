import sympy
import numpy



occur = {}
count = 0
with open('a.txt') as f:
    while True:
        c = f.read(2)
        if not c:
            break
        if not c in occur:
            occur[c] = 1
        else:
            occur[c] += 1
        

print(dict(sorted(occur.items(), key=lambda item: item[1], reverse=True)))


        
