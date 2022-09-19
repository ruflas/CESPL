#include <windows.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;
// DAR VALORES A ESTAS CONSTANTES
#define MAXPETICIONES 10
#define MAXUSUARIOS 50
#define PUERTO 57000
#define TAM_PET 1250
#define TAM_RES 1250

// INTRODUCIR VALORES DE FUNCIONAMIENTO
// BIEN AQU� O LEYENDOLOS COMO VALORES POR TECLADO

int numUsuarios;
int numPeticiones;
float tReflex;

// Estructura de almacenamiento

struct datos {
	int contPet;
	float reflex[MAXPETICIONES];
};

datos datoHilo[MAXUSUARIOS];


//------------------------------------------------------------------------
float NumeroAleatorio(float limiteInferior, float limiteSuperior) {
	float num = (float)rand();
	num = num * (limiteSuperior - limiteInferior) / RAND_MAX;
	num += limiteInferior;
	return num;
}

//------------------------------------------------------------------------
float DistribucionExponencial(float media) {
	float numAleatorio = NumeroAleatorio(0, 1);
	while (numAleatorio == 0 || numAleatorio == 1)
		numAleatorio = NumeroAleatorio(0, 1);
	return (-media) * logf(numAleatorio);
}

//------------------------------------------------------------------------
// Funci�n para cargar la librer�a de sockets
int Ini_sockets(void) {
	WORD wVersionDeseada;
	WSAData wsaData;

	int error;

	wVersionDeseada = MAKEWORD(2, 0);
	if (error = WSAStartup(wVersionDeseada, &wsaData) != 0) {
		return error;
	}

	// Comprobar si la DLL soporta la versión 2.0

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) {
		error = 27;
		cerr << "La librería no soporta la versión 2.0" << endl;
		WSACleanup();
	}
	return error;
}

//------------------------------------------------------------------------
// Funci�n para descargar la librer�a de sockets
void Fin_sockets(void) {
	WSACleanup();
}

//-----------------------------------------------------

// Funcion preparada para ser un thread

DWORD WINAPI Usuario(LPVOID parametro) {

	DWORD dwResult = 0;
	int numHilo = *((int*)parametro);
	int i;
	float tiempo;

	//Variables para los sockets

	SOCKET elSocket;
	sockaddr_in dirServidor;
	char peticion[TAM_PET];
	char respuesta[TAM_RES];
	int valorRetorno;  // Para control de errores


	srand(113 + numHilo * 7);

	datoHilo[numHilo].contPet = 0;

	// ... Resto de cosas comunes para cada usuario

	for (i = 0; i < numPeticiones; i++) {
		// PRINTF solo para depuración NUNCA en medición BORRARLO
		//printf("SOLO DEPURACION - Peticion %d para el usuario %d\n", i, numHilo);

		// Hacer petición cuando se implementen los sockets
		// Implica:
		// 1 .- Creación del socket
		//
		elSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (elSocket == INVALID_SOCKET) {
			//Control de posibles errores
			printf("Error %d INVALID_SOCKET", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// 2 .- Conexión con el servidor
		//

		dirServidor.sin_family = AF_INET;
		dirServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
		dirServidor.sin_port = htons(PUERTO + numHilo);
		valorRetorno = connect(elSocket, (struct sockaddr*) & dirServidor, sizeof(dirServidor));
		if (valorRetorno == SOCKET_ERROR) {
			printf("Error %d SOCKET_ERROR", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// 3 .- Enviar una cadena
		//
		valorRetorno = send(elSocket, peticion, sizeof(peticion), 0);
		if (valorRetorno == SOCKET_ERROR) {
			//Control de posibles errores
			printf("Error %d SOCKET_ERROR", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// 4 .- Recibir la respuesta
		//
		valorRetorno = recv(elSocket, respuesta, sizeof(respuesta), 0);
		if (valorRetorno != TAM_RES) {
			//Control de posibles errores
			printf("Error %d TAM_RES", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// 5 .- Cerrar la conexi�n
		//

		closesocket(elSocket);

		// Fin de la petici�n

		// Calcular el tiempo de reflexi�n antes de la siguiente petici�n
		tiempo = DistribucionExponencial((float)tReflex);

		// Guarda los valores de la petici�n
		datoHilo[numHilo].reflex[i] = tiempo;
		datoHilo[numHilo].contPet++;

		// Espera los milisegundos calculados previamente
		Sleep((unsigned int)tiempo / 1000);
	}
	return dwResult;
}


int main(int argc, char* argv[])
{
	int i, j;
	HANDLE handleThread[MAXUSUARIOS];
	int parametro[MAXUSUARIOS];

	// Leer por teclado los valores para realizar la prueba o asignarlos
	//POR HACER
	cout << "Numero de usuarios: " << endl;
	//cin >> argc;
	numUsuarios = 50;// atoi(argv[0]);

	cout << "Tiempo de reflexion/ Tiempo de peticiones: " << endl;
	//cin >> argv[1];
	tReflex = 5;// atoi(argv[1]);

	cout << "Numero de peticiones maximas: " << endl;
	//cin >> argv[1];
	numPeticiones = 10;// atoi(argv[2]);

	// Inicializar los sockets UNA sola vez en el programa

	Ini_sockets();

	// Lanza los hilos

	for (i = 0; i < numUsuarios; i++) {
		parametro[i] = i;
		handleThread[i] = CreateThread(NULL, 0, Usuario, &parametro[i], 0, NULL);
		if (handleThread[i] == NULL) {
			cerr << "Error al lanzar el hilo" << endl;
			exit(EXIT_FAILURE);
		}
	}

	// Hacer que el Thread principal espere por sus hijos

	for (i = 0; i < numUsuarios; i++)
		WaitForSingleObject(handleThread[i], INFINITE);

	// Descargar la librer�a de sockets, aqu� o donde se acabe
	// el programa

	Fin_sockets();

	// Recopilar resultados y mostrarlos a pantalla o 
	// guardarlos en disco
	//POR HACER
	FILE* archivo_texto;
	fopen_s(&archivo_texto, "prueba_inicial.txt", "w");
	fprintf(archivo_texto, "NumeroUsuario NumeroPeticiones TiempoReflexion\n");
	for (i = 0; i < numUsuarios; i++) {
		for (j = 0; j < numPeticiones; j++) {
			fprintf(archivo_texto, "%d ; %d ; %f\n", i, j, datoHilo[i].reflex[j]);
		}
	}
	fclose(archivo_texto);

	return 0;
}
