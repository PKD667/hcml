// web server implementation for the HCML framework

#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/hcmx.h"
#include "../include/context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>


#define BUFFER_SIZE 1024

char* load_web_file(struct server_context* ctx, char* web_root_path, char* path);

struct http_request* parse_request(char* buffer);
struct http_response* handle_request(struct http_request* request, struct server_context* ctx);

char* response_str(struct http_response* response);

int server(int port,char* web_root_path) {

    int server_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    struct server_context* ctx = srv_ctx_create(port, web_root_path);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        int client_fd;
        int addrlen = sizeof(address);


        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        int bytes = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes < 0) {
            perror("read");
            close(client_fd);
            continue;
        }
        buffer[bytes] = '\0';

        char method[10], path[256];
        sscanf(buffer, "%s %s", method, path);

        printf("Request: %s %s\n", method, path);


        struct http_request* request = parse_request(buffer);
        if (request == NULL) {
            msg(ERROR, "Error: could not parse request");
            close(client_fd);
            continue;
        }

        struct http_response* response = handle_request(request, ctx);

        char* res_str = response_str(response);

        write(client_fd, res_str, strlen(res_str));
        close(client_fd);
    }

    return 0;

}



struct http_response* handle_request(struct http_request* request, struct server_context* ctx) {

    struct http_response* response = calloc(1, sizeof(struct http_response));

    if (request->method == GET) {

        char* content = load_web_file(ctx, ctx->web_root, request->path);

        if (content != NULL) {
            response->status = 200;
            response->content = content;
            response->headers_count = 1;
            response->headers = calloc(1, sizeof(struct http_header));
            response->headers[0].name = "Content-Type";
            response->headers[0].value = "text/html";
            return response;
        } 
    }

    // handle the request
    struct htmx_handler* handler = htmx_ctx_get_handler(ctx->htmx_ctx, request);
    if (handler == NULL) {
        msg(ERROR, "Error: no handler for %s %s", http_method_str(request->method), request->path);
        
        response->status = 404;
        response->content = "Not found";
        return response;
    }

    // run the handler
    struct html_tag* hcmx_html = run_hcmx(*request, handler->hcmx_handler, ctx->htmx_ctx->functions);

    char* html_code;
    int code_size = create_html(hcmx_html, &html_code);

    response->status = 200;
    response->content = html_code;
    response->headers_count = 1;
    response->headers = calloc(1, sizeof(struct http_header));
    response->headers[0].name = "Content-Type";
    response->headers[0].value = "text/html";

    return response;
}

struct http_request* parse_request(char* buffer) {

    dbg(3, "Parsing request");

    struct http_request* request = calloc(1, sizeof(struct http_request));

    char method[10], path[256];
    sscanf(buffer, "%s %s", method, path);

    dbg(3, "Method: %s, Path: %s", method, path);

    if (strcmp(method, "GET") == 0) {
        request->method = GET;
    } else if (strcmp(method, "POST") == 0) {
        request->method = POST;
    } else if (strcmp(method, "PUT") == 0) {
        request->method = PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        request->method = DELETE;
    } else {
        msg(ERROR, "Error: invalid method %s", method);
        return NULL;
    }

    request->path = strdup(path);

// parse the headers
    char* line = strtok(buffer, "\n");
    line = strtok(NULL, "\n");
    while (line != NULL && line[0] != '\0') {

        dbg(3, "Parsing header: %s", line);

        char* sep = strchr(line, ':');
        if (sep == NULL) {
            // No ':' found, skip to next line
            line = strtok(NULL, "\n");
            continue;
        }
        sep[0] = '\0';
        char* name = line;
        char* value = sep + 1;

        // Trim leading whitespace from value
        while (*value == ' ' || *value == '\t') {
            value++;
        }

        dbg(3, "Header: %s: %s", name, value);

        struct http_header header;
        header.name = strdup(name);
        header.value = strdup(value);
        request->headers_count++;
        request->headers = realloc(request->headers, request->headers_count * sizeof(struct http_header));
        request->headers[request->headers_count - 1] = header;

        line = strtok(NULL, "\n");
    }

    // parse the body
    if (line != NULL) {
        request->body = strdup(line);
    }

    return request;
}

char* response_str(struct http_response* response) {
    char* buffer = calloc(1, 1024);
    int offset = 0;

    offset += snprintf(buffer + offset, 1024 - offset, "HTTP/1.1 %d OK\n", response->status);
    for (size_t i = 0; i < response->headers_count; i++) {
        offset += snprintf(buffer + offset, 1024 - offset, "%s: %s\n", response->headers[i].name, response->headers[i].value);
    }
    offset += snprintf(buffer + offset, 1024 - offset, "\n%s", response->content);

    return buffer;
}




#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define is_hcml(file_path) (strstr(file_path, ".hcml") != NULL || \
                            strstr(file_path, ".hcmx") != NULL)

char* load_web_file(struct server_context* ctx, char* web_root_path, char* path) {

    char* file_path = calloc(1, strlen(web_root_path) + strlen(path) + 1);
    sprintf(file_path, "%s/%s", web_root_path, path);
    
    char* file_content;
    dbg(3, "File path: %s", path);


    if (is_hcml(file_path)) {
        
        msg(INFO, "Loading HCML file %s", file_path);

        struct htmx_context* file_ctx;
        if (get_hcmx(file_path, &file_content, &file_ctx) != 0) {
            msg(ERROR, "Error: could not get hcmx for file %s", file_path);
            return NULL;
        }
        

        dbg(3, "Adding hcmx to context");
        if (htmx_ctx_merge(ctx->htmx_ctx, file_ctx) != 0) {
            msg(ERROR, "Error: could not add hcmx for file %s", file_path);
            return NULL;
        }

    }
    else {
        msg(INFO, "Loading raw file %s", file_path);
        int file_size = rdfile(file_path, &file_content);
        if (file_size < 0) {
            msg(ERROR, "Error: could not read file %s (%d)", file_path, file_size);
            return NULL;
        }
    }

    dbg(3, "Returning file content");
    
    return file_content;
}

