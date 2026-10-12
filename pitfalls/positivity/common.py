"""Independent exact semantics and verification for positivity experiments.

Polynomial coefficients are ascending powers of t. MDN atoms mean
c*t**i*(1-t)**j, with an explicit rational scale (not bounded integer digits).
No search routine is imported by the verifier.
"""
from fractions import Fraction as Q
from math import comb, gcd, lcm
import json


def trim(p):
    p = list(map(Q, p)) or [Q(0)]
    while len(p) > 1 and not p[-1]:
        p.pop()
    return p


def add(a, b):
    c = [Q(0)] * max(len(a), len(b))
    for i, v in enumerate(a):
        c[i] += v
    for i, v in enumerate(b):
        c[i] += v
    return trim(c)


def scale(p, c):
    return trim([Q(c)*v for v in p])


def mul(a, b):
    c = [Q(0)] * (len(a)+len(b)-1)
    for i, x in enumerate(a):
        for j, y in enumerate(b):
            c[i+j] += x*y
    return trim(c)


def power(p, n):
    out = [Q(1)]
    for _ in range(n):
        out = mul(out, p)
    return out


def evaluate(p, t):
    out = Q(0)
    for c in reversed(p):
        out = out*t+c
    return out


def affine(p, a, width):
    """Exact power coefficients of p(a+width*t)."""
    if a==0 and width==1:
        return trim(p)
    out = [Q(0)]
    for c in reversed(p):
        out = add(mul(out, [Q(a), Q(width)]), [c])
    return trim(out)


def atom_polynomial(i, j, c=Q(1)):
    if type(i) is not int or type(j) is not int or min(i,j) < 0:
        raise ValueError('Atom exponents must be nonnegative integers')
    return [Q(0)]*i+[Q(c)*comb(j,k)*(-1)**k for k in range(j+1)]


def expand_atoms(atoms):
    out = [Q(0)]
    for i,j,c in atoms:
        out = add(out, atom_polynomial(i,j,Q(c)))
    return out


def expand_bernstein(coeffs):
    n = len(coeffs)-1
    out = [Q(0)]
    for k,c in enumerate(coeffs):
        out = add(out, atom_polynomial(k,n-k,Q(c)*comb(n,k)))
    return out


def canonical_bernstein(p,n):
    """Exact canonical coefficients used by the independent checker.

    The basis identity is tested independently by full power expansion.
    A degree-only certificate asks the checker to recompute these coefficients.
    """
    p=trim(p)
    if type(n) is not int or n<len(p)-1:
        raise ValueError('Bernstein degree below actual polynomial degree')
    denominators=[comb(n,j) for j in range(len(p))]
    return [sum((c*Q(comb(k,j),denominators[j]) for j,c in enumerate(p) if j<=k),Q(0))
            for k in range(n+1)]


def _positive_atoms(atoms):
    if not atoms or any(Q(c)<0 for _,_,c in atoms):
        raise ValueError('Missing or negative atom coefficient')
    if sum(Q(c) for i,j,c in atoms if i==0)<=0:
        raise ValueError('Left endpoint is not strictly positive')
    if sum(Q(c) for i,j,c in atoms if j==0)<=0:
        raise ValueError('Right endpoint is not strictly positive')
    return expand_atoms(atoms)


def _get_coefficients(piece):
    factor=Q(piece.get('scale',1))
    if factor<=0:
        raise ValueError('Certificate scale must be positive')
    if 'coefficients' in piece:
        return [factor*Q(c) for c in piece['coefficients']]
    n = piece['degree']
    if type(n) is not int or n<0:
        raise ValueError('Invalid Bernstein degree')
    out = [Q(0)]*(n+1)
    seen = set()
    for k,c in piece['nonzero_coefficients']:
        if type(k) is not int or k<0 or k>n or k in seen:
            raise ValueError('Invalid or duplicate coefficient index')
        seen.add(k)
        out[k]=factor*Q(c)
    return out


