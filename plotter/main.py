import datetime
from pathlib import Path
import pandas as pd
import re

if __name__ == '__main__':
    pd.options.display.width = 0

    df = pd.DataFrame(columns=['threads',
                               'implementation',
                               'input_file',
                               'contains-cycle',
                               'time-to-finish',
                               'is_DAS'])

    p = Path('debug_data').glob('**/*')
    dirs = [x for x in p if x.is_dir()]
    print(dirs[0])
    files = []
    for directory in dirs:
        files = list(directory.glob('*_*'))

    for file in files:
        is_DAS = contains_cycle = False
        time_to_finish = []
        implementation = ''

        input_path = file.as_posix().split('/')[2].split('_')
        input_file, threads = input_path[0], input_path[1]

        with open(file.as_posix(), 'r') as file_content:
            for line in file_content.readlines():
                split_line = line.split(' ')

                if re.search(r'Running on DAS using prun.', line, re.S) is not None:
                    is_DAS = True

                if re.search(r'Graph', line, re.S) is not None and split_line[3] != 'not':
                    contains_cycle = True

                if re.search(r'ms', line, re.S) is not None:
                    implementation = split_line[0]
                    time_to_finish.append(split_line[2])

            new_df = pd.DataFrame({'threads': threads,
                                   'implementation': implementation,
                                   'input_file': input_file,
                                   'contains-cycle': contains_cycle,
                                   'time-to-finish': time_to_finish,
                                   'is_DAS': is_DAS,
                                   })

        df = df.append(new_df, ignore_index=True)

    with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
        print(df)
    now = datetime.datetime.now()

    df.to_csv('dataframes/MC-NDFS_{}-{}-{}-{}-{}.csv'.format(now.year, now.month, now.day, now.hour, now.minute),
              index=False)
