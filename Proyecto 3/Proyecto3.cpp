#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits>	//Para pausa()
#include <Windows.h>
#include <cstdlib>
#ifdef _WIN32	//Se define	automáticamente	en Visual Studio
#include <windows.h>
#undef max //Para poder usar max() en pausa()
#endif
using namespace std;

//ctes
const int NUM_FICHAS = 4, NUM_JUGADORES = 4, NUM_CASILLAS = 68, NUM_TIRADAS_DEBUG = 20;
const string Archivo = "pruebas.txt";
const bool Debug = false;

//tipos
typedef enum tColor { Amarillo, Azul, Rojo, Verde, Gris, Ninguno };
typedef int tFichas[NUM_FICHAS];

//struct para cada jugador
typedef struct {
    tColor color;
    tFichas fichas;
} tJugador;
typedef tJugador tJugadores[NUM_JUGADORES];

//struct para las casillas
typedef struct {
    tColor calle1;
    tColor calle2;
}tCasilla;
typedef tCasilla tCasillas[NUM_CASILLAS];

//struct que encapsula la información del juego
typedef struct {
    tJugadores jugadores;
    tCasillas casillas;
    tColor jugadorTurno;
    int tirada;
    int premio;
    int seises;
    int ultimaFichaMovida;
} tJuego;

//prototipos de funciones
void mostrar(const tJuego& juego);
void iniciaColores();
void setColor(tColor color);
void iniciar(tJuego& juego);
void pausa();
int salidaJugador(tColor jugTurnado);
int zanataJugador(tColor color);
int primeraEn(const tFichas jugador, int casilla);
bool esSeguro(int casilla);
int segundaEn(const tFichas jugador, int casilla);
int cuantasEn(const tFichas jugador, int casilla);
int lanzarDado();
void cargar(tJuego& juego, int tiradasDebug[], int& j);
string colorACadena(tColor color);
bool procesa5(tJuego& juego, bool& pasaTurno);
void saleFicha(tJuego& juego);
void aCasita(tJuego& juego, int casilla);
bool todasEnMeta(const tFichas jugador);
bool procesa6(tJuego& juego, bool& pasaTurno);
bool puente(const tJuego& juego, int casilla);
void abrirPuente(tJuego& juego, int casilla, int casilla2);
void mover(tJuego& juego, int ficha, int casilla);
bool puedeMover(const tJuego& juego, int ficha, int& casilla);
bool jugar(tJuego& juego, bool& fin);

int main() {
    tJuego juego;
    bool finJuego = false, jugada = false, pasarTurno = true;
    int tiradasDebug[NUM_TIRADAS_DEBUG], numTiradasDebug = 0, i = 0;
    juego.premio = 0;
    juego.seises = 0;
    iniciaColores();
    iniciar(juego);
    if (Debug) {
        cargar(juego, tiradasDebug, numTiradasDebug);
    }
    setColor(tColor(juego.jugadorTurno));
    mostrar(juego);
    while (!finJuego) {
        setColor(juego.jugadorTurno);
        cout << "Turno para el jugador " << colorACadena(juego.jugadorTurno) << endl;
        //Si hay premio, se asigna al dado y es la tirada del jugador
        if (juego.premio > 0) {
            cout << "Ha de contar " << juego.premio << "!" << endl;
            juego.tirada = juego.premio;
            juego.premio = 0;
        }
        //Si no hay premio, se tira el dado
        else {
            //Si ha leido alguna tirada desde el fichero de Debug y esta es distinta de -1 y menor al numero de tiradas en el fichero
            if (tiradasDebug[i] != -1 && i < numTiradasDebug) {
                juego.tirada = tiradasDebug[i];
                i++;
            }
            else {
                juego.tirada = lanzarDado();
            }
            cout << "Sale un " << juego.tirada << endl;
        }
        //Salir de la partida
        if (juego.tirada == 0)
            finJuego = true;
        else {
            if (juego.tirada == 5)
                jugada = procesa5(juego, pasarTurno);
            else if (juego.tirada == 6)
                jugada = procesa6(juego, pasarTurno);
            if (!jugada)
                pasarTurno = jugar(juego, finJuego);
            if (juego.premio > 7 && juego.seises > 0)
                pasarTurno = false;
        }
        jugada = false;
        //Si se pasa el turno, se reinicia premio y seises, y también se pasa de jugador
        if (pasarTurno) {
            juego.premio = 0;
            juego.seises = 0;
            juego.jugadorTurno = tColor((int(juego.jugadorTurno) + 1) % NUM_JUGADORES);
        }
        //Comprueba si están todas las fichas del jugador en meta, y si es cierto, acaba la partida
        if (todasEnMeta(juego.jugadores[juego.jugadorTurno].fichas)) {
            finJuego = true;
            setColor(juego.jugadorTurno);
            cout << endl << "Enhorabuena al jugador " << colorACadena(juego.jugadorTurno) << endl;
        }
        else {
            pausa();
            mostrar(juego);
        }
    }
    pausa();
    return 0;
}

