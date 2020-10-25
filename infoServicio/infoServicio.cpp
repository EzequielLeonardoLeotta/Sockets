#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

typedef unsigned char BYTE;

using namespace std;

// Declaraciones
vector<BYTE> readFile(const char* filename);
void escribirAlArchivo(string texto);
void leerDelArchivo();

int main() {

    //escribirAlArchivo("Mar del plata;Tarde;21122020");
    leerDelArchivo();

    return 0;
}

// Implementaciones
vector<BYTE> readFile(const char* filename) {
    // Abro el archivo:
    streampos fileSize;
    ifstream file(filename, std::ios::binary);

    // consigue su tamaño
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Lee la data del archivo:
    vector<BYTE> fileData(fileSize);
    file.read((char*)&fileData[0], fileSize);
    return fileData;
}

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

void leerDelArchivo() {
    char buffer[100];

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

    int largo = strnlen_s(buffer, sizeof(buffer));

    for (int i = 0; i < largo; i++) {
        cout << buffer[i];
    }
}