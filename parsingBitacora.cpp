#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

struct Ataque
{
    string mes;
    int dia;
    string hora;
    string ip;
    string mensaje;
};

int main()
{

    ifstream bitacora("data/bitacora.txt"); // abrimos el archivo en modo lectura

    /**
     * represents an input file stream for reading
     * characters from files in C++.
     * This alias simplifies the usage of file streams
     * by providing a more convenient name for the
     * underlying template class.
     */

    if (!bitacora) // usamos una condicion para evaluar si abrimos correctamente
    {
        // si el archivo NO PUDO ABRIRSE, mostrar mensaje de error.
        cerr << "No se pudo abrir el archivo de bitacora.txt";
        return 1; // early exit.
    }
    else
    {
        cout << "Log de ataques abierto con éxito" << endl;
    }

    vector<Ataque> bit_vectorized;

    // creamos variables para almacenar los datos que extraigamos del archivo .txt
    string month;
    int dia;
    string hora;
    string ip;
    string mensaje;

    // ">>" double shift operator: process individual bits and bit fields (GeeksforGeeks, 2023)
    while (bitacora >> month >> dia >> hora >> ip)
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

        getline(bitacora, mensaje); // lee "bitacora.txt" desde la posición actual hasta el final de la línea, y almacena el resultado en  la var mensaje.

        // si tenemos un mensaje que empieza con un espacio, lo eliminamos para que
        // quede limpio y no tenga espacios al inicio.
        if (!mensaje.empty() && mensaje[0] == ' ')
        {
            mensaje.erase(0, 1); // "removes a specified number of characters from the string, starting at a given position"
        }

        bit_vectorized.push_back({month, dia, hora, ip, mensaje}); // finalmente, insertamos cada field parseado en nuestro vector de structs del tipo Ataque
    }

    // pruebas básicas de lectura / parsing para luego crear métodos más sofisticados.
    if (!bit_vectorized.empty() && bit_vectorized[0].mes == "Oct")
    {
        cout << "El primer registro es de Octubre" << endl;
    }
    else
    {
        cout << "El primer registro NO es de Octubre" << endl;
    }

    return 0;
}