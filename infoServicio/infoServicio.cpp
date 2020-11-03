#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>


using namespace std;

// Declaraciones
void escribirAlArchivo(string &texto);
bool findTexto(char* texto);

void traerServicio();
void traerServicio(string &filtro);
void mostrarAsientos(string &asientos);
void mostrarServicio(string &servicio);

int main() {
	// Caracteres en español
	setlocale(LC_ALL, "Spanish");
	
	// Escribir registro al archivo
	/*string texto = "m;m;11122020;oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo;";
	escribirAlArchivo(texto);*/

	// Traer todos los servicios
	//traerServicio();

	// Traer servicios filtrados string
	string filtro = ";;;";
	traerServicio(filtro);

	// traerServicio()

	return 0;
}

// Implementaciones
void escribirAlArchivo(string &texto) {
	ofstream archivo;
	archivo.open("../Server/infoServicios.bin", ofstream::binary | ofstream::app);

	if (archivo) {
		archivo.write(texto.c_str(), texto.size());
		archivo.close();
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
		streamoff longitud1 = length;
		size_t longitud2 = strlen(texto);
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
	return false;
}

void traerServicio() {
	ifstream archivo("../Server/infoServicios.bin", ifstream::binary);
	if (archivo) {
		// Leer un registro del archivo
		bool finArchivo = false;
		streamoff largoRegistro = 75;
		char registro[75];
		
		// Mientras no sea el fin de archivo mostrar el registro
		while (!finArchivo) {
			archivo.get(registro, largoRegistro);
			if (archivo) {
				// Pasar el registro de char[] a string
				string servicio(registro);

				// Mostrar servicio
				mostrarServicio(servicio);
			}	
			else
				finArchivo = true;
		}
		archivo.close();
	} else
		cout << endl << "Error: no se pudo abrir el archivo infoServicios.bin" << endl;
}

void traerServicio(string &filtro) {
	ifstream archivo("../Server/infoServicios.bin", ifstream::binary);
	if (archivo) {
		// Leer un registro del archivo
		bool finArchivo = false;
		streamoff largoRegistro = 75;
		char registro[75];

		string origenFiltro, turnoFiltro, fechaFiltro, origen, turno, fecha;

		string delimitador = ";";

		// Partir el filtro en campos
		origenFiltro = filtro.substr(0, filtro.find(";"));
		filtro.erase(0, filtro.find(delimitador) + delimitador.length());
		turnoFiltro = filtro.substr(0, filtro.find(";"));
		filtro.erase(0, filtro.find(delimitador) + delimitador.length());
		fechaFiltro = filtro.substr(0, filtro.find(";"));

		// Mientras no sea el fin de archivo mostrar el registro
		while (!finArchivo) {
			archivo.get(registro, largoRegistro);
			if (archivo) {
				// Pasar el registro de char[] a string
				string servicio(registro);
				string temporal(registro);

				// Partir el registro en campos
				origen = temporal.substr(0, temporal.find(";"));
				temporal.erase(0, temporal.find(delimitador) + delimitador.length());
				turno = temporal.substr(0, temporal.find(";"));
				temporal.erase(0, temporal.find(delimitador) + delimitador.length());
				fecha = temporal.substr(0, temporal.find(";"));

				// Comparar strings
				if (origen == origenFiltro || origenFiltro == ""){
					if (turno == turnoFiltro || turnoFiltro == "") {
						if (fecha == fechaFiltro || fechaFiltro == "") {
							// Mostrar servicio
							mostrarServicio(servicio);
						}
					}
				}

			}
			else
				finArchivo = true;
		}
		archivo.close();
	}
	else
		std::cout << endl << "Error: no se pudo abrir el archivo infoServicios.bin" << endl;
}

void mostrarAsientos(string &asientos) {
	/*cout << "Tamaño string: " << asientos.size() << endl;
	cout << "String: " << asientos << endl;*/

	// Columnas
	cout << "Asientos:" << endl
		<< "   | ";
	for (size_t i = 1; i <= 20; i++){
		cout << setw(2) << i << " ";
	}
	cout << endl;

	// Separador
	cout << "-----------------------------------------------------------------" << endl;

	// Fila A
	cout << " A | ";
	for (size_t i = 0; i < 20; i++) {
		cout << setw(2) << asientos[i] << " ";
	}
	cout << endl;

	// Fila B
	cout << " B | ";
	for (size_t i = 20; i < 40; i++) {
		cout << setw(2) << asientos[i] << " ";
	}
	cout << endl;

	// Separador "Pasillo"
	cout << "   | " << "============================================================" << endl;

	// Fila C
	cout << " C | ";
	for (size_t i = 40; i < 60; i++) {
		cout << setw(2) << asientos[i] << " ";
	}
	cout << endl << endl;
}

void mostrarServicio(string &servicio) {
	// Partir el registro en campos
	string delimitador = ";";
	string origen = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string turno = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string fecha = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string asientos = servicio.substr(0, servicio.find(";"));

	// Dar formato al registro para mostrar por consola
	// Origen
	if (origen == "m")
		origen = "Mar del Plata";
	else
		origen = "Buenos Aires";
	// Turno
	if (turno == "m")
		turno = "Mañana";
	else if (turno == "t")
		turno = "Tarde";
	else
		turno = "Noche";
	// Fecha
	string separador = "/";
	fecha.insert(2, separador);
	fecha.insert(5, separador);

	cout << "Origen: " << origen << endl
		<< "Turno: " << turno << endl
		<< "Fecha: " << fecha << endl;

	// Mostrar asientos
	mostrarAsientos(asientos);
}
