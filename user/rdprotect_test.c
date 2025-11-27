#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
  char *addr = sbrk(0);
  sbrk(4096);
  addr[0] = 'Z'; // Valor inicial

  printf("--- INICIANDO TEST ---\n");

  // 1. Proteger
  if (mrdprotect(addr, 1) < 0) {
    printf("Error en mrdprotect\n");
    exit(1);
  }
  printf("Memoria protegida (Lectura OFF).\n");

  // 2. Vamos a probar LEER (Esto DEBE fallar y matar el proceso)
  // Usamos fork para que si muere el hijo, el padre nos avise.
  int pid = fork();
  
  if(pid == 0) {
      // PROCESO HIJO
      printf("[Hijo] Intentando leer (Deberia morir con scause 0xd/13)...\n");
      char c = addr[0]; // <--- AQUÍ DEBERÍA OCURRIR EL TRAP
      printf("[Hijo] ERROR: Pude leer el valor: %c\n", c);
      exit(0);
  } else {
      // PROCESO PADRE
      wait(0);
      printf("[Padre] El hijo termino/murio.\n");
  }

  // 3. Revertir
  if (munrdprotect(addr, 1) < 0) {
    printf("Error en munrdprotect\n");
    exit(1);
  }
  printf("Proteccion revertida.\n");

  // 4. Probar lectura de nuevo
  printf("Valor final leido: %c (Debe ser Z)\n", addr[0]);
  
  printf("--- TEST FINALIZADO ---\n");
  exit(0);
}