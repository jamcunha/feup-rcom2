#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_USER            "anonymous"
#define DEFAULT_PASSWORD        "anonymous"

#define FTP_PORT                21

typedef enum ftp_response {
    READY_USER = 220,
    READY_PASS = 331,
    LOGIN_SUCCESS = 230,
    LOGIN_FAILED = 530,
    PASSIVE_MODE = 227,
    OPEN_CONNECTION = 150
} ftp_response_t;

struct ftp_url {
    char        user[256];
    char        password[256];
    char        host[256];
    char        path[256];
    char        filename[256];
    const char* ip;
};

struct ftp_passive_mode {
    char        ip[32];
    int         port;
};

struct ftp_url parse_ftp_url(const char* url) {
    struct ftp_url ftp_url;

    if (strncmp(url, "ftp://\[", 7) == 0) {
        int r = sscanf(url, "ftp://\[%[^:]:%[^@]@]%[^/]/%s",
                ftp_url.user, ftp_url.password, ftp_url.host, ftp_url.path);

        if (r != 4) {
            fprintf(stderr, "Invalid ftp url\n");
            exit(-1);
        }

        if (strchr(ftp_url.path, '/') == NULL) {
            strcpy(ftp_url.filename, ftp_url.path);
        } else {
            char* filename = strrchr(ftp_url.path, '/') + 1;
            strcpy(ftp_url.filename, filename);
        }
    } else {
        int r = sscanf(url, "ftp://%[^/]/%s",
                ftp_url.host, ftp_url.path);

        if (r != 2) {
            fprintf(stderr, "Invalid ftp url\n");
            exit(-1);
        }

        strcpy(ftp_url.user, DEFAULT_USER);
        strcpy(ftp_url.password, DEFAULT_PASSWORD);
    }

    return ftp_url;
}

ftp_response_t get_ftp_response(const char* buffer) {
    int code;

    sscanf(buffer, "%d", &code);
    return code;
}

int create_socket(const char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ftp url>\n", argv[0]);
        return -1;
    }

    struct ftp_url ftp_url = parse_ftp_url(argv[1]);

    printf("\n--- PARSED FTP URL ---\n\n");
    printf("User: %s\n", ftp_url.user);
    printf("Password: %s\n", ftp_url.password);
    printf("Host: %s\n", ftp_url.host);
    printf("Path: %s\n", ftp_url.path);
    printf("Filename: %s\n", ftp_url.filename);
    printf("\n");

    struct hostent* h;
    
    if ((h = gethostbyname(ftp_url.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    ftp_url.ip = inet_ntoa(*((struct in_addr *) h->h_addr));

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", ftp_url.ip);

    printf("\n--- CONNECTING TO FTP SERVER ---\n\n");

    int sockfd = create_socket(ftp_url.ip, FTP_PORT);
    ftp_response_t res;

    char buffer[1024];

    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("read()");
        exit(-1);
    }

    res = get_ftp_response(buffer);

    if (res != READY_USER) {
        fprintf(stderr, "Invalid response\n");
        exit(-1);
    }

    sprintf(buffer, "USER %s\r\n", ftp_url.user);

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("write()");
        exit(-1);
    }

    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("read()");
        exit(-1);
    }

    res = get_ftp_response(buffer);

    if (res != READY_PASS) {
        fprintf(stderr, "Problem when sending user\n");
        exit(-1);
    }

    sprintf(buffer, "PASS %s\r\n", ftp_url.password);

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("write()");
        exit(-1);
    }

    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("read()");
        exit(-1);
    }

    res = get_ftp_response(buffer);

    if (res != LOGIN_SUCCESS) {
        fprintf(stderr, "Invalid login\n");
        exit(-1);
    }

    sprintf(buffer, "PASV\r\n");

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("write()");
        exit(-1);
    }

    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("read()");
        exit(-1);
    }

    res = get_ftp_response(buffer);

    if (res != PASSIVE_MODE) {
        fprintf(stderr, "A problem occurred when entering passive mode\n");
        exit(-1);
    }

    int ip1, ip2, ip3, ip4, port1, port2;

    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
            &ip1, &ip2, &ip3, &ip4, &port1, &port2);

    struct ftp_passive_mode ip_port;

    sprintf(ip_port.ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    ip_port.port = port1 * 256 + port2;

    printf("\n--- PASSIVE MODE ---\n\n");
    printf("IP: %s\n", ip_port.ip);
    printf("Port: %d\n", ip_port.port);

    int sockfd2 = create_socket(ip_port.ip, ip_port.port);

    printf("\n--- DOWNLOADING FILE ---\n\n");

    sprintf(buffer, "RETR %s\r\n", ftp_url.path);

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("write()");
        exit(-1);
    }

    if (read(sockfd, buffer, sizeof(buffer)) < 0) {
        perror("read()");
        exit(-1);
    }

    res = get_ftp_response(buffer);

    if (res != OPEN_CONNECTION) {
        fprintf(stderr, "A problem occurred when opening connection\n");
        exit(-1);
    }

    FILE* file = fopen(ftp_url.filename, "wb");

    if (file == NULL) {
        perror("fopen()");
        exit(-1);
    }

    while (1) {
        int r = read(sockfd2, buffer, sizeof(buffer));

        if (r < 0) {
            perror("read()");
            exit(-1);
        }

        if (r == 0) {
            break;
        }

        if (fwrite(buffer, 1, r, file) < 0) {
            perror("fwrite()");
            exit(-1);
        }
    }

    printf("File downloaded successfully\n");

    if (close(sockfd2) < 0) {
        perror("close()");
        exit(-1);
    }

    if (close(sockfd) < 0) {
        perror("close()");
        exit(-1);
    }

    return 0;
}
