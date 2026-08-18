#include <iostream>

#include "Console.h"

int main (int argc, char **argv)
{
    Console console(1, true);
    console.print("Console width: {}", console.width());
    console.print("This is the start of something new!");
    return 0;
}