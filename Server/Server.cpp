#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

//Declaraciones
string validarLogin(string &mensaje);
int enviarMensaje(string& mensaje, SOCKET& sock);
void login(SOCKET& clientSocket);

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "Spanish");

	//Bucle infinito de servicio del servidor
	while (true) {
		// Iniciar Winsock
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);

		int iResult = WSAStartup(ver, &wsData);
		if (iResult != 0)
		{
			cerr << "No se pudo iniciar Winsock" << endl;
			return 1;
		}

		// Crear socket de escucha
		string ip = "127.0.0.1";
		int puerto = 54000;

		SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
		if (listening == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket de escucha" << endl;
			WSACleanup();
			return 1;
		}

		// Vincular la direccion ip y el puerto al socket de escucha
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(puerto);
		hint.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
		bind(listening, (sockaddr*)&hint, sizeof(hint));

		// Setear el socket para recibir conexiones
		iResult = listen(listening, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al setear el socket como listen" << endl;
			closesocket(listening);
			break;
		}
		cout << "Servidor escuchando en " << ip << ":" << puerto << endl;

		// Esperar una conexion
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
		if (clientSocket == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket del cliente" << endl;
			break;
		}

		char host[NI_MAXHOST];		// Nombre del equipo remoto
		char service[NI_MAXSERV];	// Puerto del equipo remoto

		ZeroMemory(host, NI_MAXHOST);
		ZeroMemory(service, NI_MAXSERV);

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
			cout << host << " conectado en puerto " << service << endl;
		}
		else {
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cout << host << " conectado en puerto " <<
				ntohs(client.sin_port) << endl;
		}

		// Cerrar socket de escucha porque se conectó un cliente
		closesocket(listening);

		// Configurar el socket para desconexion despues de 2 minutos de inactividad
		int timeout = 60000;  // Tiempo de inactividad maximo en milisegundos 
		bool timeoutCliente = false;
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		
		// Enviar pedido de usuario y contraseña
		login(clientSocket);

		//// Atender peticiones del cliente hasta que se desconecte
		//char buf[4096];
		//string respuesta;
		//bool desconectado = false;
		//
		//// Recibir hasta que el cliente corte la conexion
		//do {
		//	iResult = recv(clientSocket, buf, 4096, 0);
		//	if (iResult == SOCKET_ERROR) {
		//		if (WSAGetLastError() == 10060) {
		//			cerr << "Cliente desconectado (TIMEOUT)" << endl;
		//			timeoutCliente = true;
		//			break;
		//		}
		//		else {
		//			cerr << "Error al intentar escuchar al cliente" << endl << endl;
		//			timeoutCliente = true;
		//			break;
		//		}
		//	}
		//	if (iResult > 0) {
		//		string mensaje = "";
		//		mensaje.assign(buf);
		//		cout << "Mensaje recibido: " << mensaje << endl;

		//		// Procesar la peticion y preparar la respuesta

		//		// Enviar respuesta al cliente
		//		int iResult = send(clientSocket, respuesta.c_str(), (int)strlen(respuesta.c_str()) + 1, 0);
		//		if (iResult == SOCKET_ERROR) {
		//			cout << "Error al enviar la respuesta" << endl;
		//		} else 
		//			cout << "Respuesta enviada: " << respuesta << endl << endl;
		//	}
		//	else if (iResult == 0)
		//		desconectado = true;
		//	else {
		//		cout << "Error in recv()" << endl;
		//		desconectado = true;
		//	}
		//} while (iResult > 0  && !timeoutCliente && !desconectado);
		
		cout << "Cliente desconectado" << endl << "--------------------------------------------------" << endl << endl;

		// Apagar el socket antes de cerrarlo
		iResult = shutdown(clientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al apagar el socket" << endl;
		}

		// Cerrar el socket
		closesocket(clientSocket);

		// Limpiar Winsock
		WSACleanup();
	}

	return 0;
}

// Implementaciones

string validarLogin(string &mensaje) {
	string delimitador = ";";
	string comando = mensaje.substr(0, mensaje.find(delimitador));
	string respuesta;

	//Validar login aqui
	respuesta = "loginOK";

	return respuesta;
}

int enviarMensaje(string& mensaje, SOCKET& sock) {
	int iResult = 0;

	// Intentar enviar mensaje
	iResult = send(sock, mensaje.c_str(), (int)strlen(mensaje.c_str()) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		cout << "Fallo en el envio del mensaje " << WSAGetLastError() << endl;
		return 1;
	}

	cout << "Mensaje enviado: " << mensaje << endl;

	return iResult;
}

void login(SOCKET &clientSocket) {
	bool logueado = false;
	bool timeoutCliente = false;
	string mensaje;
	int resultado;
	char buf[4096];
	string respuesta;

	while (!logueado && !timeoutCliente) {
		// Enviar pedido de login al cliente
		mensaje = "login";
		enviarMensaje(mensaje, clientSocket);

		// Recibir datos de login del cliente
		resultado = recv(clientSocket, buf, 4096, 0);
		if (resultado == SOCKET_ERROR) {
			if (WSAGetLastError() == 10060) {
				cerr << "Cliente desconectado (TIMEOUT)" << endl;
				timeoutCliente = true;
			}
			else {
				cerr << "Error al intentar escuchar al cliente" << endl << endl;
				timeoutCliente = true;
			}
		}
		if (!timeoutCliente) {
			respuesta.assign(buf);
			cout << "Mensaje recibido: " << respuesta << endl;
		}
	
		// Validar el login
		mensaje = validarLogin(respuesta);
		
		if (mensaje == "loginOK") {
			logueado = true;
		}

		//Enviar respuesta
		if (!timeoutCliente) {
			enviarMensaje(mensaje, clientSocket);
		}
	}
}