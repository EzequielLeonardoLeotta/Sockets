#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

//Declaraciones
bool validarLogin(string &mensaje);
int enviarMensaje(string& mensaje, SOCKET& sock);
void login(SOCKET& clientSocket);
void altaServicio(string mensaje);
void atenderPeticiones(SOCKET& clientSocket);

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "Spanish");
	altaServicio("cordoba;12/3/2020;mañana");
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
		int timeout = 120000;  // Tiempo de inactividad maximo en milisegundos 
		bool timeoutCliente = false;
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		
		// Enviar pedido de usuario y contraseña
		login(clientSocket);
		//altaServicio("cordoba;12/05/2020;manana");
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
		// Atender peticiones del cliente hasta que se desconecte
		atenderPeticiones(clientSocket);
		
		cout << "--------------------------------------------------" << endl << endl;

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
void leerServicios() {

}


void altaServicio(string mensaje) {
	//ejemplo altaServicio("cordoba");

	// si inserto un string largo explota ejemplo  string prueba = "cordoba;12/05/2020;manana";
		//string prueba1 = "cordoba;12/05/2020;manana";
		//string prueba2 = "MardelPlata;12/05/2020;tarde";
		//string prueba3 = "Salta;12/05/2020;noche";
		fstream archi("infoServicios.bin", ios::binary | ios::out | ios::app);
			//archi.write((char*)&prueba1, sizeof(string));
			//archi.write((char*)&prueba2, sizeof(string));
	archi.write((char*)&mensaje, sizeof(string));

	archi.close();
	
}

bool validarLogin(string &mensaje) {
	// Sacar del mensaje sus 3 valores
	char delimitador = ';';
	string comando, usuarioCliente, passCliente;
	istringstream input;
	input.str(mensaje);
	getline(input, comando, delimitador);
	getline(input, usuarioCliente, delimitador);
	getline(input, passCliente, delimitador);

	// Leer archivo y comparar valores
	bool encontrado=false;
	string usuario, password, respuesta;
	ifstream archivo;
	archivo.open("credenciales.txt", ios::in);
	if (archivo.fail()) {
		cout << endl << "ERROR!: No se pudo abrir el archivo de credenciales" << endl;
	}
	else {
		// Leer registro del archivo de credenciales
		while (getline(archivo, usuario, delimitador) && getline(archivo, password) && !encontrado) {
			if (usuario == usuarioCliente && password == passCliente) {
				encontrado = true;
			}
		}
	}
	archivo.close();	

	return encontrado;
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
	int intentos=0;

	// Enviar pedido de login al cliente
	mensaje = "login";
	enviarMensaje(mensaje, clientSocket);

	while (!logueado && !timeoutCliente && mensaje != "excesoDeIntentos") {
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
		if (validarLogin(respuesta)) {
			mensaje = "loginOK";
			logueado = true;
		}
		else {
			intentos++;
			// Si llegué a 3 intentos tirar error
			if (intentos == 3) {
				mensaje = "excesoDeIntentos";
			}
		}


		//Enviar respuesta
		if (!timeoutCliente) {
			enviarMensaje(mensaje, clientSocket);
		}
	}
}

void atenderPeticiones(SOCKET &clientSocket) {
	char buf[4096];
	string peticion;
	bool timeoutCliente = false;
	bool desconectado = false;
	int iResult;
	string respuesta;

	// Recibir hasta que el cliente corte la conexion
	while (!desconectado && !timeoutCliente){
		iResult = recv(clientSocket, buf, 4096, 0);
		if (iResult == SOCKET_ERROR) {
			if (WSAGetLastError() == 10060) {
				cerr << "Cliente desconectado (TIMEOUT)" << endl;
				timeoutCliente = true;
				break;
			}
			else {
				cerr << "Error al intentar escuchar al cliente" << endl;
				desconectado = true;
				break;
			}
		}

		// Si no hubo TIMEOUT del cliente
		if (!timeoutCliente || !desconectado) {
			// Mostrar la peticion recibida
			peticion.assign(buf);
			cout << "Mensaje recibido: " << peticion << endl;

			// Procesar la petición
			respuesta = "hayQueProcesarEsto";

			//Enviar respuesta
			string comando = peticion.substr(0, peticion.find(';'));
			if (comando == "cerrarSesion")
				desconectado = true;
			else
				enviarMensaje(respuesta, clientSocket);
		}
		
	}
}
