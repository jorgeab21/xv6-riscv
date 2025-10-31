#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: settickets_test <número>\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    int res = settickets(n);

    if (res < 0)
        printf("Error: número inválido de tickets (%d)\n", n);
    else
        printf("Tickets asignados correctamente: %d\n", n);

    exit(0);
}
