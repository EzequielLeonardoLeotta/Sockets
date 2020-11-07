#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
//#include <vector>
#include <iomanip>

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
bool validarServicio(string mensaje);
string getFechaHoraActual();
void verRegistroDeActividades(SOCKET& clientSocket);
void escribirAlArchivo(string& texto);
void traerServicios(string filtro, SOCKET &clientSocket);
string recibirMensaje(SOCKET& clientSocket);
void reservarAsiento(string peticion, SOCKET& clientSocket);
void liberarAsiento(string peticion, SOCKET& clientSocket);

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

	file.open("Log/server.log", ios::app | ios::out);

	if (file.fail()) {
		cout << endl << "Error al acceder al archivo server.log";
		exit(1);
	}

	file << getFechaHoraActual() << "--->" << mensaje.c_str() << endl;
}

bool validarServicio(string filtro) {
	bool encontrado = false;
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
		while (!finArchivo && !encontrado) {
			archivo.get(registro, largoRegistro);
			if (archivo) {
				// Pasar el registro de char[] a string
				string servicio(registro);

				// Partir el registro en campos
				origen = servicio.substr(0, servicio.find(";"));
				servicio.erase(0, servicio.find(delimitador) + delimitador.length());
				turno = servicio.substr(0, servicio.find(";"));
				servicio.erase(0, servicio.find(delimitador) + delimitador.length());
				fecha = servicio.substr(0, servicio.find(";"));

				// Comparar strings
				if (origen == origenFiltro && turno == turnoFiltro && fecha == fechaFiltro)
					encontrado = true;
			}
			else
				finArchivo = true;
		}
		archivo.close();
	}
	return encontrado;
}

bool altaServicio(string mensaje) {
	bool respuesta = false;
	if (!validarServicio(mensaje)) {
		// Agregar el servicio al archivo
		escribirAlArchivo(mensaje);
		archivarLogCliente(mensaje + " - AltaServicio");

		respuesta = true;
	}

	return respuesta;
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
			string delimitador = ";";

			// Quitarle el comando al mensaje
			string comando = peticion.substr(0, peticion.find(";"));
			peticion.erase(0, peticion.find(delimitador) + delimitador.length());

			// Atender si no se recibe el cierre de sesión
			if (comando == "cerrarSesion")
			{
				desconectado = true;
				estadoCliente = "deslogueado";
			}

			// Atender altaServicio
			if (comando == "altaServicio" && !desconectado) {
				if (altaServicio(peticion)) {
					respuesta = "altaOk"; 
					enviarMensaje(respuesta, clientSocket);
				}
				else {
					respuesta = "altaNegada";
					enviarMensaje(respuesta, clientSocket);
				}
			}

			// Atender reservarAsiento
			if (comando == "reservarAsiento" && !desconectado) {
				reservarAsiento(peticion, clientSocket);
			}

			// Atender liberarAsiento
			if (comando == "liberarAsiento" && !desconectado) {
				liberarAsiento(peticion, clientSocket);
			}

			// Atender traerServicios (devuelve de a uno los servicios que combinan 
			// con el filtro, por ultimo envia "finLista")
			if (comando == "traerServicios" && !desconectado) 
				traerServicios(peticion, clientSocket);
			
			// Atender verRegistro
			if (comando == "verRegistro" && !desconectado)
				verRegistroDeActividades(clientSocket);

		}
	}
}

