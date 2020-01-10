# Proxy Lab

### proxy.c

```c
#include <stdio.h>
#include "csapp.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_hdr = "Proxy-Connection: close\r\n";

/* handle a http request/response transaction */
void doit(int fd);
/* returns an error message to the client */
void client_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg);
/* parse given uri into hostname, port and path */
void parse_uri(char *uri, char *hostname, int *port, char *path);
/* build request headers */
void build_request_hdrs(rio_t *rp, char *new_req, char *hostname);
/* detach 'doit' function into a new thread */
void *thread(void *vargp);

int main(int argc, char **argv) {
    int listen_fd, *conn_fd;
    pthread_t tid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t client_len;
    struct sockaddr_storage client_addr;

    /* check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);

    init_cache();
    listen_fd = Open_listenfd(argv[1]);
    while(1) {
        printf("Listening..\n");
        client_len = sizeof(client_addr);
        conn_fd = Malloc(sizeof(int));
        *conn_fd = Accept(listen_fd, (SA *)&client_addr, &client_len);

        Getnameinfo((SA *)&client_addr, client_len, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection form (%s, %s)\n", hostname, port);
        // create a new thread to process handler function
        Pthread_create(&tid, NULL, thread, conn_fd);
    }

    Close(listen_fd);
    free_cache();
    return 0;
}

/* handle a http request/response transaction */
void doit(int client_fd) {
    int endserver_fd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t from_client, to_endserver;
    char hostname[MAXLINE], path[MAXLINE];
    int port;

    /* read request line and headers */
    Rio_readinitb(&from_client, client_fd);

    if (!Rio_readlineb(&from_client, buf, MAXLINE))
        return;
    sscanf(buf, "%s %s %s", method, uri, version);
    /* only support GET method */
    if (strcasecmp(method, "GET")) {
        client_error(client_fd, method, "501", "Not Implementd", "Proxy server does not implement this method");
        return;
    }

    /* read cache */
    int ret = read_cache(uri, client_fd);
    if (ret) return;

    /* parse usi and open a client_fd */
    parse_uri(uri, hostname, &port, path);
    char port_str[10];
    sprintf(port_str, "%d", port);
    endserver_fd = Open_clientfd(hostname, port_str);
    if (endserver_fd < 0) {
        printf("Connection failed\n");
        return;
    }
    Rio_readinitb(&to_endserver, endserver_fd);

    /* for end server http request headers, set up first line */
    char new_req[MAXLINE];
    sprintf(new_req, "GET %s HTTP/1.0\r\n", path);
    build_request_hdrs(&from_client, new_req, hostname);

    /* send client header to real server */
    Rio_writen(endserver_fd, new_req, strlen(new_req));

    int n, size = 0;
    char data[MAX_OBJECT_SIZE];
    while ((n = Rio_readlineb(&to_endserver, buf, MAXLINE))) {
        if (size <= MAX_OBJECT_SIZE) {
            memcpy(data + size, buf, n);
            size += n;
        }

        /* if server responded to buf, send the response to the client */
        printf("proxy received %d bytes, then send\n",n);
        Rio_writen(client_fd, buf, n);
    }
    printf("size: %d\n", size);
    if (size <= MAX_OBJECT_SIZE) {
        write_cache(uri, data, size);
    }
    Close(endserver_fd);
}

/* returns an error message to the client */
void client_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg) {
    char buf[MAXBUF], body[MAXBUF];

    /* build and print http response header */
    sprintf(buf, "HTTP/1,0 %s %s\r\n", err_num, short_msg);
    sprintf(buf, "Content-type: text/html\r\n");
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));

    /* build and print http response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, err_num, short_msg);
    sprintf(body, "%s<p>%s: %s\r\n", body, long_msg, cause);
    sprintf(body, "%s<hr><em>The Proxy Web Server</em>\r\n", body);
    Rio_writen(fd, body, strlen(body));
}

/* parse given uri into hostname, port and path */
void parse_uri(char *uri, char *hostname, int *port, char *path) {
    *port = 80;

    /* host start position, eg: the end of 'http://' protocol header */
    char *hostpos = strstr(uri, "//");
    if (hostpos != NULL) {
        hostpos += 2;
    } else {
        hostpos = uri;
    }

    /* port start position */
    char *portpos = strstr(hostpos, ":");
    if (portpos != NULL) {
        /* change ':' into '\0' to extract hostname string */
        *portpos = '\0';
        strncpy(hostname, hostpos, MAXLINE);
        sscanf(portpos + 1, "%d%s", port, path);
        *portpos = ':';
    } else {
        /* no port number in uri */
        portpos = strstr(hostpos, "/");
        if (portpos == NULL) {
            /* only host name in uri */
            strncpy(hostname, hostpos, MAXLINE);
            strcpy(path, "");
            return;
        } else {
            /* hostname with path */
            *portpos = '\0';
            strncpy(hostname, hostpos, MAXLINE);
            *portpos = '/';
            strncpy(path, portpos, MAXLINE);
        }
    }
}

/* build request headers */
void build_request_hdrs(rio_t *rp, char *new_req, char *hostname) {
    char buf[MAXLINE];

    /* check header info */
    while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
        if (!strcmp(buf, "\r\n")) break;
        if (strstr(buf, "Host:") != NULL) continue;
        if (strstr(buf, "User-Agent:") != NULL) continue;
        if (strstr(buf, "Connection:") != NULL) continue;
        if (strstr(buf, "Proxy-COnnection:") != NULL) continue;

        sprintf(new_req, "%s%s", new_req, buf);
    }
    sprintf(new_req, "%sHost: %s\r\n", new_req, hostname);
    sprintf(new_req, "%s%s%s%s", new_req, user_agent_hdr, conn_hdr, proxy_hdr);
    sprintf(new_req, "%s\r\n", new_req);
}

/* detach 'doit' function into a new thread */
void *thread(void *vargp) {
    int conn_fd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(conn_fd);
    Close(conn_fd);
    return NULL;
}
```



