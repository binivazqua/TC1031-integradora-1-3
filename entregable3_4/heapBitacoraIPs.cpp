/**
 * TC1031 - Act 1.3 / Entregable 3_4
 * Por: Biniza Verónica Vázquez Moreno | A01737294
 *
 * Reutiliza la lógica de parsing de bitacora.txt de entregable1/ordYBusqBitacora.cpp
 * (mismo formato de línea: "Mes Dia hora:minuto:ss ip:puerto mensaje...").
 * A partir de ahí resuelve los 3 puntos de la actividad:
 *
 *  1. Todas las IPs distintas de bitacora.txt se insertan en un BST simple
 *     (sin repetición), contando cuántas veces aparece cada IP exacta
 *     (NodoBST, insertarIP).
 *  2. Se recorre el BST y, para las IPs que comparten el primer número
 *     (ej. "423.2.230.77" y "423.15.9.2" comparten el "423"), se suman sus
 *     conteos en un arreglo de grupos (GrupoIP). Con esos grupos se
 *     construye un MAX HEAP (clase MaxHeap), ordenado por el conteo total
 *     de accesos de cada grupo.
 *  3. Se extraen del heap los 10 conteos mayores, en orden descendente,
 *     junto con las IPs (y su conteo individual) que inician con el primer
 *     número correspondiente a cada conteo.
 *
 * Todo implementado con ADTs simples: struct + punteros para el BST,
 * y vector (arreglo dinámico) para el heap. Sin estructuras auxiliares
 * adicionales (nada de unordered_map/set para la lógica nueva): la
 * agrupación por primer número se hace con búsqueda lineal sobre el
 * propio arreglo de grupos, igual que buscar en un BST.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Punto 1: BST simple de IPs distintas, contando intentos de acceso.
// ---------------------------------------------------------------------------

/**
 * @struct NodoBST
 * Nodo de un BST clásico: clave = IP completa (como string, ej. "423.2.230.77"),
 * dato = conteo de cuántas veces apareció esa IP exacta en la bitácora.
 * La comparación de claves usa el orden lexicográfico de string (operator<),
 * suficiente para garantizar un BST válido y sin repetición: no necesitamos
 * un orden numérico de IPs, solo que cada IP exacta caiga en un único nodo.
 */
struct NodoBST
{
    string ip;
    int conteo;
    NodoBST *izq;
    NodoBST *der;
};

/**
 * Inserta una IP en el BST. Si ya existe (misma IP exacta), solo incrementa
 * su conteo en vez de crear un nodo repetido ("sin repetición" del punto 1).
 * Lógica de BST de libro de texto: recursiva, sin balanceo extra.
 * Complejidad: O(h) por inserción, h = altura del árbol
 * (O(log n) en el caso promedio, O(n) en el peor caso para n IPs distintas).
 */
NodoBST *insertarIP(NodoBST *raiz, const string &ip)
{
    if (raiz == nullptr)
    {
        return new NodoBST{ip, 1, nullptr, nullptr};
    }

    if (ip == raiz->ip)
    {
        raiz->conteo++;
    }
    else if (ip < raiz->ip)
    {
        raiz->izq = insertarIP(raiz->izq, ip);
    }
    else
    {
        raiz->der = insertarIP(raiz->der, ip);
    }

    return raiz;
}

/**
 * Libera la memoria del BST (postorden: primero hijos, luego el nodo).
 * Necesario porque insertarIP usa new explícito; sin esto cada nodo quedaría
 * reservado en el heap de memoria del programa después de usar el árbol.
 * El orden postorden es obligatorio aquí (no cualquier recorrido sirve):
 * hay que liberar ambos hijos antes del nodo actual, porque una vez hecho
 * `delete raiz` ya no se puede leer `raiz->izq`/`raiz->der` para seguir bajando.
 * Complejidad: O(n), n = número de nodos (IPs distintas) del BST, ya que
 * visita cada nodo exactamente una vez.
 */
void liberarBST(NodoBST *raiz)
{
    if (raiz == nullptr)
    {
        return;
    }
    liberarBST(raiz->izq);
    liberarBST(raiz->der);
    delete raiz;
}

/**
 * Lee bitacora.txt y arma el BST de IPs distintas con su conteo de accesos.
 * Reutiliza el mismo esquema de parsing de leerBitacora() en
 * entregable1/ordYBusqBitacora.cpp: los primeros 4 campos se leen con ">>"
 * y el resto de la línea (el mensaje del ataque) se descarta con getline,
 * ya que para esta actividad solo nos interesa el campo ip:puerto.
 * Complejidad: O(n) líneas leídas, cada una con un número constante de
 * operaciones de parsing e inserción O(log n) en el BST en promedio
 * -> O(n log n) total en el caso promedio.
 */
