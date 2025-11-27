# **INFORME Tarea 3 – Sistemas Operativos**

**Integrantes:** Jorge Becerra
Matías Céspedes
**Grupo:** 6

## 1. Funcionamiento y Lógica de la Implementación

**Objetivo:**
El objetivo de esta tarea fue implementar un mecanismo de protección de memoria en xv6 que permita deshabilitar la lectura de páginas específicas de un proceso ("Write-Only memory"). Esto es útil en contextos de seguridad para proteger datos sensibles que no deben ser leídos inadvertidamente.

**Funcionamiento General:**
- La protección se logra manipulando los bits de permisos en las **Entradas de la Tabla de Páginas (PTE)**.
- Se implementaron dos llamadas al sistema que modifican el bit `PTE_R` (Read) de las páginas correspondientes a un rango de memoria virtual.
- **`mrdprotect`:** Recorre las páginas del rango indicado y apaga el bit `PTE_R` (`~PTE_R`).
- **`munrdprotect`:** Recorre las páginas y vuelve a encender el bit `PTE_R` (`| PTE_R`).
- Tras cualquier modificación en la tabla de páginas, se ejecuta la instrucción `sfence_vma()` para limpiar el **TLB** (Translation Lookaside Buffer) y asegurar que la CPU reconozca los nuevos permisos inmediatamente.

**Syscalls Implementadas:**
1. **`mrdprotect(void *addr, int len)`:** Deshabilita la lectura en `len` páginas a partir de `addr`. Retorna -1 si hay error.
2. **`munrdprotect(void *addr, int len)`:** Restaura el permiso de lectura en el rango especificado.

## 2. Modificaciones Realizadas

### Archivos modificados

**`kernel/vm.c`**
- Se implementó la lógica principal en las funciones `mrdprotect` y `munrdprotect`.
- Se utilizó la función `walk()` con el argumento `alloc=0` para obtener la PTE correspondiente a una dirección virtual sin crear nuevas páginas.
- Se agregaron validaciones: alineación de página (`PGSIZE`), existencia de la PTE (`PTE_V`) y permisos de usuario (`PTE_U`).
- Se realizó la manipulación de bits y el flush del TLB con `sfence_vma()`.

**`kernel/sysproc.c`**
- Se crearon las funciones `sys_mrdprotect` y `sys_munrdprotect` para recibir los argumentos desde el espacio de usuario.
- Se adaptó la captura de argumentos (`argaddr`, `argint`) considerando que en esta versión de xv6 estas funciones tienen retorno `void`.

**`kernel/defs.h`**
- Se añadieron los prototipos de `mrdprotect` y `munrdprotect` para hacerlas visibles desde `sysproc.c`.

**`kernel/syscall.h`**
- Se definieron los números de syscall `SYS_mrdprotect` (25) y `SYS_munrdprotect` (26).

**`kernel/syscall.c`**
- Se registraron las nuevas llamadas en el arreglo de punteros a funciones y se declararon sus `extern`.

**`user/user.h` y `user/usys.pl`**
- Se definieron las firmas para que los programas de usuario puedan invocar estas syscalls.
- Se generaron los stubs de ensamblador para la transición a modo kernel.

**`Makefile`**
- Se añadió el programa de prueba `_rdprotect_test` a la lista `UPROGS`.

## 3. Dificultades Encontradas y Soluciones Implementadas

**Dificultad 1: Error de compilación en `sysproc.c`**
**Problema:** Al intentar validar el retorno de `argaddr()` dentro de un `if`, el compilador arrojaba el error *"void value not ignored as it ought to be"*. Esto se debía a que en la distribución de xv6 utilizada, estas funciones auxiliares son `void`.
**Solución:** Se eliminó la comprobación de retorno en el `if` dentro de `sysproc.c` y se delegó la validación lógica (direcciones válidas) a la implementación en `vm.c`.

---

**Dificultad 2: Persistencia de permisos antiguos en caché**
**Problema:** Aunque se modificaban los bits en la tabla de páginas, existía el riesgo de que el procesador siguiera usando permisos antiguos almacenados en el TLB.
**Solución:** Se investigó el funcionamiento de la paginación en RISC-V y se añadió la llamada a `sfence_vma()` al final de cada función de protección para invalidar el TLB.

---

**Dificultad 3: Interpretación de la excepción del procesador**
**Problema:** Al probar la protección, el proceso hijo terminaba abruptamente con un mensaje `usertrap` y `scause 0xd`. Inicialmente pareció un error de código.
**Solución:** Tras analizar la documentación de RISC-V, se confirmó que `scause 0xd` (13) corresponde a *Load Page Fault*. Esto validó que la tarea estaba correcta: el hardware detectó y bloqueó el intento de lectura no autorizado.

## 4. Programa de Prueba y Resultados

**Programa de prueba: `rdprotect_test.c`**

Se desarrolló un test que realiza el siguiente flujo:
1. Escribe un dato en memoria.
2. Protege la página contra lectura (`mrdprotect`).
3. Hace `fork()`. El hijo intenta leer esa dirección.
4. El padre espera, luego revierte la protección (`munrdprotect`) y verifica el dato.

### Resultados obtenidos
Al ejecutar el test en QEMU:

```text
Test 1: Protegiendo memoria (lectura deshabilitada)...
[Hijo] Intentando leer (Deberia morir con scause 0xd/13)...
usertrap(): unexpected scause 0xd pid=4 ...
[Padre] El hijo termino/murio.
Proteccion revertida.
Valor final leido: Z (Debe ser Z)
--- TEST FINALIZADO ---