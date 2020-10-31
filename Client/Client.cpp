#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <fstream>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Declaraciones
void altaServicio(SOCKET& sock);
void gestionPasaje(SOCKET& sock);
void verRegistro(SOCKET& sock);
void menu(SOCKET& sock);
int enviarMensaje(string& mensaje, SOCKET& sock);
string recibirMensaje(SOCKET& sock);

// Variables globales
string usuario;

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "Spanish");

	int puerto = 0;
	string ip = "";

	// Pedir IP y puerto. Intentar conexion a esos datos
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
			cerr << "No se pudo arrancar Winsock" << endl;
			return 1;
		}

		// Crear socket
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket" << endl;
			WSACleanup();
			return 1;
		}

		// Hint de estructura de informacion para la conexion
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(puerto);
		inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

		// Conectar al servidor
		int connResult = connect(sock, (sockaddr*)& hint, sizeof(hint));
		if (connResult == SOCKET_ERROR) {
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
				cout << "Respuesta del login: " << respuesta << endl;
				menu(sock);
			}

			if (respuesta == "excesoDeIntentos") {
				cout << "Se superó la cantidad máxima de intentos de ingreso" << endl;
			}
			else {
				// Mostrar el menú
				menu(sock);
			}

			// Enviar peticion de cierre de sesion
			respuesta = "cerrarSesion;" + usuario;
			enviarMensaje(respuesta, sock);
			cout << endl << "Saliendo del sistema" << endl << endl;
			system("pause");
		}

		// Apagar el socket antes de cerrarlo
		int iResult = shutdown(sock, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al apagar el socket" << endl;
		}

		// Cerrar el socket y limpiar Winsock
		closesocket(sock);
		WSACleanup();
	}

	return 0;
}

// Implementaciones
void altaServicio(SOCKET& sock) {
	string origen, fecha, turno, alta;

	system("cls");
	cout << "Alta de servicio" << endl;
	cout << "Ingrese origen: "; cin >> origen;
	cout << "Ingrese Fecha: "; cin >> fecha;
	cout << "Ingrese Turno: "; cin >> turno;
	alta = "altaServicio;" + origen + ";" + fecha + ";" + turno + ";";

	if (enviarMensaje(alta, sock) != 1) {
		while (recibirMensaje(sock) != "")
		{
			string res = recibirMensaje(sock);
			if (res == "altaOk") {
				cout << "Se genero con exito el serivicio";
				system("pause");
				menu(sock);
			}
			
			if (res == "altaNegada") {
					cout << "El servicio se repite o no cumple con las condiciones requeridas";
					system("pause");
					menu(sock);
			}
		    
		}
	}
	else {
		cout << "Error al enviar mensaje";
		menu(sock);
	}
}

void gestionPasaje() {
	system("cls");
	cout << "Alta de servicios" << endl << endl;
	system("pause");
}

void gestionPasaje(SOCKET& sock) {
	system("cls");

	int opcion;
	bool conectado = true;

	while (conectado) {
		opcion = 0;
		while (opcion < 1 || opcion>7) {
			system("cls");
			cout << "Busqueda de servicio" << endl << endl
				<< "Filtrar por:" << endl
				<< "1: Origen" << endl
				<< "2: Fecha" << endl
				<< "3: Turno" << endl
				<< "4: Origen y Fecha" << endl
				<< "5: Origen y Turno" << endl
				<< "6: Fecha y Turno" << endl
				<< "7: Cerrar sesión" << endl << endl
				<< "Elija una opción: ";
			cin >> opcion;
		}

		system("cls");
		string mensaje = "getServicios;", origen, fecha, turno;
		cout << "Busqueda de servicio" << endl << endl
			<< "Origen: M = Mar del Plata | B = Buenos Aires" << endl
			<< "Fecha: DD/MM/AAAA" << endl
			<< "Turno: M = Mañana | T = Tarde | N = Noche" << endl << endl;

		switch (opcion) {
		case 1:
			cout << "Ingrese origen: "; cin >> origen;
			mensaje += "origen" + origen + ";";
			break;
		case 2:
			cout << "Ingrese fecha: "; cin >> fecha;
			mensaje += "fecha" + fecha + ";";
			break;
		case 3:
			cout << "Ingrese turno: "; cin >> turno;
			mensaje += "turno" + turno + ";";
			break;
		case 4:
			cout << "Ingrese origen: "; cin >> origen;
			cout << "Ingrese fecha: "; cin >> fecha;
			mensaje += "origen_fecha" + origen + ";" + fecha + ";";
			break;
		case 5:
			cout << "Ingrese origen: "; cin >> origen;
			cout << "Ingrese turno: "; cin >> turno;
			mensaje += "origen_turno" + origen + ";" + turno + ";";
			break;
		case 6:
			cout << "Ingrese fecha: "; cin >> fecha;
			cout << "Ingrese turno: "; cin >> turno;
			mensaje += "fecha_turno" + fecha + ";" + turno + ";";
			break;
		case 7:
			// Salir del menú para volver a la conexión al servidor
			conectado = false;
			break;
		default:
			break;
		}
	}

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