void mostrar(const tJuego& juego) {
    int casilla, ficha;
    tColor jug;

    cout << "\x1b[2J\x1b[H"; // Se situa en la esquina superior izquierda
    setColor(Gris);
    cout << endl;

    // Filas con la numeraci�n de las casillas...
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << i / 10;
    cout << endl;
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << i % 10;
    cout << endl;

    // Borde superior...
    for (int i = 0; i < NUM_CASILLAS; i++)
        cout << '>';
    cout << endl;

    // Primera fila de posiciones de fichas...
    for (int i = 0; i < NUM_CASILLAS; i++) {
        setColor(juego.casillas[i].calle2);
        if (juego.casillas[i].calle2 != Ninguno)
            cout << primeraEn(juego.jugadores[juego.casillas[i].calle2].fichas, i) + 1;
        else
            cout << ' ';
        setColor(Gris);
    }
    cout << endl;

    // "Mediana"   
    for (int i = 0; i < NUM_CASILLAS; i++)
        if (esSeguro(i))
            cout << 'o';
        else
            cout << '-';
    cout << endl;

    // Segunda fila de posiciones de fichas...
    for (int i = 0; i < NUM_CASILLAS; i++) {
        setColor(juego.casillas[i].calle1);
        if (juego.casillas[i].calle1 != Ninguno)
            cout << segundaEn(juego.jugadores[juego.casillas[i].calle1].fichas, i) + 1;
        else
            cout << ' ';
        setColor(Gris);
    }
    cout << endl;

    jug = Amarillo;
    // Borde inferior...
    for (int i = 0; i < NUM_CASILLAS; i++)
        if (i == zanataJugador(jug)) {
            setColor(jug);
            cout << "V";
            setColor(Gris);
        }
        else if (i == salidaJugador(jug)) {
            setColor(jug);
            cout << "^";
            setColor(Gris);
            jug = tColor(int(jug) + 1);
        }
        else
            cout << '>';
    cout << endl;

    // Metas y casas...
    for (int i = 0; i < NUM_FICHAS; i++) {
        casilla = 0;
        jug = Amarillo;
        setColor(jug);
        while (casilla < NUM_CASILLAS) {
            if (casilla == zanataJugador(jug)) {
                ficha = primeraEn(juego.jugadores[jug].fichas, 101 + i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantasEn(juego.jugadores[jug].fichas, 101 + i) > 1) {
                        ficha = segundaEn(juego.jugadores[jug].fichas, 101 + i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                casilla++;
            }
            else if (casilla == salidaJugador(jug)) {
                if (juego.jugadores[jug].fichas[i] == -1) // En casa
                    cout << i + 1;
                else
                    cout << "^";
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            casilla++;
        }
        cout << endl;
    }

    // Resto de metas...
    for (int i = 105; i <= 107; i++) {
        casilla = 0;
        jug = Amarillo;
        setColor(jug);
        while (casilla < NUM_CASILLAS) {
            if (casilla == zanataJugador(jug)) {
                ficha = primeraEn(juego.jugadores[jug].fichas, i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantasEn(juego.jugadores[jug].fichas, i) > 1) {
                        ficha = segundaEn(juego.jugadores[jug].fichas, i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else
                            cout << "V";
                    }
                    else
                        cout << "V";
                }
                else
                    cout << "VV";
                casilla++;
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else
                cout << ' ';
            casilla++;
        }
        cout << endl;
    }

    casilla = 0;
    jug = Amarillo;
    setColor(jug);
    while (casilla < NUM_CASILLAS) {
        cout << ((juego.jugadores[jug].fichas[0] == 108) ? '1' : '.');
        cout << ((juego.jugadores[jug].fichas[1] == 108) ? '2' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        casilla += 17;
    }
    cout << endl;
    casilla = 0;
    jug = Amarillo;
    setColor(jug);
    while (casilla < NUM_CASILLAS) {
        cout << ((juego.jugadores[jug].fichas[2] == 108) ? '3' : '.');
        cout << ((juego.jugadores[jug].fichas[3] == 108) ? '4' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        casilla += 17;
    }
    cout << endl << endl;
    setColor(Gris);
}

void iniciaColores() {
#ifdef _WIN32
    for (DWORD stream : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        DWORD mode;
        HANDLE handle = GetStdHandle(stream);

        if (GetConsoleMode(handle, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);
        }
    }
#endif
}

void setColor(tColor color) {
    if (color == Amarillo)
        cout << "\x1b[33;107m";
    else if (color == Azul)
        cout << "\x1b[34;107m";
    else if (color == Verde)
        cout << "\x1b[32;107m";
    else if (color == Rojo)
        cout << "\x1b[31;107m";
    else
        cout << "\x1b[90;107m";
}

void iniciar(tJuego& juego) {
    srand(time(NULL));
    for (int i = 0; i < NUM_JUGADORES; i++) {
        for (int j = 0; j < NUM_FICHAS; j++) {
            juego.jugadores[i].fichas[j] = -1;
        }
    }
    for (int i = 0; i < NUM_CASILLAS; i++) {
        juego.casillas[i].calle1 = Ninguno;
        juego.casillas[i].calle2 = Ninguno;
    }

    juego.jugadorTurno = tColor(rand() % NUM_JUGADORES);

    setColor(Gris);
}

void pausa() {
    cout << "Pulsa Intro para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int salidaJugador(tColor color) {
    return 5 + 17 * int(color);
}

int zanataJugador(tColor color) {
    return salidaJugador(color) - 5;
}

//si no encuentra ninguna posicion que contenga casilla, devuelve -1 como posicion
int primeraEn(const tFichas jugador, int casilla) {
    int i = 0, pos = -1;
    while (i < NUM_FICHAS && jugador[i] != casilla) {
        if (jugador[i] == casilla)
            pos = i;
        i++;
    }
    if (i < NUM_FICHAS && jugador[i] == casilla)
        pos = i;
    return pos;
}

bool esSeguro(int casilla) {
    bool seguro = false;
    for (int i = 0; i < NUM_JUGADORES; i++)
        if (casilla == 0 + 17 * i || casilla == 5 + 17 * i || casilla == 12 + 17 * i)
            seguro = true;
    return seguro;
}

int segundaEn(const tFichas jugador, int casilla) {
    int segunda = primeraEn(jugador, casilla), i = segunda + 1;
    if (segunda > -1) {
        while (i < NUM_FICHAS && jugador[i] != casilla) {
            if (jugador[i] == casilla)
                segunda = i;
            i++;
        }
        if (i < NUM_FICHAS && jugador[i] == casilla)
            segunda = i;
    }
    return segunda;
}

//calcula numero de fichas en una casilla (-1 para casa)
int cuantasEn(const tFichas jugador, int casilla) {
    int cont = 0;
    for (int i = 0; i < NUM_FICHAS; i++)
        if (jugador[i] == casilla)
            cont++;
    return cont;
}

int lanzarDado() {
    int tirada;
    if (Debug) {
        cout << "Dame tirada (0 para salir): ";
        cin >> tirada;
        cin.get();
    }
    else tirada = rand() % 6 + 1;
    return tirada;
}

void cargar(tJuego& juego, int tiradas[], int& j) {
    ifstream archivo;
    int jugador, casilla, tirada;

    archivo.open(Archivo);
    if (archivo.is_open()) {
        for (int i = 0; i < NUM_JUGADORES; i++)
            for (int f = 0; f < NUM_FICHAS; f++) {
                archivo >> casilla;
                juego.jugadores[i].fichas[f] = casilla;
                if ((casilla >= 0) && (casilla < NUM_CASILLAS))
                    if (juego.casillas[casilla].calle1 == Ninguno)
                        juego.casillas[casilla].calle1 = tColor(i);
                    else
                        juego.casillas[casilla].calle2 = tColor(i);
            }
        archivo >> jugador;
        juego.jugadorTurno = tColor(jugador);
        do {
            archivo >> tirada;
            tiradas[j] = tirada;
            j++;
        } while (tirada != -1 || j == NUM_TIRADAS_DEBUG);

        archivo.close();
    }
}

string colorACadena(tColor color) {
    string nombre;
    switch (color) {
    case Amarillo: nombre = "Amarillo"; break;
    case Azul: nombre = "Azul"; break;
    case Rojo: nombre = "Rojo"; break;
    case Verde: nombre = "Verde"; break;
    }
    return nombre;
}

bool procesa5(tJuego& juego, bool& pasaTurno) {
    bool podidoSacar = true;
    int salida = salidaJugador(juego.jugadorTurno), ficha;
    tColor aux;
    pasaTurno = true;
    //si hay alguna en casa
    if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, -1) != 0) {
        ficha = primeraEn(juego.jugadores[juego.jugadorTurno].fichas, -1);
        //si no hay 2 del jugador en la salida
        if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, salida) != 2)
            //si hay 2 fichas distintas
            if (juego.casillas[salida].calle2 != Ninguno) {
                //el caso de calle1 = otro color y calle2 = color del jugador se convierte en calle1 = color del jugador y calle2 = el otro color
                if (juego.casillas[salida].calle2 == juego.jugadorTurno && juego.casillas[salida].calle1 != juego.jugadorTurno) {
                    aux = juego.casillas[salida].calle1;
                    juego.casillas[salida].calle1 = juego.casillas[salida].calle2;
                    juego.casillas[salida].calle2 = aux;
                }
                aCasita(juego, salida);
                juego.casillas[salida].calle2 = juego.jugadorTurno;
                juego.jugadores[juego.jugadorTurno].fichas[ficha] = salida;
                juego.premio = 20;
                pasaTurno = false;                
            }
            else saleFicha(juego);
        else {
            cout << "No puedes sacar ficha, tienes tus casillas de salida ocupadas." << endl;
            podidoSacar = false;
        }
    }
    else {
        cout << "No tienes fichas en casa" << endl;
        podidoSacar = false;
    }
    return podidoSacar;
}

void saleFicha(tJuego& juego) {
    int salida = salidaJugador(juego.jugadorTurno), ficha = primeraEn(juego.jugadores[juego.jugadorTurno].fichas, -1);
    if (ficha != -1 && cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, salida) < 2) {
        if (juego.casillas[salida].calle1 != Ninguno)
            juego.casillas[salida].calle2 = tColor(juego.jugadorTurno);
        else
            juego.casillas[salida].calle1 = tColor(juego.jugadorTurno);
        juego.jugadores[juego.jugadorTurno].fichas[ficha] = salida;
        cout << "Sale una ficha de casa." << endl;
    }
}

void aCasita(tJuego& juego, int casilla) {
    int numJugadorComido = juego.casillas[casilla].calle2, ficha = segundaEn(juego.jugadores[numJugadorComido].fichas, casilla);
    juego.jugadores[numJugadorComido].fichas[ficha] = -1;
    juego.casillas[casilla].calle2 = Ninguno;
    cout << "Una ficha se ha ido a casa" << endl;
}

bool todasEnMeta(const tFichas jugador) {
    return(cuantasEn(jugador, 108) == 4);
}

bool procesa6(tJuego& juego, bool& pasaTurno) {
    bool jugado = false, sePuede1, sePuede2;
    int casillaPuente1 = 0, casillaPuente2 = 0;
    bool puente1 = false, puente2 = false;
    int i, ficha;

    juego.seises++;
    //Si no quedan fichas en casa, tirada vale 7
    if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, -1) == 0) {
        juego.tirada = 7;
        cout << "No tiene fichas en casa, cuenta 7!" << endl;
    }
    //Si el numero de seises es 3
    if (juego.seises == 3) {
        //Si está en la subida a meta no hacemos nada
        if (juego.ultimaFichaMovida > 100) {
            pasaTurno = true;
            jugado = true;
            cout << "Has sacado 3 seises seguidos... La ultima ficha movida se salva por estar en la subida a meta!" << endl;
        }
        //Si no está en la subida a meta se va a casa
        else {
            //Si la calle1 está vacia hace falta poner la ficha en la calle2 y poner como vacia la calle1
            if (juego.casillas[juego.ultimaFichaMovida].calle1 != Ninguno) {
                juego.casillas[juego.ultimaFichaMovida].calle1 = juego.casillas[juego.jugadorTurno].calle2;
                juego.casillas[juego.ultimaFichaMovida].calle2 = juego.jugadorTurno;
            }
            pasaTurno = true;
            jugado = true;
            cout << "Has sacado 3 seises seguidos. ";
            aCasita(juego, juego.ultimaFichaMovida);
        }
    }
    //Si el número de seises es menor a 3
    else {
        pasaTurno = false;
        i = 0;
        //Buscamos el primer puente
        while (puente1 != true && i < NUM_CASILLAS) {
            if (puente(juego, i) && juego.casillas[i].calle1 == juego.jugadorTurno) {
                puente1 = true;
                casillaPuente1 = i;
            }
            i++;
        }
        //Si ha encontrado un puente, buscamos el segundo
        if (i < NUM_CASILLAS) {
            i = casillaPuente1 + 1;
            while (i < NUM_CASILLAS && puente2 != true) {
                if (puente(juego, i) && juego.casillas[i].calle1 == juego.jugadorTurno) {
                    puente2 = true;
                    casillaPuente2 = i;
                }
                i++;
            }
        }
        //Si hay un solo puente y puede moverse, lo abrimos
        if (puente1 && !puente2) {
            ficha = segundaEn(juego.jugadores[juego.jugadorTurno].fichas, casillaPuente1);
            if (puedeMover(juego, ficha, casillaPuente1)) {
                abrirPuente(juego, casillaPuente1 - juego.tirada, casillaPuente1);
                jugado = true;
            }
        }
        //Si hay dos puentes
        else if (puente1 && puente2) {
            sePuede1 = false;
            sePuede2 = false;

            ficha = segundaEn(juego.jugadores[juego.jugadorTurno].fichas, casillaPuente1);
            if (puedeMover(juego, ficha, casillaPuente1))
                sePuede1 = true;

            ficha = segundaEn(juego.jugadores[juego.jugadorTurno].fichas, casillaPuente2);
            if (puedeMover(juego, ficha, casillaPuente2))
                sePuede2 = true;
            //Si no se puede abrir ninguno de los dos
            if (sePuede1 && sePuede2)
                jugado = false;
            //Si se puede abrir el primero
            else if (sePuede1 && !sePuede2) {
                abrirPuente(juego, casillaPuente1 - juego.tirada, casillaPuente1);
                jugado = true;
            }
            //Si se puede abrir el segundo
            else if (sePuede2 && !sePuede1) {
                abrirPuente(juego, casillaPuente2 - juego.tirada, casillaPuente2);
                jugado = true;
            }
            //Si no puede abrirse ninguno
            else cout << "No se puede abrir ningun puente! Se ignora la tirada..." << endl;

        }
    }
    return jugado;
}

