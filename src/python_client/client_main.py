#!/usr/bin/python

import client
import argparse
import hashlib
import os
import socket
import getpass
import sys
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
        1 : clientSock.get_sha1_file,  2 : clientSock.get_sha1_string, 3 :
	clientSock.symmetric_key,      4 : clientSock.symmetric_key,  5  :
	clientSock.symmetric_key,      6 : clientSock.symmetric_key,  7  :
	clientSock.symmetric_key,      8 : clientSock.DES_encr_decr, 9  : clientSock.DES_encr_decr,
	10 : clientSock.AES_encr_decr, 11 : clientSock.AES_encr_decr, 12 :
	clientSock.AES_encr_decr, 13 : clientSock.DES_encr_decr, 14 : clientSock.DES_encr_decr, 15 :
	clientSock.AES_encr_decr, 16 : clientSock.AES_encr_decr, 17 : clientSock.AES_encr_decr}
	
rec_message = clientSock.recieveMessage()

reg_or_log = -1	

if rec_message == "Authorize!":
    
    while 1:
	reg_or_log = raw_input ('Enter 0 for registration, 1 for sign in\n>>> ')
	
	if reg_or_log == "0" or reg_or_log == "1":
	    break

if reg_or_log == '0': # registration
    clientSock.sendMessage('0')
    
    while client.registration(clientSock) == 1:
	continue

if reg_or_log == '1': # signing in
    clientSock.sendMessage('1')
   
    while client.sign_up(clientSock) == 1:
	continue 


while 1:

    client.demand_services(clientSock)
    
    serviceId = input("Choose service: ")

    if client.call_corresponding_service(serviceId, options, clientSock) == -1:
	exit()
    else:
	continue
   
    pretty = clientSock.byteToHex(result)
    
    if serviceId == 1 or serviceId == 2:
	print '\nSHA 1: ', pretty
	print '\n'

    if serviceId >=3 and serviceId <= 7:
	print 'Symmetric key generated, ID: ', result
	print '\n'
    else:
	continue
