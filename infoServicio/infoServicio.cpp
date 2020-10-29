#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Declaraciones
void escribirAlArchivo(string texto);
void leerDelArchivo();
bool findTexto(char* texto);
int main() {
    //escribirAlArchivo("BuenosAires;Tarde;21122020;");
    leerDelArchivo();

    return 0;
}

// Implementaciones
void escribirAlArchivo(string texto) {
    fstream f;
    f.open("../Server/infoServicios.bin", ios::app | ios::binary);

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
    ifstream archivo("../Server/infoServicios.bin", ifstream::binary);
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

bool findTexto(char* texto) {
    ifstream archivo("../Server/infoServicios.bin", ifstream::binary);
    if (archivo) {
        // get length of file:
        archivo.seekg(0, archivo.end);
        streamoff length = archivo.tellg();
        archivo.seekg(0, archivo.beg);

        char* buffer = new char[length]; // buffer = toda la info del archivo
        int longitud1 = length;
        int longitud2 = strlen(texto);
        // read data as a block:
        string data = "";
        char c;
        archivo.read(buffer, length);
        if (archivo) {
            // Procesar buffer
            c = texto[0];
            for (int i = 0; i < length; i++) {
                if (buffer[i] == c) {
                    if (strncmp(&buffer[i], texto, longitud2) == 0) {
                        return true;
                    }
                }
            }
        }
        else
            cout << "Error al leer el archivo para leer" << endl;
        archivo.close();
        // ...buffer contains the entire file...
        delete[] buffer;
        return false;
    }

}