import pandas as pd
#import sys

#if len(sys.argv) < 2:
#    print("missing arguments")
#    quit()
#sys.argv[1]

#input_file = sys.argv[1]

input_files = [
        {
            'file': 'performance_test_openFOAM_VirgilioRe22Pr7_2_worker_40_cores_influx_60_cores',
            'worker_nodes': '2',
            'influx_cores': '60'
        },
        {
            'file': 'performance_test_openFOAM_VirgilioRe22Pr7_2_worker_40_cores_influx_40_cores',
            'worker_nodes': '2',
            'influx_cores': '40'
        },
        {
            'file': 'performance_test_openFOAM_VirgilioRe22Pr7_2_worker_40_cores_influx_20_cores',
            'worker_nodes': '2',
            'influx_cores': '20'
        },
        {
            'file': 'performance_test_openFOAM_VirgilioRe22Pr7_2_worker_40_cores_influx_10_cores',
            'worker_nodes': '2',
            'influx_cores': '10'
        }
]

df = pd.DataFrame()
for input_file in input_files:
    with open(input_file['file']) as f:
        first_line = True
        column_count = 0
        all_lines_split = []
        column_names = []
        for line in f:
            line_split = line.strip().split("\t")
            if first_line:
                column_names = ['worker_nodes', 'influx_cores'] + line_split
                column_count = len(line_split)
                first_line = False
            else:
                if len(line_split) == column_count:
                    #print(line_split)
                    all_lines_split.append([input_file['worker_nodes'], input_file['influx_cores']] + line_split)
    df = pd.concat([df, pd.DataFrame(all_lines_split, columns=column_names)])
print(df)

minutes = df['real h:m:s'].replace(":[0-9]+\.[0-9]+$", "", regex=True)
minutes = pd.to_numeric(minutes, errors='coerce')
minutes = minutes.mul(60, fill_value=0)
seconds = df['real h:m:s'].replace("^[0-9]+:", "", regex=True)
seconds = pd.to_numeric(seconds, errors='coerce')
df['real h:m:s'] = minutes.add(seconds, fill_value=0)

print(df)

tmp_df = df.groupby(['worker_nodes', 'name', 'influx_cores'])['real h:m:s'].median()

plot = tmp_df.plot(kind='bar', use_index=True, grid=True)
plot.get_figure().savefig('libiotrace_influxdb_test.jpg')