### cache.h

```c
#include "csapp.h"
#include <sys/time.h>

#define TYPES 6
extern const int cache_block_size[];
extern const int cache_cnt[];

typedef struct cache_block {
    char *url;
    char *data;
    int datasize;
    int64_t time;
    pthread_rwlock_t rwlock;
} cache_block;

typedef struct cache_type {
    cache_block *pcache_block;
    int size;
} cache_type;

cache_type caches[TYPES];

/* initialize cache */
void init_cache();
/* read content from cache into fd if hit, return 0 if miss */
int read_cache(char *url, int fd);
/* write value into cache */
void write_cache(char *url, char *data, int len);
/* free the allocated memory of cache */
void free_cache();
```



### cache.c

```c
#include "cache.h"

/* 
 * largest cache obj: 100KB, total cache memory space: 1M
 * seperate the memory space by creating cache block hierarchy
 * 100KB: 5 (500KB), 25KB: 12 (300KB), 10KB: 10 (100KB), 5KB: 20 (100KB), 1KB: 20 (20KB), 100B: 40 (4KB)
 */

const int cache_block_size[] = {102, 1024, 5120, 10240, 25600, 102400};
const int cache_cnt[] = {40, 20, 20, 10, 12, 5};
int64_t currentTimeMillis();

void init_cache() {
    int i, j;
    for (i = 0; i < TYPES; ++i) {
        caches[i].size = cache_cnt[i];
        caches[i].pcache_block = (cache_block *)malloc(cache_cnt[i] * sizeof(cache_block));
        cache_block *block_iter = caches[i].pcache_block;
        for (j = 0; j < cache_cnt[i]; ++block_iter, ++j) {
            block_iter->time = 0;
            block_iter->datasize = 0;
            block_iter->url = malloc(sizeof(char) * MAXLINE);
            strcpy(block_iter->url, "");
            block_iter->data = malloc(sizeof(char) * cache_block_size[i]);
            memset(block_iter->data, 0, cache_block_size[i]);
            pthread_rwlock_init(&block_iter->rwlock, NULL);
        }
    }
}

int read_cache(char *url, int fd) {
    cache_type cur;
    cache_block *block_iter;
    printf("read cache %s \n", url);
    int i, j;
    for (i = 0; i < TYPES; ++i) {
        cur = caches[i];
        block_iter = cur.pcache_block;
        for (j = 0; j < cur.size; ++block_iter, ++j) {
            if (block_iter->time != 0 && strcmp(url, block_iter->url) == 0) 
                break;
        }
        if (j < cur.size) break;
    }

    if (j == cur.size) {
        printf("fail to read cache\n");
        return 0;
    }

    pthread_rwlock_rdlock(&block_iter->rwlock);
    if (strcmp(url, block_iter->url)) {
        pthread_rwlock_unlock(&block_iter->rwlock);
        return 0;
    }
    pthread_rwlock_unlock(&block_iter->rwlock);

    if (!pthread_rwlock_trywrlock(&block_iter->rwlock)) {
        block_iter->time = currentTimeMillis();
        pthread_rwlock_unlock(&block_iter->rwlock);
    }

    pthread_rwlock_rdlock(&block_iter->rwlock);
    Rio_writen(fd, block_iter->data, block_iter->datasize);
    pthread_rwlock_unlock(&block_iter->rwlock);
    printf("success to read cache\n");
    return 1;
}

void write_cache(char *url, char *data, int len) {
    int i;
    for (i = 0; i < TYPES && len > cache_block_size[i]; ++i);
    printf("write cache %s %d\n", url, i);
    /* find empty block */
    cache_type cur = caches[i];
    cache_block *block_iter = cur.pcache_block, *block_temp_iter = cur.pcache_block;
    for (i = 0; i < cur.size; ++block_iter, ++i) {
        if(block_iter->time == 0)
            break;
    }
    /* find last visited */
    int64_t min = currentTimeMillis();
    if (i == cur.size) {
        for (i = 0; i < cur.size; ++block_temp_iter, ++i) {
            if (block_temp_iter->time <= min) {
                min = block_temp_iter->time;
                block_iter = block_temp_iter;
            }
        }
    }
    pthread_rwlock_wrlock(&block_iter->rwlock);
    block_iter->time = currentTimeMillis();
    block_iter->datasize = len;
    memcpy(block_iter->url, url, MAXLINE);
    memcpy(block_iter->data, data, len);
    pthread_rwlock_unlock(&block_iter->rwlock);
    printf("write cache\n");
}

int64_t currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (int64_t)(time.tv_sec) * 1000 + (int64_t)(time.tv_usec / 1000);
}

void free_cache() {
    int i, j;
    for (i = 0; i < TYPES; ++i) {
        cache_block *block_iter = caches[i].pcache_block;
        for (j = 0; j < cache_cnt[i]; ++block_iter, ++j) {
            free(block_iter->url);
            free(block_iter->data);
            pthread_rwlock_destroy(&block_iter->rwlock);
        }
        free(caches[i].pcache_block);
    }
}
```



### esult 

```
*** Basic ***
Starting tiny on 10865
Starting proxy on 12432
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.

2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy

basicScore: 40/40

*** Concurrency ***
Starting tiny on port 12150
Starting proxy on port 3147
Starting the blocking NOP server on port 11595
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15

*** Cache ***
Starting tiny on port 21401
Starting proxy on port 18603
Fetching ./tiny/tiny.c into ./.proxy using the proxy
Fetching ./tiny/home.html into ./.proxy using the proxy
Fetching ./tiny/csapp.c into ./.proxy using the proxy
Killing tiny
Fetching a cached copy of ./tiny/home.html into ./.noproxy
Success: Was able to fetch tiny/home.html from the cache.
Killing proxy
cacheScore: 15/15

totalScore: 70/70
```



