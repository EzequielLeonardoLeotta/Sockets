#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Declaraciones
void altaServicio(SOCKET& sock);
void gestionPasaje();
void verRegistro();
void cerrarSesion();
void menu(SOCKET& sock);
int enviarMensaje(string& mensaje, SOCKET& sock);
string recibirMensaje(SOCKET& sock);

int main()
{	
	//Caracteres en español
	setlocale(LC_ALL, "Spanish");

	int puerto=0;
	string ip="";

	// Pedir IP y puerto. Intentar conexion a esos datos
	while (true) {
		system("cls");
		cout << "Ingrese los siguientes datos para intentar conectarse al sistema: " << endl ;
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
		int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (connResult == SOCKET_ERROR){
			cerr << "No se pudo conectar al servidor" << endl << endl;
			system("pause");
		}
		else {
			// Si se conectó bien hacer ...

			// Recibir peticion de login del servidor
			string respuesta;
			respuesta = recibirMensaje(sock);
			
			while (respuesta == "login") {
				system("cls");
				
				// Solicitar datos de login al usuario
				string usuario = "";
				string password = "";
				cout << "Usuario: ";
				cin >> usuario;
				cout << "Contraseña: ";
				cin >> password;

				string mensaje = "login;" + usuario + ";" + password;

				// Enviar pedido de login
				if (enviarMensaje(mensaje, sock) == 1) {
					cout << "Error al enviar el mensaje" << endl;
				}
				
				respuesta = recibirMensaje(sock);
				cout << "Respuesta del login: " << respuesta << endl;
				menu(sock);
			}

			if (respuesta == "excesoDeIntentos") {
				cout << "Ha excedido la cantidad de intentos permitidos. Desconectado" << endl;
			}
			else {
				// Mostrar el menú
				cout << endl << "Te logueaste. Pasá al menú amiguito" << endl;
			}
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
	string origen,fecha,turno,alta;

	system("cls");
	cout << "ALTA DE SERVICIO" << endl;
	cout << "Ingrese origen: "; cin >> origen; 
	cout << "ingrese Fecha: "; cin >> fecha;
	cout << "ingrese Turno: "; cin >> turno;
	alta ="1;" +origen + ";" + fecha + ";" + turno;

	if (enviarMensaje(alta, sock)!=1){
		cout << "Servicio Generado: "+alta;
	}
	else {
		cout << "Error al enviar mensaje";
	}
	
	system("pause");

}

void gestionPasaje() {
	system("cls");
	cout << "gestion";
	system("pause");

}

void verRegistro() {
	system("cls");
	cout << "ver Registro";
	system("pause");

}

void cerrarSesion() {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	system("cls");
	closesocket(sock);
	cout << "Cerrar sesion" << endl;
	system("pause");

}

void menu(SOCKET& sock) {
	int opcion;
	cout << "1_Alta servicio" << endl;
	cout << "2_Gestionar pasajes" << endl;
	cout << "3_Ver registro de actividades" << endl;
	cout << "4_Cerrar Sesion" << endl;
	cout << "Elija una opcion: ";
	cin >> opcion;
	switch (opcion) {
	case 1:
		altaServicio(sock);
		break;
	case 2:
		gestionPasaje();
		break;
	case 3:
		verRegistro();
	case 4:
		cerrarSesion();
	}
}

int enviarMensaje(string &mensaje, SOCKET &sock) {
	int iResult=0;

	// Intentar enviar mensaje
	iResult = send(sock, mensaje.c_str(), (int)strlen(mensaje.c_str()) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		cout << "Fallo en el envio del mensaje" << endl;
		return 1;
	}

	cout << "Mensaje enviado: " << mensaje << endl;

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