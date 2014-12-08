import signal, sys
import socket
import struct
import os, os.path

publishUidList = []
subsList = []
client = None

def signal_handler(signal, frame):
  print "Shutting down."
  try:
    client.send("quit")
    client.close()
  except:
    print "Host down"
  sys.exit(0)

def printHelp():
  print "Commands:"
  print "\tquit"
  print "\tsend data"

def dataEntry():
  try:
    x = raw_input("Enter Data > " )
    x = x.strip()
    return x, len(x)
  except KeyboardInterrupt, k:
    return 

def getKeywords():
  keywords = []
  print "Input keyword (one keyword per line, when done enter 'q')"
  count = 1
  while True: 
    try:
      x = raw_input( str(count) + " " )
      x = x.strip()
      if "q" == x:
        keywords.sort()
        strKeywords = ';'.join(keywords)
        strKeywords += ';'
        return strKeywords, len(strKeywords)
      keywords.append(x)
      count = count + 1
      continue
    except KeyboardInterrupt, k:
      return 

def sendData(client):
  keyword, length = getKeywords()
  data, dataLen = dataEntry()
  packet = keyword + data
  client.send(packet)

print "Connecting..."
if os.path.exists( "/tmp/hostSocket" ):
  client = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )
  client.connect( "/tmp/hostSocket" )
  print "Ready."
  signal.signal(signal.SIGINT, signal_handler)
  printHelp()
  while True:
    try:
      x = raw_input( "> " )
      x = x.strip()
      if "" != x:
        if "help" == x:
          printHelp()
          continue
        elif "send data" == x:
          sendData(client)
          continue
        elif "quit" == x:
          print "Shutting down."
          client.send(x)
          break
        else:
          print "Invalid Command"
          continue
        data = client.recv(1440);
    except KeyboardInterrupt, k:
      print "Shutting down."
  client.close()
else:
  print "Couldn't Connect!"
  exit(1)
print "Done"
