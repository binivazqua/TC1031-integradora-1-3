#include <iostream>
#include <random>
#include <vector>

using namespace std;

/**
 * Función auxiliar del algortimo Mergesort, que realiza la lógica de
 * selección del elemento más pequeño entre los subproblemas obtenidos
 * a partir de la división en el método principal.
 * @param vec_desordenado es una referencia al vector base creado por el usuario en main().
 * @param start, end, half son los índices de referencia para iterar y comparar.
 * @param compares es una referencia a la variable inicializada en main(), que mantiene la cuenta de
 * comparaciones dentro de la función.
 *
 * Compone el factor de complejidad temporal lineal del algortimo merge sort, dado que
 * se realizan sólo comparaciones y copias de elementos, sin loops anidados, con un número de iteraciones
 *  no mayor al tamaño total del vector.
 */

void join(vector<int> &vec_desordenado, int start, int end, int half, int &compares)
{
    // usamos un arreglo auxiliar para almacenar los datos ordenados:
    int n = (end - start) + 1;
    vector<int> aux(n);

    // indexes para trabajar y copias:
    int left_index = start;
    int right_index = half + 1;

    // index para iterar el arreglo auxiliar:
    int aux_index = 0;

    while (left_index <= half && right_index <= end)
    {
        // update de comparaciones:
        compares++;

        // seguimos lógica de merge:
        // si el dato de la primera mitad es menor, lo copio:
        if (vec_desordenado[left_index] <= vec_desordenado[right_index])
        {
            // copia:
            aux[aux_index] = vec_desordenado[left_index];

            // aumentamos el índice:
            left_index++;
        }
        else
        {
            // si la segunda mitad es menor, entonces copiamos ESOS VALORES.
            aux[aux_index] = vec_desordenado[right_index];
            right_index++;
        }

        // Update del iterador del vector auxiliar:
        aux_index++;
    }

    // SALIMOS DEL LOOP Y EVALUAMOS CUÁL DE LAS LISTAS TERMINO PRIMERO:

    // CASO 1: LA MITAD IZQUIERDA NO SE HA TERMINADO:
    if (left_index <= half)
    {
        // significa que la mitad izquierda aún tiene elementos, entonces copiamos los elementos restantes:
        while (left_index <= half)
        {
            // TAMBIÉN INCREMENTAR COMPARES:
            compares++;
            aux[aux_index] = vec_desordenado[left_index];
            left_index++;
            aux_index++;
        }
        // CASO 2: LA MITAD DERECHA NO SE HA TERMINADO:
    }
    else if (right_index <= end)
    {
        while (right_index <= end)
        {
            compares++;
            aux[aux_index] = vec_desordenado[right_index];
            right_index++;
            aux_index++;
        }
    }

    // COPIAMOS LOS DATOS AL VECTOR ORIGINAL:
    for (int i = 0; i < n; i++)
    {
        // aux siempre va de 0 a n -1, mientras que vec_desordenado va de start a end,
        // entonces sumamos start para que el índice sea correcto:
        vec_desordenado[start + i] = aux[i];
    }
}

// ======================= FUNCIONES DE ORDENAMIENTO ============================ //

/**
 * Función principal (recursiva) del algortimo mergeSort:
 * Emplea el paradigma de "divide y vencerás", dividiendo el arreglo en dos subproblemas hasta
 * alcanzar el caso más sencillo (base), cuando el arreglo a dividir tiene sólo 1 elemento.
 * @param vec_desordenado es el vector creado por el usuario en main() --> a ordenar.
 * @param start, end son los índices de referencia para iterar y comparar.
 * @param calls es una referencia a la variable inicializada en main(), que mantiene la cuenta de llamadas dentro de la función.
 * @param compares es una referencia a la variable inicializada en main(), que mantiene la cuenta de comparaciones dentro de la función.
 *
 * Complejidad Temporal Asintótica:
 * Este algortimo es particularmente estable, pues, al dividir el arreglo en subproblemas
 * de forma simétrica, independientemente de su composición, tendrá una complejidad uniforme de orden
 * logarítmico. Adicionalmente, el cómputo del método join, al implementar un loop con un número de
 * iteraciones no mayor al tamaño total del vector, nos encontramos a un algoritmo de orden:
 * O(n log n)
 */
void mergeSort(vector<int> &vec_desordedado, int start, int end, int &compares)
{

    // condición base: vector de sólo un elemento:
    if (start < end)
    {
        // dividimos nuestro array por la mitad, para hacer llamadas adecuadas:
        int half = (start + end) / 2;

        // hacemos la primera llamada recursiva (del inicio a la mitad):
        mergeSort(vec_desordedado, start, half, compares);

        // hacemos la primera llamada recursiva (de la mitad al final):
        mergeSort(vec_desordedado, half + 1, end, compares);

        // unimos ambas su partes:
        join(vec_desordedado, start, end, half, compares);
    }
}