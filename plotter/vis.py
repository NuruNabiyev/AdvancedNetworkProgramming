import pandas as pd
import numpy as np
import seaborn as sns

df = pd.DataFrame({'time in seconds to finish one tcp exchange':np.array(['0.000100',
    '0.000181',
    '0.000090',
    '0.000085',
    '0.000090',
    '0.000130',
    '0.000200',
    '0.000145',
    '0.000198',
    '0.000115']).astype(np.float)})

# Get to the CDF directly
df['cdf'] = df.rank(method = 'average', pct = True)

# Sort and plot
ax = df.sort_values('time in seconds to finish one tcp exchange').plot(x = 'time in seconds to finish one tcp exchange',
                                                                       y = 'cdf',
                                                                       grid = True)

fig = ax.get_figure()
fig.savefig('plot.png')
