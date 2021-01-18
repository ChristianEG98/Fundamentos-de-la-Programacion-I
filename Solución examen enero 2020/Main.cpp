#include <iostream>
#include <string>
#include <fstream>
using namespace std;

const int MAX_ESTANTERIAS = 15;
const int MAX_LISTA = 100;

typedef enum tArea { Matematicas, Fisica, Informatica };

typedef struct {
	tArea area;
	int num_paginas;
	string titulo;
} tLibro;

typedef struct {
	tLibro libros[MAX_LISTA];
	int contador;
} tListaLibros;

typedef struct {
	tArea area;
	tListaLibros listaLibros;
	int capacidad;
	int numPaginasDisponibles;
} tEstanteria;

typedef struct {
	tEstanteria estanterias[MAX_ESTANTERIAS];
	int contador;
} tBiblioteca;

tArea strToArea(string str);
string areaToStr(tArea area);
bool cargarEstanterias(tBiblioteca& biblioteca);
bool cargarLibros(tListaLibros& listaLibros);
void borrarLibro(tListaLibros& listaLibros, int indice);
tLibro mayor(tListaLibros& listaLibros);
int buscarEstanteria(const tBiblioteca& biblioteca, tLibro libro);
void asignar(tBiblioteca& biblioteca, tListaLibros listaLibros, tListaLibros& sinColocar);
void mostrarLibros(const tListaLibros& listaLibros);
void mostrarBiblioteca(const tBiblioteca& biblioteca);

int main() {
	tBiblioteca biblioteca;
	tListaLibros listaLibros, sinColocar;
	tLibro libro;

	if (cargarEstanterias(biblioteca)) {
		for (int i = 0; i < biblioteca.contador; i++) {
			biblioteca.estanterias[i].numPaginasDisponibles = biblioteca.estanterias[i].capacidad;
			biblioteca.estanterias[i].listaLibros.contador = 0;
		}
		if (cargarLibros(listaLibros)) {
			sinColocar.contador = 0;
			asignar(biblioteca, listaLibros, sinColocar);
			mostrarBiblioteca(biblioteca);
			cout << endl << "Libros que no pudieron ser colocados: " << endl;
			mostrarLibros(sinColocar);
		}
	}

	return 0;
}

tArea strToArea(string str) {
	tArea area;
	if (str == "Mat")
		area = Matematicas;
	else if (str == "Fis")
		area = Fisica;
	else
		area = Informatica;
	return area;
}

string areaToStr(tArea area) {
	string nombreArea;
	if (area == 0)
		nombreArea = "Matematicas";
	else if (area == 1)
		nombreArea = "Fisica";
	else
		nombreArea = "Informatica";
	return nombreArea;
}

bool cargarEstanterias(tBiblioteca& biblioteca) {
	ifstream archivo;
	bool podidoAbrir = false;
	string area;

	archivo.open("biblioteca.txt");
	if (archivo.is_open()) {
		podidoAbrir = true;
		archivo >> biblioteca.contador;
		for (int i = 0; i < biblioteca.contador; i++) {
			archivo >> area >> biblioteca.estanterias[i].capacidad;
			biblioteca.estanterias[i].area = strToArea(area);
		}
	}
	archivo.close();
	return podidoAbrir;
}

bool cargarLibros(tListaLibros& listaLibros) {
	ifstream archivo;
	bool podidoAbrir = false;
	int i = 0;
	string area;

	archivo.open("libros.txt");
	if (archivo.is_open()) {
		podidoAbrir = true;
		do {
			archivo >> area >> listaLibros.libros[i].num_paginas;
			getline(archivo, listaLibros.libros[i].titulo);
			listaLibros.libros[i].area = strToArea(area);
			i++;
		} while (area != "XXX" || i >= MAX_LISTA);
	}
	listaLibros.contador = i - 1 ;
	archivo.close();
	return podidoAbrir;
}

void borrarLibro(tListaLibros& listaLibros, int indice) {
	for (indice; indice < listaLibros.contador - 1; indice++) {
		listaLibros.libros[indice] = listaLibros.libros[indice + 1];
	}
	listaLibros.contador--;
}

tLibro mayor(tListaLibros& listaLibros) {
	int indice = 0;
	tLibro libro = listaLibros.libros[0];
	for (int i = 1; i < listaLibros.contador; i++) {
		if (listaLibros.libros[i].num_paginas > libro.num_paginas) {
			libro = listaLibros.libros[i];
			indice = i;
		}
	}
	borrarLibro(listaLibros, indice);
	return libro;
}

int buscarEstanteria(const tBiblioteca& biblioteca, tLibro libro) {
	int i = 0, estanteria = -1;
	bool continuar = true;
	do {
		if (biblioteca.estanterias[i].area == libro.area && biblioteca.estanterias[i].numPaginasDisponibles >= libro.num_paginas){
			estanteria = i;
			continuar = false;
		}
		i++;
	} while (continuar && i < biblioteca.contador);

	return estanteria;
}

void asignar(tBiblioteca& biblioteca, tListaLibros listaLibros, tListaLibros& sinColocar) {
	tLibro libro;
	int estanteria;
	int contador = listaLibros.contador;
	for (int i = 0; i < contador; i++) {
		libro = mayor(listaLibros);
		estanteria = buscarEstanteria(biblioteca, libro);
		if (estanteria == -1) {
			sinColocar.libros[sinColocar.contador] = libro;
			sinColocar.contador++;
		}
		else {
			biblioteca.estanterias[estanteria].listaLibros.libros[biblioteca.estanterias[estanteria].listaLibros.contador] = libro;
			biblioteca.estanterias[estanteria].numPaginasDisponibles -= libro.num_paginas;
			biblioteca.estanterias[estanteria].listaLibros.contador++;
		}
	}
}

void mostrarLibros(const tListaLibros& listaLibros) {
	for (int i = 0; i < listaLibros.contador; i++) {
		cout << listaLibros.libros[i].titulo << " (" << listaLibros.libros[i].num_paginas << " paginas)" << endl;
	}
}

void mostrarBiblioteca(const tBiblioteca& biblioteca) {
	int cont_mat = 1, cont_inf = 1, cont_fis = 1;
	for (int i = 0; i < biblioteca.contador; i++) {
		if (biblioteca.estanterias[i].area == Matematicas) {
			cout << areaToStr(biblioteca.estanterias[i].area) << " - " << cont_mat << " (" << biblioteca.estanterias[i].capacidad << "/" << biblioteca.estanterias[i].numPaginasDisponibles << ")" << endl;
			mostrarLibros(biblioteca.estanterias[i].listaLibros);
			cont_mat++;
			cout << endl;
		}
	}
	for (int i = 0; i < biblioteca.contador; i++) {
		if (biblioteca.estanterias[i].area == Fisica) {
			cout << areaToStr(biblioteca.estanterias[i].area) << " - " << cont_fis << " (" << biblioteca.estanterias[i].capacidad << "/" << biblioteca.estanterias[i].numPaginasDisponibles << ")" << endl;
			mostrarLibros(biblioteca.estanterias[i].listaLibros);
			cont_fis++;
			cout << endl;
		}
	}
	for (int i = 0; i < biblioteca.contador; i++) {
		if (biblioteca.estanterias[i].area == Informatica) {
			cout << areaToStr(biblioteca.estanterias[i].area) << " - " << cont_inf << " (" << biblioteca.estanterias[i].capacidad << "/" << biblioteca.estanterias[i].numPaginasDisponibles << ")" << endl;
			mostrarLibros(biblioteca.estanterias[i].listaLibros);
			cont_inf++;
			cout << endl;
		}
	}
}