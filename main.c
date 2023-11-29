#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define DEFAULT_USER        "anonymous"
#define DEFAULT_PASSWORD    "anonymous"

struct ftp_url {
    char user[256];
    char password[256];
    char host[256];
    char path[256];
};

struct ftp_url parse_ftp_url(const char* url) {
    struct ftp_url ftp_url;

    if (strncmp(url, "ftp://\[", 7) == 0) {
        int r = sscanf(url, "ftp://\[%s:%s@]%[^/]/%s",
                ftp_url.user, ftp_url.password, ftp_url.host, ftp_url.path);

        if (r != 4) {
            fprintf(stderr, "Invalid ftp url\n");
            exit(1);
        }
    } else {
        int r = sscanf(url, "ftp://%[^/]/%s",
                ftp_url.host, ftp_url.path);

        if (r != 2) {
            fprintf(stderr, "Invalid ftp url\n");
            exit(1);
        }

        strcpy(ftp_url.user, DEFAULT_USER);
        strcpy(ftp_url.password, DEFAULT_PASSWORD);
    }

    return ftp_url;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ftp url>\n", argv[0]);
        return 1;
    }

    struct ftp_url ftp_url = parse_ftp_url(argv[1]);

    printf("--- PARSED FTP URL ---\n");
    printf("User: %s\n", ftp_url.user);
    printf("Password: %s\n", ftp_url.password);
    printf("Host: %s\n", ftp_url.host);
    printf("Path: %s\n", ftp_url.path);
    printf("\n");

    struct hostent *h;
    
    if ((h = gethostbyname(ftp_url.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

    return 0;
}
