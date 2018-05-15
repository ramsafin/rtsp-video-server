import json

# benchmark's parameters
JSON_OUT_FILE_NAME = 'benchmark.json'

# resolutions = [(240, 180), (320, 240), (480, 360), (640, 480)]
resolutions = [(240, 180), (480, 360), (640, 480)]

frame_rates = [3, 5, 7, 9, 15]

bit_rates = [100, 200, 300, 400, 500]

buf_size_factor = 2

udp_datagram_sizes = [500, 1000, 1500, 2000]

trial_num = 0

benchmark = {'benchmark': []}

# generate a benchmark object
for (w, h) in resolutions:
    for fps in frame_rates:
        for bit_rate in bit_rates:
            for datagram_size in udp_datagram_sizes:
                trial = {
                    'trial': trial_num,
                    'width': w,
                    'height': h,
                    'frame_rate': fps,
                    'bit_rate': bit_rate,
                    'buf_size': int(bit_rate * buf_size_factor),
                    'datagram_size': datagram_size
                }

                trial_num += 1

                # append new trial
                benchmark['benchmark'].append(trial)

# convert benchmark to a json and write it to the file
with open(JSON_OUT_FILE_NAME, 'wb') as outfile:
    json.dump(benchmark, outfile, indent=True)
