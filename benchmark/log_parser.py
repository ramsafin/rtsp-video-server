import re
import json
import collections


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
    Retrieves entries of clinet's QoS statistics.

    :param string logs: Log file text.
    :return List of entries with QoS stats.
    """
    return re.findall(r'Created output file:(.*?)end_QOS_statistics', logs, re.S)


def parse_qos_stat(stat, cam_name):
    """
    Parses clinet's QoS stat entry.

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
                      r'.*inter_packet_gap_ms_min\s(.*?)\n'
                      r'.*inter_packet_gap_ms_ave\s(.*?)\n'
                      r'.*inter_packet_gap_ms_max\s(.*?)\n', stat, re.S).groups()

    return {'trial': int(groups[0]),
            cam_name + '_packets_received': int(groups[1]),
            cam_name + '_packets_lost': int(groups[2]),
            cam_name + '_bitrate_min': float(groups[3].strip()),
            cam_name + '_bitrate_avg': float(groups[4].strip()),
            cam_name + '_bitrate_max': float(groups[5].strip()),
            cam_name + '_inter_packet_gap_min': float(groups[6].strip()),
            cam_name + '_inter_packet_gap_avg': float(groups[7].strip()),
            cam_name + '_inter_packet_gap_max': float(groups[8].strip())}


def parse_server_logs(logifle_path):
    """
    Parses server log file.

    :param string logfile_path: Server log file path.
    :return List of parsed log entries
    """
    with open(logifle_path, 'r') as logfile:
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
    
    return {'trial': int(groups[0]),
            'frame_width': int(groups[1]),
            'frame_height': int(groups[2]),
            'target_fps': int(groups[3]),
            'target_bitrate': int(groups[4]),
            'datagram_size': float(groups[5]),
            'avg_qp_right_cam': float(groups[6].strip()),
            'codec_bitrate_right_cam': float(groups[7].strip()),
            'avg_qp_left_cam': float(groups[8].strip()),
            'codec_bitrate_left_cam': float(groups[9].strip())}


def merge_dicts_list(fst, snd):
    """
    Merges two lists of dicts.

    :param fst: List of dicts.
    :param snd: List of dicst.
    :return Merged `fst` and `snd` dicts.
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


def main():
    one_pass = merge_dicts_list(parse_client_logs('stereo_logs/left_camera_output.log', 'left_cam') ,parse_server_logs('stereo_logs/server_output.log'))
    res = merge_dicts_list(parse_client_logs('stereo_logs/right_camera_output.log', 'right_cam') , one_pass)
    


if __name__ == '__main__':
    main()
