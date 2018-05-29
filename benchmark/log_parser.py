#!/usr/bin/env python

import re

import pandas as pd

from collections import OrderedDict

from openpyxl import load_workbook

import os.path

import sys

import json


LEFT_CAM_LOG_FILE = 'left_camera_output.log'
RIGHT_CAM_LOG_FILE = 'right_camera_output.log'
SERVER_LOG_FILE = 'stereo_logs/server_output.log'
EXCEL_FILE = 'Benchmark-stereo-results.xlsx'
EXCEL_SHEET_NAME = 'Experiment #1'


def parse_client_logs(logfile_path, cam_name):
    """
    Parses client log file located at `logfile_path`.

    :param string logfile_path: Log file path.
    :param string cam_name: Parsed data's camera name.
    :return List of parsed trials.
    """
    with open(logfile_path, 'r', 512) as logfile:
        qos_stats_list = retrieve_qos_stats(logfile.read())

    return [parse_qos_stat(e, cam_name) for e in qos_stats_list]


def retrieve_qos_stats(logs):
    """
    Retrieves entries of client's QoS statistics.

    :param string logs: Log file text.
    :return List of entries with QoS stats.
    """
    return re.findall(r'Created output file:(.*?)end_QOS_statistics', logs, re.S)


def parse_qos_stat(stat, cam_name):
    """
    Parses client's QoS stat entry.

    :param string stat: one entry of QoS statistics.
    :param string cam_name: Parsed data's camera name.
    :return Dict of parsed values.
    """
    groups = re.match(r'.*trial_(\d{1,3}).*_video-H265'
                      r'.*num_packets_received\s(\d+)'
                      r'.*num_packets_lost\s(\d+)'
                      r'.*kbits_per_second_min\s(.*?)\n'
                      r'.*kbits_per_second_ave\s(.*?)\n'
                      r'.*kbits_per_second_max\s(.*?)\n'
                      r'.*packet_loss_percentage_ave\s(.*?)\n'
                      r'.*inter_packet_gap_ms_min\s(.*?)\n'
                      r'.*inter_packet_gap_ms_ave\s(.*?)\n'
                      r'.*inter_packet_gap_ms_max\s(.*?)\n',
                      stat, re.S).groups()

    return {
        'trial': int(groups[0]),
        cam_name + '_pkts_recv': int(groups[1]),
        cam_name + '_pkts_lost': int(groups[2]),
        # cam_name + '_b_min': float(groups[3].strip()),
        # cam_name + '_b_avg': float(groups[4].strip()),
        # cam_name + '_b_max': float(groups[5].strip()),
        # cam_name + '_pkt_gap_min': float(groups[6].strip()),
        cam_name + '_pkts_loss': float(groups[6].strip()),
        cam_name + '_pkt_gap_avg': float(groups[8].strip()),
        cam_name + '_pkt_gap_max': float(groups[9].strip())
    }


def parse_server_logs(logfile_path):
    """
    Parses server log file.

    :param string logfile_path: Server log file path.
    :return List of parsed log entries
    """
    with open(logfile_path, 'r') as logfile:
        log_entries = retrieve_log_entries(logfile.read())

    return [parse_log_entry(e) for e in log_entries]


def retrieve_log_entries(logs):
    """
    Retrieves server log entries (trials).

    :param string logs: Log file text.
    :return List of log entries (string).
    """
    # this is for two camera config only
    return re.findall(r'(trial.*?encoded .*? frames in .*?s \(.*? fps\),'
                      r'.*? kb/s, Avg QP:.*?Transcoder destructed.*?Transcoder destructed)', logs, re.S)


def parse_log_entry(entry):
    """
    Parses server log entry.

    :param string entry: Server log file entry.
    :return Dict of parsed values.
    """
    groups = re.match(r'^trial:\s(\d{1,3})'
                      r'.*out_frame_width:\s(\d{3})'
                      r'.*out_frame_height:\s(\d{3})'
                      r'.*out_framerate:\s(\d+)'
                      r'.*bitrate:\s(\d{3})'
                      r'.*UDP:\s(\d{3,4})'
                      r'.*x265 \[info\]: frame I:.*, Avg QP:(.*?)\skb/s: (.*?)\n'
                      r'.*x265 \[info\]: frame I:.*, Avg QP:(.*?)\skb/s: (.*?)\n',
                      entry, re.S).groups()

    return {
        'trial': int(groups[0]),
        'width': int(groups[1]),
        'height': int(groups[2]),
        'fps': int(groups[3]),
        'bitrate': int(groups[4]),
        'pkt_size': float(groups[5]),
        'right_avg_qp': float(groups[6].strip()),
        # 'codec_bitrate_r': float(groups[7].strip()),
        'left_avg_qp': float(groups[8].strip())
        # 'codec_bitrate_l': float(groups[9].strip())
    }


