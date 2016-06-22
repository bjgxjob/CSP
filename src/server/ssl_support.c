#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "ssl_support.h"

SSL_CTX* InitServerCTX()
{
    SSL_METHOD* method;
    SSL_CTX* ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_server_method());

    if ( ctx == NULL )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}

    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}

    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
	ERR_print_errors_fp(stderr);
	abort();
    }
    if ( !SSL_CTX_check_private_key(ctx) )
    {
	fprintf(stderr, "Private key does not match the public certificate\n");
	abort();
    }
}

void ShowCerts(SSL* ssl)
{
    X509* cert;
    char* line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if( cert != NULL )
    {
	printf("Server certificates:\n");
	line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
	printf("Subject: %s\n",line);
	line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
	printf("Issuer: %s\n", line);
	X509_free(cert);
    }
    else
	printf("No certificates.\n");
}
