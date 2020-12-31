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
const int NUM_FICHAS = 4, NUM_JUGADORES = 4, NUM_CASILLAS = 68;
const string Archivo = "pruebas.txt";
const bool Debug = false;
//tipos

typedef enum tColor { Amarillo, Azul, Rojo, Verde, Gris, Ninguno };
typedef tColor tCasillas[NUM_CASILLAS];
typedef int tFichas[NUM_FICHAS];
typedef tFichas tJugadores[NUM_JUGADORES];

//prototipos
void cargar(tJugadores jugadores, tColor& jugadorTurno, tCasillas calle1, tCasillas calle2);
void mostrar(const tJugadores jugadores, const tCasillas calle1, const tCasillas calle2);
void iniciaColores();
void setColor(tColor color);
void iniciar(tJugadores jugadores, tCasillas calle1, tCasillas calle2, tColor& jugadorTurno);
void pausa();
void saleFicha(tJugadores jugadores, tColor jugadorTurno, tCasillas calle1, tCasillas calle2);
void aCasita(tJugadores jugadores, int casilla, tCasillas calle2);
void mover(tJugadores jugadores, tColor jugadorTurno, int ficha, int casilla, int& premio, int& ultimaFichaMovida, tCasillas calle1, tCasillas calle2);
void abrirPuente(tJugadores jugadores, tColor jugadorTurno, int casilla, int casilla2, int& premio, int& ultimaFichaMovida, tCasillas calle1, tCasillas calle2);

int lanzarDado();
int salidaJugador(tColor jugTurnado);
int zanataJugador(tColor jugador);
int cuantas(const tFichas jugador, int casilla);
int primeraEn(const tFichas jugador, int casilla);
int segundaEn(const tFichas jugador, int casilla);

bool esSeguro(int casilla);
bool procesa6(tJugadores jugadores, tColor jugadorTurno, int& premio, bool& pasaTurno, int& seises, int& ultimaFichaMovida, int& tirada, tCasillas calle1, tCasillas calle2);
bool jugar(tJugadores jugadores, tColor jugadorTurno, int& premio, bool& fin, int& seises, int& ultimaFichaMovida, int tirada, tCasillas calle1, tCasillas calle2);
bool todasEnMeta(const tFichas jugador);
bool puente(const tCasillas calle1, const tCasillas calle2, int casilla);
bool procesa5(tJugadores jugadores, tColor jugadorTurno, int& premio, bool& pasaTurno, tCasillas calle1, tCasillas calle2);
bool puedeMover(const tJugadores jugadores, tColor jugadorTurno, int ficha, int& casilla, int tirada, const tCasillas calle1, const tCasillas calle2);

string colorACadena(tColor color);

int main() {
    tColor jugTurnado;
    tJugadores jugadores;
    tCasillas calle1, calle2;
    int ultimaFichaMovida, premio = 0, tirada, seises = 0;
    bool finJuego = false;
    bool jugadaForzosa = false, jugada = false;
    bool pasarTurno = true;

    iniciaColores();
    iniciar(jugadores, calle1, calle2, jugTurnado);
    if (Debug) {
        cargar(jugadores, jugTurnado, calle1, calle2);
    }
    setColor(tColor(jugTurnado));
    mostrar(jugadores, calle1, calle2);
    pausa();

    while (!finJuego) {
        setColor(jugTurnado);
        cout << "Turno para el jugador " << colorACadena(jugTurnado) << endl;
        //Si hay premio, se asigna al dado y es la tirada del jugador
        if (premio > 0) {
            cout << "Ha de contar " << premio << "!" << endl;
            tirada = premio;
            premio = 0;
        }
        //Si no hay premio, se tira el dado
        else {
            tirada = lanzarDado();
            cout << "Sale un " << tirada << endl;
        }
        //Salir de la partida
        if (tirada == 0)
            finJuego = true;
        else {
            if (tirada == 5)
                jugada = procesa5(jugadores, jugTurnado, premio, pasarTurno, calle1, calle2);
            else if (tirada == 6)
                jugada = procesa6(jugadores, jugTurnado, premio, pasarTurno, seises, ultimaFichaMovida, tirada, calle1, calle2);
            if (!jugada)
                pasarTurno = jugar(jugadores, jugTurnado, premio, finJuego, seises, ultimaFichaMovida, tirada, calle1, calle2);
            if (tirada > 7 && seises > 0)
                pasarTurno = false;
        }
        jugada = false;
        //Si se pasa el turno, se reinicia premio y seises, y también se pasa de jugador
        if (pasarTurno) {
            premio = 0;
            seises = 0;
            jugTurnado = tColor((int(jugTurnado) + 1) % NUM_JUGADORES);
        }
        pausa();
        mostrar(jugadores, calle1, calle2);

        //Comprueba si están todas las fichas del jugador en meta, y si es cierto, acaba la partida
        if (todasEnMeta(jugadores[jugTurnado])) {
            finJuego = true;
            setColor(jugTurnado);
            cout << "Enhorabuena al jugador " << colorACadena(jugTurnado) << endl;
        }

    }
    pausa();
    return 0;
}


