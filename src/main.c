#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "../include/cutils.h"
#include "../include/server.h"
#include "../include/hcml.h"

#define VERSION "0.1.0"

int _run_server_(int argc, char** argv);
int _compile_ (int argc, char** argv);
int _help_ (int argc, char** argv);
int _version_ (int argc, char** argv);

void* args[5][2] = {
    {"server"  , _run_server_},
    {"compile" , _compile_},
    {"help"    , _help_},
    {"version" , _version_},
    {NULL, NULL}
};


int _run_server_(int argc, char** argv) {
    if (argc < 3) {
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

int _compile_ (int argc, char** argv) {
    if (argc < 3) {
        msg(ERROR, "Usage: %s <hcml_file> <html_file>", argv[0]);
        return 1;
    }

    // first file is the hcml file (input)
    char* hcml_file = argv[1];
    // second file is the html file (output)
    char* html_file = argv[2];

    char* hcml_str;
    size_t hcml_size = rdfile(hcml_file, &hcml_str);
    if (hcml_size == 0) {
        msg(ERROR, "Failed to read file %s", hcml_file);
        return 1;
    }

    // parse html
    struct html_tag* html = html_parser(hcml_str);

    // compile the html
    int res = hcml_compile(html);
    if (res != 0) {
        msg(ERROR, "Failed to compile HCML");
        return 1;
    }

    // create the html string
    char* html_str;
    size_t html_size = create_html(html, &html_str);

    // write the html string to the file
    res = wrnfile(html_file, html_str, html_size);

    // free the html
    destroy_html(html);
    free(hcml_str);
    return res;
}

int _help_ (int argc, char** argv) {
    msg(INFO, "Usage: %s <command> [args]", argv[0]);
    msg(INFO, "Commands:");
    for (int i = 0; i < sizeof(args) / sizeof(args[0]); i++) {
        msg(INFO, "  %s", args[i][0]);
    }
    return 0;
}

int _version_ (int argc, char** argv) {
    msg(INFO, "HCML v%s", VERSION);
    return 0;
}




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

    // check if we have a command
    hashtable* cmds = hm_init(args, sizeof(args) / sizeof(args[0]));

    if (argc < 2) {
        msg(ERROR, "Usage: %s <command> [args]", argv[0]);
        return 1;
    }

    char* cmd = argv[1];
    void* fn = hm_get(cmds, cmd);
    if (fn == NULL) {
        msg(ERROR, "Unknown command %s", cmd);
        return 1;
    }

    // call the function
    int (*f)(int, char**) = fn;
    return f(argc - 1, argv + 1);
}