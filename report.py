#! /usr/bin/python
#########################################
# report.py - Perfect Numbers Reporter
# Hugh McDonald
# 12.6.13
#########################################
import sys, socket, getopt, struct

class Report:
	"""Queries the perfect number manager and reports on the data"""

	def __init__(self):
		#SERVER IP ADDRESS
		self.server = "128.193.37.168"
		#SERVER PORT
		self.port = 7070
		self.sock = None
		self.tested = None
		self.numbers = []
		self.clients = {}

	def run(self, kill):
		request = "<query>1</query>"
		if kill:
			request += "<exit>1</exit>"
		self.connect()
		length = str(socket.htons(len(request)))
		while len(length) < 5:
			length += '\0'
		self.sock.sendall(length)
		self.sock.sendall(request)
		data = self.readrequest()
		self.parse(data)
		self.printstats()

	def readrequest(self):
		total = 0
		target = 5
		data = ""
		#get the header to determine data size
		while total < target:
			msg = self.sock.recv(target - total)
			total += len(msg)
			data += msg
		data = data.rstrip('\0')
		size = socket.ntohs(int(data))
		#use size to get complete data
		total = 0
		data = ""
		while total < size:
			msg = self.sock.recv(size - total)
			total += len(msg)
			data += msg
		return data

	def printstats(self):
		print "======  Perfect Numbers Report  ======"
		print "Numbers Tested: %s" % (self.tested)
		print "Perfect Numbers Found: ",
		for num in self.numbers:
			print "%s  " % (num),
		print ""
		for client, performance in self.clients.items():
			print "Client: %s" % (client),
			print "Perf: %s" % (performance)

	def parse(self, data):
		tag = ""
		value = ""
		while data is not "":
			data = data.lstrip("<")
			temp = data.partition(">")
			tag = "".join(temp[0])
			temp = temp[2].partition("</")
			value = "".join(temp[0])
			data = "".join(temp[2].partition(">")[2])
			if tag == "tested":
				self.tested = value
			elif tag == "perfect":
				self.numbers.append(value)
			elif tag == "client":
				client = value
				data = data.lstrip("<")
				temp = data.partition(">")
				tag = "".join(temp[0])
				temp = temp[2].partition("</")
				value = "".join(temp[0])
				data = "".join(temp[2].partition(">")[2])
				self.clients[client] = value

	def connect(self):
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		try:
			self.sock.connect((self.server, self.port))
		except socket.error, msg:
			self.sock.close()
			print "Error: Server not found"
			sys.exit(1)

def main():
	try:
		opt, argv = getopt.getopt(sys.argv[1:], "k")
	except getopt.GetoptError as error:
		print "Error: " + str(error)
		return 1
	kill = False
	for o, a in opt:
		if o == "-k":
			kill = True
	client = Report()
	client.run(kill)

if __name__ == '__main__':
	main()
