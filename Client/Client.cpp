#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Declaraciones
void altaServicio(SOCKET& sock);
void gestionPasaje(SOCKET& sock);
void verRegistro(SOCKET& sock);
void menu(SOCKET& sock);
int enviarMensaje(string& mensaje, SOCKET& sock);
string recibirMensaje(SOCKET& sock);
void mostrarAsientos(string& asientos);
void mostrarServicio(string servicio);

// Variables globales
string usuario;

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "es_AR.UTF8");

	// Pedir IP y puerto. Intentar conexion a esos datos
	int puerto = 0;
	string ip = "";

	while (true) {
		system("cls");
		cout << "Ingrese los siguientes datos para intentar conectarse al sistema: " << endl << endl;
		cout << "Direccion IP: ";
		cin >> ip;
		cout << "Puerto: ";
		cin >> puerto;

		// Iniciar Winsock
		WSAData data;
		WORD ver = MAKEWORD(2, 2);
		int wsResult = WSAStartup(ver, &data);
		if (wsResult != 0)
		{
			cerr << "No se pudo arrancar Winsock" << endl << endl;
			system("pause");
			exit(1);
		}

		// Crear socket
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket" << endl << endl;
			system("pause");
			WSACleanup();

			exit(1);
		}

		// Hint de estructura de informacion para la conexion
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(puerto);
		inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

		// Conectar al servidor
		int connResult = connect(sock, (sockaddr*)& hint, sizeof(hint));
		if (connResult == SOCKET_ERROR) {
			closesocket(sock);
			WSACleanup();

			cerr << endl << "No se pudo conectar al servidor" << endl << endl;
			system("pause");
		}
		else {
			// Si se conectó bien hacer ...

			// Recibir peticion de login del servidor
			string respuesta;
			string mensaje;
			string password;
			respuesta = recibirMensaje(sock);
			
			while (respuesta == "login") {
				system("cls");

				// Solicitar datos de login al usuario
				cout << "Usuario: ";
				cin >> usuario;
				cout << "Contraseña: ";
				cin >> password;

				mensaje = "login;" + usuario + ";" + password;

				// Enviar pedido de login
				if (enviarMensaje(mensaje, sock) == 1) {
					cout << "Error al enviar el mensaje" << endl;
				}

				respuesta = recibirMensaje(sock);
				if(respuesta=="login"){
					cout << "Error: Usuario o contraseña incorrectos" << endl << endl;
					system("pause");
				}
				
			}

			if (respuesta == "excesoDeIntentos") {
				cout << "Error: Se superó la cantidad máxima de intentos de ingreso" << endl << endl;
				system("pause");
			}
			else {
				// Entrar al menú principal
				menu(sock);
			}

			// Enviar peticion de cierre de sesion
			respuesta = "cerrarSesion;" + usuario;
			enviarMensaje(respuesta, sock);
			cout << endl << "Saliendo del sistema" << endl << endl;
			system("pause");

			// Apagar el socket antes de cerrarlo
			int iResult = shutdown(sock, SD_SEND | SD_RECEIVE);
			if (iResult == SOCKET_ERROR) {
				cout << "Error al apagar el socket" << endl;
			}
		}
		closesocket(sock);
		WSACleanup();
	}

	return 0;
}

// Implementaciones
void altaServicio(SOCKET& sock) {
	string log;

	// Pedir datos al usuario
	system("cls");
	cout << "Alta de servicio" << endl;

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
	int dia = 0;
	int mes = 0;
	int anio = 0;
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
	servicioS << "altaServicio;";

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
	stringstream fechaS;
	fechaS << std::setfill('0') << std::setw(2) << dia
		<< std::setfill('0') << std::setw(2) << mes
		<< anio << ";";
	string fecha;
	fechaS >> fecha;
	servicioS << fecha;
	string asientos = "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo;";
	servicioS << asientos;
	string alta;
	servicioS >> alta;

	if (enviarMensaje(alta, sock) != 1) {
		string res = recibirMensaje(sock);
		if (res == "altaOk") {
			system("cls");

			cout << "Se ha dado de alta el siguiente servicio: " << endl << endl;

			// Quitarle el comando al alta
			string delimitador = ";";
			alta.erase(0, alta.find(delimitador) + delimitador.length());
			// Mostrar el servicio
			mostrarServicio(alta);

			system("pause");
		}

		if (res == "altaNegada") {
				cout << endl << "Error: El servicio se repite o no cumple con las condiciones requeridas" << endl << endl;

				system("pause");
		}
	}
	else 
		cout << "Error al enviar mensaje";
}

void gestionPasaje() {
	system("cls");
	cout << "Alta de servicios" << endl << endl;
	system("pause");
}

void gestionPasaje(SOCKET& sock) {
	system("cls");
	cout << "Gestion de pasajes" << endl << endl;
	system("pause");
}

void verRegistro(SOCKET& sock) {
	system("cls");
	string verRegistro = "verRegistro;";
	string respuesta;

	if (enviarMensaje(verRegistro, sock) != 1) {
		cout << "Ver registro de actividades" << endl;
		while (recibirMensaje(sock) != "")
		{
			cout << recibirMensaje(sock) << endl;
		}
	}
	else {
		cout << "Error al enviar mensaje";
	}

	system("pause");
}

void menu(SOCKET& sock) {
	int opcion;
	bool conectado = true;

	while (conectado) {
		opcion = 0;
		while (opcion < 1 || opcion>4) {
			system("cls");
			cout << "Menú principal" << endl << endl
				<< "1: Alta servicio" << endl
				<< "2: Gestionar pasajes" << endl
				<< "3: Ver registro de actividades" << endl
				<< "4: Cerrar sesión" << endl << endl
				<< "Elija una opción: ";
			cin >> opcion;
		}

		switch (opcion) {
		case 1:
			altaServicio(sock);
			break;
		case 2:
			gestionPasaje(sock);
			break;
		case 3:
			verRegistro(sock);
			break;
		case 4:
			// Salir del menú para volver a la conexión al servidor
			conectado = false;
			break;
		default:
			break;
		}
	}
}

int enviarMensaje(string& mensaje, SOCKET& sock) {
	int iResult = 0;

	// Intentar enviar mensaje
	iResult = send(sock, mensaje.c_str(), (int)strlen(mensaje.c_str()) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		cout << "Fallo en el envio del mensaje" << endl;
		return 1;
	}

	/*cout << "Mensaje enviado: " << mensaje << endl;*/

	return iResult;
}

string recibirMensaje(SOCKET& sock) {
	int iResultado;
	char bufer[4096];
	string respuesta;

	// Recibir hasta que el servidor corte la conexion
	iResultado = recv(sock, bufer, 4096, 0);

	respuesta.assign(bufer);

	return respuesta;
}

void mostrarAsientos(string& asientos) {
	// Columnas
	cout << "Asientos:" << endl
		<< "   | ";
	for (size_t i = 1; i <= 20; i++) {
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

void mostrarServicio(string servicio) {
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