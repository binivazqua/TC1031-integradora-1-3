# TC1031-integradora-1-3

_Por: Biniza Verónica Vázquez Moreno | A01737294_

El presente trabajo organiza los fragmentos de código modular que resuelven la **Act 1.3 - Actividad Integral de Conceptos Básicos y Algoritmos Fundamentales (Evidencia Competencia)** de la UF **TC1031**, con el objetivo de aplicar el vectores dinámicos en el lenguaje C++ para almacenar registros cronológicos de ataques por botnets, implementando el primer paso en el algoritmo de detección: el ordenamiento y búsqueda.

Los datos presentados, se almacenan en formato .txt, emulando un registro de ataques cibernéticos aparentemente aislados entre sí.

La dificultad de la tarea asignada radica en unificar la estructura de datos de la bitácora para así implementar algoritmos básicos de búsqueda y ordenamiento.

```
Aug 4 03:18:56 960.96.3.29:5268 Failed password for admin

```

## Implementación

Todo el código vive en [`ordYBusqBitacora.cpp`](ordYBusqBitacora.cpp). Aquí documento las piezas clave, en el orden en que se usan.

### 1. `struct Registro` — ¿por qué struct y no class?

```cpp
struct Registro
{
    int id;
    string mesTexto;
    int mesIdx;   // 1-12, resuelto UNA vez por registro
    int dia, hora, minuto, segundo;
    string ipPuerto;
    string mensaje;
};
```

Se eligió `struct` (campos públicos, sin métodos) en lugar de `class` por tres razones, no por costumbre:

1. **No hay invariantes que proteger.** La única regla real (`mesIdx` debe corresponder a `mesTexto`) se garantiza una sola vez, al construir el registro en `leerBitacora`. No hay nada que validar en cada lectura, así que no hace falta esconder los campos detrás de getters/setters.
2. **El comportamiento vive en funciones libres a propósito** (`leerBitacora`, `compararRegistros`, `mergeSort`, `binarySearch`). `compararRegistros` necesita ser reentrante y llamarse con distinto `maxCampo` según el contexto — convertirlo en método forzaría un dueño arbitrario entre dos `Registro` que se están comparando.
3. **Inicialización agregada.** `registros.push_back({...})` depende de que `Registro` sea un agregado (sin constructores propios). Una `class` con campos privados rompería esa sintaxis y exigiría un constructor de 9 parámetros sin lógica real que lo justifique.

### 2. `mesAIndice` — traducir mes a número, una sola vez

```cpp
unordered_map<string, int> mesAIndice = { {"Jan", 1}, ..., {"Dec", 12} };
```

Se resuelve **durante el parsing**, no dentro del comparador. Si lo resolviéramos en cada comparación, cada llamada a `compararRegistros` pagaría el costo de una búsqueda en el hash map; al resolverlo una vez y guardarlo en `mesIdx`, comparar dos registros es solo comparar dos `int`.

### 3. `compararRegistros` — un comparador recursivo y reutilizable

```cpp
int compararRegistros(const Registro &a, const Registro &b, int campo = 0, int maxCampo = 5)
{
    if (campo == maxCampo) return 0;
    // ... obtiene valA, valB según el campo actual (mes, día, hora, minuto o segundo)
    if (valA != valB) return (valA < valB) ? -1 : 1;
    return compararRegistros(a, b, campo + 1, maxCampo); // empate: pasa al siguiente campo
}
```

Compara mes → día → hora → minuto → segundo, en ese orden, y solo avanza al siguiente campo cuando el actual empata (caso base: `campo == maxCampo`). `maxCampo` es lo que permite reutilizar la misma función para dos propósitos distintos:

- `maxCampo = 5`: compara la fecha-hora completa → se usa para **ordenar**.
- `maxCampo = 2`: compara solo mes+día → se usa para la **búsqueda por rango de fecha**.

### 4. `mergeSort` — se ordenan índices, no registros

```cpp
vector<int> idx(registros.size());
for (int i = 0; i < (int)idx.size(); i++) idx[i] = i;
mergeSort(idx, registros, 0, (int)idx.size() - 1);
```

El ordenamiento mueve `vector<int> idx`, nunca `vector<Registro>` directamente. Un `Registro` carga varias strings (`mesTexto`, `ipPuerto`, `mensaje`); moverlo completo en cada intercambio cuesta proporcional a la longitud de esas strings. Mover un `int` cuesta O(1). Al comparar, se accede a los campos reales por referencia constante (sin copiar el `Registro`) dentro de `compararRegistros`.

**Complejidad:** O(n log n) en tiempo (garantizado, no solo en promedio), O(n) de espacio auxiliar para el buffer de mezcla (función `merge`).

### 5. `binarySearch` — la misma función sirve para los dos extremos del rango

```cpp
int binarySearch(const vector<int> &idx, const vector<Registro> &registros,
                     const Registro &objetivo, int izq, int der, bool estricto = false)
{
    if (izq >= der) return izq;
    int mid = izq + (der - izq) / 2;
    int cmp = compararRegistros(registros[idx[mid]], objetivo, 0, 2);
    bool irDerecha = estricto ? (cmp <= 0) : (cmp < 0);
    return irDerecha
        ? binarySearch(idx, registros, objetivo, mid + 1, der, estricto)
        : binarySearch(idx, registros, objetivo, izq, mid, estricto);
}
```

No busca "un registro exacto": busca una **frontera** en el arreglo ya ordenado. El rango de fechas que pedimos tiene dos fronteras, y son casi la misma pregunta — por eso es una sola función con un interruptor (`estricto`) en vez de dos funciones casi idénticas:

- **`estricto = false` (límite inferior, "primer registro `>= fechaInicio`")**: un empate (`cmp == 0`) ya es la respuesta, así que solo seguimos buscando a la derecha mientras el registro de en medio sea *antes* del objetivo (`cmp < 0`).
- **`estricto = true` (límite superior, "primer registro `> fechaFin`")**: un empate NO alcanza — hay que saltarse todos los registros de ese mismo día, así que seguimos a la derecha también cuando hay empate (`cmp <= 0`).

Esa diferencia de un solo operador (`<` vs `<=`) es lo que evita: (a) devolver una posición arbitraria entre varios registros del mismo día en vez de la más a la izquierda, y (b) cortar el rango antes de incluir todos los registros del último día.

**Complejidad:** O(log n) en tiempo, O(log n) de espacio por la pila de recursión.

## Temario del curso (TC1031)

Esta actividad aplica directamente los siguientes temas:

- **Recursividad**: `compararRegistros` y `binarySearch` están implementados de forma recursiva, con caso base y caso recursivo explícitos.
- **Complejidad algorítmica (Big-O)**: cada función está documentada con su complejidad en el código (parsing O(n), comparador O(1), merge sort O(n log n), búsqueda binaria O(log n)).
- **Algoritmos de ordenamiento**: merge sort, aplicado sobre índices en vez de sobre los datos completos.
- **Algoritmos de búsqueda**: búsqueda binaria, variantes `lower_bound`/`upper_bound` para resolver una búsqueda por rango.
- **Estructuras de datos**: `struct` como agregado de datos, `vector` como arreglo dinámico, `unordered_map` (hash table) para traducción mes→índice en tiempo O(1) promedio.
- **Structs vs. clases**: decisión de diseño justificada por invariantes, dueño del comportamiento e inicialización agregada.
