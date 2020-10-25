#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Declaraciones
void escribirAlArchivo(string texto);
void leerDelArchivo();

int main() {
    //escribirAlArchivo("BuenosAires;Tarde;21122020;");
    leerDelArchivo();

    return 0;
}

// Implementaciones
void escribirAlArchivo(string texto) {
    fstream f;
    f.open("servicios.bin", ios::app | ios::binary);

    if (f) {
        size_t largo = strnlen(texto.c_str(), sizeof(texto));
        for (int i = 0; i < largo; i++) {
            f.put(texto[i]);
        }
        f.close();
    }
    else {
        cout << "Error al abrir el archivo para escribir" << endl;
        exit(1);
    }
}

void leerDelArchivo() {
    ifstream archivo("servicios.bin", ifstream::binary);
    if (archivo) {
        // get length of file:
        archivo.seekg(0, archivo.end);
        streamoff length = archivo.tellg();
        archivo.seekg(0, archivo.beg);

        char* buffer = new char[length]; // buffer = toda la info del archivo

        // read data as a block:
        archivo.read(buffer, length);

        if (archivo) {
            // Procesar buffer
            for (int i = 0; i < length; i++)
                cout << buffer[i];
        }
        else
            cout << "Error al leer el archivo para leer" << endl;
        archivo.close();

        // ...buffer contains the entire file...
        delete[] buffer;
    }

}