bool puente(const tJuego& juego, int casilla) {
    return juego.casillas[casilla].calle1 == juego.casillas[casilla].calle2 && juego.casillas[casilla].calle1 != Ninguno;
}

void abrirPuente(tJuego& juego, int casilla, int casilla2) {
    //Buscamos cual es la ficha que hay en la calle2 y la movemos
    int ficha = segundaEn(juego.jugadores[juego.jugadorTurno].fichas, casilla);
    cout << "Se abre el puente en la casilla: " << casilla << endl;
    mover(juego, ficha, casilla2);
}

void mover(tJuego& juego, int ficha, int casilla) {
    int posActual = juego.jugadores[juego.jugadorTurno].fichas[ficha];
    //Si la ficha está en la calle2, marcamos esa casilla como vacia
    if (juego.casillas[posActual].calle2 == juego.jugadorTurno)
        juego.casillas[posActual].calle2 = Ninguno;
    //Si la ficha está en la calle1, movemos la de la calle2 a la 1, y marcamos la de la 2 como vacia
    else {
        juego.casillas[posActual].calle1 = juego.casillas[posActual].calle2;
        juego.casillas[posActual].calle2 = Ninguno;
    }
    //Movemos la ficha a su nueva casilla
    juego.jugadores[juego.jugadorTurno].fichas[ficha] = casilla;
    juego.ultimaFichaMovida = casilla;
    //Si llega a meta
    if (casilla > 100) {
        if (casilla == 108) {
            juego.premio = 10;
            setColor(juego.jugadorTurno);
            cout << "La ficha del jugador ha llegado a la meta" << endl;
        }
    }
    //Si está en el tablero y calle1 está vacia
    else if (juego.casillas[casilla].calle1 == Ninguno)
        juego.casillas[casilla].calle1 = juego.jugadorTurno;
    else {
        //Si es un seguro o calle1 está ocupada por otro color
        if (esSeguro(casilla) || juego.casillas[casilla].calle1 == juego.jugadorTurno)
            juego.casillas[casilla].calle2 = juego.jugadorTurno;
        //Si hay una ficha de otro jugador
        else {
            juego.casillas[casilla].calle2 = juego.casillas[casilla].calle1;
            juego.casillas[casilla].calle1 = juego.jugadorTurno;
            aCasita(juego, casilla);
            juego.premio = 20;
        }
    }
}

