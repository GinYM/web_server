#!/usr/bin/env python
#
# This script serves as a simple TLSv1 client for 15-441
#
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,
#          Charles Rang <rang972@gmail.com>,
#          Wolfgang Richter <wolf@cs.cmu.edu>

import pprint
import socket
import ssl

# try a connection
sock = socket.create_connection(('localhost', 9990))
tls = ssl.wrap_socket(sock, cert_reqs=ssl.CERT_REQUIRED,
                            ca_certs='../CA/private/ca.crt',
                            ssl_version=ssl.PROTOCOL_TLSv1)

# what cert did he present?
pprint.pprint(tls.getpeercert())

tls.sendall('POST /~prs/15-441-F15/ HTTP/1.1\r\n\
Host: www.cs.cmu.edu\r\n\
Connection: keep-alive\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.99 Safari/537.36\r\n\
Accept-Encoding: gzip, deflate, sdch\r\n\
Accept-Language: en-US,en;q=0.8\r\n\
\r\n\
Hello World!')

#while True:
#    new  = tls.recv(4096)
#    if not new:
#        tls.close()
#        break
print(tls.recv(4096))
tls.close()



# try another connection!
sock = socket.create_connection(('localhost', 9990))
tls = ssl.wrap_socket(sock, cert_reqs=ssl.CERT_REQUIRED,
                            ca_certs='../CA/private/ca.crt',
                            ssl_version=ssl.PROTOCOL_TLSv1)

tls.sendall('POST /~prs/15-441-F15/ HTTP/1.1\r\n\
Host: www.cs.cmu.edu\r\n\
Connection: keep-alive\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.99 Safari/537.36\r\n\
Accept-Encoding: gzip, deflate, sdch\r\n\
Accept-Language: en-US,en;q=0.8\r\n\
\r\n\
Hello World!')
print(tls.recv(4096))
tls.close()

exit(0)
