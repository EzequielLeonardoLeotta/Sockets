#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include<sstream>

using namespace std;

// Declaraciones
void escribirAlArchivo(string &texto);
bool findTexto(char* texto);
void traerServicio();
void traerServicio(string &filtro);
void mostrarAsientos(string &asientos);
void mostrarServicio(string &servicio);
void menu();
void alta();
void mostrarTodo();
void mostrarFiltrado();
bool validarServicio(string &filtro);

int main() {
	// Caracteres en español
	setlocale(LC_ALL, "Spanish");
	
	menu();

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

void menu() {
	int opcion;
	bool conectado = true;

	while (conectado) {
		opcion = 0;

		while (opcion < 1 || opcion>4) {
			system("cls");
			cout << "Menú principal (infoServicios):" << endl << endl
				<< "1: Dar de alta un servicio" << endl
				<< "2: Ver todos los servicios" << endl
				<< "3: Ver servicios filtrados por usuario" << endl
				<< "4: Salir" << endl << endl
				<< "Elija una opción: ";
			cin >> opcion;
		}

		switch (opcion) {
			case 1:
				alta();
				break;
			case 2:
				mostrarTodo();
				break;
			case 3:
				mostrarFiltrado();
				break;
			case 4:
				conectado = false;
				break;
		}
	}
}

void alta() {
	system("cls");
	cout << "Alta de servicio (infoServicios.bin)" << endl;

	// Pedir origen
	int origen = 0;
	while (origen > 2 || origen < 1) {
		cout << endl << "Origen: " << endl
			<< "1- Mar del Plata" << endl
			<< "2- Buenos Aires" << endl
			<< endl << "Ingrese una opción: ";
		cin >> origen;
	}

	// Pedir turno
	int turno = 0;
	while (turno > 3 || turno < 1) {
		cout << endl << "Turno: " << endl
			<< "1- Mañana" << endl
			<< "2- Tarde" << endl
			<< "3- Noche" << endl
			<< endl << "Ingrese una opción: ";
		cin >> turno;
	}

	// Pedir fecha
	int dia=0;
	int mes=0;
	int anio=0;
	cout << endl << "Fecha: " << endl;
	while (dia < 1 || dia>31) {
		cout << "Ingrese el día (ej: 11): ";
		cin >> dia;
	}
	while (mes < 1 || mes>12) {
		cout << "Ingrese el mes (ej: Enero sería 1): ";
		cin >> mes;
	}
	while (anio < 1900 || anio>2500) {
		cout << "Ingrese el año (ej: 2020): ";
		cin >> anio;
	}

	// Preparar datos en string
	stringstream servicioS;
	// Origen
	switch (origen) {
		case 1:
			servicioS << "m;";
			break;
		case 2:
			servicioS << "b;";
			break;
	}
	// Turno
	switch (turno) {
	case 1:
		servicioS << "m;";
		break;
	case 2:
		servicioS << "t;";
		break;
	case 3:
		servicioS << "n;";
		break;
	}
	// Fecha
	stringstream dato;
	dato << std::setfill('0') << std::setw(2) << dia 
		<< std::setfill('0') << std::setw(2) << mes
		<< anio << ";";
	string fecha;
	dato >> fecha;
	servicioS << fecha;
	string asientos = "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo;";
	servicioS << asientos;
	string servicio;
	servicioS >> servicio;
	
	// Mostrar el servicio dado de alta
	system("cls");
	if (!validarServicio(servicio)) {
		// Agregar el servicio al archivo
		escribirAlArchivo(servicio);
		cout << "Se ha dado de alta el siguiente servicio: " << endl << endl;
		traerServicio(servicio);
	}
	else 
		cout << endl << "Error: ya existe un servicio con los datos ingresados, intentelo nuevamente con otros valores" << endl << endl;
	
	system("pause");
}

void mostrarTodo() {
	// Traer todos los servicios
	system("cls");
	cout << "Todos los servicios existentes en el sistema (infoServicios.bin):" << endl << endl;
	
	traerServicio();

	cout << endl;
	system("pause");
}

void mostrarFiltrado() {
	system("cls");
	cout << "Ver servicios en base a su elección: " << endl;

	// Pedir origen
	int origen = 3;
	while (origen > 2 || origen < 0) {
		cout << endl << "Ingrese la opción del origen que deben tener el/los servicio/s a mostrar: " << endl
			<< "1- Mar del Plata" << endl
			<< "2- Buenos Aires" << endl
			<< "IMPORTANTE!: Si desea no filtrar por origen ingrese 0" << endl
			<< endl << "Ingrese una opción: ";
		cin >> origen;
	}

	// Pedir turno
	int turno = 4;
	while (turno > 3 || turno < 0) {
		cout << endl << "Ingrese la opción del turno que deben tener el/los servicio/s a mostrar: " << endl
			<< "1- Mañana" << endl
			<< "2- Tarde" << endl
			<< "3- Noche" << endl
			<< "IMPORTANTE!: Si desea no filtrar por turno ingrese 0" << endl
			<< endl << "Ingrese una opción: ";
		cin >> turno;
	}

	// Pedir fecha
	int dia = 32;
	int mes = 0;
	int anio = 0;
	cout << endl << "Ingrese fecha del turno que deben tener el/los servicio/s a mostrar." << endl
		<< "IMPORTANTE!: Si desea no filtrar por fecha ingrese 0" << endl;
	bool ignorarFecha = false;
	while (dia < 0 || dia>31) {
		cout << "Ingrese el día (ej: 11): ";
		cin >> dia;
		if (dia == 0)
			ignorarFecha = true;
	}
	if (!ignorarFecha) {
		while (mes < 1 || mes>12) {
			cout << "Ingrese el mes (ej: Enero sería 1): ";
			cin >> mes;
		}
		while (anio < 1900 || anio>2500) {
			cout << "Ingrese el año (ej: 2020): ";
			cin >> anio;
		}
	}
	
	// Preparar datos en string
	stringstream servicioS;
	// Origen
	switch (origen) {
	case 0:
		servicioS << ";";
		break;
	case 1:
		servicioS << "m;";
		break;
	case 2:
		servicioS << "b;";
		break;
	}
	// Turno
	switch (turno) {
	case 0:
		servicioS << ";";
		break;
	case 1:
		servicioS << "m;";
		break;
	case 2:
		servicioS << "t;";
		break;
	case 3:
		servicioS << "n;";
		break;
	}
	// Fecha
	string filtro;
	if (dia != 0) {
		stringstream dato;
		dato << std::setfill('0') << std::setw(2) << dia
			<< std::setfill('0') << std::setw(2) << mes
			<< anio << ";";
		string fecha;
		dato >> fecha;
		servicioS << fecha;
	} else
		servicioS << ";";
	servicioS >> filtro;

	// Traer servicios filtrados por string
	system("cls");
	cout << "Servicios encontrados en base su filtro: " << endl << endl;
	traerServicio(filtro);

	cout << endl;
	system("pause");
}

bool validarServicio(string &filtro) {
	bool resultado = false;
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
				if (origen == origenFiltro && turno == turnoFiltro && fecha == fechaFiltro)
					resultado = true;

			}
			else
				finArchivo = true;
		}
		archivo.close();
	}

	return resultado;
}