bool puedeMover(const tJuego& juego, int ficha, int& casilla) {
    bool puedeMover = false;
    int contMovts = 0;// contador de movimientos
    int i;
    int posJugador = juego.jugadores[juego.jugadorTurno].fichas[ficha];

    if (posJugador != -1) {
        i = 0;
        //Mientras que i sea distinto que la tirada, se continua
        while (i != juego.tirada) {
            posJugador++;
            //Si la ficha del jugador pasa del final del tablero, vuelve al principio
            if (posJugador >= NUM_CASILLAS && posJugador < 100)
                posJugador %= NUM_CASILLAS;

            //Si una ficha del jugador está en las calles
            if (posJugador > -1 && posJugador < NUM_CASILLAS) {
                //si no hay 2 fichas del jugador, se ejecuta
                if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, posJugador) != 2) {
                    //Si alguna de las dos calle están vacias, suma uno a movimiento
                    if (juego.casillas[posJugador].calle1 == Ninguno || juego.casillas[posJugador].calle2 == Ninguno)
                        contMovts++;
                    else if (i != juego.tirada - 1)
                        //Si la casilla es seguro 
                        if (esSeguro(posJugador)) {
                            if (juego.casillas[posJugador].calle1 != juego.casillas[posJugador].calle2)
                                contMovts++;
                        }
                        else contMovts++;
                }
                //Si la ficha llega se pasa de la casilla que indica que ha llegado a su zanata
                if (posJugador == zanataJugador(juego.jugadorTurno) + 1)
                    posJugador = 101;
            }
            //Si la ficha del jugador está en subida a meta
            else if (posJugador > 100 && posJugador < 109) {
                if (posJugador == 108) {
                    contMovts++;
                }
                //Si el jugador no tiene dos fichas en la misma casilla y no es la última tirada suma uno al contador
                else if (cuantasEn(juego.jugadores[juego.jugadorTurno].fichas, posJugador) != 2 || i != juego.tirada - 1)
                    contMovts++;
            }
            i++;
        }
    }
    else puedeMover = false;
    //Si el contador de movimiento llega al mismo numero que el de la tirada, se asigna a casilla la posición a la que puede mover
    if (contMovts == juego.tirada) {
        casilla = posJugador;
        puedeMover = true;
    }
    return puedeMover;
}

