import datetime
from pathlib import Path
import pandas as pd
import re

if __name__ == '__main__':
    pd.options.display.width = 0

    df = pd.DataFrame(columns=['delta']) # declare schema

    p = Path('../benchmark_results') # open folder where results are in
    files = [x for x in p.iterdir()] # flatten list

    for f in files:
        time = float(0)
        with open(f.as_posix(), 'r') as file_content:
            for line in file_content.readlines():

                if re.search(r'<<BENCHMARK>>', line, re.S) is not None:
                    time = line.split()[1]

            new_df = pd.DataFrame({'delta': [time]})

        df = df.append(new_df, ignore_index=True)

    with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
        print(df)

    now = datetime.datetime.now()

    df.to_csv('dataframes/MC-NDFS_{}-{}-{}-{}-{}.csv'.format(now.year, now.month, now.day, now.hour, now.minute),
              index=False)
