#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ow-crypt.h"

int main(void)
{

   // void *data = NULL;
   // int size = 0;

    char *enc = crypt("guildmud", "$2y$12$usesomesillystringforsalt$");
    // $2y$12$usesomesillystringforeK4UQKqQz32k32guri03ARP4/qWtPveG
    printf("Encrypted data %s %lu\n", (char*) enc, strlen(enc));
    //char *result = crypt_ra("la clave es asi", "$2y$12$usesomesillystringforsalt$", &data, &size);
    //printf("Encrypted data %s %lu, %d\n", (char*) data, strlen(data), size);
}
