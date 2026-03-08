#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http_server.h"
#include "database.h"
#include "file_io.h"

#define BACKLOG 10
#define BUF_SIZE 8192

static char *url_decode(char *src) {
    char *dst = malloc(strlen(src) + 1);
    char *p = src, *q = dst;
    while (*p) {
        if (*p == '+') { *q++ = ' '; p++; }
        else if (*p == '%' && isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2])) {
            char hex[3] = { p[1], p[2], '\0' };
            *q++ = (char) strtol(hex, NULL, 16);
            p += 3;
        } else { *q++ = *p++; }
    }
    *q = '\0';
    return dst;
}

static char *get_query_value(const char *query, const char *key) {
    if (!query) return NULL;
    char *copy = strdup(query);
    char *tok = strtok(copy, "&");
    while (tok) {
        char *eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(tok, key) == 0) {
                char *val = url_decode(eq + 1);
                char *ret = strdup(val);
                free(val);
                free(copy);
                return ret;
            }
        }
        tok = strtok(NULL, "&");
    }
    free(copy);
    return NULL;
}

static char *json_extract_string(const char *json, const char *key) {
    char *p = strstr(json, key);
    if (!p) return NULL;
    p = strchr(p, ':');
    if (!p) return NULL;
    p++;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p == '"') p++;
    const char *start = p;
    while (*p && *p != '"') p++;
    size_t len = p - start;
    char *out = malloc(len + 1);
    if (!out) return NULL;
    strncpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int json_extract_int(const char *json, const char *key, int *out) {
    char *p = strstr(json, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '"')) p++;
    if (!*p) return 0;
    *out = atoi(p);
    return 1;
}

static void send_response(int client_fd, const char *status, const char *content_type, const char *body) {
    char header[2048];
    size_t body_len = body ? strlen(body) : 0;
    int len = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Accept, Authorization\r\n"
        "Connection: close\r\n\r\n",
        status, content_type, body_len);
    send(client_fd, header, len, 0);
    if (body_len > 0) send(client_fd, body, body_len, 0);
}

static void send_cors_options(int client_fd) {
    const char *header =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Accept, Authorization\r\n"
        "Access-Control-Max-Age: 3600\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    send(client_fd, header, strlen(header), 0);
}

static char *str_tolower_dup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *d = malloc(n + 1);
    if (!d) return NULL;
    for (size_t i = 0; i < n; ++i) d[i] = (char) tolower((unsigned char)s[i]);
    d[n] = '\0';
    return d;
}