void archivarLogCliente(string mensaje)
{
	fstream archivo("Log/Clientes/" + usuarioCliente + ".txt", ios::app | ios::out);

	if (archivo.is_open())
	{
		archivo << getFechaHoraActual() + "--->" + mensaje + "\n";
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

	stringstream fechaS;
	fechaS << std::setfill('0') << std::setw(2) << dia
		<< "/"
		<< std::setfill('0') << std::setw(2) << mes
		<< "/"
		<< ano << "__"
		<< std::setfill('0') << std::setw(2) << hora
		<< ":"
		<< std::setfill('0') << std::setw(2) << minutos
		<< ":"
		<< std::setfill('0') << std::setw(2) << segundos;
	string fechaHora;
	fechaS >> fechaHora;

	return fechaHora;
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

void escribirAlArchivo(string& texto) {
	ofstream archivo;
	archivo.open("infoServicios.bin", ofstream::binary | ofstream::app);

	if (archivo) {
		archivo.write(texto.c_str(), 74);
		archivo.close();
	}
}

void traerServicios(string filtro, SOCKET& clientSocket) {
	ifstream archivo("infoServicios.bin", ifstream::binary);
	if (archivo) {
		// Leer un registro del archivo
		bool finArchivo = false;
		streamoff largoRegistro = 75;
		char registro[75];
		string origenFiltro, turnoFiltro, fechaFiltro, origen, turno, fecha;
		string delimitador = ";";
		int serviciosEncontrados = 0;

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
				if (origen == origenFiltro || origenFiltro == "") {
					if (turno == turnoFiltro || turnoFiltro == "") {
						if (fecha == fechaFiltro || fechaFiltro == "") {
							// Enviar servicio al cliente y recibir OK
							serviciosEncontrados++;
							enviarMensaje(servicio, clientSocket);
							if (recibirMensaje(clientSocket) != "recibido") {
								break;
							}
						}
					}
				}

			}
			else
				finArchivo = true;
		}
		archivo.close();

		string mensaje;
		// Si no se encontró ningun servicio avisarle al cliente
		if (serviciosEncontrados == 0) {
			mensaje = "noEncontrado";
			enviarMensaje(mensaje, clientSocket);
		}
		else {
			mensaje = "finLista";
			enviarMensaje(mensaje, clientSocket);
		}
	}
	else
		std::cout << endl << "Error: no se pudo abrir el archivo infoServicios.bin" << endl;
}

string recibirMensaje(SOCKET& clientSocket) {
	int iResultado;
	char bufer[4096];
	string respuesta = "";

	// Recibir hasta que el servidor corte la conexion
	iResultado = recv(clientSocket, bufer, 4096, 0);

	respuesta.assign(bufer);

	return respuesta;
}

void reservarAsiento(string peticion, SOCKET& clientSocket) {
	// Partir peticion en fila, columna y servicio (origen, turno, fecha)
	string delimitador = ";";
	string fila = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string columna = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string origen = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string turno = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string fecha = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string servicio = origen + ";" + turno + ";" + fecha + ";";

	// Procesar fila y columna, generar posición del string de asientos del servicio
	int posicion = 0;
	if (fila == "A") {
		posicion--;
	}
	else if (fila == "B") {
		posicion += 19;
	}
	else if (fila == "C") {
		posicion += 39;
	}
	posicion += stoi(columna);

	string respuesta;
	// Buscar el servicio en el archivo infoServicios
	if (validarServicio(servicio)) {
		// Si existe...
		// Abrir archivo infoServicios y un archivo temporal
		ifstream archivo("infoServicios.bin", ifstream::binary);
		ofstream archivoTemporal("temporal.bin", ofstream::binary | ofstream::app);
		if (archivo && archivoTemporal) {
			// Leer un registro del archivo
			bool finArchivo = false;
			streamoff largoRegistro = 75;
			char registro[75];
			string origenTemporal, turnoTemporal, fechaTemporal, asientosTemporal;
			int serviciosEncontrados = 0;

			// Mientras no sea el fin de archivo leer registro
			while (!finArchivo) {
				archivo.get(registro, largoRegistro);
				if (archivo) {
					// Guardar el registro leido sin modificar
					string servicio(registro);

					// Pasar el registro de char[] a string
					string temporal(registro);

					// Partir el registro leido en campos
					origenTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					turnoTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					fechaTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					asientosTemporal = temporal.substr(0, temporal.find(";"));

					// Si el servicio leido es el que buscaba
					if (origen == origenTemporal && turno == turnoTemporal && fecha == fechaTemporal) {
						// Modificar servicio con los datos correspondientes
						asientosTemporal[posicion] = 'x';
						servicio = origenTemporal + ";" + turnoTemporal + ";" + fechaTemporal + ";" + asientosTemporal + ";";
					}
					archivoTemporal.write(servicio.c_str(), 74);
				}
				else
					finArchivo = true;
			}
			archivo.close();
			archivoTemporal.close();

			// Eliminar infoServicios.bin
			remove("infoServicios.bin");
			// Renombrar temporal.bin como infoServicios.bin
			if (rename("temporal.bin", "infoServicios.bin") == 0) {
				// Si no hubo problemas responderle OK al cliente
				respuesta = "reservaOK";
			}
		}
		else
			respuesta = "reservaError";
	}
	else 
		respuesta = "reservaError";

	// Enviar respuesta al cliente
	enviarMensaje(respuesta, clientSocket);
}