void cargar(tJugadores jugadores, tColor& jugadorTurno, tCasillas calle1, tCasillas calle2) {
    ifstream archivo;
    int jugador, casilla;

    archivo.open(Archivo);
    if (archivo.is_open()) {
        for (int i = 0; i < NUM_JUGADORES; i++)
            for (int f = 0; f < NUM_FICHAS; f++) {
                archivo >> casilla;
                jugadores[i][f] = casilla;
                if ((casilla >= 0) && (casilla < NUM_CASILLAS))
                    if (calle1[casilla] == Ninguno)
                        calle1[casilla] = tColor(i);
                    else
                        calle2[casilla] = tColor(i);
            }
        archivo >> jugador;
        jugadorTurno = tColor(jugador);
        archivo.close();
    }
}

void mostrar(const tJugadores jugadores, const tCasillas calle1, const tCasillas calle2) {
    int casilla, ficha;
    tColor jug;

    cout << "\x1b[2J\x1b[H"; // Se situa en la esquina superior izquierda
    setColor(Gris);
    cout << endl;

    // Filas con la numeracion de las casillas...
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
        setColor(calle2[i]);
        if (calle2[i] != Ninguno)
            cout << primeraEn(jugadores[calle2[i]], i) + 1;
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
        setColor(calle1[i]);
        if (calle1[i] != Ninguno)
            cout << segundaEn(jugadores[calle1[i]], i) + 1;
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
                ficha = primeraEn(jugadores[jug], 101 + i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantas(jugadores[jug], 101 + i) > 1) {
                        ficha = segundaEn(jugadores[jug], 101 + i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else cout << "V";
                    }
                    else cout << "V";
                }
                else cout << "VV";
                casilla++;
            }
            else if (casilla == salidaJugador(jug)) {
                if (jugadores[jug][i] == -1) // En casa
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
                ficha = primeraEn(jugadores[jug], i);
                if (ficha != -1) {
                    cout << ficha + 1;
                    if (cuantas(jugadores[jug], i) > 1) {
                        ficha = segundaEn(jugadores[jug], i);
                        if (ficha != -1) {
                            cout << ficha + 1;
                        }
                        else cout << "V";
                    }
                    else cout << "V";
                }
                else cout << "VV";
                casilla++;
                jug = tColor(int(jug) + 1);
                setColor(jug);
            }
            else cout << ' ';
            casilla++;
        }
        cout << endl;
    }

    casilla = 0;
    jug = Amarillo;
    setColor(jug);
    while (casilla < NUM_CASILLAS) {
        cout << ((jugadores[jug][0] == 108) ? '1' : '.');
        cout << ((jugadores[jug][1] == 108) ? '2' : '.');
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
        cout << ((jugadores[jug][2] == 108) ? '3' : '.');
        cout << ((jugadores[jug][3] == 108) ? '4' : '.');
        jug = tColor(int(jug) + 1);
        setColor(jug);
        cout << "               ";
        casilla += 17;
    }
    cout << endl << endl;
    setColor(Gris);
}

