import pandas as pd
import openpyxl
import json

BENCH_JSON_FILE = 'benchmark_results.json'

def retrieve_values(dict_list, val_name):
    return list(map(lambda x: x[val_name], dict_list))

if __name__ == '__main__':

    with open(BENCH_JSON_FILE, 'r') as json_file:
    
        trials_dict_list = json.load(json_file)

        df_data = {}

        for header in trials_dict_list[0].keys():
            df_data[header] = retrieve_values(trials_dict_list, header)

        data_frame = pd.DataFrame(df_data)

        writer = pd.ExcelWriter('Benchmark-results.xlsx')
        data_frame.to_excel(writer, 'Sheet1', index=False)
        writer.save()

        

        
        