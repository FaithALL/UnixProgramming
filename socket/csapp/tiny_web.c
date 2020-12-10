//一个微型迭代Web服务器，支持动态资源和静态资源
#include "socket_package.c"
#include "rio.c"
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <wait.h>

extern char **environ;
#define MAXBUF RIO_BUFSIZE

//处理一个HTTP事务
void doit(int fd);

//读取get请求报头(request header)
void read_requesthdrs(rio_t *rp);

//解析uri，取得文件名和cgi参数，并返回是否是静态资源
int parse_uri(char *uri, char *filename, char *cgiargs);

//服务静态资源
void serve_static(int fd, char *filename, int filesize);

//根据文件后缀名获取文件类型
void get_filetype(char *filename, char *filetype);

//服务动态资源
void serve_dynamic(int fd, char *filename, char *cgiargs);

//发送代表异常的HTTP响应报文
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s port\n", basename(argv[0]));
        return 1;
    }
    int listenfd = open_listenfd(argv[1], 5);
    int connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        getnameinfo((struct sockaddr *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }
    return 0;
}

//处理一个HTTP事务
void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    rio_t rio;
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET") != 0) {
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    char filename[MAXLINE], cgiargs[MAXLINE];
    int is_static = parse_uri(uri, filename, cgiargs);

    struct stat sbuf;
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    } else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
    }
    serve_dynamic(fd, filename, cgiargs);
}

//向fd发送表示异常的HTTP报文
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

//忽略，请求报头
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n") != 0) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
}

//解析url
int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/') strcat(filename, "home.html");
        return 1;
    } else {
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

//服务静态资源
void serve_static(int fd, char *filename, int filesize) {
    char filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));
    printf("Response headers: \n");
    printf("%s", buf);

    int srcfd = open(filename, O_RDONLY, 0);
    char *srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

//通过文件后缀名判断文件类型
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif")) strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png")) strcpy(filetype, "image/png");
    else if (strstr(filetype, ".jpg")) strcpy(filetype, "image/jpeg");
    else strcpy(filetype, "text/plain");
}

//服务动态资源
void serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = {NULL};
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    rio_writen(fd, buf, strlen(buf));

    if (fork() == 0) {
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, emptylist, environ);
    }
    wait(NULL);
}
