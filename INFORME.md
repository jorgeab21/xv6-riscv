# INFORME Tarea 1 – Sistemas Operativos

**Integrantes:** Matías Céspedes, Jorge Becerra 
**Grupo:** 6 

## 1. Funcionamiento de las Llamadas al Sistema

### getppid()

**Objetivo:** Obtener el Process ID (PID) del proceso padre. 

**Funcionamiento:** 
- En el kernel se accede al proceso actual con `myproc()`. 
- A partir de esta estructura se obtiene el puntero al proceso padre (`parent`). 
- Finalmente se retorna el PID del padre (`parent->pid`). 
- Esta syscall permite comprender la jerarquía de procesos dentro de xv6. 

### getancestor(int nivel)

**Objetivo:** Obtener el PID de un ancestro a diferentes niveles en el árbol de procesos. 

**Funcionamiento:** 
- `getancestor(0)` retorna el PID del proceso actual. 
- `getancestor(1)` retorna el PID del padre. 
- `getancestor(2)` retorna el PID del abuelo. 
- `getancestor(n)` retorna el PID del *n*-ésimo ancestro. 
- Si no existe el ancestro, la función retorna `-1`. 

**Implementación:** 
1. Se obtiene el parámetro `nivel` desde el usuario con `argint()`. 
2. Se comienza desde el proceso actual (`myproc()`). 
3. Se recorre hacia arriba en el árbol tantas veces como indique `nivel`. 
4. Si en algún punto no existe padre, se retorna `-1`. 
5. En caso contrario, se retorna el PID del ancestro encontrado. 


## 2. Modificaciones Realizadas

### Archivos modificados

- **kernel/syscall.h** 
  - Se definió el número de syscall para `getppid` y `getancestor`. 

- **kernel/syscall.c** 
  - Se agregaron las declaraciones externas de las funciones. 
  - Se añadió cada syscall en la tabla de syscalls. 

- **kernel/sysproc.c** 
  - Se implementó `sys_getppid()`, que retorna el PID del proceso padre. 
  - Se implementó `sys_getancestor()`, que recorre los punteros `parent` según el nivel recibido. 

- **user/user.h** 
  - Se agregaron las declaraciones de `getppid()` y `getancestor(int)` para que puedan ser usadas en programas de usuario. 

- **user/usys.pl** 
  - Se agregó la entrada correspondiente a `getancestor` para generar el wrapper en user space. 

- **Makefile** 
  - Se agregaron los programas de prueba `yosoytupadre` y `ancestros` en la sección `UPROGS`. 

## 3. Programas de Prueba

- **yosoytupadre.c** 
  Programa que imprime el PID actual y el PID del padre, validando la syscall `getppid()`. 

- **ancestros.c** 
  Programa que invoca `getancestor(n)` con distintos valores de `n` (0, 1, 2, 3 y -1), mostrando los PIDs esperados o `-1` cuando corresponde. 

Estos programas fueron utilizados para validar que ambas syscalls funcionen correctamente. 

## 4. Dificultades Encontradas y Soluciones

**Dificultad 1: Configuración de Git** 
- *Problema:* Existían cambios locales que impedían cambiar de rama. 
- *Solución:* Se usó `git checkout -- .` para limpiar cambios y luego trabajar en la rama correspondiente. 

**Dificultad 2: Uso de `argint()`** 
- *Problema:* Al intentar usar `argint()` como condición se generaba un error de compilación. 
- *Solución:* Se investigó cómo se utilizaba en otras syscalls y se corrigió invocándolo directamente con un puntero a la variable. 

**Dificultad 3: Comprender el flujo de una syscall** 
- *Problema:* No estaba claro cómo viaja la llamada desde el espacio de usuario hasta el kernel. 
- *Solución:* Se estudió la implementación de `getpid()` y se entendió el flujo: 
  1. User space invoca la función. 
  2. `usys.pl` genera el wrapper en ensamblador. 
  3. El kernel busca la entrada en la tabla de syscalls. 
  4. Se ejecuta la función en el kernel. 
  5. El resultado se retorna a user space. 

**Dificultad 4: Recorrer el árbol de procesos** 
- *Problema:* Implementar la lógica de recorrer los ancestros. 
- *Solución:* Se utilizó un bucle que avanza por el puntero `parent`, validando en cada paso que no sea nulo. 

**Dificultad 5: Caracteres extraños en el código** 
- *Problema:* El editor a veces insertaba caracteres invisibles que producían errores. 
- *Solución:* Se usó `make clean` y se revisó cuidadosamente el código antes de recompilar. 

## 5. Resultados de Pruebas

**Prueba de yosoytupadre:** 
- El programa mostró el PID del proceso actual y el PID del padre. 
- El resultado fue correcto según lo esperado. 

**Prueba de ancestros:** 
- `getancestor(0)` → retorna el PID actual. 
- `getancestor(1)` → retorna el PID del padre. 
- `getancestor(2)` → retorna el PID del abuelo. 
- `getancestor(3)` → retorna `-1` (no existe ancestro). 

En todos los casos, los resultados fueron los esperados. 

## 6. Conclusión

- Se implementaron correctamente dos nuevas syscalls en xv6: `getppid()` y `getancestor()`. 
- Los programas de prueba validaron el funcionamiento esperado. 
- El código quedó consistente con los patrones del sistema xv6. 
- La principal dificultad fue comprender cómo funciona el paso de parámetros en syscalls, lo que se resolvió revisando ejemplos ya implementados. 

**Repositorio:** [GitHub - xv6-riscv Tarea 1](https://github.com/Maticespedes/xv6-riscv/tree/users/mati/t1) 
