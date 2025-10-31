# **INFORME Tarea 2 – Sistemas Operativos**

**Integrantes:** Matías Céspedes, Jorge Becerra 
**Grupo:** 6 

## 1. Funcionamiento y Lógica de la Implementación

**Objetivo:** 
Implementar un planificador de procesos (*scheduler*) basado en el algoritmo **Lottery Scheduling** dentro del sistema operativo xv6. 
El objetivo es asignar CPU de forma proporcional al número de tickets que posee cada proceso, de manera que la probabilidad de ser elegido sea directamente proporcional a los tickets asignados.

**Funcionamiento General:** 
- Cada proceso posee un campo `tickets` que representa su cantidad de boletos. 
- Durante la ejecución del *scheduler*, se calcula el total de tickets de todos los procesos en estado *RUNNABLE*. 
- Se genera un número aleatorio entre `0` y `total_tickets - 1`. 
- Se recorre la lista de procesos acumulando los tickets hasta encontrar el proceso ganador. 
- El proceso ganador es ejecutado mediante el cambio de contexto (`swtch`). 
- Si un proceso tiene menos de 1 ticket, se le asigna 1 por seguridad.

**Syscall Implementada – `settickets(int n)`:** 
Permite modificar el número de tickets del proceso actual. 
El valor ingresado define su probabilidad de ser elegido por el *scheduler*.

**Comportamiento esperado:** 
- Si se llama con un número válido, los tickets se actualizan correctamente. 
- Si se entrega un valor negativo o cero, se retorna un error. 
- Esto permite controlar la prioridad relativa de los procesos de usuario.

## 2. Modificaciones Realizadas

### Archivos modificados

**`kernel/proc.h`** 
- Se agregó el campo `int tickets;` para almacenar la cantidad de boletos de cada proceso. 
- Se agregó también `int cpu_slices;` para llevar la cuenta de cuántas veces ha sido elegido por el scheduler.

**`kernel/sysproc.c`** 
- Se implementó la syscall `sys_settickets(void)` para asignar tickets al proceso actual. 
- La función recibe un parámetro `n` desde el espacio de usuario mediante `argint(0, &n);`. 
- Valida que el número de tickets sea positivo antes de actualizar `p->tickets`.

**`kernel/syscall.h`** 
- Se añadió el número de syscall correspondiente a `settickets`.

**`kernel/syscall.c`** 
- Se declaró externamente la función `sys_settickets`. 
- Se agregó la entrada respectiva en la tabla de syscalls para asociar el número con la función del kernel.

**`kernel/proc.c`** 
- Se modificó la función `scheduler()` para implementar la lógica del **Lottery Scheduling**. 
- Se calcula el total de tickets de los procesos *RUNNABLE*, se genera un número aleatorio (`winner`) y se selecciona el proceso ganador sumando tickets. 
- Se ejecuta el proceso elegido mediante `swtch(&c->context, &p->context);`. 
- Se garantiza que cada proceso tenga al menos 1 ticket por seguridad. 
- Se incrementa `cpu_slices` cada vez que un proceso es ejecutado.

**`user/user.h`** 
- Se añadió la declaración de la nueva syscall `int settickets(int n);`.

**`user/usys.pl`** 
- Se agregó la línea correspondiente para generar el *wrapper* de `settickets()` en espacio de usuario.

**`Makefile`** 
- Se incluyó el nuevo programa de prueba `sttest` en la lista `UPROGS`.

## 3. Dificultades Encontradas y Soluciones Implementadas

** Dificultad 1: Error de compilación por variables no utilizadas** 
**Problema:** Durante las primeras pruebas del `scheduler()` aparecían errores del tipo *“unused variable 'p'”* o *“function defined but not used”* 
**Solución:** Se revisó cuidadosamente el alcance de las variables y se ajustaron las llaves de cierre (`{}`) para asegurar que todas las declaraciones estuvieran dentro de los bloques correctos. Esto permitió compilar el kernel sin advertencias.

--

** Dificultad 2: Lógica de selección aleatoria del proceso ganador** 
**Problema:** El algoritmo de lotería debía recorrer todos los procesos *RUNNABLE* sumando los tickets hasta alcanzar el número ganador, pero en las primeras versiones el cálculo de `total_tickets` y `winner` no coincidía 
**Solución:** Se reestructuró el bucle `for` asegurando que el conteo de tickets se hiciera antes del sorteo, y se agregó una validación de seguridad para asignar 1 ticket mínimo a cada proceso. También se usó `rand() % total_tickets` para garantizar uniformidad en la selección.

---

** Dificultad 3: Implementación de la syscall `settickets()`** 
**Problema:** La syscall no actualizaba correctamente los tickets porque el valor ingresado desde el espacio de usuario no se transfería al kernel. 
**Solución:** Se corrigió la lectura del argumento utilizando `argint(0, &n)` y se agregó un control para evitar valores negativos. Con esto, el proceso pudo actualizar su campo `tickets` correctamente al ejecutar `sttest`.

