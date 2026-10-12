"""Render the explicit-certificate comparison from saved, verified results."""
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from analyze import B_METHODS, M_METHODS, smallest


def main():
    directory = Path(__file__).resolve().parent / 'results'
    main_data = json.loads((directory / 'holdout.json').read_text())
    sensitivity = json.loads((directory / 'lp_sensitivity.json').read_text())
    extended = {row['name']: row['lp_extended'] for row in sensitivity['cases']
                if row['dataset'] == 'holdout'}
    counts = [[0, 0, 0], [0, 0, 0]]
    for row in main_data['cases']:
        mdn = smallest(row, M_METHODS)[0]
        bernstein = smallest(row, B_METHODS)[0]
        conventional = min(bernstein, row['methods']['handelman_lp'].get(
            'metrics', {}).get('payload_bits', float('inf')))
        if row['name'] in extended:
            conventional = min(conventional,
                               extended[row['name']]['metrics']['payload_bits'])
        for index, comparator in enumerate([bernstein, conventional]):
            counts[index][0 if mdn < comparator else 1 if mdn == comparator else 2] += 1
    assert sum(counts[0]) == sum(counts[1]) == len(main_data['cases'])
    plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 12})
    fig, ax = plt.subplots(figsize=(11.5, 4.8), dpi=160)
    fig.patch.set_facecolor('#fbfaf7')
    ax.set_facecolor('#fbfaf7')
    colors = ['#137f78', '#c3c8cb', '#69539a']
    labels = ['MDN smaller', 'Equal', 'Conventional smaller']
    for row_index, row_counts in enumerate(counts):
        left = 0
        for category, count in enumerate(row_counts):
            ax.barh(row_index, count, left=left, height=0.44,
                    color=colors[category], label=labels[category] if row_index == 0 else None)
            if count:
                ax.text(left + count / 2, row_index, str(count), ha='center', va='center',
                        color='#253038' if category == 1 else 'white', weight='bold', fontsize=15)
            left += count
    ax.set_yticks([0, 1], ['Bernstein alone', 'Bernstein + sparse LP'])
    ax.invert_yaxis()
    ax.set_xlim(0, len(main_data['cases']))
    ax.set_xticks([0, 20, 40, 60, 80, 100])
    ax.set_xlabel('Number of fresh test polynomials (100 total)', labelpad=10)
    ax.tick_params(axis='both', length=0, pad=10)
    for spine in ax.spines.values():
        spine.set_visible(False)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.34), ncol=3, frameon=False)
    fig.suptitle('The apparent MDN size advantage shrinks with a stronger comparison',
                 x=0.03, y=0.98, ha='left', fontsize=17, weight='bold')
    fig.text(0.03, 0.87, 'Explicit certificates · estimated numeric payload · best observed portfolios',
             color='#485760', fontsize=12)
    fig.text(0.03, 0.035,
             'LP uses 25 objectives where degree ≤ 32. No global optimality claim.\n'
             'Sizes include coefficients and indices; they are not an implemented binary codec.',
             color='#485760', fontsize=10)
    fig.subplots_adjust(left=0.245, right=0.975, top=0.77, bottom=0.35)
    for suffix in ('png', 'svg'):
        fig.savefig(directory / f'compactness.{suffix}', facecolor=fig.get_facecolor())
    print(directory / 'compactness.png')


if __name__ == '__main__':
    main()
