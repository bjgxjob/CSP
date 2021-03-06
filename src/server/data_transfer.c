#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "hashtable.h"
#include "server.h"
#include "data_transfer.h"

#define DATA_SIZE 1024

int send_services(SSL* ssl)
{
	int file_fd = open("server/services.txt", O_RDONLY);

	if ( file_fd < 0 )
	{   
		SSL_free(ssl);
		fprintf(stderr, "%s", "couldn't open services.txt file");	
		strerror(errno);
		pthread_exit(NULL);
		return 1;
	}

	if( 1 == send_file(file_fd, ssl) )
	{
		return 1;
	}
	return 0;
}

int send_file(int file_fd, SSL* ssl)
{
	char buf[DATA_SIZE] = { 0 };

	while(1)
	{
		int num_read;
		char* p = buf;

		num_read = read(file_fd, buf, DATA_SIZE - 1);

		if(num_read < 0)
		{
			fprintf(stderr,"%s\n",strerror(errno));
			return 1;
		}

		if(num_read == 0)
		{
			break;
		}
		
		while(num_read > 0)
		{
			int num_write =  SSL_write(ssl, p, num_read);
			if(num_write < 0)
			{
				fprintf(stderr, "%s\n", strerror(errno));
				return 1;
			}

			num_read -= num_write;
			p += num_write;
		} 

	}
    
	SSL_write(ssl, "END", 3);

	return 0;
}

int receive_file(SSL* ssl, char* buf) // reads while ##END## not occur
{
    int bytes_read;
    char* tmp = buf;
    
     while(1)
     {
	    bytes_read = SSL_read(ssl, tmp, 100);
    
	    if(bytes_read <= 0)
	    {
		SSL_free(ssl);
		return 1;
	    }

	    if(strcmp(tmp,"##END##") == 0)
	    {
		memset(tmp, 0, 6);
		break;
	    }
	    tmp += bytes_read;
     }

     return 0;
}


int send_buff(SSL* ssl, const char* buf, size_t buf_size)
{
    const unsigned char* tmp = buf;
    size_t sent_bytes = 0;
    int num_write = 0;

    do 
    {
	num_write = SSL_write(ssl, tmp, buf_size);
	tmp += num_write;
	sent_bytes += num_write;
    }
    while (sent_bytes < buf_size);

    if( num_write < 0)
    {
	fprintf(stderr, "send_buff: %s\n", strerror(errno));
	pthread_exit(NULL);
    }
    return 0;
}

int receive_buff(SSL* ssl, char* buff, int buff_size)
{
    memset(buff, 0, buff_size);
    
    int read_size = SSL_read(ssl, buff, buff_size);

    if(read_size < 0)
    {
	handle_error("Could not read from socket");
    }

    if(read_size == 0)
    {
	SSL_free(ssl);
	fprintf(stderr, "%s\n","Client disconnected, corresponding thread exited. SSL structures freed.");
	pthread_exit(NULL);
    }
	    
    return read_size;

}
int read_request(SSL* ssl,char request[DATA_SIZE])
{
	memset(request, 0, DATA_SIZE);
	int read_size = SSL_read(ssl, request, DATA_SIZE);

	if(read_size < 0)
	{
		handle_error("Could not read from socket");
	}

	if(read_size == 0)
	{
	    return 0;
	}
	    
	return read_size;
}
