// DEMO how to use the server library.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cds_public.h"

/* TODO - we need create a public API header in the future */

int my_handler(int size, void *req, int *len, void *resp)
{
    int i = atoi(req);
    if(i  > 0)
    {
        i = (i * 2);
        sprintf(resp, "%d", i);
        *len = (strlen(resp) + 1);
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

    cfg.ac_cfgfile = NULL; /* TODO add confg file name here */
    cfg.ac_handler = my_handler;
    cds_init(&cfg, argc, argv);
    return 0;
}
