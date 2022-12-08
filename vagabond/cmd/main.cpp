//
// Created by Dimitris on 20/09/2022.
//
#include <stdio.h>
#include <cstring>
#include <curl/curl.h>
#include "Dictator.h"

float test = 0;

int main (int argc, char **argv)
{
    curl_global_init(CURL_GLOBAL_ALL);

    Dictator *dictator = new Dictator();

    std::vector<std::string> args;
    // Skip loading the json file if the first argument isn't help
    if (argc == 0 && (strcmp(argv[0], "help") != 0 &&
                      strcmp(argv[0], "--help") != 0 &&
                      strcmp(argv[0], "-h") != 0))
    {
        args.push_back("environment=rope.json");
    }
    for (size_t i = 1; i < argc; i++)
    {
        args.push_back(std::string(argv[i]));
    }

    dictator->setArgs(args);
    dictator->setup();
    dictator->start();

    dictator->wait();
    delete dictator;

    curl_global_cleanup();
    return 0;
}
