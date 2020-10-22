from   pathlib    import Path
import pandas     as pd
import matplotlib.pyplot as plt
import seaborn    as sns
import numpy      as np
import datetime
import math
import re

if __name__ == '__main__':
    pd.options.display.width = 0

    # declare schema
    df = pd.DataFrame(columns=['delta'])
    # open folder where results are in
    p = Path('../benchmark_results_verify')
    # flatten list
    files = [x for x in p.iterdir()]
    for f in files:
        # init delta variable
        d = 0
        with open(f.as_posix(), 'r') as file_content:
            # parse line
            for line in file_content.readlines():
                # search for keyword
                if re.search(r'<<BENCHMARK>>', line, re.S) is not None:
                    result = line.split()[1]
                    d = int(result) / 1000 # convert to microseconds
            # create a row
            df_row = pd.DataFrame({'delta': [d]})
        # add row to dataframe
        df = df.append(df_row, ignore_index=True)

    # Median Absolute Deviation
    delta_median = df.median()[0]
    MAD = df.apply(lambda x: abs(float(x) - delta_median), axis=1).median()

    standard_deviation = 3
    right_tail = delta_median + MAD * standard_deviation
    left_tail  = delta_median - MAD * standard_deviation
    # find outliers
    df['outlier'] = df['delta'].apply(lambda x: 'True' if ((left_tail > float(x)) or (float(x) > right_tail)) else 'False')
    # filter the outliers
    df.delta.where(df.outlier.isin(['False']),'', inplace=True)
    # number of good measurements
    N = df.outlier.value_counts()['False']
    uncertainty = MAD / math.sqrt(N)

    # Drop rows with any empty cells
    df.drop(columns = ['outlier'], inplace = True)
    df.replace('', np.nan, inplace=True)
    df.dropna(inplace=True)

    # with pd.option_context('display.max_rows', None, 'display.max_columns', None):
    #     print(df)

    now = datetime.datetime.now()
    df.to_csv('dataframes/ANP_BENCHMARKS_VERIFY_{}-{}-{}-{}-{}.csv'.format(now.year, now.month, now.day, now.hour, now.minute), index=False)
    # http://stanford.edu/~raejoon/blog/2017/05/16/python-recipes-for-cdfs.html
    num_bins = 20
    data = df[['delta']].to_numpy(dtype='float')
    print(type(data))
    counts, bin_edges = np.histogram (data)
    cdf = np.cumsum (counts)
    plt.plot (bin_edges[1:], cdf/cdf[-1])
    plt.title('CDF of Latency in TCP Exchange')
    plt.xlabel('Delta in microseconds')
    plt.ylabel('Probability')

    plt.savefig('plot_verify.png')
    # statistics
    print(f'Uncertainty {uncertainty}')
    print(f'Average {np.average(data)}')
    print(f'Median {delta_median}')
    print(f'Max Observed {np.max(data)}')
    print(f'Min Observed {np.min(data)}')
    print(f'N {N}')
    for q in [95, 99]:
        print ("{}%% percentile: {}".format (q, np.percentile(data, q)))

