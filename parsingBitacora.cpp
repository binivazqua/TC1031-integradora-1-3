#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

/**
 * @struct Registro:
 *  Almacena los campos de cada línea de bitacora.txt, con tipos de datos adecuados.
 *  Cada línea de bitacora.txt tiene el formato:
 *  "Mes Dia hora:minuto:ss ip:puerto mensaje..."
 *
 */
struct Registro
{
    int id;          // número de línea (1-indexado), asignado durante el parsing
    string mesTexto; // mes tal como aparece en el archivo (ej: "Oct")
    int mesIdx;      // 1-12, resuelto UNA vez por registro vía mesAIndice
    int dia;
    int hora;
    int minuto;
    int segundo;
    string ipPuerto; // payload de la bitácora: "IP:PUERTO"
    string mensaje;  // payload de la bitácora: resto de la línea
};

/**
 * Traduce nombre de mes (abreviado, formato "Oct") a índice 1-12.
 * Se resuelve una sola vez por registro durante el parsing (revisar leerBitacora()),
 * nunca dentro del comparador, para no pagar el costo de la búsqueda en el
 * hash map en cada comparación durante el ordenamiento/búsqueda.
 */
unordered_map<string, int> mesAIndice = {
    {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4}, {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8}, {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}};

/**
 * Parsing de bitacora.txt a vector<Registro> con campos tipados adecuadamente.
 * Complejidad: O(n) sobre el número de líneas del archivo, ya que cada línea
 * se procesa con un número constante de operaciones de lectura/parsing
 * (los operadores >> y getline son O(longitud de línea), pero la longitud de
 * línea es acotada, así que el total es O(n) para n líneas).
 */

vector<Registro> leerBitacora(const string &ruta)
{
    /**
     * represents an input file stream for reading
     * characters from files in C++.
     * This alias simplifies the usage of file streams
     * by providing a more convenient name for the
     * underlying template class.
     */

    ifstream bitacora(ruta); // usamos una condicion para evaluar si abrimos correctamente
    if (!bitacora)
    {
        // si el archivo NO PUDO ABRIRSE, mostrar mensaje de error.
        cerr << "No se pudo abrir el archivo de bitacora: " << ruta << endl;
        return {}; // early exit.
    }

    vector<Registro> registros;

    // creamos variables para almacenar los datos que extraigamos del archivo .txt
    string mesTexto;
    int dia;
    string horaStr;
    string ipPuerto;
    string mensaje;
    int idActual = 1;

    // ">>" double shift operator: process individual bits and bit fields (GeeksforGeeks, 2023)
    while (bitacora >> mesTexto >> dia >> horaStr >> ipPuerto)
    {
        // extraemos los datos del archico de bitacora.txt y los almacenamos en variables individuales,
        // creadas en línea 44.

        /**
         * Ejemplo, para un registro como: "Oct 9 10:32:24 423.2.230.77:6166 Failed password for admin"
         * Los operadores obtienen:
         * month = "Oct"; dia = 9; hora = "10:32:24"; ip = "423.2.230.77:6166";
         *
         * Pero, cuando llegamos a una string con espacios, como lo que sigue en el record:
         *  "Failed password for admin", el operador ">>" no puede procesar correctamente la cadena completa,
         *  solo obtendría "Failed", entonces usamos el método getline() para obtener el resto de la línea,
         *  que es el mensaje completo del ataque.
         */

        getline(bitacora, mensaje);

        // si tenemos un mensaje que empieza con un espacio, lo eliminamos para que
        // quede limpio y no tenga espacios al inicio.
        if (!mensaje.empty() && mensaje[0] == ' ')
        {
            mensaje.erase(0, 1); // "removes a specified number of characters from the string, starting at a given position"
        }

        // horaStr viene como "HH:MM:SS", la separamos en sus 3 componentes.
        int hora = 0, minuto = 0, segundo = 0;
        stringstream ss(horaStr);
        string parte;
        getline(ss, parte, ':'); // "getline" lee hasta el primer ':' y lo almacena en "parte"
        hora = stoi(parte);      // stoi() convierte la string a int (geeks for geeks, 2023)
        getline(ss, parte, ':');
        minuto = stoi(parte);
        getline(ss, parte, ':');
        segundo = stoi(parte);

        // ASIGNAMOS INDEX ÚNICO DEL MES AQUÍ
        int mesIdx = mesAIndice.count(mesTexto) ? mesAIndice[mesTexto] : 0;

        registros.push_back({idActual, mesTexto, mesIdx, dia, hora, minuto, segundo, ipPuerto, mensaje});
        idActual++;
    }

    return registros;
}

