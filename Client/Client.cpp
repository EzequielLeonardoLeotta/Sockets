#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>

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
void mostrarServicio(string servicio, bool ocupacion);
void modificarServicio(SOCKET& sock, list<string> lista);
string getServicio(list<string> lista, int &pos);
void reservarAsiento(SOCKET& sock, string servicio);
void liberarAsiento(SOCKET& sock, string servicio);
void elegirOtroServicio(list<string> &lista);

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
					cout << endl << "Error: Usuario o contraseña incorrectos" << endl << endl;
					system("pause");
				}
				
			}

			if (respuesta == "excesoDeIntentos") {
				cout << endl << "Error: Se superó la cantidad máxima de intentos de ingreso" << endl;
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
			mostrarServicio(alta, true);

			cout << endl;
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

void gestionPasaje(SOCKET& sock) {
	std::system("cls");
	cout << "Gestion de pasajes" << endl << endl;

	// Pedir datos al usuario para filtrar
	cout << "Ingrese los siguientes datos y se le informarán los servicios existentes" << endl;

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
	servicioS << "traerServicios;";

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
	if (dia != 0) {
		stringstream dato;
		dato << std::setfill('0') << std::setw(2) << dia
			<< std::setfill('0') << std::setw(2) << mes
			<< anio << ";";
		string fecha;
		dato >> fecha;
		servicioS << fecha;
	}
	else
		servicioS << ";";

	// String de petición
	string filtro;
	servicioS >> filtro;

	list<string> listaServicios;

	string res, mensaje;
	// Enviar la petición de servicios con el filtro
	if (enviarMensaje(filtro, sock) != 1) {
		// Recibir de a uno la lista de servicios
		std::system("cls");
		cout << "Servicios encontrados en base a su filtro: " << endl;
		do {
			res = recibirMensaje(sock);
			if (res != "finLista" && res != "noEncontrado") {
				// Mostrar el servicio por pantalla, sin ocupación
				listaServicios.push_back(res);
				cout << endl << "Servicio: " << listaServicios.size() << endl;
				mostrarServicio(res, false);			

				// Confirmar recepcion al servidor para que mande el siguiente
				mensaje = "recibido";
				enviarMensaje(mensaje, sock);
			}				
		} while (res != "finLista" && res != "noEncontrado");
	}
	else
		cout << endl << "Error al enviar petición de lista de servicios" << endl << endl;

	// Ofrecer opcion de alta servicio o de elección de servicio
	int opcion = 0;
	bool salir = false;

	if (res == "noEncontrado") {
		// Si no se encontró ningun servicio ofrecer unicamente alta servicio
		std::system("cls");
		cout << "No se encontraron servicios que coincidan con los datos ingresados."
			<< endl << endl << "Desea realizar una alta de servicio?: " << endl;
		while (opcion < 1 || opcion>2) {
			cout << endl << "1- Si, quiero crear un nuevo servicio" << endl
				<< "2- No, quiero volver al menú anterior" << endl
				<< endl << "Ingrese una opción: ";
			cin >> opcion;
		}
		switch (opcion) {
		case 1:
			altaServicio(sock);
			break;
		case 2:
			salir = true;
			break;
		}
	}
	else {
		// Si se mostraron servicios ofrecer reservar pasaje y alta servicio
		cout << endl << "Que desea hacer?: " << endl;
		while (opcion < 1 || opcion>3) {
			cout << endl << "1- Reservar o liberar pasajes" << endl
				<< "2- Alta servicio" << endl
				<< "3- Volver al menú anterior" << endl
				<< endl << "Ingrese una opción: ";
			cin >> opcion;
		}
		switch (opcion) {
		case 1:
			modificarServicio(sock, listaServicios);
			break;
		case 2:
			altaServicio(sock);
			break;
		case 3:
			salir = true;
			break;
		}
	}
}

void verRegistro(SOCKET& sock) {
	system("cls");
	cout << "Ver registro de actividades" << endl << endl;

	string verRegistro = "verRegistro;";
	string mensaje="";

	if (enviarMensaje(verRegistro, sock) != 1) {
		while (mensaje != "finLog") {
			mensaje = recibirMensaje(sock);
			if (mensaje != "finLog") 
				cout << mensaje << endl;			
		}
	}
	else 
		cout << endl << "Error al enviar el pedido del registro" << endl;

	cout << endl;
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
	string respuesta="";

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

	cout << endl << endl
		<< "o = Asiento libre" << endl
		<< "x = Asiento ocupado" << endl;
}

void mostrarServicio(string servicio, bool ocupacion) {
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
	if (ocupacion)
		mostrarAsientos(asientos);
}

void modificarServicio(SOCKET& sock, list<string> lista) {
	int idServicio = 0;
	// Antes de limpiar la pantalla pedir el numero de servicio que se mostró
	while (idServicio < 1 || idServicio > lista.size()) {
		cout << endl << "Por favor ingrese el numero del servicio el cual desea modificar: ";
		cin >> idServicio;
	}

	// Si el numero de servicio es valido proceder al menú de reserva de pasaje
	std::system("cls");
	cout << "Modificar servicio" << endl << endl;

	// Mostrar el servicio seleccionado incluida la ocupación
	idServicio--;
	string servicio = getServicio(lista, idServicio);
	mostrarServicio(servicio, true);

	// Mostrar menú con opciones 
	int opcion = 0;
	while (opcion < 1 || opcion > 4) {
		cout << endl << "1- Reservar asiento" << endl
			<< "2- Liberar asiento" << endl
			<< "3- Elegir otro servicio" << endl
			<< "4- Volver al menú anterior" << endl
			<< endl << "Ingrese una opción: ";
		cin >> opcion;
	}

	switch (opcion) {
	case 1:
		// Reservar asiento 
		reservarAsiento(sock, servicio);
		break;
	case 2:
		// Liberar asiento
		liberarAsiento(sock, servicio);
		break;
	case 3:
		// Elegir otro servicio 
		elegirOtroServicio(lista);
		break;
	case 4:
		// Volver al menú anterior
		break;
	}
}

void reservarAsiento(SOCKET &sock, string servicio) {
	system("cls");
	cout << "Reserva de asiento" << endl << endl;
	mostrarServicio(servicio, true);

	cout << endl << "A continuación se le solicitará fila y columna del asiento a reservar según se muestra arriba" << endl;
	// Pedir fila
	int fila = 0;
	while (fila < 1 || fila>3) {
		cout << endl << "Fila:" << endl 
			<< "1- Fila A" << endl
			<< "2- Fila B" << endl
			<< "3- Fila C" << endl
			<< "Ingrese una opción: ";
		cin >> fila;
	}
	// Pedir columna
	int columna = 0;
	while (columna < 1 || columna>20) {
		cout << endl << "Columna" << endl
			<< "Ingrese el número de columna: ";
		cin >> columna;
	}

	// Procesar fila y columna, generar posición del string de asientos del servicio
	int posicion = 0;
	string filaS;
	switch (fila) {
		case 1:
			posicion--;
			filaS = "A";
			break;
		case 2:
			posicion += 19;
			filaS = "B";
			break;
		case 3:
			posicion += 39;
			filaS = "C";
			break;
	}
	posicion += columna;

	// Partir el servicio en campos
	string delimitador = ";";
	string origen = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string turno = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string fecha = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string asientos = servicio.substr(0, servicio.find(";"));

	// Validar si el asiento indicado ya está ocupado (x)
	if (asientos[posicion] != 'x') {
		// Si no está ocupado ...
		
		// Armar petición (reservarAsiento;fila;columna;origen;turno;fecha;)
		stringstream mensajeS;
		mensajeS << "reservarAsiento;"
			// Agregar asiento
			<< filaS << ";" << columna << ";"
			// Agregar servicio
			<< origen << ";"
			<< turno << ";"
			<< fecha << ";";
		
		// Guardar peticion en string
		string mensaje;
		mensajeS >> mensaje;
		
		// Enviar peticion de reserva al servidor
		enviarMensaje(mensaje, sock);
		
		// Procesar respuesta del servidor
		string res = recibirMensaje(sock);
		if (res == "reservaOK") {
			cout << endl << "Se ha reservado el asiento " << filaS << "-" << columna << " correctamente." << endl;
		}
		else if (res == "reservaError") {
			cout << endl << endl << "Error: No se pudo reservar el asiento " << fila << " " << columna << "." << endl;
		}
	}
	else {
		// Si está ocupado informar al usuario que no puede reservar un asiento ocupado 
		cout << endl << endl 
			<< "Error: el asiento " << filaS << "-" << columna << " ya está ocupado." << endl;
	}
	
	cout << endl;
	system("pause");
}

void liberarAsiento(SOCKET& sock, string servicio) {
	system("cls");
	cout << "Liberar un asiento" << endl << endl;
	mostrarServicio(servicio, true);

	string servicioOriginal = servicio;

	cout << endl << "A continuación se le solicitará fila y columna del asiento a reservar según se muestra arriba" << endl;
	// Pedir fila
	int fila = 0;
	while (fila < 1 || fila>3) {
		cout << endl << "Fila:" << endl
			<< "1- Fila A" << endl
			<< "2- Fila B" << endl
			<< "3- Fila C" << endl
			<< "Ingrese una opción: ";
		cin >> fila;
	}
	// Pedir columna
	int columna = 0;
	while (columna < 1 || columna>20) {
		cout << endl << "Columna" << endl
			<< "Ingrese el número de columna: ";
		cin >> columna;
	}

	// Procesar fila y columna, generar posición del string de asientos del servicio
	int posicion = 0;
	string filaS;
	switch (fila) {
	case 1:
		posicion--;
		filaS = "A";
		break;
	case 2:
		posicion += 19;
		filaS = "B";
		break;
	case 3:
		posicion += 39;
		filaS = "C";
		break;
	}
	posicion += columna;

	// Partir el servicio en campos
	string delimitador = ";";
	string origen = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string turno = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string fecha = servicio.substr(0, servicio.find(";"));
	servicio.erase(0, servicio.find(delimitador) + delimitador.length());
	string asientos = servicio.substr(0, servicio.find(";"));

	// Validar si el asiento indicado ya está libre (o)
	if (asientos[posicion] != 'o') {
		// Si no está libre ...

		// Armar petición (reservarAsiento;fila;columna;origen;turno;fecha;)
		stringstream mensajeS;
		mensajeS << "liberarAsiento;"
			// Agregar asiento
			<< filaS << ";" << columna << ";"
			// Agregar servicio
			<< origen << ";"
			<< turno << ";"
			<< fecha << ";";

		// Guardar peticion en string
		string mensaje;
		mensajeS >> mensaje;

		// Enviar peticion de reserva al servidor
		enviarMensaje(mensaje, sock);

		// Procesar respuesta del servidor
		string res = recibirMensaje(sock);
		if (res == "liberarOK") {
			cout << endl << "Se ha liberado el asiento " << filaS << "-" << columna << " correctamente." << endl;
		}
		else if (res == "liberarError") {
			cout << endl << endl << "Error: No se pudo liberar el asiento " << fila << " " << columna << "." << endl;
		}
	}
	else {
		// Si está ocupado informar al usuario que no puede reservar un asiento ocupado 
		cout << endl << "Error: el asiento " << filaS << "-" << columna << " ya está libre." << endl;
	}

	cout << endl;
	system("pause");
}

void elegirOtroServicio(list<string> &lista) {

}

string getServicio(list<string> lista, int &pos) {
	list<string>::iterator it = lista.begin();
	for (int i = 0; i < pos; i++) {
		++it;
	}
	return *it;
}