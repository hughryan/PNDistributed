Perfect Numbers Distributed Computation
By: Hugh McDonald
======================
[Compute] Written in C. Performs a benchmark that results in a performance number that represents the number of mod operations per second that the client is capable of. Compute then connects to the server(Manage) using sockets and sends the performance number. The server then returns a range of numbers to check for perfect numbers.

[Manage] Written in Python. Recieves and parses XML requests. Keeps track of connected Compute clients and maintains a list of perfect numbers found so that it may provide new ranges to check.

[Report] Written in Python. Queries the server to report on the current status of perfect number finding. If run with -k option, report will tell Manage to shut down the compute clients and then exit.

All data between clients and the server is in XML.

Difficulty:
The most difficult part of this project was to write the send and recieve code so that the other side would know how many bytes of data to expect. I chose to do this by finding the size of the send data, converting that number using htons(), sending it as a string, receiving it, converting it back to an int, and then to a readable form with ntohs(). I then use that number to ensure that all the data is properly transferred before continuing. I ran into a problem with this value itself having a variable length, so I fixed it to 5 characters (max size of int after htons()) and padded with ’\0’ if necessary, which I striped on the recieving end.