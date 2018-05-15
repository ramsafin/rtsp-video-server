import re
import json
import collections

FILE_IO_BUF_SIZE = 4096

CLIENT_LOG_FILE = 'client_output.log'

SERVER_LOG_FILE = 'server_output.log'

PARSED_DATA_FILE = 'benchmark_results.json'


def retrieve_client_qos_stats(logs):
    """Retrieves QOS statistics from client logs"""
    return re.findall(r'Created output file:(.*?)end_QOS_statistics', logs, re.S)


def parse_client_bench_trials(trials):
    """Parses client benchmark's statistics into the resulting list of dicts"""
    res_lst = []
    for trial in trials:
        res_lst.append(parse_client_bench_trial(trial))
    return res_lst


def parse_client_bench_trial(trial):
    """Parses client bench trial and returns resulting dict"""
    groups = re.match(r'.*trial_(\d{1,3})_video-H265.*num_packets_received\s(\d+).*num_packets_lost\s(\d+)'
                      r'.*kbits_per_second_min\s(.*?)\n.*kbits_per_second_ave\s(.*?)\n.*kbits_per_second_max\s(.*?)\n'
                      r'.*inter_packet_gap_ms_min\s(.*?)\n.*inter_packet_gap_ms_ave\s(.*?)\n'
                      r'.*inter_packet_gap_ms_max\s(.*?)\n', trial, re.S).groups()

    return {'trial': int(groups[0]),
            'packets_received': int(groups[1]),
            'packets_lost': int(groups[2]),
            'bitrate_min': float(groups[3].strip()),
            'bitrate_avg': float(groups[4].strip()),
            'bitrate_max': float(groups[5].strip()),
            'inter_packet_gap_min': float(groups[6].strip()),
            'inter_packet_gap_avg': float(groups[7].strip()),
            'inter_packet_gap_max': float(groups[8].strip())}


def retrieve_server_bench_info(logs):
    """Retrieves server bench info"""
    return re.findall(r'(trial.*?)consecutive B-frames: 100\.0%', logs, re.S)


def parse_server_trials(trial_entries):
    """Parses each server bench trial and returns resulting list of dicts"""
    res_lst = []
    for e in trial_entries:
        res_lst.append(parse_server_trial(e))
    return res_lst


def parse_server_trial(e):
    """Parses server trial and returns resulting dict"""

    groups = re.match(r'trial:\s(\d{1,3})'
                      r'.*out_frame_width:\s(\d{3})'
                      r'.*out_frame_height:\s(\d{3})'
                      r'.*out_framerate:\s(\d+)'
                      r'.*bitrate:\s(\d{3}).*UDP:\s(\d{3,4})'
                      r'.*Max NALU size:\s(\d+)'
                      r'.*x265 \[info\]: frame I:.*, Avg QP:(.*?)\skb/s: (.*?)'
                      r'\n', e, re.S).groups()

    return {'trial': int(groups[0]),
            'frame_width': int(groups[1]),
            'frame_height': int(groups[2]),
            'target_fps': int(groups[3]),
            'target_bitrate': int(groups[4]),
            'datagram_size': float(groups[5]),
            'nalu': int(groups[6]),
            'avg_qp': float(groups[7].strip()),
            'codec_bitrate': float(groups[8].strip())}


def join_client_server_benchmarks(client_bench, server_bench):
    """
    Joins client and server log information considering 'trial' number.

    See https://stackoverflow.com/questions/38987/how-to-merge-two-dictionaries-in-a-single-expression#26853961
    """

    res_list = []

    if len(client_bench) != len(server_bench):
        raise ValueError('client and server dicts sizes must be equal')

    for x in xrange(len(client_bench)):
        assert client_bench[x]['trial'] == server_bench[x]['trial']
        res_dict = client_bench[x].copy()
        res_dict.update(server_bench[x])
        ordered_res_dict = collections.OrderedDict(reversed(sorted(res_dict.items())))
        res_list.append(ordered_res_dict)

    return res_list


if __name__ == '__main__':

    with open(SERVER_LOG_FILE, 'r', FILE_IO_BUF_SIZE) as server_log:
        server_logs = retrieve_server_bench_info(server_log.read())

    with open(CLIENT_LOG_FILE, 'r', FILE_IO_BUF_SIZE) as client_log:
        qos_stats = retrieve_client_qos_stats(client_log.read())

    # save parsed logs as json file
    with open(PARSED_DATA_FILE, 'wb', FILE_IO_BUF_SIZE) as result_file:
        json.dump(join_client_server_benchmarks(parse_client_bench_trials(qos_stats), parse_server_trials(server_logs)), result_file, indent=True)
