#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>  //net address socketaddr_in htons
#include<arpa/inet.h>   //definitions for internet operations htonl/s ntohl/s
#include<unistd.h>     //fork() pipe()
#include<ctype.h>     //isspace  my name is daly
#include<strings.h>
#include<string.h>
#include<sys/stat.h>   
#include<pthread.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdint.h>

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING"Server:jdbhttpd/0.1.0\r\n"
#define STDIN  0
#define STDOUT 1
#define STDERR 2

void accept_request(void *);
void bad_accquest(int);
void cat(int,FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *,const char *,const char *);
int  get_line(int,char *,int);
void headers(int,const char *);
void not_found(int);
void serve_file(int,const char *);
int  startup(u_short *);
void unimplemented(int);

void accept_request(void *arg)
{
	int client = (intptr_t)arg;
	char buf[1024];
	size_t numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i,j;
	struct stat st;
	int cgi = 0;

	char *query_string = NULL;
	
	numchars = get_line(client, buf, sizeof(buf));
	i = 0;j = 0;
	while (!ISspace(buf[i]) && (i < sizeof(method) -1))
	{
		method[i] = buf[i];
		i++;
	}
	j=i;
	method[i] = '\0';
	
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		unimplemented(client);
		return;
	}

	if (strcasecmp(method, "POST") == 0)
		cgi = 1;
	i = 0;
	while (ISspace(buf[j]) && (j < numchars))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url) -1) &&(j < numchars))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';

	if (strcasecmp(method,


void bad_request(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content_Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}

void execute_cgi(int client, const char *path,
		const char *method, const char *query_string)
{
	char buf[1024];
	int  cgi_output[2];
	int  cgi_input[2];
	pid_t pid;
	int  status;
	int  i;
	char c;
	int  numchars = 1;
	int  content_length = -1;

	buf[0] = 'A';buf[1] = '\0';
	if(strcasecmp(method, "GET") == 0)
		while ((numchars > 0) && strcmp("\n", buf))
			numchars = get_line(client, buf, sizeof(buf));
	else if(strcasecmp(method, "POST") == 0)
	{
		numchars = get_line(client, buf, sizeof(buf));
		while ((numchars > 0) && strcmp("\n", buf))
		{
			buf[15] = '\0';
			if(strcasecmp(buf, "Content-Length:") == 0)
				content_length = atoi(&(buf[16]));
			numchars = get_line(client, buf, sizeof(buf));
		}
		if(content_length == -1){
			bad_request(client);
			return;
		}
	}

int get_line(int sock, char *buf, int size)
{
	int  i = 0;
	char c = '\0';
	int  n;

	while((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if( n > 0)
		{
			if(c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);
				if((n>0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}

void error_die(const char *sc)
{
	perroe(sc);
	exit(1);
}

int startup(u_short *port)
{
	int httpd = 0;
	int on    = 1;
	struct sockaddr_in name;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if(httpd == -1)
		error_die("socket");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port   = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
        if((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0)
	{
		error_die("setsockopt failed");
	}
	if(bind(httpd, (struct sockaddr *)&name, sizeof(name))<0)
		error_die("bind");
	if(*port == 0)
	{
		socklen_t namelen = sizeof(name);
		if(getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
		error_die("getsockname");
		*port = ntohs(name.sin_port);
 	if(listen(httpd, 5)<0)
		error_die("listen");
	return(httpd);
}

void unimplemented(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 501 Method Not implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

int main(void)
{
	int server_sock = -1;
	u_short port    = 4000;
	int client_sock = -1;
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	pthread_t newthread;

	server_sock = startup(&port);
	printf("httpd running on port %d\n",port);

	while (1)
	{
		client_sock = accept(server_sock, 
				    (struct sockaddr *)&client_name,
				    &client_name_len);
		







