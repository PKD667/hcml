
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/hcml.h"
#include "../include/cutils.h"
#include "../include/server.h"
#include "../include/context.h"


int main(int argc, char** argv) { 
    
    DEBUG = 4;


    server(8080, "web/");

    return 0;
}