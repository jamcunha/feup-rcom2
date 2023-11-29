#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ftp_url {
    char user[256];
    char password[256];
    char host[256];
    char path[256];
};

struct ftp_url parse_ftp_url(const char* url) {
    struct ftp_url ftp_url;

    int r = sscanf(url, "ftp://\[%s:%s@]%s/%s",
            ftp_url.user, ftp_url.password, ftp_url.host, ftp_url.path);

    if (r != 4) {
        fprintf(stderr, "Invalid ftp url\n");
        exit(1);
    }

    return ftp_url;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ftp url>\n", argv[0]);
        return 1;
    }

    struct ftp_url ftp_url = parse_ftp_url(argv[1]);

    printf("user: %s\n", ftp_url.user);
    printf("password: %s\n", ftp_url.password);
    printf("host: %s\n", ftp_url.host);
    printf("path: %s\n", ftp_url.path);

    return 0;
}
