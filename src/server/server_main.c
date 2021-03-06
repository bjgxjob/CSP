#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include <resolv.h>
#include <unistd.h>

#include "sqlite3.h"

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "server.h"
#include "ssl_support.h"
#include "data_transfer.h"
#include "services.h"
#include "server_db.h"

#define LISTEN_BACKLOG 50
#define thread_count 100

int main(int argc, char *argv[])
{
    int i = 0;
    struct sigaction sPipe, sInt;
    int socket_desc;
    struct sockaddr_in server, client;

    parse_args(argc, argv);
    set_hash_table();

    // let's do the main job
    
    db = 0; 

    if ( access("SERVER_DB.dblite", F_OK) != -1)
    {
	//DB exists
	unlink("SERVER_DB.dblite");
    }

    if(connect_to_db(&db, "SERVER_DB.dblite") == 1)
    {
	exit(EXIT_FAILURE);
    }

    printf("%s\n", "Database created.");
    
    if( create_table_users(&db) == 1)
    {
	exit(EXIT_FAILURE);
    }

    printf("%s\n", "Table users created.");
    
    if ( create_table_keys(&db) == 1)
    {
	exit(EXIT_FAILURE);
    }

    printf("%s\n", "Table keys created.");
   
    memset(&sPipe, 0, sizeof(sPipe));
    memset(&sInt,  0, sizeof(sInt)); 
    
    sPipe.sa_handler = &sigpipe_handler;
    sigaction(SIGPIPE, &sPipe, NULL);
    
    sInt.sa_handler = &sigint_handler;
    sigaction(SIGINT, &sInt, NULL);
       
    create_socket(&socket_desc);
    
    if( !configure(conf_file) )
    {
	handle_error("invalid conf file");
    }

    initialize_server(&server);

    if(bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
	handle_error("bind failed");
    }

    if (listen(socket_desc, LISTEN_BACKLOG) == -1)
    {
	handle_error("listen failed");
    }

    printf("Waiting for incoming connection...\n");

    socklen_t address_len = sizeof(struct sockaddr_in);

    pthread_t helper_thread[thread_count];

    for(; i < thread_count; ++i)
    {
	struct handler_args* args = calloc(sizeof(struct handler_args), 1);
/*
	SSL_library_init();
	args.ctx = init_server_ctx();
	load_certificates(args.ctx,"mycert.pem","mycert.pem");
*/	
	args->socket = accept(socket_desc, (struct sockaddr *)&client, &address_len);
	printf("Args socket: %d\n", args->socket);
	if(args->socket == -1)
	{
	    handle_error("accept failed");
	}

	printf("connection accepted: %s:%d\n",inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	args->client_id = i;
    
	if(pthread_create(&helper_thread[i], NULL, connection_handler, (void*)args) != 0)
	{
	    handle_error("could not create thread");
	}
	
    }
 
    for(i = 0; i < thread_count; ++i)
    {
	pthread_join(helper_thread[i], NULL);
    }

    close(socket_desc);
    sqlite3_close(db);
    return 0;
}
