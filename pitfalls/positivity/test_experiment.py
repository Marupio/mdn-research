"""Meaningful independent exact-identity and adversarial-verifier checks."""
from copy import deepcopy
from fractions import Fraction as Q
from random import Random
import unittest

from common import (add, mul, affine, trim, verify, metrics, expand_bernstein,
                    expand_atoms, atom_polynomial)
from bernstein import power_to_bernstein, elevate, split, certify_uniform, certify_subdivision, certify_hybrid
from mdn_search import coarsen_bernstein


class ExactSemanticsTests(unittest.TestCase):
    def test_random_basis_conversions_and_subdivision(self):
        rng=Random(7301)
        for _ in range(50):
            d=rng.randrange(1,9)
            p=[Q(rng.randrange(-20,21),rng.randrange(1,8)) for _ in range(d+1)]
            for n in [d,d+1,d+5]:
                b=power_to_bernstein(p,n)
                self.assertEqual(trim(p),expand_bernstein(b))
                self.assertEqual(trim(p),expand_bernstein(elevate(b)))
                left,right=split(b,Q(2,5))
                self.assertEqual(affine(p,0,Q(2,5)),expand_bernstein(left))
                self.assertEqual(affine(p,Q(2,5),Q(3,5)),expand_bernstein(right))

    def test_minimum_global_degree_and_compact_roundtrip(self):
        for epsilon in [Q(1),Q(1,3),Q(1,10),Q(1,50)]:
            p=[1+epsilon,-4,4]
            result=certify_uniform(p,128)
            n=len(result['certificate']['pieces'][0]['coefficients'])-1
            brute=next(k for k in range(2,129) if min(power_to_bernstein(p,k))>=0)
            self.assertEqual(n,brute)
            verify(p,result['certificate'])
            verify(p,metrics(result['certificate'])[1])

    def test_random_carry_and_partition_certificates(self):
        rng=Random(3401)
        for n in range(1,15):
            b=[Q(rng.randrange(1,20),rng.randrange(1,6)) for _ in range(n+1)]
            p=expand_bernstein(b)
            result=coarsen_bernstein(b,strategy='compact',restarts=1)
            verify(p,result['certificate'])
            verify(p,metrics(result['certificate'])[1])
        for method in [certify_subdivision,certify_hybrid]:
            p=[Q(1,9)+Q(1,10000),Q(-2,3),1]
            result=method(p)
            verify(p,result['certificate'])
            verify(p,metrics(result['certificate'])[1])

    def test_reject_tiny_corruption_and_domain_gaps(self):
        p=[11,-40,40]
        valid=certify_subdivision(p)['certificate']
        bad=deepcopy(valid);bad['pieces'][0]['coefficients'][0]+=Q(1,10**100)
        with self.assertRaises(ValueError):verify(p,bad)
        bad=deepcopy(valid);bad['pieces'][1]['a']+=Q(1,100)
        with self.assertRaises(ValueError):verify(p,bad)
        bad=deepcopy(valid);bad['pieces'][1]['a']-=Q(1,100)
        with self.assertRaises(ValueError):verify(p,bad)
        bad=deepcopy(valid);bad['pieces'][0]['scale']=-1
        with self.assertRaises(ValueError):verify(p,bad)

    def test_reject_nonpositive_and_invalid_atoms(self):
        for p,atoms in [([0],[]),([0,1],[(1,0,1)]),([-1],[(0,0,-1)]),([1],[(0,0,2)])]:
            with self.assertRaises(ValueError):verify(p,dict(kind='mdn_atoms',atoms=atoms))
        with self.assertRaises(ValueError):verify([1],dict(kind='mdn_atoms',atoms=[(-1,0,1)]))
        with self.assertRaises(ValueError):verify([1],dict(kind='mdn_atoms',atoms=[(0,0,1)],scale=0))

    def test_strict_positivity_controls(self):
        for p in [[1,-4,4],[0,1],[Q(24,100),-1,1],[-1],[0]]:
            for method in [certify_uniform,certify_subdivision,certify_hybrid]:
                self.assertEqual(method(p)['status'],'limit')

    def test_shared_scale_and_sparse_encoding(self):
        cert=dict(kind='mdn_atoms',atoms=[(0,0,Q(7,11)),(2,3,Q(14,11)),(4,1,Q(21,11))])
        p=expand_atoms(cert['atoms'])
        m,packed=metrics(cert)
        self.assertIn('scale',packed)
        verify(p,packed)
        m2,packed2=metrics(packed)
        self.assertEqual(m,m2)
        verify(p,packed2)
        self.assertLess(m['payload_bits'],200)
        cert=dict(kind='bernstein_pieces',pieces=[dict(a=0,b=1,coefficients=[Q(7,11)]+[0]*10+[Q(14,11)])])
        p=expand_bernstein(cert['pieces'][0]['coefficients'])
        _,packed=metrics(cert)
        self.assertIn('nonzero_coefficients',packed)
        verify(p,packed)
        verify(p,metrics(packed)[1])
        cert=dict(kind='mdn_atoms',scale=2,atoms=[(0,0,1)])
        verify([2],metrics(cert)[1])

    def test_degree_only_certificates(self):
        p=[11,-40,40]
        verify(p,dict(kind='bernstein_recipe',degree=11))
        with self.assertRaises(ValueError):verify(p,dict(kind='bernstein_recipe',degree=10))
        cert=dict(kind='bernstein_recipe',pieces=[dict(a=0,b='1/2',degree=2),dict(a='1/2',b=1,degree=2)])
        verify(p,cert)
        verify(p,metrics(cert)[1])
        with self.assertRaises(ValueError):verify(p,dict(kind='bernstein_recipe',degree=1))


if __name__=='__main__':
    unittest.main()
