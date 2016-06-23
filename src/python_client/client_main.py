#!/usr/bin/python

import client
import argparse
import hashlib
import os
import socket
from OpenSSL import SSL, crypto

parser = argparse.ArgumentParser()
parser.add_argument("--host", help="specify the host to connect with", default="127.0.0.1")
parser.add_argument("--port", help="specify the port of the host", type=int, default=8888)

args = parser.parse_args()

# Initialize context
ctx = client.context()

clientSock = client.clientSocket(ctx)

clientSock.connect(args.host, args.port)

print "Secure connection established"

options = {
        1 : clientSock.get_sha1_file, 2 : clientSock.get_sha1_string, 3 :
	clientSock.symmetric_key,  4 : clientSock.symmetric_key,  5 :
	clientSock.symmetric_key,  6 : clientSock.symmetric_key,  7 :
	clientSock.symmetric_key
        }

while 1:
    clientSock.sendMessage("CSP1.0://Get Services")
    services = clientSock.recieveMessage()
    print services
    serviceId = input("Choose service: ")

    if serviceId < 0 or serviceId > 7:
	print 'Wrong order'
	exit(1)
    if serviceId == 2:
	print ' Your specified order now is not available, it will available soon'
	exit(1)

    result = options.get(serviceId)(str(serviceId))
    pretty = clientSock.byteToHex(result) 
    print 'SHA 1 : ', pretty