---

** Dificultad 4: Fallo de ejecución del programa de usuario (`exec $ failed`)** 
**Problema:** El programa de prueba `sttest` no se ejecutaba, arrojando el mensaje “exec $ failed”. 
**Solución:** Se verificó que el archivo estuviera correctamente listado en la variable `UPROGS` del `Makefile`, y que existiera en la carpeta `/user`. Tras recompilar (`make clean` + `make qemu`), el programa funcionó correctamente mostrando la asignación de tickets.

---

** Dificultad 5: Depuración de errores aleatorios en el scheduler** 
**Problema:** En ocasiones, el sistema no ejecutaba correctamente los procesos o se colgaba al iniciar. Esto se debía a bloqueos de locks (`acquire()` / `release()`) en secciones mal ubicadas. 
**Solución:** Se reordenaron las llamadas a `acquire()` y `release()` alrededor de las secciones críticas, y se comprobó que solo se ejecutara un proceso en estado *RUNNABLE* a la vez durante el cambio de contexto.

## 4. Programa de Prueba y Resultados

** Programa de prueba: `sttest.c`**

El programa `sttest` fue creado con el objetivo de verificar el correcto funcionamiento de la nueva syscall `settickets(int n)`.

### Descripción:
- El programa recibe como argumento un número entero `n`, correspondiente a la cantidad de tickets que se desea asignar al proceso actual.
- Llama a la syscall `settickets(n)` e imprime un mensaje confirmando si la asignación fue exitosa o si el valor fue inválido.

### Ejemplo de uso:
$ sttest 50
Tickets asignados correctamente: 50

### Validación en el kernel

Para confirmar que la syscall efectivamente modificaba los tickets del proceso, se inspeccionó la estructura del proceso (`struct proc`) en el archivo `proc.c`, observando que el valor de `p->tickets` cambiaba correctamente tras la llamada a `settickets()`.

Además, mediante el uso del scheduler modificado, se verificó que:
- Los procesos con más tickets aumentan su frecuencia de ejecución (medida con `cpu_slices`). 
- Cada ejecución incrementa el contador `p->cpu_slices++`, lo cual permite observar la distribución probabilística de ejecución según la cantidad de tickets asignados.

---

### Resultados obtenidos
- El sistema compiló y ejecutó correctamente sin errores ni bloqueos. 
- La syscall `settickets()` respondió de manera estable para distintos valores de entrada. 
- Los procesos con más tickets fueron seleccionados más veces, validando el funcionamiento del **Lottery Scheduling**. 
- El kernel mantuvo su estabilidad durante las pruebas, sin fugas ni estados muertos.

---

## 5. Posibles Problemas del Lottery Scheduling

A pesar de su simplicidad y eficiencia probabilística, el algoritmo de **Lottery Scheduling** presenta algunas limitaciones importantes:

1. **Variabilidad en tiempos de ejecución:** 
   Dado que la selección de procesos es aleatoria, pueden existir fluctuaciones en los tiempos de respuesta, especialmente en sistemas con pocos procesos activos.

2. **Falta de determinismo:** 
   No garantiza que todos los procesos reciban CPU en un orden predecible. En entornos donde la equidad temporal es crítica, puede generar comportamientos inesperados.

3. **Injusticia a corto plazo:** 
   Un proceso con pocos tickets podría tardar mucho más en ser seleccionado si el azar no lo favorece, afectando su rendimiento momentáneo.

4. **Sobrecarga mínima por conteo de tickets:** 
   Aunque leve, la suma de todos los tickets en cada iteración del scheduler introduce un costo computacional adicional comparado con algoritmos deterministas como Round Robin.

5. **Dependencia de la calidad del generador aleatorio:** 
   Si la función `rand()` no produce una distribución verdaderamente uniforme, la equidad del sistema puede verse afectada.

---

## 6. Conclusión

Se implementó exitosamente un **sistema de planificación por lotería (Lottery Scheduling)** en el kernel de XV6, integrando correctamente el campo de tickets, la syscall `settickets(int n)` y las modificaciones en el `scheduler()`.

El programa de prueba `sttest` permitió comprobar la funcionalidad del sistema, mostrando resultados consistentes con la teoría: los procesos con más tickets obtienen una mayor proporción de CPU time.

El desarrollo implicó diversos desafíos técnicos relacionados con el manejo de locks, la lógica de selección aleatoria y la correcta comunicación entre espacio de usuario y kernel. 
Sin embargo, la versión final del sistema logró un funcionamiento estable y coherente.

---

**Repositorio:** [GitHub - xv6-riscv Tarea 2](https://github.com/Matiescespedes/xv6-riscv/tree/users/mati/t2)



