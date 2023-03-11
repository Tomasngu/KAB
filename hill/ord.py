from sympy import Matrix

most_frequent = ('HO', 'WX', 'MT', 'OD', 'IL', 'ZT', 'PP', 'AW', 'EZ', 'MR')
text = 'XAEMOPTBHOQQBJOMVHRZPEWNBVYJMIEZAWSKKZZTIWSNUXOLPIMIEZAWHHBLYPLLIHEMTCAHPPLXILNV'

P = Matrix([[19, 7], [7, 4]]) # TH a HE
P_inv = Matrix([[4, 19], [19, 19]]) # Vzato z 1. prednasky


def get_num(letter: str) -> int:
    return ord(letter) - ord('A')
def to_char(p: int) -> str:
    return chr(p + 65)

def decipher(c_matrix):
    A = c_matrix*P_inv
    A_inv = A.inv_mod(26)
    OT = ''
    for i in range(0, len(text), 2):
        c_vector = Matrix([get_num(text[i]), get_num(text[i+1])])
        p_vector = (A_inv * c_vector) % 26
        OT += to_char(p_vector[0])
        OT += to_char(p_vector[1])
    print(OT)

def hill():
    for c1 in most_frequent:
        for c2 in most_frequent:
            if c1 == c2:
                continue
            c_matrix = Matrix([[get_num(c1[0]), get_num(c2[0])], [get_num(c1[1]), get_num(c2[1])]])
            print(f'TH -> {c1}, HE -> {c2}', end=' ')
            try:
                decipher(c_matrix)
            except ValueError:
                print('Matrix is not invertible')

def get_text():
    c_matrix = Matrix([[get_num('H'), get_num('P')], [get_num('O'), get_num('P')]])
    A = c_matrix*P_inv
    A_inv = A.inv_mod(26)
    with open('a.txt') as f:
        while True:
            c = f.read(2)
            if not c:
                break
            c_vector = Matrix([get_num(c[0]), get_num(c[1])])
            p_vector = (A_inv * c_vector) % 26
            print(to_char(p_vector[0]) + to_char(p_vector[1]), end = '')
hill()


