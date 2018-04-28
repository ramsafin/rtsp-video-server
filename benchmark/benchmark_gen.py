import json

# benchmark's parameters
JSON_OUT_FILE_NAME = 'benchmark.json'

resolutions = [(240, 180), (320, 240), (360, 270), (480, 360), (640, 480), (744, 480)]

frame_rates = [3, 5, 7, 9]

bitrates = [100, 200, 300, 400]

bufsize_factor = 2.5

udp_datagram_sizes = [500, 1000, 1500, 2000]

trial_num = 0

benchmark = {'benchmark': []}

# generate a benchmark object
for (w, h) in resolutions:
    for fps in frame_rates:
        for b in bitrates:
            for udp in udp_datagram_sizes:
                trial = {
                    'trial': trial_num,
                    'width': w,
                    'height': h,
                    'framerate': fps,
                    'bitrate': b,
                    'bufsize': int(b * bufsize_factor),
                    'udp': udp
                }

                trial_num += 1

                # append new trial
                benchmark['benchmark'].append(trial)

# convert benchmark to a json and write it to the file
with open(JSON_OUT_FILE_NAME, 'w') as outfile:
    json.dump(benchmark, outfile, indent=True)