def parse_pidstat(pistat_logfile):
    
    with open(pistat_logfile, 'r') as pidstat_is:
        pidstat_logs = pidstat_is.read()

    entries = re.split('\n\s*\n', pidstat_logs)

    stats = OrderedDict()
    trial_counter = 0

    for entry in entries:
        
        elems = re.findall(r'.*? RTSPServer\n', entry)
        
        for e in elems:
            #                                PID                       CPU
            groups = re.match(r'.*?\s+\d+\s+(.*?)\s+.*?\s+.*?\s+.*?\s+(.*?)\s', e, re.S).groups()
            
            pid = int(groups[0].strip())
            cpu_usage = float(groups[1].strip().replace(',', '.'))

            if pid in stats:
                stats[pid]['cpu'] = max(stats[pid]['cpu'], cpu_usage)
            else:
                stats[pid] = {'trial': trial_counter, 'cpu': cpu_usage}
                trial_counter += 1

    return map(lambda x: {'trial': stats[x]['trial'], 'pid': x, 'cpu': stats[x]['cpu']}, stats)


def merge_dicts_list(fst, snd):
    """
    Merges two lists of dicts.

    :param fst: List of dicts.
    :param snd: List of dicts.
    :return Merged `fst` and `snd` dicts.
    :raise ValueError: in case of merging data sizes are not equal.
    """
    if len(fst) != len(snd):
        raise ValueError('client and server data must have equal size')

    res_lst = []

    for x in xrange(len(fst)):
        assert fst[x]['trial'] == snd[x]['trial']
        res_dict = fst[x].copy()
        res_dict.update(snd[x])
        res_lst.append(res_dict)

    return res_lst


def retrieve_values(dict_list, val_name):
    """
    Retrieves a slice of values with key `val_name` form list of dicts.

    :param dict_list: List of dicts.
    :param string val_name: Dict's key for which a slice must be retrieved.
    :return a slice(list) of values for the specified key among dicts.
    """
    return list(map(lambda x: x[val_name], dict_list))


def save_excel(filename, headers, data):
    """
    Exports parsed data into an excel file.

    :param string filename: Excel file name.
    :param headers: List of excel table headers.
    :param data: parsed data.
    """

    df_data = OrderedDict()

    if os.path.isfile(filename):
        book = load_workbook(filename)

    for header in headers:
        df_data[header] = retrieve_values(data, header)

    data_frame = pd.DataFrame(df_data)

    with pd.ExcelWriter(filename, engine='openpyxl') as  writer:
        
        if os.path.isfile(filename):
            writer.book = book
            writer.sheets = dict((ws.title, ws) for ws in book.worksheets)

        data_frame.to_excel(writer, sheet_name= EXCEL_SHEET_NAME, index=False)
        writer.save()

def main():

    one_pass = merge_dicts_list(parse_client_logs(sys.argv[1], 'left'),
                                parse_server_logs(sys.argv[4]))

    res = merge_dicts_list(parse_client_logs(sys.argv[2], 'right'), one_pass)

    res = merge_dicts_list(parse_pidstat(sys.argv[3]), res)

    save_excel(EXCEL_FILE, ['trial', 'width', 'height', 'fps', 'bitrate', 'pkt_size',
        'left_pkts_loss', 'right_pkts_loss',
        'left_avg_qp', 'right_avg_qp',
        'cpu',
        'left_pkts_lost', 'left_pkts_recv',
        'right_pkts_lost', 'right_pkts_recv',
        'left_pkt_gap_avg', 'left_pkt_gap_max',
        'right_pkt_gap_avg', 'right_pkt_gap_max'],
         res)


if __name__ == '__main__':
    main()
