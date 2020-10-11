#include <iostream>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;


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



void main()
{
	string ipAddress = "127.0.0.1";			// IP Address of the server
	int port = 54000;						// Listening port # on the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return;
	}

	// Do-while loop to send and receive data
	char buf[4096];
	string userInput;
	string res="loginOk";
	int errLogin = 0;
	cout << "Ingrese Puerto;";
	getline(cin, userInput);
	do
	{
		if (errLogin<=3) {

			if (userInput.size() > 0)		// Make sure the user has typed in something
			{
				// Send the text
				int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
				if (sendResult != SOCKET_ERROR)
				{
					// Wait for response
					ZeroMemory(buf, 4096);
					int bytesReceived = recv(sock, buf, 4096, 0);
					if (bytesReceived > 0)
					{
						if (res == "loginOk") {
							menu();
						}
						else {
							errLogin++;
							cout << "error User o contraseña";
						}
						// Echo response to console
						//cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
					}
				}
			}
		}
		else {
			cout << "Se supero la cantidad maxima de intentos de ingreso";
		}

	} while (userInput.size() > 0);

	// Gracefully close down everything
	closesocket(sock);
	WSACleanup();
}