void pausa() {
    cout << "Pulsa Intro para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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

void iniciar(tJugadores jugadores, tCasillas calle1, tCasillas calle2, tColor& jugTurnado) {
    srand(time(NULL));
    for (int i = 0; i < NUM_JUGADORES; i++) {
        for (int j = 0; j < NUM_FICHAS; j++) {
            jugadores[i][j] = -1;
        }
    }
    for (int i = 0; i < NUM_CASILLAS; i++) {
        calle1[i] = Ninguno;
        calle2[i] = Ninguno;
    }

    jugTurnado = tColor(rand() % NUM_JUGADORES);

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

//saca ficha si en la calle 1 o 2 la salida no tiene color
void saleFicha(tJugadores jugadores, tColor jugTurnado, tCasillas calle1, tCasillas calle2) {
    int salida = salidaJugador(jugTurnado), ficha = primeraEn(jugadores[jugTurnado], -1);
    if (ficha != -1 && cuantas(jugadores[int(jugTurnado)], salida) < 2) {
        if (calle1[salida] != Ninguno)
            calle2[salida] = tColor(jugTurnado);
        else
            calle1[salida] = tColor(jugTurnado);
        jugadores[jugTurnado][ficha] = salida;
        cout << "Sale una ficha de casa." << endl;
    }
}

void aCasita(tJugadores jugadores, int casilla, tCasillas calle2) {
    int numJugadorComido = calle2[casilla], ficha = segundaEn(jugadores[numJugadorComido], casilla);
    jugadores[numJugadorComido][ficha] = -1;
    calle2[casilla] = Ninguno;
}

void abrirPuente(tJugadores jugadores, tColor jugadorTurno, int casilla, int casilla2, int& premio, int& ultimaFichaMovida, tCasillas calle1, tCasillas calle2) {
    //Buscamos cual es la ficha que hay en la calle2 y la movemos
    int ficha = segundaEn(jugadores[jugadorTurno], casilla);
    cout << "Se abre el puente en la casilla: " << casilla << endl;
    mover(jugadores, jugadorTurno, ficha, casilla2, premio, ultimaFichaMovida, calle1, calle2);
}

void mover(tJugadores jugadores, tColor jugadorTurno, int ficha, int casilla, int& premio, int& ultimaFichaMovida, tCasillas calle1, tCasillas calle2) {
    int posActual = jugadores[jugadorTurno][ficha];
    //Si la ficha está en la calle2, marcamos esa casilla como vacia
    if (calle2[posActual] == jugadorTurno)
        calle2[posActual] = Ninguno;
    //Si la ficha está en la calle1, movemos la de la calle2 a la 1, y marcamos la de la 2 como vacia
    else {
        calle1[posActual] = calle2[posActual];
        calle2[posActual] = Ninguno;
    }
    //Movemos la ficha a su nueva casilla
    jugadores[jugadorTurno][ficha] = casilla;
    ultimaFichaMovida = casilla;
    //Si llega a meta
    if (casilla > 100) {
        if (casilla == 108) {
            premio = 10;
            setColor(jugadorTurno);
            cout << "La ficha del jugador ha llegado a la meta" << endl;
        }
    }
    //Si está en el tablero y calle1 está vacia
    else if (calle1[casilla] == Ninguno)
        calle1[casilla] = jugadorTurno;
    else {
        //Si es un seguro o calle1 está ocupada por otro color
        if (esSeguro(casilla) || calle1[casilla] == jugadorTurno)
            calle2[casilla] = jugadorTurno;
        //Si hay una ficha de otro jugador
        else {
            calle2[casilla] = calle1[casilla];
            calle1[casilla] = jugadorTurno;
            aCasita(jugadores, casilla, calle2);
            premio = 20;
        }
    }
}

//Devuelven INT
int lanzarDado() {
    int tirada;
    if (Debug) {
        cout << "Dame tirada (0 para salir): ";
        cin >> tirada;
        cin.get(); // Saltar el Intro (para que la pausa
        // funcione bien en modo Debug)
    }
    else tirada = rand() % 6 + 1;
    return tirada;
}

int salidaJugador(tColor jugTurnado) {
    return 5 + 17 * int(jugTurnado);
}

int zanataJugador(tColor jugador) {
    return salidaJugador(jugador) - 5;
}

//calcula numero de fichas en una casilla (-1 para casa)
int cuantas(const tFichas jugador, int casilla) {
    int cont = 0;
    for (int i = 0; i < NUM_FICHAS; i++)
        if (jugador[i] == casilla)
            cont++;
    return cont;
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

//FUNCIONES BOLEANAS
bool esSeguro(int casilla) {
    bool seguro = false;
    for (int i = 0; i < NUM_JUGADORES; i++)
        if (casilla == 0 + 17 * i || casilla == 5 + 17 * i || casilla == 12 + 17 * i)
            seguro = true;
    return seguro;
}

bool puente(const tCasillas calle1, const tCasillas calle2, int casilla) {
    return calle1[casilla] == calle2[casilla] && calle1[casilla] != Ninguno;
}

bool todasEnMeta(const tFichas jugador) {
    return cuantas(jugador, 108) == 4;
}

bool procesa5(tJugadores jugadores, tColor jugTurnado, int& premio, bool& pasaTurno, tCasillas calle1, tCasillas calle2) {
    bool podidoSacar = true;
    int salida = salidaJugador(jugTurnado), ficha;
    tColor aux;

    pasaTurno = true;
    //si hay alguna en casa
    if (cuantas(jugadores[jugTurnado], -1) != 0) {
        ficha = primeraEn(jugadores[jugTurnado], -1);
        //si no hay 2 del jugador en la salida
        if (cuantas(jugadores[jugTurnado], salida) != 2)
            //si hay 2 fichas distintas
            if (calle2[salida] != Ninguno) {
                //el caso de calle1 = otro color y calle2 = color del jugador se convierte en calle1 = color del jugador y calle2 = el otro color
                if (calle2[salida] == jugTurnado && calle1[salida] != jugTurnado) {
                    aux = calle1[salida];
                    calle1[salida] = calle2[salida];
                    calle2[salida] = aux;
                }
                aCasita(jugadores, salida, calle2);
                calle2[salida] = jugTurnado;
                jugadores[jugTurnado][ficha] = salida;
                premio = 20;
                pasaTurno = false;
            }
            else saleFicha(jugadores, jugTurnado, calle1, calle2);
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

bool procesa6(tJugadores jugadores, tColor jugTurnado, int& premio, bool& pasaTurno, int& seises, int& ultimaFichaMovida, int& tirada, tCasillas calle1, tCasillas calle2) {
    bool jugado = false, sePuede1, sePuede2;
    int casillaPuente1 = 0, casillaPuente2 = 0;
    bool puente1 = false, puente2 = false;
    int i, ficha;

    seises++;
    //Si no quedan fichas en casa, tirada vale 7
    if (cuantas(jugadores[jugTurnado], -1) == 0) {
        tirada = 7;
        cout << "no tiene fichas en casa, cuenta 7!" << endl;
    }
    //Si el numero de seises es 3
    if (seises == 3) {
        //Si está en la subida a meta no hacemos nada
        if (ultimaFichaMovida > 100) {
            pasaTurno = true;
            jugado = true;
            cout << "Has sacado 3 seises seguidos... La ultima ficha movida se salva por estar en la subida a meta!" << endl;
        }
        //Si no está en la subida a meta se va a casa
        else {
            //Si la calle1 está vacia hace falta poner la ficha en la calle2 y poner como vacia la calle1
            if (calle1[ultimaFichaMovida] != Ninguno) {
                calle1[ultimaFichaMovida] = calle2[jugTurnado];
                calle2[ultimaFichaMovida] = jugTurnado;
            }
            pasaTurno = true;
            jugado = true;
            cout << "Has sacado 3 seises seguidos, a casa!" << endl;
            aCasita(jugadores, ultimaFichaMovida, calle2);
        }
    }
    //Si el número de seises es menor a 3
    else {
        pasaTurno = false;
        i = 0;
        //Buscamos el primer puente
        while (puente1 != true && i < NUM_CASILLAS) {
            if (puente(calle1, calle2, i) && calle1[i] == jugTurnado) {
                puente1 = true;
                casillaPuente1 = i;
            }
            i++;
        }
        //Si ha encontrado un puente, buscamos el segundo
        if (i < NUM_CASILLAS) {
            i = casillaPuente1 + 1;
            while (i < NUM_CASILLAS && puente2 != true) {
                if (puente(calle1, calle2, i) && calle1[i] == jugTurnado) {
                    puente2 = true;
                    casillaPuente2 = i;
                }
                i++;
            }
        }
        //Si hay un solo puente y puede moverse, lo abrimos
        if (puente1 && !puente2) {
            ficha = segundaEn(jugadores[jugTurnado], casillaPuente1);
            if (puedeMover(jugadores, jugTurnado, ficha, casillaPuente1, tirada, calle1, calle2)) {
                abrirPuente(jugadores, jugTurnado, casillaPuente1 - tirada, casillaPuente1, premio, ultimaFichaMovida, calle1, calle2);
                jugado = true;
            }
        }
        //Si hay dos puentes
        else if (puente1 && puente2) {
            sePuede1 = false;
            sePuede2 = false;

            ficha = segundaEn(jugadores[jugTurnado], casillaPuente1);
            if (puedeMover(jugadores, jugTurnado, ficha, casillaPuente1, tirada, calle1, calle2))
                sePuede1 = true;

            ficha = segundaEn(jugadores[jugTurnado], casillaPuente2);
            if (puedeMover(jugadores, jugTurnado, ficha, casillaPuente2, tirada, calle1, calle2))
                sePuede2 = true;
            //Si no se puede abrir ninguno de los dos
            if (sePuede1 && sePuede2)
                jugado = false;
            //Si se puede abrir el primero
            else if (sePuede1 && !sePuede2) {
                abrirPuente(jugadores, jugTurnado, casillaPuente1 - tirada, casillaPuente1, premio, ultimaFichaMovida, calle1, calle2);
                jugado = true;
            }
            //Si se puede abrir el segundo
            else if (sePuede2 && !sePuede1) {
                abrirPuente(jugadores, jugTurnado, casillaPuente2 - tirada, casillaPuente2, premio, ultimaFichaMovida, calle1, calle2);
                jugado = true;
            }
            //Si no puede abrirse ninguno
            else cout << "No se puede abrir ningun puente! Se ignora la tirada..." << endl;

        }
    }
    return jugado;
}

bool puedeMover(const tJugadores jugadores, tColor jugadorTurno, int ficha, int& casilla, int tirada, const tCasillas calle1, const tCasillas calle2) {
    bool puedeMover = false;
    int contMovts = 0;// contador de movimientos
    int i;
    int posJugador = jugadores[jugadorTurno][ficha];

    if (posJugador != -1) {
        i = 0;
        //Mientras que i sea distinto que la tirada, se continua
        while (i != tirada) {
            posJugador++;
            //Si la ficha del jugador pasa del final del tablero, vuelve al principio
            if (posJugador >= NUM_CASILLAS && posJugador < 100)
                posJugador %= NUM_CASILLAS;

            //Si una ficha del jugador está en las calles
            if (posJugador > -1 && posJugador < NUM_CASILLAS) {
                //si no hay 2 fichas del jugador, se ejecuta
                if (cuantas(jugadores[jugadorTurno], posJugador) != 2) {
                    //Si alguna de las dos calle están vacias, suma uno a movimiento
                    if (calle1[posJugador] == Ninguno || calle2[posJugador] == Ninguno)
                        contMovts++;
                    else if (i != tirada - 1)
                        //Si la casilla es seguro 
                        if (esSeguro(posJugador)) {
                            if (calle1[posJugador] != calle2[posJugador])
                                contMovts++;
                        }
                        else contMovts++;
                }
                //Si la ficha llega se pasa de la casilla que indica que ha llegado a su zanata
                if (posJugador == zanataJugador(jugadorTurno) + 1)
                    posJugador = 101;
            }
            //Si la ficha del jugador está en subida a meta
            else if (posJugador > 100 && posJugador < 109) {
                if (posJugador == 108) {
                    contMovts++;
                }
                //Si el jugador no tiene dos fichas en la misma casilla y no es la última tirada suma uno al contador
                else if (cuantas(jugadores[jugadorTurno], posJugador) != 2 || i != tirada - 1)
                    contMovts++;
            }
            i++;
        }
    }
    else puedeMover = false;
    //Si el contador de movimiento llega al mismo numero que el de la tirada, se asigna a casilla la posición a la que puede mover
    if (contMovts == tirada) {
        casilla = posJugador;
        puedeMover = true;
    }
    return puedeMover;
}

bool jugar(tJugadores jugadores, tColor jugTurnado, int& premio, bool& finJuego, int& seises, int& ultimaFichaMovida, int tirada, tCasillas calle1, tCasillas calle2) {
    bool puede = false, debePasar = true, fichaElegidaCorrectamente = false;
    int ficha = 0, fichasPuedeMover = 0, puedeUnaFicha;
    int fichasPosibles[4] = { 0, 0, 0, 0 };
    //vemos cuantas fichas puede mover el jugador
    for (int ficha = 0; ficha < NUM_FICHAS; ficha++) {
        int casilla = jugadores[jugTurnado][ficha];
        if (puedeMover(jugadores, jugTurnado, ficha, casilla, tirada, calle1, calle2)) {
            fichasPuedeMover++;
            puedeUnaFicha = ficha;
        }
    }
    //si no puede mover ninguna
    if (fichasPuedeMover == 0) {
        cout << "No puedes mover ninguna ficha, se pasa turno" << endl;
        puede = false;
        if (tirada == 6 || tirada == 7)
            debePasar = false;
    }
    //si puede mover una
    else if (fichasPuedeMover == 1) {
        int casilla = jugadores[jugTurnado][puedeUnaFicha];
        puedeMover(jugadores, jugTurnado, puedeUnaFicha, casilla, tirada, calle1, calle2);
        mover(jugadores, jugTurnado, puedeUnaFicha, casilla, premio, ultimaFichaMovida, calle1, calle2);
        cout << "Solo puede moverse la ficha " << puedeUnaFicha + 1 << endl;
        cout << "Premio: " << premio << endl;
        if (premio > 0)
            debePasar = false;
        if (tirada == 6 | tirada == 7)
            debePasar = false;
    }
    //si puede mover más de una
    else {
        for (int ficha = 0; ficha < NUM_FICHAS; ficha++) {
            int casilla = jugadores[jugTurnado][ficha];
            if (puedeMover(jugadores, jugTurnado, ficha, casilla, tirada, calle1, calle2)) {
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
        int casillaFicha = jugadores[jugTurnado][ficha];
        if (puedeMover(jugadores, jugTurnado, ficha, casillaFicha, tirada, calle1, calle2)) {
            mover(jugadores, jugTurnado, ficha, casillaFicha, premio, ultimaFichaMovida, calle1, calle2);
            cout << "Premio: " << premio << endl;
            if (premio > 0)
                debePasar = false;
        }
        if (tirada == 6 || tirada == 7)
            debePasar = false;
    }
    return debePasar;
}
//string
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