/**
 * Comparador únicO --> LÓGICA RECURSIVA POR PRACTICIDAD.
 * Compara mes, día, hora, minuto, segundo (campos 0..4) en ese orden,
 * "saltando" al siguiente campo solo cuando el actual empata.
 *  maxCampo es "parametrizable":
 *  maxCampo=5 compara la fecha-hora completa (para el ordenamiento);
 *  maxCampo=2 compara solo mes+día (para la búsqueda de rango por fecha).
 * Complejidad: O(maxCampo), y como maxCampo es a lo más 5 (constante),
 * esto es O(1) --> CTE.
 */
int compararRegistros(const Registro &a, const Registro &b, int campo = 0, int maxCampo = 5)
{
    if (campo == maxCampo)
    {
        return 0;
    }

    int valA, valB;

    // Switch case para mayor limpieza y claridad, en lugar de if-else anidados.
    switch (campo)
    {
    case 0:
        valA = a.mesIdx;
        valB = b.mesIdx;
        break;
    case 1:
        valA = a.dia;
        valB = b.dia;
        break;
    case 2:
        valA = a.hora;
        valB = b.hora;
        break;
    case 3:
        valA = a.minuto;
        valB = b.minuto;
        break;
    case 4:
        valA = a.segundo;
        valB = b.segundo;
        break;
    default:
        return 0;
    }

    if (valA != valB)
    {
        return (valA < valB) ? -1 : 1; // primera cond. ---> devuelve -1; segunda cond. ---> devuelve 1
    }

    return compararRegistros(a, b, campo + 1, maxCampo);
}

// ---------------------------------------------------------------------------
// Merge sort sobre vector<int> idx (índices hacia registros), no sobre
// vector<Registro> directamente.
// Razón: Registro carga varios campos string (mesTexto, ipPuerto, mensaje);
// mover/copiar un Registro completo en cada intercambio del ordenamiento
// cuesta O(longitud total de esas strings). Un int cuesta O(1) copiarlo.
// Ordenando índices, el costo de "mover" durante el sort es O(1) por
// elemento, y el acceso a los campos reales para comparar se hace por
// referencia constante (sin copiar el Registro) dentro de compararRegistros.
//
// Complejidad de mergeSort: O(n log n) en tiempo (siempre, no solo en
// promedio), O(n) de espacio auxiliar para el buffer de mezcla (auxiliar
// sobre vector<int>, no sobre vector<Registro>).
// ---------------------------------------------------------------------------

/**
 *  Merge sort sobre vector<int> idx --> indices que apuntan a registros, no sobre vector<Registro> directamente.
 *  Razón: Registro carga varios campos string (mesTexto, ipPuerto, mensaje);
 *  mover/copiar un Registro completo en cada intercambio del ordenamiento
 * cuesta O(longitud total de esas strings). Un int cuesta O(1) copiarlo.
 * Ordenando índices, el costo de "mover" durante el sort es O(1) por
 * elemento, y el acceso a los campos reales para comparar se hace por
 * referencia constante (sin copiar el Registro) dentro de compararRegistros.
 * Complejidad de mergeSort: O(n log n) en tiempo.
 */
void merge(vector<int> &idx, const vector<Registro> &registros, int inicio, int medio, int fin)
{
    vector<int> temp;
    temp.reserve(fin - inicio + 1);

    int i = inicio, j = medio + 1;
    while (i <= medio && j <= fin)
    {
        if (compararRegistros(registros[idx[i]], registros[idx[j]], 0, 5) <= 0)
        {
            temp.push_back(idx[i++]);
        }
        else
        {
            temp.push_back(idx[j++]);
        }
    }
    while (i <= medio)
    {
        temp.push_back(idx[i++]);
    }
    while (j <= fin)
    {
        temp.push_back(idx[j++]);
    }

    for (int k = 0; k < (int)temp.size(); k++)
    {
        idx[inicio + k] = temp[k];
    }
}

void mergeSort(vector<int> &idx, const vector<Registro> &registros, int inicio, int fin)
{
    if (inicio >= fin)
    {
        return;
    }
    int medio = inicio + (fin - inicio) / 2;
    mergeSort(idx, registros, inicio, medio);
    mergeSort(idx, registros, medio + 1, fin);
    merge(idx, registros, inicio, medio, fin);
}

