.PHONY:all
all:httpserver cgi_main

httpserver:http_server.cc http_server_main.cc 
	g++ $^ -o $@ -std=c++11 -lpthread -lboost_filesystem -lboost_system

cgi_main:cgi_main.cc
	g++ $^ -o $@ -std=c++11 -lpthread -lboost_filesystem -lboost_system
	cp cgi_main ./wwwroot/add

.PHONY:clean
clean:
	rm httpserver cgi_main
