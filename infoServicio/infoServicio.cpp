#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Declaraciones
void escribirAlArchivo(string texto);
string leerDelArchivo();

int main() {

    escribirAlArchivo("Mardelplata;Tarde;21122020");
    cout << leerDelArchivo();

    return 0;
}

// Implementaciones
void escribirAlArchivo(string texto){
    char buffer[100];
    strcpy_s(buffer, texto.c_str());

    fstream f;

    f.open("servicios.bin", ios::out | ios::binary);

    if (f) {
        f.write(buffer, sizeof(buffer));
        f.close();
    }
    else {
        cout << "Error al abrir el archivo para escribir" << endl;
        exit(1);
    }
}

string leerDelArchivo() {
    char buffer[100];
    string respuesta = "";

    fstream f;

    f.open("servicios.bin", ios::in | ios::binary);

    if (f) {
        f.read(buffer, sizeof(buffer));
        f.close();
    }
    else {
        cout << "Error al abrir el archivo para leer" << endl;
        exit(2);
    }

    const size_t largo = strnlen_s(buffer, sizeof(buffer));

    for (int i = 0; i < largo; i++) {
        respuesta = respuesta + buffer[i];
    }

    return respuesta;
}