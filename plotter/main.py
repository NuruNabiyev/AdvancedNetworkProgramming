from   pathlib import Path
import pandas as pd
import seaborn as sns
import numpy as np
import datetime
import math
import re

if __name__ == '__main__':
    pd.options.display.width = 0

    # declare schema
    df = pd.DataFrame(columns=['delta'])
    # open folder where results are in
    p = Path('../benchmark_results')
    # flatten list
    files = [x for x in p.iterdir()]
    for f in files:
        d = 0
        with open(f.as_posix(), 'r') as file_content:
            # parse line
            for line in file_content.readlines():
                # search for keyword
                if re.search(r'<<BENCHMARK>>', line, re.S) is not None:
                    d = line.split()[1]
            # create a row
            df_row = pd.DataFrame({'delta': [d]})
        # add row to dataframe
        df = df.append(df_row, ignore_index=True)

    # Median Absolute Deviation
    delta_median = df.median()[0]
    MAD = df.apply(lambda x: abs(int(x) - delta_median), axis=1).median()

    standard_deviation = 3
    right_tail = delta_median + MAD * standard_deviation
    left_tail  = delta_median - MAD * standard_deviation
    # find outliers
    df['outlier'] = df['delta'].apply(lambda x: 'True' if ((left_tail > int(x)) or (int(x) > right_tail)) else 'False')
    # filter the outliers
    df.delta.where(df.outlier.isin(['False']),'', inplace=True)
    # number of good measurements
    N = df.outlier.value_counts()['False']
    uncertainty = MAD / math.sqrt(N)

    # Drop rows with any empty cells
    df.drop(columns = ['outlier'], inplace = True)
    df.replace('', np.nan, inplace=True)
    df.dropna(inplace=True)

    with pd.option_context('display.max_rows', None, 'display.max_columns', None):
        print(df)

    now = datetime.datetime.now()
    df.to_csv('dataframes/ANP_BENCHMARKS_{}-{}-{}-{}-{}.csv'.format(now.year, now.month, now.day, now.hour, now.minute), index=False)

    # make CDF
    # https://stackoverflow.com/questions/25577352/plotting-cdf-of-a-pandas-series-in-python
    df['cdf'] = df['delta'].rank(method='average', pct=True)
    # sort
    ax = df.sort_values('cdf').plot(x='delta', y='cdf', grid=True)
    # plot
    fig = ax.get_figure()
    fig.savefig('plot.png')
