#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Declaraciones
void altaServicio();
void gestionPasaje();
void verRegistro();
void cerrarSesion();
void menu();
int enviarMensaje(string& mensaje, SOCKET& sock);
string recibirMensaje(SOCKET& sock);

int main()
{	
	// Iniciar Winsock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "No se arrancar Winsock" << endl;
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

	// Pedir IP y puerto
	int puerto=0;
	string ip="";
	
	while (ip != "127.0.0.1" || puerto != 54000) {
		system("cls");
		cout << "Ingrese los siguientes datos para intentar conectarse al sistema: " << endl ;
		cout << "Direccion IP: "; 
		cin >> ip;
		cout << "Puerto: ";
		cin >> puerto;
	}

	if (puerto == 54000 && ip == "127.0.0.1"){
		// Hint de estructura de informacion para la conexion
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(puerto);
		inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

		// Conectar al servidor
		int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
		if (connResult == SOCKET_ERROR)
		{
			cerr << "No se pudo conectar al servidor: " << WSAGetLastError() << endl;
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		// Enviar pedido de login
		string mensaje = "login;javi;123";
		if (enviarMensaje(mensaje, sock) == 1) {
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		// Recibir respuesta del servidor
		string respuesta = recibirMensaje(sock);
		if (respuesta != "") {
			cout << "Respuesta del servidor: " << respuesta << endl;
		}

	}
	
	// Cerrar el socket y limpiar Winsock
	closesocket(sock);
	return WSACleanup();
}

// Implementaciones
void altaServicio() {
	system("cls");
	cout << "Alta serv" << endl;
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

void menu() {
	int opcion;
	cout << "1_Alta servicio" << endl;
	cout << "2_Gestionar pasajes" << endl;
	cout << "3_Ver registro de actividades" << endl;
	cout << "4_Cerrar Sesion" << endl;
	cout << "Elija una opcion: ";
	cin >> opcion;
	switch (opcion) {
	case 1:
		altaServicio();
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

	// Apagar el socket ya que no se enviará mas data
	iResult = shutdown(sock, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "Fallo al apagar el socket" << endl;
		return 1;
	}

	return iResult;
}

string recibirMensaje(SOCKET& sock) {
	int iResultado;
	char bufer[4096];

	// Recibir hasta que el servidor corte la conexion
	do {
		iResultado = recv(sock, bufer, 4096, 0);
		if (iResultado > 0)
			cout << "Bytes recibidos: " << iResultado << endl;
		else if (iResultado == 0)
			cout << "Conexion cerrada" << endl;
		else
			cout << "Error en recv()" << endl;
	} while (iResultado > 0);

	string respuesta="";
	respuesta.assign(bufer);

	return respuesta;
}