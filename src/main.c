
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "../include/cutils.h"
#include "../include/server.h"





int main(int argc, char** argv) {
    
    // get debug from env
    char* debug = getenv("DEBUG");
    if (debug != NULL) {
        DEBUG = atoi(debug);
        dbg(1, "Debug level: %d", DEBUG);
    }

    char* debug_file = getenv("DEBUG_FILE");
    if (debug_file != NULL) {
        DEBUG_FILE = debug_file;
    }

    char* debug_fn = getenv("DEBUG_FN");
    if (debug_fn != NULL) {
        DEBUG_FN = debug_fn;
    }


    // start the server
    if (argc < 2) {
        msg(ERROR, "Usage: %s <port> <web_dir>", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port == 0) {
        msg(ERROR, "Invalid port %s", argv[1]);
        return 1;
    }

    char* web_dir = argv[2];


    struct webserver* srv = create_server(port,web_dir);
    if (srv == NULL) {
        msg(FATAL,"Weberver creation failed, sorry !");
    }
    msg(INFO, "Serving HCML at %s on port %d",web_dir,port);
    // running the srver until death
    return run_server(srv);
}