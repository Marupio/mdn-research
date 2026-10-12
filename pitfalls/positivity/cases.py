"""Deterministic benchmarks. Search methods receive power coefficients only.

Families intentionally include both sparse-positive constructions and hard valleys.
Known generating formulas justify positivity; they are never passed to a solver.
"""
from fractions import Fraction as Q
from random import Random
from common import add, mul, scale, power, atom_polynomial, expand_bernstein, trim


def valley(center, epsilon, order=2):
    return add(power([-Q(center),Q(1)],order),[Q(epsilon)])


def chebyshev_shifted(n):
    a,b=[Q(1)],[-Q(1),Q(2)]
    if n==0:
        return a
    for _ in range(1,n):
        a,b=b,add(scale(mul([-1,2],b),2),scale(a,-1))
    return b


def make_cases(seed=20261010, per_family=12):
    rng=Random(seed)
    cases=[]
    def put(name,family,p,description,split='fixed'):
        cases.append(dict(name=name,family=family,p=trim(p),description=description,split=split))
    put('constant','simple',[1],'1')
    put('affine','simple',[1,1],'1+t')
    put('carry_identity','simple',[1,-3,3],'t^2-t(1-t)+(1-t)^2')
    put('previous_example','simple',[11,-40,40],'10(2t-1)^2+1')
    put('sparse_bump','sparse_atoms',add([1],atom_polynomial(1,7)),'1+t(1-t)^7')
    put('sparse_mixed','sparse_atoms',add(add([1],atom_polynomial(2,10,7)),atom_polynomial(9,1,3)),'1+7t^2(1-t)^10+3t^9(1-t)')
    for den in [10,100,1000,10000]:
        put(f'center_valley_{den}','near_zero',valley(Q(1,2),Q(1,den)),f'(t-1/2)^2+1/{den}')
        put(f'offcenter_valley_{den}','near_zero',valley(Q(1,3),Q(1,den)),f'(t-1/3)^2+1/{den}')
    for order in [4,6,8]:
        put(f'flat_valley_{order}','near_zero',valley(Q(1,2),Q(1,10000),order),f'(t-1/2)^{order}+1/10000')
    put('two_valleys','near_zero',mul(valley(Q(1,3),Q(1,1000)),valley(Q(2,3),Q(1,1000))),'Product of two positive quadratics')
    put('endpoint_valley','near_zero',[Q(1,10000),0,1],'t^2+1/10000')
    for degree in [3,5,7]:
        g=chebyshev_shifted(degree)
        put(f'oscillatory_{degree}','oscillatory',add(mul(g,g),[Q(1,100)]),f'T_{degree}(2t-1)^2+1/100')
    for k in range(per_family):
        d=[4,8,12,16][k%4]
        p=[Q(rng.randint(1,5),rng.choice([1,2,5,10]))]
        for _ in range(rng.randint(2,5)):
            total=rng.randint(1,d);i=rng.randint(0,total)
            p=add(p,atom_polynomial(i,total-i,Q(rng.randint(1,9),rng.choice([1,2,3,5]))))
        put(f'planted_atoms_{k:02}','sparse_atoms',p,'Positive constant plus random positive atoms','generated')
        coeffs=[Q(rng.randint(1,30),rng.choice([1,2,3,5])) for _ in range(d+1)]
        put(f'positive_bernstein_{k:02}','positive_bernstein',expand_bernstein(coeffs),'Random positive Bernstein coefficients','generated')
        gd=[2,3,4,5][k%4]
        g=[Q(rng.randint(-4,4)) for _ in range(gd)]+[Q(rng.choice([-3,-1,1,3]))]
        eps=Q(1,[10,100,1000][k%3])
        put(f'random_square_{k:02}','random_squares',add(mul(g,g),[eps]),'Random polynomial square plus positive epsilon','generated')
        h=[Q(rng.randint(-3,3)) for _ in range(gd+1)]
        put(f'two_squares_{k:02}','random_squares',add(add(mul(g,g),mul(h,h)),[eps]),'Two random squares plus positive epsilon','generated')
        p=[Q(1)]
        for _ in range([2,3,4][k%3]):
            p=mul(p,valley(Q(rng.randint(1,9),10),Q(1,rng.choice([10,100,1000]))))
        put(f'product_valleys_{k:02}','product_valleys',p,'Product of positive rational quadratics','generated')
    return cases


def negative_controls():
    return [dict(name='interior_zero',p=[1,-4,4]),
            dict(name='negative_valley',p=[Q(1,4)-Q(1,100),-1,1]),
            dict(name='endpoint_zero',p=[0,1]),
            dict(name='negative_constant',p=[-1])]