def verify(p, certificate):
    """Raise ValueError for any failure; exact identities, no sample points."""
    p = trim(p)
    kind = certificate['kind']
    if kind=='bernstein_recipe':
        pieces=certificate.get('pieces',[dict(a=0,b=1,degree=certificate.get('degree'))])
        if not pieces:
            raise ValueError('Empty domain partition')
        boundary=Q(0)
        for piece in pieces:
            a,b=Q(piece['a']),Q(piece['b'])
            if a!=boundary or b<=a or b>1:
                raise ValueError('Domain partition has gap, overlap, or reversed interval')
            coefficients=canonical_bernstein(affine(p,a,b-a),piece['degree'])
            if min(coefficients)<0 or coefficients[0]<=0 or coefficients[-1]<=0:
                raise ValueError('Recomputed Bernstein coefficients do not certify positivity')
            boundary=b
        if boundary!=1:
            raise ValueError('Domain partition does not cover [0,1]')
        return True
    if kind=='bernstein':
        certificate=dict(kind='bernstein_pieces',pieces=[dict(certificate,a=0,b=1)])
        kind='bernstein_pieces'
    if kind == 'mdn_atoms':
        factor=Q(certificate.get('scale',1))
        if factor<=0:
            raise ValueError('Certificate scale must be positive')
        if trim(scale(_positive_atoms(certificate['atoms']),factor)) != p:
            raise ValueError('Atom identity differs from the input polynomial')
        return True
    if kind not in ('mdn_pieces', 'bernstein_pieces'):
        raise ValueError('Unsupported certificate kind')
    pieces = certificate['pieces']
    if not pieces:
        raise ValueError('Empty domain partition')
    boundary = Q(0)
    for piece in pieces:
        a,b = Q(piece['a']),Q(piece['b'])
        if a!=boundary or b<=a or b>1:
            raise ValueError('Domain partition has gap, overlap, or reversed interval')
        target = affine(p,a,b-a)
        if kind=='mdn_pieces':
            factor=Q(piece.get('scale',1))
            if factor<=0:
                raise ValueError('Certificate scale must be positive')
            actual = scale(_positive_atoms(piece['atoms']),factor)
        else:
            coeffs = _get_coefficients(piece)
            if not coeffs or any(c<0 for c in coeffs) or coeffs[0]<=0 or coeffs[-1]<=0:
                raise ValueError('Bernstein coefficients do not certify strict positivity')
            if coeffs!=canonical_bernstein(target,len(coeffs)-1):
                raise ValueError('Piece Bernstein coefficients differ from input polynomial')
            actual=target
        if trim(actual)!=target:
            raise ValueError('Piece identity differs from the restricted input polynomial')
        boundary = b
    if boundary!=1:
        raise ValueError('Domain partition does not cover [0,1]')
    return True


def jsonable(value):
    if isinstance(value,Q):
        return value.numerator if value.denominator==1 else str(value)
    if isinstance(value,dict):
        return {k:jsonable(v) for k,v in value.items()}
    if isinstance(value,(list,tuple)):
        return [jsonable(v) for v in value]
    return value


def rational_bits(c):
    c=Q(c)
    return max(1,abs(c.numerator).bit_length())+1+c.denominator.bit_length()


def uint_bits(n):
    return max(1,int(n).bit_length())