void liberarAsiento(string peticion, SOCKET& clientSocket) {
	// Partir peticion en fila, columna y servicio (origen, turno, fecha)
	string delimitador = ";";
	string fila = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string columna = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string origen = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string turno = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string fecha = peticion.substr(0, peticion.find(";"));
	peticion.erase(0, peticion.find(delimitador) + delimitador.length());
	string servicio = origen + ";" + turno + ";" + fecha + ";";

	// Procesar fila y columna, generar posición del string de asientos del servicio
	int posicion = 0;
	if (fila == "A") {
		posicion--;
	}
	else if (fila == "B") {
		posicion += 19;
	}
	else if (fila == "C") {
		posicion += 39;
	}
	posicion += stoi(columna);

	string respuesta;
	// Buscar el servicio en el archivo infoServicios
	if (validarServicio(servicio)) {
		// Si existe...
		// Abrir archivo infoServicios y un archivo temporal
		ifstream archivo("infoServicios.bin", ifstream::binary);
		ofstream archivoTemporal("temporal.bin", ofstream::binary | ofstream::app);
		if (archivo && archivoTemporal) {
			// Leer un registro del archivo
			bool finArchivo = false;
			streamoff largoRegistro = 75;
			char registro[75];
			string origenTemporal, turnoTemporal, fechaTemporal, asientosTemporal;
			int serviciosEncontrados = 0;

			// Mientras no sea el fin de archivo leer registro
			while (!finArchivo) {
				archivo.get(registro, largoRegistro);
				if (archivo) {
					// Guardar el registro leido sin modificar
					string servicio(registro);

					// Pasar el registro de char[] a string
					string temporal(registro);

					// Partir el registro leido en campos
					origenTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					turnoTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					fechaTemporal = temporal.substr(0, temporal.find(";"));
					temporal.erase(0, temporal.find(delimitador) + delimitador.length());
					asientosTemporal = temporal.substr(0, temporal.find(";"));

					// Si el servicio leido es el que buscaba
					if (origen == origenTemporal && turno == turnoTemporal && fecha == fechaTemporal) {
						// Modificar servicio con los datos correspondientes
						asientosTemporal[posicion] = 'o';
						servicio = origenTemporal + ";" + turnoTemporal + ";" + fechaTemporal + ";" + asientosTemporal + ";";
					}
					archivoTemporal.write(servicio.c_str(), 74);
				}
				else
					finArchivo = true;
			}
			archivo.close();
			archivoTemporal.close();

			// Eliminar infoServicios.bin
			remove("infoServicios.bin");
			// Renombrar temporal.bin como infoServicios.bin
			if (rename("temporal.bin", "infoServicios.bin") == 0) {
				// Si no hubo problemas responderle OK al cliente
				respuesta = "liberarOK";
			}
		}
		else
			respuesta = "liberarError";
	}
	else
		respuesta = "liberarError";

	// Enviar respuesta al cliente
	enviarMensaje(respuesta, clientSocket);
}