NodoBST *leerIPsDeBitacora(const string &ruta)
{
    ifstream bitacora(ruta);
    if (!bitacora)
    {
        cerr << "No se pudo abrir el archivo de bitacora: " << ruta << endl;
        return nullptr;
    }

    NodoBST *raiz = nullptr;

    string mesTexto, horaStr, ipPuerto, mensaje;
    int dia;

    while (bitacora >> mesTexto >> dia >> horaStr >> ipPuerto)
    {
        getline(bitacora, mensaje); // descartamos el mensaje: no se usa en esta actividad

        // ipPuerto viene como "IP:PUERTO" (ej. "423.2.230.77:6166"); nos
        // quedamos solo con la IP, antes de los ":".
        string ip = ipPuerto.substr(0, ipPuerto.find(':'));
        raiz = insertarIP(raiz, ip);
    }

    return raiz;
}

// ---------------------------------------------------------------------------
// Punto 2: agrupar por primer número de la IP y construir el MAX HEAP.
// ---------------------------------------------------------------------------

/**
 * @struct GrupoIP
 * Un grupo = todas las IPs distintas de la bitácora que comparten el mismo
 * primer número (ej. primerNumero = 423 agrupa "423.2.230.77" y
 * "423.15.9.2"). conteoTotal es la suma de los conteos individuales (del
 * BST) de todas esas IPs: el total de intentos de acceso para ese primer
 * número, que es lo que pide el punto 2.
 */
struct GrupoIP
{
    int primerNumero;
    int conteoTotal;
    vector<pair<string, int>> ips; // (ip completa, conteo individual de esa IP)
};

/**
 * Extrae el primer número de una IP en formato "A.B.C.D" (el valor antes
 * del primer '.'). Se usa substr+find en vez de un parseo manual carácter
 * por carácter porque la IP ya viene validada por el formato fijo de la
 * bitácora: no hace falta una máquina de estados para un split tan simple.
 * Complejidad: O(1), ya que la longitud de "A" (como mucho unos pocos
 * dígitos) está acotada por una constante, sin importar el tamaño de la
 * bitácora.
 */
int primerNumeroDeIP(const string &ip)
{
    return stoi(ip.substr(0, ip.find('.')));
}

/**
 * Recorre el BST (inorden, aunque el orden de recorrido no importa aquí)
 * y, para cada IP distinta que encuentra, la acumula en el grupo de su
 * primer número dentro de `grupos`. Buscamos el grupo con una búsqueda
 * lineal sobre el arreglo (igual que se buscaría una clave bajando por un
 * BST): no se usa ninguna tabla hash adicional, solo el arreglo de grupos.
 * Complejidad: O(n * g), n = IPs distintas en el BST, g = número de grupos
 * distintos encontrados hasta el momento (g es pequeño en la práctica,
 * acotado por cuántos primeros números distintos aparecen en la bitácora).
 */
void agruparPorPrimerNumero(NodoBST *raiz, vector<GrupoIP> &grupos)
{
    if (raiz == nullptr)
    {
        return;
    }

    agruparPorPrimerNumero(raiz->izq, grupos);

    int primerNumero = primerNumeroDeIP(raiz->ip);

    int posicion = -1;
    for (int i = 0; i < (int)grupos.size(); i++)
    {
        if (grupos[i].primerNumero == primerNumero)
        {
            posicion = i;
            break;
        }
    }

    if (posicion == -1)
    {
        grupos.push_back({primerNumero, raiz->conteo, {{raiz->ip, raiz->conteo}}});
    }
    else
    {
        grupos[posicion].conteoTotal += raiz->conteo;
        grupos[posicion].ips.push_back({raiz->ip, raiz->conteo});
    }

    agruparPorPrimerNumero(raiz->der, grupos);
}

/**
 * @class MaxHeap
 * Max heap de GrupoIP, ordenado por conteoTotal, implementado sobre un
 * arreglo dinámico (vector<GrupoIP>) con la lógica clásica de heap binario
 * completo: el padre de un nodo en la posición i está en (i-1)/2, y sus
 * hijos en 2i+1 / 2i+2. Es la misma idea de comparar un nodo contra sus
 * hijos/padre que en un BST se compara un nodo contra el que se inserta,
 * solo que aquí el "árbol" es implícito dentro del arreglo en vez de usar
 * punteros.
 * Único invariante que mantiene: cada nodo tiene conteoTotal >= que el de
 * sus hijos. No se agregan métodos que no sirvan directamente a insertar
 * o extraer el máximo.
 */
class MaxHeap
{
private:
    vector<GrupoIP> datos;

    // Aritmética de índices del heap implícito en el arreglo: para el nodo
    // en la posición i, su padre e hijos se calculan, no se guardan con
    // punteros, porque un heap binario completo siempre llena el arreglo
    // de izquierda a derecha sin huecos (a diferencia de un BST, donde la
    // forma del árbol es arbitraria y sí necesita punteros explícitos).
    // Complejidad: O(1) cada una, son solo operaciones aritméticas.
    int padre(int i) const { return (i - 1) / 2; }
    int hijoIzq(int i) const { return 2 * i + 1; }
    int hijoDer(int i) const { return 2 * i + 2; }