/**
 *  Búsqueda binaria recursiva sobre idx ya ordenado (por fecha-hora completa,
 * que a su vez ordena por mes+día como prefijo).
 * Compara solo mes+día --> (compararRegistros(..., 0, 2)).
 * Es una variante "lower_bound":
 *      devuelve la posición (dentro de idx, en el rango [izq, der)) del
 *      primer elemento cuyo (mes,día) es >= objetivo, es decir, la
 *      posición MÁS A LA IZQUIERDA en caso de empates.
 * Si no hay ninguno, devuelve der (pos "una pasada del final").
 * Se reutiliza la MISMA función dos veces con el parámetro `estricto`:
 *  - estricto = false -> lower_bound: primer registro con fecha >= objetivo
 *  - estricto = true  -> upper_bound: primer registro con fecha >  objetivo
 * (límite superior / primera fecha después del fin).
 * OJO: podría ser implementada en dos métodos separados,
 *  pero se hace en uno solo para evitar duplicación de código.
 *  Complejidad: O(log n) en tiempo, O(log n) de espacio por la pila de
 * recursión (cada llamada reduce el rango [izq,der) a la mitad).
 *
 * JUSTIFICACIÓN:
 * Si solo usáramos una búsqueda de "igual" (cmp == 0 --> encontrado) sin este matiz,
 * tendríamos dos problemas: 1) Con varios registros del mismo día (que sí existen en la bitácora),
 * un binary search normal devolvería cualquier posición que empate,
 * 2) Para el límite de fin, si te detienes en el primer empate,
 * te falta incluir el resto de los registros de ese mismo día que están a la derecha.
 */

int binarySearch(const vector<int> &idx, const vector<Registro> &registros,
                 const Registro &objetivo, int izq, int der, bool estricto = false)
{
    if (izq >= der)
    {
        return izq;
    }

    int mid = izq + (der - izq) / 2;
    int cmp = compararRegistros(registros[idx[mid]], objetivo, 0, 2);

    bool irDerecha = estricto ? (cmp <= 0) : (cmp < 0); // compara la fecha del registro de en medio (mid) contra la fecha que buscamos:
    if (irDerecha)                                      // ¿todavía no llegué a la frontera?
    {
        return binarySearch(idx, registros, objetivo, mid + 1, der, estricto);
    }
    else
    {
        return binarySearch(idx, registros, objetivo, izq, mid, estricto);
    }
}

void showLog(const Registro &r)
{
    cout << r.id << "\t" << r.mesTexto << " " << r.dia << " "
         << r.hora << ":" << r.minuto << ":" << r.segundo << " "
         << r.ipPuerto << " " << r.mensaje << endl;
}

int main()
{
    vector<Registro> registros = leerBitacora("data/bitacora.txt");
    if (registros.empty())
    {
        return 1;
    }
    cout << "Log de ataques abierto con éxito (" << registros.size() << " registros)" << endl;

    // Ordenamos los ÍNDICES, no los registros (ver comentario de mergeSort).
    vector<int> idx(registros.size());
    for (int i = 0; i < (int)idx.size(); i++)
    {
        idx[i] = i;
    }
    mergeSort(idx, registros, 0, (int)idx.size() - 1);

    // Pedimos el rango de fechas a buscar.
    string mesInicioTexto, mesFinTexto;
    int diaInicio, diaFin;
    cout << "Mes de inicio (ej: Jun): ";
    cin >> mesInicioTexto;
    cout << "Dia de inicio: ";
    cin >> diaInicio;
    cout << "Mes de fin (ej: Oct): ";
    cin >> mesFinTexto;
    cout << "Dia de fin: ";
    cin >> diaFin;

    if (!mesAIndice.count(mesInicioTexto) || !mesAIndice.count(mesFinTexto))
    {
        cerr << "Mes invalido." << endl;
        return 1;
    }

    Registro fechaInicio{};
    fechaInicio.mesIdx = mesAIndice[mesInicioTexto];
    fechaInicio.dia = diaInicio;

    Registro fechaFin{};
    fechaFin.mesIdx = mesAIndice[mesFinTexto];
    fechaFin.dia = diaFin;

    // Límite inferior: primer registro con fecha >= fechaInicio.
    // Si no existe un registro exacto en esa fecha, esto ya cae naturalmente
    // en "el primero posterior", por ser lower_bound.
    int posInicio = binarySearch(idx, registros, fechaInicio, 0, (int)idx.size(), false);

    // Límite superior: primer registro con fecha > fechaFin (upper_bound),
    // usando la misma función con estricto = true.
    int posFin = binarySearch(idx, registros, fechaFin, 0, (int)idx.size(), true);

    if (posInicio >= posFin)
    {
        cout << "No hay registros en el rango solicitado." << endl;
        return 0;
    }

    cout << "Registros entre " << mesInicioTexto << " " << diaInicio
         << " y " << mesFinTexto << " " << diaFin << ":" << endl;
    for (int pos = posInicio; pos < posFin; pos++)
    {
        showLog(registros[idx[pos]]);
    }

    return 0;
}
