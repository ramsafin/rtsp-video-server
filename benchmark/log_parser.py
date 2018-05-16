import re
import json
import collections


def parse_client_logs(logfile_path):
    """
    Parses `logfile_path` file.

    :param string logfile_path: Log file path.
    :return List of parsed trials.
    """
    with open(logfile_path, 'r', 512) as logfile:
        qos_stats_list = retrieve_qos_stats(logfile.read())

    return [parse_qos_stat(e) for e in qos_stats_list]


def retrieve_qos_stats(logs):
    """
    Retrieves entries of QoS stats measurements.

    :param string logs: Log file text.
    :return List of entries with QoS stats.
    """
    return re.findall(r'Created output file:(.*?)end_QOS_statistics', logs, re.S)


def parse_qos_stat(stat):
    """
    Parses QoS stat entry.

    :param string stat: one entry of QoS statistics.
    :return Dict of parsed values.
    """
    groups = re.match(r'.*trial_(\d{1,3}).*_video-H265'
                      r'.*num_packets_received\s(\d+)'
                      r'.*num_packets_lost\s(\d+)'
                      r'.*kbits_per_second_min\s(.*?)\n'
                      r'.*kbits_per_second_ave\s(.*?)\n'
                      r'.*kbits_per_second_max\s(.*?)\n'
                      r'.*inter_packet_gap_ms_min\s(.*?)\n'
                      r'.*inter_packet_gap_ms_ave\s(.*?)\n'
                      r'.*inter_packet_gap_ms_max\s(.*?)\n', stat, re.S).groups()

    return {'trial': int(groups[0]),
            'packets_received': int(groups[1]),
            'packets_lost': int(groups[2]),
            'bitrate_min': float(groups[3].strip()),
            'bitrate_avg': float(groups[4].strip()),
            'bitrate_max': float(groups[5].strip()),
            'inter_packet_gap_min': float(groups[6].strip()),
            'inter_packet_gap_avg': float(groups[7].strip()),
            'inter_packet_gap_max': float(groups[8].strip())}


def parse_server_logs(logifle_path, is_stereo=False):
    """
    Parses server log file.

    :param string logfile_path: Server log file path.
    :param bool is_stereo: Indicates if two camera log file is being parsed.
    :return List of parsed log entries
    """
    with open(logifle_path, 'r') as logfile:
        log_entries = retrieve_log_entries(logfile.read())

    return [parse_log_entry(e, is_stereo) for e in log_entries]


def retrieve_log_entries(logs):
    """
    Retrieves server log entries (trials).

    :param string logs: Log file text.
    :return List of log entries (string).
    """
    return re.findall(r'(trial.*?encoded .*? frames in .*?s \(.*? fps\),'
                      r'.*? kb/s, Avg QP:.*?\n)', logs, re.DOTALL)


def parse_log_entry(entry, is_stereo):
    """
    Parses server log entry.

    :param string entry: Server log file entry.
    :param bool is_stereo: Indicates if logs contain two cameras info.
    :return Dict of parsed values.
    """
    groups = re.match(r'trial:\s(\d{1,3})'
                  r'.*out_frame_width:\s(\d{3})'
                  r'.*out_frame_height:\s(\d{3})'
                  r'.*out_framerate:\s(\d+)'
                  r'.*bitrate:\s(\d{3})'
                  r'.*UDP:\s(\d{3,4})'
                  r'.*x265 \[info\]: frame I:.*, Avg QP:(.*?)\skb/s: (.*?)'
                  r'\n', entry, re.S).groups()
    
    return {'trial': int(groups[0]),
            'frame_width': int(groups[1]),
            'frame_height': int(groups[2]),
            'target_fps': int(groups[3]),
            'target_bitrate': int(groups[4]),
            'datagram_size': float(groups[5]),
            'avg_qp': float(groups[6].strip()),
            'codec_bitrate': float(groups[7].strip())}


def main():
    pass
    # print parse_server_logs('mono_logs/server_output.log')
    # print parse_server_logs('stereo_logs/server_output.log')


if __name__ == '__main__':
    main()


