#!/usr/bin/env python

import socket
from time import sleep
import subprocess32 as subprocess

HOST_PORT = ('10.42.0.1', 15555)

# socket buffer size
BUFFER_SIZE = 1024


# wait for data from the socket
def wait_for_data(sock, sleep_sec=1):
    data = ''
    while data == '':
        data = sock.recv(BUFFER_SIZE)
        sleep(sleep_sec)
    return data


if __name__ == '__main__':

    with open('client_output.log', 'wb') as logfile:

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        s.connect(HOST_PORT)

        while True:

            print "Waiting for data ..."

            server_data = wait_for_data(s)

            print "Data: {}".format(server_data)

            if server_data == 'KILL':
                break
            else:

                while True:
                    try:
                        print "Launch openRTSP ..."

                        subprocess.check_call(server_data.split(), stdout=logfile, stderr=logfile)

                        print "Send FINISHED signal ..."
                        break
                    except subprocess.CalledProcessError as e:
                        print str(e)
                        print "Retrying ..."
                        sleep(5)
                        continue

                s.sendall("FINISHED")
    s.close()
