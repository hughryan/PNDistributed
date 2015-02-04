#! /usr/bin/python
#########################################
# manage.py - Perfect Numbers Manager
# Hugh McDonald
# 12.6.13
#########################################
import sys, socket, copy, struct

class Manage:
	"""Manages perfect number clients"""

	def __init__(self):
		self.ssock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.csock = None
		self.caddress = None
		self.maxtested = 0L
		self.perfectnumbers = []
		self.clientsocks = []
		self.clients = {}

	def start(self):
		"""Starts the server"""
		self.ssock.bind(("os-class.engr.oregonstate.edu", 7070))
		self.ssock.listen(1024)
		while 1:
			#wait for a client
			self.csock, self.caddress = self.ssock.accept()
			#client connected read request
			request = self.readrequest()
			#process the request
			if not self.parserequest(request):
				#close the connection if not signal
				self.csock.close()

	def parserequest(self, request):
		"""Processes a client request"""
		functions = {"request": self.request,
				"signal": self.signal,
				"perfect": self.perfect,
				"query": self.query,
				"exit": self.exit}
		signal = False
		while request is not "":
			#parse the xml
			request = request.lstrip("<")
			temp = request.partition(">")
			tag = "".join(temp[0])
			temp = temp[2].partition("</")
			value = "".join(temp[0])
			request = "".join(temp[2].partition(">")[2])
			#execute the function requested with value specified
			func = functions[tag]
			func(value)
			if tag == "signal":
				signal = True
		return signal

	def readrequest(self):
		"""Reads request from the client"""
		#get the header to determine size of data
		total = 0
		target = 5
		data = ""
		while total < target:
			msg = self.csock.recv(target - total)
			total += len(msg)
			data += msg
		data = data.rstrip('\0')
		size = socket.ntohs(int(data))
		#use header size to retrieve complete data
		total = 0
		data = ""
		while total < size:
			msg = self.csock.recv(size - total)
			total += len(msg)
			data += msg
		return data

	def request(self, value):
		"""Returns a number range to the client to compute"""
		#determine the range to send
		performance = long(value) * 15L
		range = 1L
		while performance > 0:
			performance -= self.maxtested + range
			range += 1L
		#send range to client
		start = self.maxtested + 1L
		end = start + range
		data = "<start>" + str(start) + "</start>" + "<end>" + str(end) + "</end>"
		length = str(socket.htons(len(data)))
		while len(length) < 5:
			length += '\0'
		self.csock.sendall(length)
		self.csock.sendall(data)
		self.maxtested = end

	def signal(self, value):
		"""Adds the signal socket to the process list"""
		signalsocket = copy.deepcopy(self.csock)
		self.clientsocks.append(signalsocket)
		client = str(self.caddress[0]) + ":" + str(self.caddress[1])
		self.clients[client] = value

	def perfect(self, value):
		"""Adds the perfect number found to the perfect number list"""
		self.perfectnumbers.append(value)

	def query(self, value):
		"""Returns stats to the client"""
		data = "<tested>" + str(self.maxtested) + "</tested>"
		for num in self.perfectnumbers:
			data += "<perfect>" + num + "</perfect>"
		for client, performance in self.clients.items():
			data += "<client>" + client + "</client>"
			data += "<performance>" + performance + "</performance>"
		length = str(socket.htons(len(data)))
		while len(length) < 5:
			length += '\0'
		self.csock.sendall(length)
		self.csock.sendall(data)

	def exit(self, value):
		"""Signals all clients to exit and shuts down server"""
		for client in self.clientsocks:
			client.close()
		self.csock.close()
		exit()

def main():
	server = Manage()
	server.start()

if __name__ == '__main__':
	main()