def coefficient_encodings(values):
    """Individual rationals or primitive integers with one shared scale."""
    values=list(map(Q,values))
    yield values,None,sum(rational_bits(c) for c in values)
    denominator=lcm(*(c.denominator for c in values)) if values else 1
    integers=[c.numerator*(denominator//c.denominator) for c in values]
    divisor=gcd(*integers) if integers else 1
    if not divisor:
        return
    factor=Q(divisor,denominator)
    integers=[c//divisor for c in integers]
    bits=(rational_bits(factor) if factor!=1 else 0)+sum(max(1,abs(c).bit_length())+1 for c in integers)
    yield integers,(factor if factor!=1 else None),bits


def metrics(certificate):
    """Explicit payload model: rational values, exponents, degrees, endpoints.

    For each Bernstein leaf choose dense or zero-sparse encoding. All methods
    may use primitive integer coefficients with one shared rational scale.
    This is a defined comparison format, not a bound on all possible compression.
    """
    kind=certificate['kind']
    if kind=='bernstein_recipe':
        pieces=certificate.get('pieces',[dict(a=0,b=1,degree=certificate.get('degree'))])
        index_bits=sum(uint_bits(p['degree']) for p in pieces)
        domain_bits=(sum(rational_bits(p['a'])+rational_bits(p['b']) for p in pieces)
                     if 'pieces' in certificate else 0)
        packed=jsonable(certificate)
        return dict(support=0,coefficient_slots=0,coefficient_bits=0,index_bits=index_bits,
                    domain_bits=domain_bits,payload_bits=index_bits+domain_bits,
                    json_bytes=len(json.dumps(packed,separators=(',',':')).encode()),
                    max_degree=max(p['degree'] for p in pieces),leaves=len(pieces)),packed
    if kind=='bernstein':
        certificate=dict(kind='bernstein_pieces',pieces=[dict(certificate,a=0,b=1)])
        kind='bernstein_pieces'
    coefficients=index_bits=domain_bits=support=slots=max_degree=0
    packed={'kind':kind}
    if kind=='mdn_atoms':
        pieces=[{'a':Q(0),'b':Q(1),'atoms':certificate['atoms'],'scale':certificate.get('scale',1)}]
    else:
        pieces=certificate['pieces']
    global_domain=(len(pieces)==1 and Q(pieces[0]['a'])==0 and Q(pieces[0]['b'])==1)
    encoded=[]
    for piece in pieces:
        result={}
        if kind!='mdn_atoms' and not global_domain:
            a,b=Q(piece['a']),Q(piece['b'])
            domain_bits+=rational_bits(a)+rational_bits(b)
            result.update(a=a,b=b)
        if 'atoms' in piece:
            factor=Q(piece.get('scale',1))
            if factor<=0:
                raise ValueError('Certificate scale must be positive')
            atoms=[(int(i),int(j),factor*Q(c)) for i,j,c in piece['atoms'] if Q(c)]
            choices=[]
            for vals,factor,bits in coefficient_encodings(c for _,_,c in atoms):
                representation={'atoms':[(i,j,c) for (i,j,_),c in zip(atoms,vals)]}
                if factor is not None:
                    representation['scale']=factor
                choices.append((bits,representation))
            bits,representation=min(choices,key=lambda x:x[0])
            coefficients+=bits
            index_bits+=sum(uint_bits(i)+uint_bits(j) for i,j,_ in atoms)
            support+=len(atoms);slots+=len(atoms)
            max_degree=max(max_degree,max((i+j for i,j,_ in atoms),default=0))
            result.update(representation)
        else:
            coeffs=_get_coefficients(piece);n=len(coeffs)-1
            nz=[(k,c) for k,c in enumerate(coeffs) if c]
            choices=[]
            for vals,factor,bits in coefficient_encodings(coeffs):
                representation={'coefficients':vals}
                if factor is not None:
                    representation['scale']=factor
                choices.append((bits+uint_bits(n),bits,uint_bits(n),representation))
            for vals,factor,bits in coefficient_encodings(c for _,c in nz):
                indices=uint_bits(n)+sum(uint_bits(k) for k,_ in nz)
                representation={'degree':n,'nonzero_coefficients':[(k,c) for (k,_),c in zip(nz,vals)]}
                if factor is not None:
                    representation['scale']=factor
                choices.append((bits+indices,bits,indices,representation))
            _,bits,indices,representation=min(choices,key=lambda x:x[0])
            coefficients+=bits;index_bits+=indices
            result.update(representation)
            support+=len(nz);slots+=len(coeffs);max_degree=max(max_degree,n)
        encoded.append(result)
    if global_domain:
        packed['kind']='mdn_atoms' if 'atoms' in encoded[0] else 'bernstein'
        packed.update(encoded[0])
    else:
        packed['pieces']=encoded
    packed=jsonable(packed)
    return dict(support=support,coefficient_slots=slots,coefficient_bits=coefficients,
                index_bits=index_bits,domain_bits=domain_bits,
                payload_bits=coefficients+index_bits+domain_bits,
                json_bytes=len(json.dumps(packed,separators=(',',':')).encode()),
                max_degree=max_degree,leaves=len(pieces)),packed