bool jugar(tJuego& juego, bool& finJuego) {
    bool puede = false, debePasar = true, fichaElegidaCorrectamente = false;
    int ficha = 0, fichasPuedeMover = 0, puedeUnaFicha;
    int fichasPosibles[4] = { 0, 0, 0, 0 };
    //vemos cuantas fichas puede mover el jugador
    for (int ficha = 0; ficha < NUM_FICHAS; ficha++) {
        int casilla = juego.jugadores[juego.jugadorTurno].fichas[ficha];
        if (puedeMover(juego, ficha, casilla)) {
            fichasPuedeMover++;
            puedeUnaFicha = ficha;
        }
    }
    //si no puede mover ninguna
    if (fichasPuedeMover == 0) {
        cout << "No puedes mover ninguna ficha, se pasa turno" << endl;
        puede = false;
        if (juego.tirada == 6 || juego.tirada == 7)
            debePasar = false;
    }
    //si puede mover una
    else if (fichasPuedeMover == 1) {
        int casilla = juego.jugadores[juego.jugadorTurno].fichas[puedeUnaFicha];
        puedeMover(juego, puedeUnaFicha, casilla);
        mover(juego, puedeUnaFicha, casilla);
        cout << "Solo puede moverse la ficha " << puedeUnaFicha + 1 << endl;
        cout << "Premio: " << juego.premio << endl;
        if (juego.premio > 0)
            debePasar = false;
        if (juego.tirada == 6 || juego.tirada == 7)
            debePasar = false;
    }
    //si puede mover más de una
    else {
        for (int ficha = 0; ficha < NUM_FICHAS; ficha++) {
            int casilla = juego.jugadores[juego.jugadorTurno].fichas[ficha];
            if (puedeMover(juego, ficha, casilla)) {
                fichasPosibles[ficha] = 1;
                cout << ficha + 1 << ": " << "Puede ir a la casilla " << casilla << endl;
                puede = true;
            }
        }
    }

    if (puede) {
        do {
            cout << "Ficha a mover: ";
            cin >> ficha;
            cin.get();
            ficha--;
            if (fichasPosibles[ficha] == 1) {
                fichaElegidaCorrectamente = true;
            }
            else
                cout << "Ficha no disponible, elige de nuevo" << endl;
        } while (!fichaElegidaCorrectamente);
        int casillaFicha = juego.jugadores[juego.jugadorTurno].fichas[ficha];
        if (puedeMover(juego, ficha, casillaFicha)) {
            mover(juego, ficha, casillaFicha);
            cout << "Premio: " << juego.premio << endl;
            if (juego.premio > 0)
                debePasar = false;
        }
        if (juego.tirada == 6 || juego.tirada == 7)
            debePasar = false;
    }
    return debePasar;
}