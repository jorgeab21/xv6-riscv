#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    printf("Ancestros:\n");
    printf("getancestor(0) = %d (debería ser mi pid)\n", getancestor(0));
    printf("getancestor(1) = %d (debería ser mi padre)\n", getancestor(1));
    printf("getancestor(2) = %d (debería ser mi abuelo)\n", getancestor(2));
    printf("getancestor(3) = %d (debería ser -1 si no existe)\n", getancestor(3));
    printf("getancestor(-1) = %d (debería ser -1 por inválido)\n", getancestor(-1));

    exit(0);
}