    /**
     * Sube el elemento en la posición i mientras sea mayor que su padre.
     * Se usa al insertar: el nuevo elemento entra al final del arreglo
     * (única posición libre de un heap-arreglo) y "burbujea" hacia arriba.
     * Complejidad: O(log n), n = elementos en el heap.
     */
    void siftUp(int i)
    {
        while (i > 0 && datos[padre(i)].conteoTotal < datos[i].conteoTotal)
        {
            swap(datos[i], datos[padre(i)]);
            i = padre(i);
        }
    }

    /**
     * Baja el elemento en la posición i mientras alguno de sus hijos sea
     * mayor que él. Se usa al extraer el máximo: la raíz se reemplaza por
     * el último elemento del arreglo, que luego "se hunde" hasta su lugar.
     * Complejidad: O(log n).
     */
    void siftDown(int i)
    {
        int n = (int)datos.size();
        while (true)
        {
            int mayor = i;
            int izq = hijoIzq(i);
            int der = hijoDer(i);

            if (izq < n && datos[izq].conteoTotal > datos[mayor].conteoTotal)
            {
                mayor = izq;
            }
            if (der < n && datos[der].conteoTotal > datos[mayor].conteoTotal)
            {
                mayor = der;
            }
            if (mayor == i)
            {
                break;
            }

            swap(datos[i], datos[mayor]);
            i = mayor;
        }
    }

public:
    bool vacio() const { return datos.empty(); }
    int tamano() const { return (int)datos.size(); }

    /** Inserta un grupo y restaura el invariante de heap. O(log n). */
    void insertar(const GrupoIP &g)
    {
        datos.push_back(g);
        siftUp((int)datos.size() - 1);
    }

    /**
     * Extrae y devuelve el grupo con el mayor conteoTotal (la raíz).
     * Complejidad: O(log n).
     */
    GrupoIP extraerMax()
    {
        GrupoIP maximo = datos[0];
        datos[0] = datos.back();
        datos.pop_back();
        if (!datos.empty())
        {
            siftDown(0);
        }
        return maximo;
    }
};

// ---------------------------------------------------------------------------
// Punto 3: top 10 conteos (descendente) + IPs de cada primer número.
// ---------------------------------------------------------------------------

/**
 * Imprime un lugar del ranking: el primer número, su conteo total de
 * accesos, y la lista completa de IPs (con su conteo individual) que
 * comparten ese primer número. Se separa de main() porque es la única
 * parte de "mostrar" que se repite 10 veces; no hay más lógica detrás
 * que justifique dividirla en piezas más pequeñas.
 * Complejidad: O(k), k = IPs distintas dentro del grupo g (recorre
 * g.ips una sola vez).
 */
void mostrarGrupo(int lugar, const GrupoIP &g)
{
    cout << lugar << ". Primer numero: " << g.primerNumero
         << " | Total de accesos: " << g.conteoTotal << endl;
    cout << "   IPs: ";
    for (int i = 0; i < (int)g.ips.size(); i++)
    {
        cout << g.ips[i].first << " (" << g.ips[i].second << " intentos)";
        if (i + 1 < (int)g.ips.size())
        {
            cout << ", ";
        }
    }
    cout << endl;
}

/**
 * Orquesta los 3 puntos de la actividad en orden: lee la bitácora al BST,
 * agrupa por primer número y construye el heap, y extrae/imprime el top 10.
 * Complejidad total: O(n log n) en el caso promedio, dominada por las n
 * inserciones en el BST (leerIPsDeBitacora), n = líneas de la bitácora;
 * el resto de los pasos trabaja sobre g = número de grupos (g << n), por
 * lo que no cambian el orden de crecimiento: agrupar es O(g^2) en el peor
 * caso, construir el heap es O(g log g), y extraer el top 10 es O(log g).
 */
int main()
{
    // Punto 1: BST de IPs distintas con su conteo de accesos.
    NodoBST *raiz = leerIPsDeBitacora("data/bitacora.txt");
    if (raiz == nullptr)
    {
        return 1;
    }

    // Punto 2: agrupamos por primer número y construimos el MAX HEAP.
    vector<GrupoIP> grupos;
    agruparPorPrimerNumero(raiz, grupos);

    MaxHeap heap;
    for (const GrupoIP &g : grupos)
    {
        heap.insertar(g);
    }

    cout << "Bitacora procesada: " << grupos.size()
         << " grupos distintos de IPs (por primer numero)." << endl
         << endl;

    // Punto 3: top 10 conteos en orden descendente, con las IPs de cada grupo.
    int top = min(10, heap.tamano());
    cout << "Top " << top << " conteos de accesos por primer numero de IP:" << endl;
    for (int lugar = 1; lugar <= top; lugar++)
    {
        GrupoIP g = heap.extraerMax();
        mostrarGrupo(lugar, g);
    }

    liberarBST(raiz);
    return 0;
}
