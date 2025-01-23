
# Webserv 💁🏅

This project aims to implment an HTTP Server from scratch in C++.  
HTTP (Hypertext Transfer Protocol) is the most used protocol on Internet. 
This protocol is the basis of data communication for the World Wide Web.  
A Web server has for main role to store, treat and serve web pages to client.  
Usualy, a server receive a request for a specific ressource. HTTP requests can be GET, POST, DELETE, PUT etc.  

## 👨‍💻 Project 

First, as usual, we had to parse config file (inspired by NGINX config file), handling errors and store config to use it.

### HTTP Request
We used epoll functions to build Webserv because they provide flexibility for managing state-based communication and handling multiple file descriptors in the same time in an asynchromous way. 
It enables storing file descriptors and monitoring them for various events :  
- EPOLLIN means incoming request
- EPOLLOUT means socket is ready for writing
- EPOLLERR means error received   

We have two different type of connecion :  
- A Listenning socket that only listen incoming connections and accept them. This connection is established according to config file and its interfaces (adress:port)
- Normal connections which monitor to EPOLLIN, EPOLLOUT, EPOLLERR events to handle clients requests

The process we used is accepting incoming request from a new client with Listenning socket, monitore EPOLLIN events on clients sockets and manage EPOLLOUT events and processing response depending on request parsing.

### CGI (Common Gateway Interface)

One of the key features implemented is Python CGI management for GET and POST method.  
Common Gateway Interface is a method allowing a webserver to run a script to generate dynamic response to client. It is often used with Python or PHP.  
To implement it, we had to determine wether or not the request was for CGI execution. In that case, we had to set environment for CGI execution, use pipe and fork to redirect  stdin and stdout and execute CGI in a child process.  

Handling errors and close ressources proprely in both process was a real challenge.

### Others challenges

Webserv is implemented to handle chunked request by reading and stocking chunked requests and wait for processing until ended chunked requests is received. 

Webserv is also implemented to handle multi-part request with a specific parsing to extract each part from the request.


## 🔧 Build program  
Download, clone repo and use Makefile
```bash
git clone https://github.com/BdClement/Webserv.git
cd Webserv
make
```
Run executable file ./webserv with a config file as argument. You can use for example test/conf/test.conf.


## 💼 Contributors  
- [Bastien Mirlicourtois](https://github.com/bmirlico)