static void handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    char buf[BUF_SIZE];
    ssize_t received = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (received <= 0) { close(client_fd); return; }
    buf[received] = '\0';

    // Parse request line
    char method[16], path[256];
    if (sscanf(buf, "%15s %255s", method, path) != 2) {
        send_response(client_fd, "400 Bad Request", "text/plain", "Bad Request");
        close(client_fd);
        return;
    }

    // Separate path and query
    char *qmark = strchr(path, '?');
    char *query = NULL;
    if (qmark) {
        *qmark = '\0';
        query = qmark + 1;
    }

    if (strcmp(method, "OPTIONS") == 0) {
        send_cors_options(client_fd);
        close(client_fd);
        return;
    }

    if (strcmp(method, "POST") == 0) {
        // locate body
        char *body = strstr(buf, "\r\n\r\n");
        if (body) body += 4; else body = "";

        if (strcmp(path, "/api/song") == 0) {
            char *title = json_extract_string(body, "\"title\"");
            char *artist = json_extract_string(body, "\"artist\"");
            int bpm = 0, energy = 0, danceability = 0;
            json_extract_int(body, "\"bpm\"", &bpm);
            json_extract_int(body, "\"energy\"", &energy);
            json_extract_int(body, "\"danceability\"", &danceability);

            if (!title || !artist) {
                send_response(client_fd, "400 Bad Request", "application/json", "{\"error\":\"missing title/artist\"}");
            } else {
                SongNode *new_song = malloc(sizeof(SongNode));
                if (!new_song) {
                    send_response(client_fd, "500 Internal Server Error", "application/json", "{\"error\":\"alloc failed\"}");
                } else {
                    strncpy(new_song->title, title, sizeof(new_song->title)-1); new_song->title[sizeof(new_song->title)-1] = '\0';
                    strncpy(new_song->artist, artist, sizeof(new_song->artist)-1); new_song->artist[sizeof(new_song->artist)-1] = '\0';
                    new_song->bpm = bpm;
                    new_song->energy = energy;
                    new_song->danceability = danceability;
                    new_song->mood[0] = '\0';
                    new_song->next = NULL; new_song->prev = NULL;
                    if (!head) head = new_song;
                    else {
                        SongNode *tmp = head;
                        while (tmp->next) tmp = tmp->next;
                        tmp->next = new_song;
                        new_song->prev = tmp;
                    }
                    classify_songs();
                    save_songs();
                    send_response(client_fd, "200 OK", "application/json", "{\"status\":\"added\"}");
                }
            }
            if (title) free(title);
            if (artist) free(artist);
            close(client_fd);
            return;
        } else {
            send_response(client_fd, "404 Not Found", "text/plain", "Not Found");
            close(client_fd);
            return;
        }
    }

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/api/songs") == 0) {
            // Build JSON array from head
            // approximate size, grow if necessary
            size_t cap = 4096;
            char *out = malloc(cap);
            size_t off = 0;
            off += snprintf(out + off, cap - off, "[");
            SongNode *t = head;
            while (t) {
                char entry[512];
                snprintf(entry, sizeof(entry), "{\"title\":\"%s\",\"artist\":\"%s\",\"bpm\":%d,\"energy\":%d,\"danceability\":%d,\"mood\":\"%s\"}",
                         t->title, t->artist, t->bpm, t->energy, t->danceability, t->mood);
                size_t need = strlen(entry) + 2;
                if (off + need + 32 > cap) { cap *= 2; out = realloc(out, cap); }
                off += snprintf(out + off, cap - off, "%s", entry);
                t = t->next;
                if (t) off += snprintf(out + off, cap - off, ",");
            }
            off += snprintf(out + off, cap - off, "]");
            send_response(client_fd, "200 OK", "application/json", out);
            free(out);
        } else if (strcmp(path, "/api/classify") == 0) {
            classify_songs();
            send_response(client_fd, "200 OK", "application/json", "{\"status\":\"classified\"}");
        } else if (strcmp(path, "/api/search") == 0) {
            // search by title, artist, mood (case-insensitive substring match)
            char *q_title = get_query_value(query, "title");
            char *q_artist = get_query_value(query, "artist");
            char *q_mood = get_query_value(query, "mood");

            char *lt = str_tolower_dup(q_title);
            char *la = str_tolower_dup(q_artist);
            char *lm = str_tolower_dup(q_mood);

            size_t cap = 2048;
            char *out = malloc(cap);
            size_t off = 0;
            off += snprintf(out + off, cap - off, "[");
            SongNode *t = head;
            int first = 1;
            while (t) {
                int match = 1;
                if (lt) {
                    char *low = str_tolower_dup(t->title);
                    if (!low || !strstr(low, lt)) match = 0;
                    if (low) free(low);
                }
                if (la && match) {
                    char *low = str_tolower_dup(t->artist);
                    if (!low || !strstr(low, la)) match = 0;
                    if (low) free(low);
                }
                if (lm && match) {
                    char *low = str_tolower_dup(t->mood);
                    if (!low || !strstr(low, lm)) match = 0;
                    if (low) free(low);
                }
                if (match) {
                    char entry[512];
                    snprintf(entry, sizeof(entry), "{\"title\":\"%s\",\"artist\":\"%s\",\"bpm\":%d,\"energy\":%d,\"danceability\":%d,\"mood\":\"%s\"}",
                             t->title, t->artist, t->bpm, t->energy, t->danceability, t->mood);
                    size_t need = strlen(entry) + 2;
                    if (off + need + 32 > cap) { cap *= 2; out = realloc(out, cap); }
                    if (!first) off += snprintf(out + off, cap - off, ",");
                    first = 0;
                    off += snprintf(out + off, cap - off, "%s", entry);
                }
                t = t->next;
            }
            off += snprintf(out + off, cap - off, "]");
            send_response(client_fd, "200 OK", "application/json", out);
            free(out);
            if (q_title) free(q_title);
            if (q_artist) free(q_artist);
            if (q_mood) free(q_mood);
            if (lt) free(lt);
            if (la) free(la);
            if (lm) free(lm);
        } else if (strcmp(path, "/api/add") == 0) {
            // accept params via query string: title,artist,bpm,energy,danceability
            char *title = get_query_value(query, "title");
            char *artist = get_query_value(query, "artist");
            char *bpm_s = get_query_value(query, "bpm");
            char *energy_s = get_query_value(query, "energy");
            char *dance_s = get_query_value(query, "danceability");
            if (!title || !artist || !bpm_s || !energy_s || !dance_s) {
                send_response(client_fd, "400 Bad Request", "application/json", "{\"error\":\"missing params\"}");
            } else {
                SongNode *new_song = malloc(sizeof(SongNode));
                if (!new_song) {
                    send_response(client_fd, "500 Internal Server Error", "application/json", "{\"error\":\"alloc failed\"}");
                } else {
                    strncpy(new_song->title, title, sizeof(new_song->title)-1); new_song->title[sizeof(new_song->title)-1] = '\0';
                    strncpy(new_song->artist, artist, sizeof(new_song->artist)-1); new_song->artist[sizeof(new_song->artist)-1] = '\0';
                    new_song->bpm = atoi(bpm_s);
                    new_song->energy = atoi(energy_s);
                    new_song->danceability = atoi(dance_s);
                    new_song->mood[0] = '\0';
                    new_song->next = NULL; new_song->prev = NULL;
                    // insert at end
                    if (!head) head = new_song;
                    else {
                        SongNode *tmp = head;
                        while (tmp->next) tmp = tmp->next;
                        tmp->next = new_song;
                        new_song->prev = tmp;
                    }
                    classify_songs();
                    save_songs();
                    send_response(client_fd, "200 OK", "application/json", "{\"status\":\"added\"}");
                }
            }
            free(title); free(artist); free(bpm_s); free(energy_s); free(dance_s);
        } else {
            send_response(client_fd, "404 Not Found", "text/plain", "Not Found");
        }
    } else {
        send_response(client_fd, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
    }

    close(client_fd);
}

void start_http_server(int port) {
    // Load songs (ensure data present)
    load_songs();

    int sockfd, new_fd;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    memset(&(serv_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        close(sockfd);
        return;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        close(sockfd);
        return;
    }

    printf("HTTP server listening on port %d\n", port);

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        int *pclient = malloc(sizeof(int));
        *pclient = new_fd;
        pthread_t tid;
        pthread_create(&tid, NULL, (void *(*)(void *))handle_client, pclient);
        pthread_detach(tid);
    }

    close(sockfd);
}
