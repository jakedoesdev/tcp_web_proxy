Author: Jacob Everett

INCOMPLETE, NOTES ON CURRENT VERSION:
Making an early submission in case canvas goes down later. This version of the program accepts a url from the user and sends it to the server.
If the server can find the url in list.txt, it sends any files found with the same name as the date string in list.txt.
If the server cannot find the url in list.txt, it contacts the origin server, reads the response, checks the response for "HTML/1.1 200 OK" and, if found, creates an entry in
list.txt, saves the entire server reply in a cache file named for the current time/date, and forwards that reply to the client. 

3 biggest issues currently:
No logic to check how many list.txt entries there are, no method to delete them, just always appends.
Memory is not dynamically allocated which is causing memory problems somewhere/sometimes when pages are too large (I think?)
Depending on the url requested, the client isn't always able to make a second request 
(probably related to the above memory issue, read() not getting to read the whole message the first time)

Make file instructions

To compile:  make
To clean:    make clean
To run: .\pyserver <port number>
        .\client   <port number>

	
Usage instructions

When client is executed, user will be prompted with to enter a URL with "url: ". Enter a URL beginning with "www." and ending with
a top-level domain name (.com/.edu/.info/etc) press return, and the server will process the client's string.
Server will check list.txt for the url string and time/date that represents the cache filename. 
If found, it will find the cache file in the same directory as the program and write to the client a cached page for that website,
named for the time/date it was originally retrieved (i.e. 20200915090300).
If not found, it will open a connection to the origin server, make a new entry in list.txt, create a new cache file name for the current time,
and forward the origin server reply to the client. The cache is limited to 5 entries at once, with the oldest being overwritten.


