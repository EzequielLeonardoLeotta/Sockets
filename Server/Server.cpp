#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

//Declaraciones
bool validarLogin(string& mensaje);
int enviarMensaje(string& mensaje, SOCKET& sock);
void login(SOCKET& clientSocket);
bool altaServicio(string mensaje);
void atenderPeticiones(SOCKET& clientSocket);
void serverLog(string mensaje);
void archivarLogCliente(string mensaje);
bool validarServicio(char* texto);
string getFechaHoraActual();
void verRegistroDeActividades(SOCKET& clientSocket);

//Variables globales
string usuarioCliente;
string estadoCliente;

int main()
{
	//Caracteres en español
	setlocale(LC_ALL, "es_AR.UTF8");

	// Limpiar Winsock para evitar bugs al reactivar la libreria
	WSACleanup();

	//Bucle infinito de servicio del servidor
	while (true) {
		// Inicia servidor
		cout << "=================================\n          Inicia servidor        \n=================================" << endl;
		serverLog("=================================");
		serverLog("          Inicia servidor        ");
		serverLog("=================================");

		// Iniciar Winsock
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);
		int iResult = WSAStartup(ver, &wsData);
		if (iResult != 0)
		{
			cerr << "No se pudo iniciar Winsock" << endl;
			break;
		}

		// Crear socket de escucha
		string ip = "127.0.0.1";
		int puerto = 54000;

		SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
		if (listening == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket de escucha" << endl;
			WSACleanup();
			break;
		}

		// Vincular la direccion ip y el puerto al socket de escucha
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(puerto);
		hint.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
		bind(listening, (sockaddr*)& hint, sizeof(hint));

		// Setear el socket para recibir conexiones
		iResult = listen(listening, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al setear el socket como listen" << endl;
			closesocket(listening);
			break;
		}

		cout << "Socket creado. Servidor escuchando en " << ip << ":" << puerto << endl;
		serverLog("Socket creado. Servidor escuchando en puerto: " + to_string(puerto));

		// Esperar una conexion
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(listening, (sockaddr*)& client, &clientSize);
		if (clientSocket == INVALID_SOCKET)
		{
			cerr << "No se pudo crear el socket del cliente" << endl;
			serverLog("No se pudo crear socket del cliente");
			break;
		}
		char host[NI_MAXHOST];		// Nombre del equipo remoto
		char service[NI_MAXSERV];	// Puerto del equipo remoto

		ZeroMemory(host, NI_MAXHOST);
		ZeroMemory(service, NI_MAXSERV);

		if (getnameinfo((sockaddr*)& client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
			cout << host << " conectado en puerto " << service << endl;	
			serverLog("Conexion establecida con cliente: " + (string)host + ": " + service);
		}
		else {
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cout << host << " conectado en puerto " <<
				ntohs(client.sin_port) << endl;
			serverLog("Conexion establecida con cliente: " + (string)host + ": " + service);
		}		

		// Cerrar socket de escucha porque se conectó un cliente
		closesocket(listening);

		// Configurar el socket para desconexion despues de 2 minutos de inactividad
		int timeout = 120000;  // Tiempo de inactividad maximo en milisegundos 
		bool timeoutCliente = false;
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)& timeout, sizeof(timeout));

		// Enviar pedido de usuario y contraseña
		login(clientSocket);

		if (estadoCliente == "logueado") {
			// Atender peticiones del cliente hasta que se desconecte
			atenderPeticiones(clientSocket);

			// Loguear el cierre de sesión
			string log = "Usuario " + usuarioCliente + " cierra sesión";
			archivarLogCliente(log);
			serverLog(log);
			estadoCliente = "deslogueado";
			cout << log << endl;
		}

		// Apagar el socket antes de cerrarlo
		iResult = shutdown(clientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			cout << "Error al apagar el socket" << endl;
		}

		// Cerrar el socket
		closesocket(clientSocket);
		serverLog("Conexion terminada con cliente: " + (string)host + ": " + service);
		cout << endl;

		// Limpiar Winsock
		WSACleanup();
	}

	return 0;
}

// Implementaciones
void leerServicios() {

}

void serverLog(string mensaje)
{
	fstream file;
	/*cout << mensaje.c_str();*/

	file.open("Log/server.log", ios::app | ios::out);

	if (file.fail()) {
		cout << endl << "Error al acceder al archivo server.log";
		exit(1);
	}

	/*file << "------------------------------------------------" << endl;*/
	file << getFechaHoraActual() << "--->" << mensaje.c_str() << endl;
}

