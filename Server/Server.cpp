#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

//Declaraciones
string procesarMensaje(string &mensaje, SOCKET &clientSocket);
int enviarMensaje(string& mensaje, SOCKET& sock);

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "Spanish");

	while (true) {
		// Iniciar Winsock
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);
		char buf[4096];

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
			WSACleanup();
			return 1;
		}
		cout << "Servidor escuchando en " << ip << ":" << puerto << endl;

		// Esperar una conexion
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
		if (clientSocket == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket del cliente" << endl;
			WSACleanup();
			return 1;
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
		int timeout = 120000;  // Tiempo de inactividad maximo en milisegundos 
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

		// Enviar pedido de usuario y contraseña

		// Recibir hasta que el cliente corte la conexion
		string respuesta = "";
		bool timeoutCliente = false;
		do {
			iResult = recv(clientSocket, buf, 4096, 0);
			if (iResult == SOCKET_ERROR) {
				if (WSAGetLastError() == 10060) {
					cerr << "Cliente desconectado (TIMEOUT)" << endl;
					timeoutCliente = true;
					break;
				}
			}
			if (iResult > 0) {
				/*cout << "Bytes recibidos: " << iResult << endl;*/
				string mensaje = "";
				mensaje.assign(buf);
				cout << "Mensaje recibido: " << mensaje << endl;

				// Procesar la peticion y responder
				respuesta = procesarMensaje(mensaje, clientSocket);
			}
			else if (iResult == 0);
				/*cout << "Cliente desconectado" << endl;*/
			else {
				cout << "Error in recv()" << endl;
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
		} while (iResult > 0);

		// Enviar respuesta al cliente
		if (respuesta != "" && !timeoutCliente) {
			int iResult = send(clientSocket, respuesta.c_str(), (int)strlen(respuesta.c_str()) + 1, 0);
			if (iResult == SOCKET_ERROR) {
				cout << "Error al enviar la respuesta" << endl;
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
			/*cout << "Bytes enviados: " << iResult << endl;*/
			cout << "Respuesta enviada: " << respuesta << endl << endl;
		}

		// Apagar la conexion
		iResult = shutdown(clientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al apagar conexion" << endl;
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		// Cerrar el socket
		closesocket(clientSocket);

		// Limpiar Winsock
		WSACleanup();
	}

	return 0;
}

// Implementaciones

string procesarMensaje(string &mensaje, SOCKET &clientSocket) {
	string delimitador = ";";
	string comando = mensaje.substr(0, mensaje.find(delimitador));
	string respuesta = "";

	cout << "Comando recibido: " << comando << endl;

	if (comando == "login") {
		respuesta = "loginOk";
	}

	return respuesta;
}

int enviarMensaje(string& mensaje, SOCKET& sock) {
	int iResult = 0;

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