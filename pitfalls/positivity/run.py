"""Run a reproducible, exactly checked MDN/Bernstein comparison.

python3 pitfalls/positivity/run.py --output pitfalls/positivity/results/development.json
python3 pitfalls/positivity/run.py --seed 20261117 --generated-only --output .../holdout.json
All solvers receive only expanded rational power coefficients.
"""
import argparse
import hashlib
import json
import platform
from pathlib import Path
from time import perf_counter

from common import verify, metrics, jsonable
from cases import make_cases
from bernstein import certify_uniform, certify_subdivision, certify_hybrid
from mdn_search import coarsen_bernstein
from handelman import certify_handelman


def checked(p,result,elapsed,extra=None):
    out=dict(status=result['status'],search_seconds=elapsed,
             metadata=result.get('metadata',{}))
    if extra:
        out.update(extra)
    if result['status']!='certified':
        out['reason']=result.get('reason','search limit')
        return out
    cert=result['certificate']
    started=perf_counter()
    verify(p,cert)
    verification_seconds=perf_counter()-started
    stats,packed=metrics(cert)
    # Verify the actual saved certificate separately from the in-memory one.
    verify(p,packed)
    out.update(metrics=stats,certificate=packed,verification_seconds=verification_seconds,
               exact_identity_verified=True)
    return out


def timed(method,*args,**kwargs):
    started=perf_counter()
    result=method(*args,**kwargs)
    return result,perf_counter()-started


def coarsen_partition(parent,strategy,seed):
    if parent['status']!='certified':
        return dict(status='limit',reason='parent Bernstein search reached its cap',metadata={})
    pieces=[];metadata=[]
    for k,piece in enumerate(parent['certificate']['pieces']):
        result=coarsen_bernstein(piece['coefficients'],strategy=strategy,seed=seed+k,restarts=4)
        pieces.append(dict(a=piece['a'],b=piece['b'],atoms=result['certificate']['atoms']))
        # Keep compact search provenance; detailed portfolios are reproducible.
        meta={key:val for key,val in result['metadata'].items() if key!='candidates'}
        metadata.append(meta)
    if len(pieces)==1 and pieces[0]['a']==0 and pieces[0]['b']==1:
        certificate=dict(kind='mdn_atoms',atoms=pieces[0]['atoms'])
    else:
        certificate=dict(kind='mdn_pieces',pieces=pieces)
    return dict(status='certified',certificate=certificate,
                metadata=dict(strategy=strategy,pieces=metadata,optimality_proved=False))


def recipe(parent):
    if parent['status']!='certified':
        return dict(status='limit',reason='parent Bernstein search reached its cap',metadata={})
    pieces=[dict(a=p['a'],b=p['b'],degree=len(p['coefficients'])-1)
            for p in parent['certificate']['pieces']]
    if len(pieces)==1 and pieces[0]['a']==0 and pieces[0]['b']==1:
        cert=dict(kind='bernstein_recipe',degree=pieces[0]['degree'])
    else:
        cert=dict(kind='bernstein_recipe',pieces=pieces)
    return dict(status='certified',certificate=cert,
                metadata=dict(coefficient_storage='none; checker recomputes from input polynomial'))


def run_case(case,args):
    p=case['p'];methods={};raw={};times={}
    for name,fun,kw in [
        ('bernstein_global',certify_uniform,dict(max_degree=args.max_degree)),
        ('bernstein_subdivision',certify_subdivision,dict(max_depth=16,max_leaves=4096)),
        ('bernstein_hybrid',certify_hybrid,dict(max_depth=16,max_leaves=4096,local_elevation=16))]:
        result,elapsed=timed(fun,p,**kw)
        raw[name]=result;times[name]=elapsed
        methods[name]=checked(p,result,elapsed)
        methods[name+'_recipe']=checked(p,recipe(result),elapsed,dict(parent=name))
    for name,parent,strategy in [
        ('mdn_local_global','bernstein_global','compact'),
        ('mdn_targeted_global','bernstein_global','targeted'),
        ('mdn_targeted_subdivision','bernstein_subdivision','targeted'),
        ('mdn_targeted_hybrid','bernstein_hybrid','targeted')]:
        result,elapsed=timed(coarsen_partition,raw[parent],strategy,args.search_seed)
        methods[name]=checked(p,result,elapsed+times[parent],
                             dict(parent=parent,parent_search_seconds=times[parent],
                                  additional_search_seconds=elapsed))
    if raw['bernstein_global']['status']=='certified':
        n=len(raw['bernstein_global']['certificate']['pieces'][0]['coefficients'])-1
    else:
        n=args.max_degree+1
    if n>args.lp_degree:
        methods['handelman_lp']=dict(status='ineligible',search_seconds=0,
                                    reason='minimum global degree exceeds LP dictionary cap',
                                    metadata=dict(degree_cap=args.lp_degree,minimum_degree=n if n<=args.max_degree else None))
    else:
        result,elapsed=timed(certify_handelman,p,max_degree=args.lp_degree,degree=n,
                             restarts=args.lp_restarts,seed=args.search_seed,
                             time_limit=5.0,objective='compact')
        methods['handelman_lp']=checked(p,result,elapsed+times['bernstein_global'],
                                       dict(parent_search_seconds=times['bernstein_global'],
                                            additional_search_seconds=elapsed))
    return dict(case,methods=methods)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--seed',type=int,default=20261010)
    parser.add_argument('--search-seed',type=int,default=1729)
    parser.add_argument('--per-family',type=int,default=12)
    parser.add_argument('--generated-only',action='store_true')
    parser.add_argument('--max-degree',type=int,default=128)
    parser.add_argument('--lp-degree',type=int,default=32)
    parser.add_argument('--lp-restarts',type=int,default=5)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    import numpy,scipy
    # Warm the numerical solver before measured calls. No input-specific cache.
    certify_handelman([1],max_degree=0,restarts=1,objective='compact')
    cases=make_cases(args.seed,args.per_family)
    if args.generated_only:
        cases=[case for case in cases if case['split']=='generated']
    here=Path(__file__).resolve().parent
    manifest={path.name:hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(here.glob('*.py'))}
    output=dict(settings=vars(args)|{'output':str(args.output)},
                environment=dict(python=platform.python_version(),numpy=numpy.__version__,scipy=scipy.__version__,
                                 platform=platform.platform()),
                source_sha256=manifest,method_scope='Strict positivity on the closed unit interval; rational proof weights',
                timing_note='One warmed run, not a precise microbenchmark. Derived method time includes parent search.',
                cases=[])
    started=perf_counter()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    for index,case in enumerate(cases):
        row=run_case(case,args)
        output['cases'].append(row)
        print(f"{index+1}/{len(cases)} {case['name']}: "+', '.join(
            f"{name}={result['metrics']['payload_bits'] if result['status']=='certified' else result['status']}"
            for name,result in row['methods'].items()),flush=True)
        # Retain completed results if a later case exposes an implementation fault.
        if (index+1)%10==0:
            args.output.write_text(json.dumps(jsonable(output),indent=2)+'\n')
    output['total_wall_seconds']=perf_counter()-started
    args.output.write_text(json.dumps(jsonable(output),indent=2)+'\n')
    print(f"Saved {len(cases)} cases to {args.output}; {output['total_wall_seconds']:.2f}s",flush=True)


if __name__=='__main__':
    main()