bool validarServicio(char* texto) {
	bool resultado = false;
	ifstream archivo("infoServicios.bin", ifstream::binary);
	if (archivo) {
		// get length of file:
		archivo.seekg(0, archivo.end);
		streamoff length = archivo.tellg();
		archivo.seekg(0, archivo.beg);

		char* buffer = new char[length]; // buffer = toda la info del archivo
		streamoff longitud1 = length;
		size_t longitud2 = strlen(texto);

		// read data as a block:
		char c;
		archivo.read(buffer, length);
		if (archivo) {
			// Procesar buffer
			c = texto[0];
			for (int i = 0; i < length; i++) {
				if (buffer[i] == c) {
					if (strncmp(&buffer[i], texto, longitud2) == 0) {
						resultado = true;
					}
				}
			}
		}
		else
			cout << "Error al acceder al archivo infoServicios.bin" << endl;
		archivo.close();
		// liberar memoria del buffer
		delete[] buffer;
	}
	return resultado;
}

bool altaServicio(string mensaje) {
	size_t largo = strnlen(mensaje.c_str(), mensaje.length());
	char d[100] = "";
	char* prueba = d;
	strcpy(prueba, mensaje.c_str());
	string butacas = "00000000000000000000000000000000000000000000000000000000;";
	string init = mensaje + butacas;
	fstream f;
	bool value=false;
	f.open("infoServicios.bin", ios::app | ios::binary);

	if (f) {
		size_t largo = strnlen(init.c_str(), init.length());

		bool flag = validarServicio(prueba);
		if (flag != true) {
			for (int i = 0; i < largo; i++) {
				f.put(init[i]);
			}
			archivarLogCliente(mensaje + " - AltaServicio");
			
			value = true;
			//return value;
		}
		else {
			cout << "no se puede dar de alta" << endl;
			value = false;
			//return value;
		}
		
	}
	f.close();
	cout << "VALOR DEl ALTA"<<value;
	return value;
}

bool validarLogin(string& mensaje) {
	// Sacar del mensaje sus 3 valores
	char delimitador = ';';
	string comando, usuario, passCliente;
	istringstream input;
	input.str(mensaje);
	getline(input, comando, delimitador);
	getline(input, usuarioCliente, delimitador);
	getline(input, passCliente, delimitador);

	// Leer archivo y comparar valores
	bool encontrado = false;
	string password, respuesta;
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

void login(SOCKET& clientSocket) {
	bool logueado = false;
	bool timeoutCliente = false;
	bool error = false;
	string mensaje;
	int resultado;
	char buf[4096];
	string respuesta;
	int intentos = 0;

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
		if (validarLogin(respuesta)&&!timeoutCliente) {
			mensaje = "loginOK";
			estadoCliente = "logueado";
			string log = "Usuario " + usuarioCliente + " inicia sesión";
			archivarLogCliente(log);
			serverLog(log);
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

void atenderPeticiones(SOCKET& clientSocket) {
	char buf[4096];
	string peticion;
	bool timeoutCliente = false;
	bool desconectado = false;
	int iResult;
	string respuesta;

	// Recibir hasta que el cliente corte la conexion
	while (!desconectado && !timeoutCliente && estadoCliente=="logueado") {
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

			// Procesar la petición dependiendo del tipo de comando que llegó
			size_t delimitador = peticion.find(';');
			string comando = peticion.substr(0, delimitador);
			// Peticion sin comando 
			string mensaje = peticion.substr(delimitador).replace(0, 1, "");

			// Atender altaServicio
			

			// Enviar respuesta
			if (comando == "cerrarSesion")
			{
				desconectado = true;
			}
			else if (comando == "altaServicio") {
				bool value = altaServicio(mensaje);
				if (value) {
					respuesta = "altaOk";
					enviarMensaje(respuesta, clientSocket);
				}
				else {
					respuesta = "altaNegada";
					enviarMensaje(respuesta, clientSocket);
				}
			}
			else if (comando == "verRegistro")
				verRegistroDeActividades(clientSocket);
		}
	}
}

void archivarLogCliente(string mensaje)
{
	fstream archivo("Log/Clientes/" + usuarioCliente + ".txt", ios::app | ios::out);

	if (archivo.is_open())
	{
		archivo << getFechaHoraActual() + " " + mensaje + "\n";
		archivo.close();
	}
	else
	{
		cout << endl << "Error al abrir el archivo de log del usuario " << usuarioCliente << endl;
		EXIT_FAILURE; // para que?
	}
}

string getFechaHoraActual()
{
	int dia, mes, ano, hora, minutos, segundos;
	time_t t = time(NULL);
	struct tm  today = *localtime(&t);
	mes = today.tm_mon + 1;
	dia = today.tm_mday;
	ano = today.tm_year + 1900;
	hora = today.tm_hour;
	minutos = today.tm_min;
	segundos = today.tm_sec;

	return to_string(dia) + "/" + to_string(mes) + "/" + to_string(ano) + "__" + to_string(hora) + ":" + to_string(minutos) + ":" + to_string(segundos);
}

void verRegistroDeActividades(SOCKET& clientSocket)
{
	string linea;
	ifstream archivo("Log/Clientes/" + usuarioCliente + ".txt");
	if (archivo.is_open())
	{
		while (getline(archivo, linea))
		{
			enviarMensaje(linea, clientSocket);
		}
		archivo.close();
	}
	else
	{
		cout << "Error al abrir el archivo";
		EXIT_FAILURE;
	}
}
