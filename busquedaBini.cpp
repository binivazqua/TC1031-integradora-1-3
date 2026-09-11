#include <iostream>
#include <random>
#include <vector>

using namespace std;

// =================== BINARY SEARCH ====================== //
/**
 * Este algortimo se basa en el paradigma de "divide y vencerás",
 * aprovechando que busca dentro de un arreglo ordenado y es posible
 * acotar la ventana de búsqueda, dependiendo del valor a encontrar.
 * Si el elemento es menor a la mitad:
 *  la ventana se recorre hacia la izquierda (inferior).
 * Si el elemento es mayor a la mitad:
 *  la ventana se recorre hacia la derecha (superior).
 * @param vec_ordenado: el vector creado por el usuario en main, por cualquier método (por referencia.)
 * @param key el elemento a encontrar, por referencia, y cte, pues no lo modificaremos.
 * @param compares: número de comparaciones realizadas para encontrar "key" (por referencia)
 *
 * Complejidad Temporal Asintótica:
 * Dado que se trata de un algoritmo logarítmico, con un loop que divide
 * la ventana de búsqueda a la mitad en cada iteración, este algoritmo alcanza un orden de O(log2 n).
 */
int binarySearch(vector<int> &vec_ordenado, const int &key, int &compares)
{

    // creamos variables de control:
    // índices de búsqueda.

    int size = vec_ordenado.size();
    int bottom = 0, half = 0, top = size - 1; // IMPORTANTE EL TOP.
    while (bottom <= top)
    {
        half = (bottom + top) / 2;
        // MEJOR CASO: si nuestro elemento a buscar está exactamente en posición media:

        // actualizamos el número de comparaciones:
        compares++;
        if (key == vec_ordenado[half])
        {
            return half;
        }
        else if (key < vec_ordenado[half])
        {
            top = half - 1;
        }
        else
        {
            bottom = half + 1;
        }
    }

    return -1;
}
