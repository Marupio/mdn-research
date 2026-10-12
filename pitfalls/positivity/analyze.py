"""Summarize all cases, including caps, and reverify saved certificates."""
import argparse
import json
from collections import defaultdict
from fractions import Fraction as Q
from pathlib import Path
from statistics import median
from common import verify

B_METHODS=['bernstein_global','bernstein_subdivision','bernstein_hybrid']
M_METHODS=['mdn_local_global','mdn_targeted_global','mdn_targeted_subdivision','mdn_targeted_hybrid']


def smallest(row,names):
    choices=[(row['methods'][name]['metrics']['payload_bits'],name)
             for name in names if row['methods'][name]['status']=='certified']
    return min(choices) if choices else None


def compare(rows,left_names,right_names):
    wins=ties=losses=0;ratios=[];deltas=[];paired=[]
    for row in rows:
        left,right=smallest(row,left_names),smallest(row,right_names)
        if left is None or right is None:
            continue
        a,b=left[0],right[0]
        wins+=a<b;ties+=a==b;losses+=a>b
        ratios.append(a/b);deltas.append(b-a)
        paired.append(dict(name=row['name'],left_method=left[1],right_method=right[1],left_bits=a,right_bits=b,ratio=a/b))
    return dict(pairs=len(ratios),left_smaller=wins,ties=ties,left_larger=losses,
                median_ratio=median(ratios) if ratios else None,
                total_left_bits=sum(p['left_bits'] for p in paired),
                total_right_bits=sum(p['right_bits'] for p in paired),
                largest_improvements=sorted(paired,key=lambda p:p['ratio'])[:5],
                largest_regressions=sorted(paired,key=lambda p:p['ratio'],reverse=True)[:5])


def summarize(data):
    rows=data['cases']
    summary=dict(cases=len(rows),methods={},families={},comparisons={})
    for name in rows[0]['methods']:
        values=[r['methods'][name] for r in rows]
        good=[v for v in values if v['status']=='certified']
        summary['methods'][name]=dict(certified=len(good),
            limits=sum(v['status']=='limit' for v in values),
            ineligible=sum(v['status']=='ineligible' for v in values),
            median_payload_bits=median(v['metrics']['payload_bits'] for v in good) if good else None,
            median_search_ms=1000*median(v['search_seconds'] for v in good) if good else None,
            median_verify_ms=1000*median(v['verification_seconds'] for v in good) if good else None)
    summary['comparisons']['best_mdn_vs_best_bernstein']=compare(rows,M_METHODS,B_METHODS)
    summary['comparisons']['best_mdn_vs_best_conventional']=compare(rows,M_METHODS,B_METHODS+['handelman_lp'])
    summary['comparisons']['best_mdn_vs_bernstein_recipes']=compare(rows,M_METHODS,[name+'_recipe' for name in B_METHODS])
    summary['comparisons']['targeted_global_vs_local']=compare(rows,['mdn_targeted_global'],['mdn_local_global'])
    summary['comparisons']['targeted_global_vs_lp_matched']=compare(rows,['mdn_targeted_global'],['handelman_lp'])
    summary['comparisons']['mdn_subdivision_vs_bernstein_subdivision']=compare(rows,['mdn_targeted_subdivision'],['bernstein_subdivision'])
    for family in sorted({r['family'] for r in rows}):
        selected=[r for r in rows if r['family']==family]
        summary['families'][family]=dict(cases=len(selected),mdn_vs_bernstein=compare(selected,M_METHODS,B_METHODS),
                                         mdn_vs_conventional=compare(selected,M_METHODS,B_METHODS+['handelman_lp']))
    return summary


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('inputs',nargs='+',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    result={};count=0
    for path in args.inputs:
        data=json.loads(path.read_text())
        for row in data['cases']:
            for name,method in row['methods'].items():
                if method['status']=='certified':
                    verify(list(map(Q,row['p'])),method['certificate'])
                    count+=1
        result[path.stem]=summarize(data)
    result['independent_saved_certificate_rechecks']=count
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({key:val['comparisons'] for key,val in result.items() if isinstance(val,dict)},indent=2))
    print(f'Reverified {count} saved certificates exactly.')


if __name__=='__main__':
    main()
