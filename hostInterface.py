import signal, sys
import math
import socket
import struct
import os, os.path
from random import choice
from string import lowercase

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

def getSize(dataLen):
  try:
    x = raw_input("Amount of data> " )
    x = x.strip()
    data = x.split()
    size = int(data[0])
    if len(data) == 2:
      unit = data[1]
    else:
      unit = ''
    if unit == 'Mb' or unit == 'mb':
      totalSize = size * 1024 * 1024
    elif unit == 'Gb' or unit == 'gb':
      totalSize = size * 1024 * 1024 *1024
    elif unit == 'Kb' or unit == 'kb':
      totalSize = size * 1024
    else:
      # consider size in bytes
      totalSize = size
    #find the number of packets
    count = totalSize / dataLen
    count = int(math.floor(count))
    lastSize = int(totalSize - (dataLen * count))
    return count, lastSize 
  except:
    return

def generateData(keywordLen):
  n = 1440 - keywordLen
  data = "".join(choice(lowercase) for i in range(n))
  return data, len(data)

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
  #data, dataLen = dataEntry()
  data, dataLen = generateData(length)
  count, lastSize = getSize(dataLen)
  lastSize = lastSize + length
  print count, lastSize
  packet = "g" + struct.pack("I", count) + struct.pack("I", lastSize) + keyword + data
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
        elif "show packet received counter" == x:
          client.send(x)
          data = client.recv(1500)
          print data
          continue
        elif "clear packet received counter" == x:
          client.send(x)
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
