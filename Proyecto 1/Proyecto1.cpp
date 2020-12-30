#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
using namespace std;

//ctes
const int NUM_CASILLAS = 68;

//tipos
typedef enum tColor { rojo, verde, amarillo, azul };

//prototipos
void mostrar(int jugador1, int jugador2, int casilla1, int casilla2);
int ejecutarTurno(int jugador1, int jugador2, int casilla1, int& casilla2, int& dist_zanata1, int& dist_zanata2);
int mover(int jugador2, int casilla1, int& casilla2, int& dist_zanata1, int& dist_zanta2, int dado);
int elegirColorJugador();
int lanzarDado();
int salidaJugador(int jugador);
int zanataJugador(int jugador);
bool esSeguro(int pos);
string colorACadena(tColor color);
char letra(tColor color);

int main() {
    int jugador1, jugador2;
    int casilla1 = -1, casilla2 = -1;
    int dist_zanata1 = 63, dist_zanata2 = 63;
    bool finjuego = false;

    //eleccion de jugador
    jugador1 = elegirColorJugador(1);
    jugador2 = elegirColorJugador(2);
    while (jugador1 == jugador2) {
        cout << "Ese color esta cogido, elija otro para el segundo jugador: ";
        cin >> jugador2;
        while (jugador2 < 1 || jugador2 > 4) {
            cout << "Color invalido, elija uno valido: ";
            jugador2 = elegirColorJugador(2);
        }
    }
    mostrar(jugador1, jugador2, casilla1, casilla2);

    //eleccion aleatoria de jugador para primer turno
    srand(time(NULL));
    int primer_turno = 1 + rand() % 2;
    if (primer_turno == 2) {
        int auxiliar = jugador1;
        jugador1 = jugador2;
        jugador2 = auxiliar;
    }

    //ejecucion del juego

    while (!finjuego) {
        cout << "Turno del jugador " << colorACadena(tColor(jugador1 - 1)) << endl;
        casilla1 = ejecutarTurno(jugador1, jugador2, casilla1, casilla2, dist_zanata1, dist_zanata2);
        //Sleep(3000);
        if (casilla1 != zanataJugador(jugador1)) {
            cout << "Turno del jugador " << colorACadena(tColor(jugador2 - 1)) << endl;
            casilla2 = ejecutarTurno(jugador2, jugador1, casilla2, casilla1, dist_zanata2, dist_zanata1);
            //Sleep(3000);
        }
        if (casilla2 == zanataJugador(jugador2) || casilla1 == zanataJugador(jugador1))
            finjuego = true;
    }

    if (casilla2 == zanataJugador(jugador2))
        cout << "el ganador es el jugador " << colorACadena(tColor(jugador2 - 1)) << endl;
    else if (casilla1 == zanataJugador(jugador1))
        cout << "el ganador es el jugador " << colorACadena(tColor(jugador1 - 1)) << endl;
    system("pause");
    return 0;
}

void mostrar(int jugador1, int jugador2, int casilla1, int casilla2) {
    // Esta "función" no devuelve nada y debe ser llamada con una instrucción independiente: mostrar(...);
    // casilla1 y casilla2: número de la casilla donde se encuentra la ficha de cada jugador (-1 significa que está en casa)
    // jugador1 y jugador2: números de los jugadores que están jugando (1=Rojo, 2=Verde, 3=Amarillo y 4=Azul)
    // NUM_CASILLAS es una contante global con el número total de casilla que hay en la calle (68)
    // salidaJugador() es una función que nos dice cuál es la casilla de salida del número de jugador que se pasa
    char c;

    // Mostramos los números de las casillas...
    cout << endl << "      ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++)
        cout << pos / 10;
    cout << endl << "      ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++)
        cout << pos % 10;

    // Mostramos la fila de seguros...
    cout << endl << "      ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++) {
        if (esSeguro(pos))
            cout << "o";
        else
            cout << "-";
    }

    // Mostramos las fichas que están fuera de casa...
    cout << endl << "CALLE ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++)
        if (pos == casilla1)
            cout << letra(tColor(jugador1 - 1));
        else if (pos == casilla2)
            cout << letra(tColor(jugador2 - 1));
        else
            cout << " ";

    // Mostramos la otra fila de seguros...
    cout << endl << "      ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++)
        if (esSeguro(pos))
            cout << "o";
        else
            cout << "-";

    // Mostramos las fichas que están en casa...
    cout << endl << "CASA  ";
    for (int pos = 0; pos < NUM_CASILLAS; pos++) {
        c = ' ';
        if (pos == salidaJugador(jugador1)) {
            if (casilla1 == -1)
                c = letra(tColor(jugador1 - 1));
        }
        else if (pos == salidaJugador(jugador2))
            if (casilla2 == -1)
                c = letra(tColor(jugador2 - 1));
        cout << c;
    }
    cout << endl;

    system("pause");
}

//DEVUELVEN INT
int ejecutarTurno(int jugador1, int jugador2, int casilla1, int& casilla2, int& dist_zanata1, int& dist_zanata2) {
    // Ejecuta el turno de jugador1, distinguiendo si esta en casa o no y, si no en casa, se distingue entre si en el dado no ha salido un 6 o si.
    // En el caso de los seises, hace un buche que si mientras el numero de seises sea < 3 y el dado sea 6, vueva a ejecutarse de forma que, si en una ejecucion el numero de seises es 3,
    // el destino pasa a ser casa(-1) y se sale del bucle
    int dado = lanzarDado();
    int destino = casilla1, seises;

    //jugador en casa
    if (casilla1 == -1)
        if (dado == 5) {
            destino = salidaJugador(jugador1);
            if (casilla2 == destino)
                casilla2 = -1;
            mostrar(jugador1, jugador2, destino, casilla2);
        }
        else {
            cout << "El jugador esta en casa... no puede mover";
            mostrar(jugador1, jugador2, destino, casilla2);
        }

    //jugador fuera de casa
    else
        //movimiento normal
        if (dado != 6) {
            destino = mover(jugador2, destino, casilla2, dist_zanata1, dist_zanata2, dado);
            mostrar(jugador1, jugador2, destino, casilla2);
        }
    //secuencia seises
        else {
            seises = 1;
            destino = mover(jugador2, destino, casilla2, dist_zanata1, dist_zanata2, dado);
            mostrar(jugador1, jugador2, destino, casilla2);
            //Sleep(3000);

            while (seises < 3 && dado == 6) {
                cout << "te vuelve a tocar, jugador " << colorACadena(tColor(jugador1 - 1)) << endl;
                dado = lanzarDado();
                if (dado == 6) seises++;
                if (seises != 3) {
                    destino = mover(jugador2, destino, casilla2, dist_zanata1, dist_zanata2, dado);
                    mostrar(jugador1, jugador2, destino, casilla2);
                    //Sleep(3000);
                }
                else {
                    destino = -1;
                    dist_zanata1 = 63;
                    cout << "como el jugador " << colorACadena(tColor(jugador1 - 1)) << " ha sacado 3 seises consecutivos, se va a casa";
                    mostrar(jugador1, jugador2, destino, casilla2);
                    //Sleep(3000);
                }
            }
        }
    return destino;
}
//devuelve el destino de la casilla1, la casilla2 por si la casilla 1 la come y la diastancia(su valor inicial es 63, 68-5 que recorre la casilla1) entre la casilla 1 y su zanata 
int mover(int jugador2, int casilla1, int& casilla2, int& dist_zanata1, int& dist_zanata2, int dado) {
    int destino = casilla1;
    if (dist_zanata1 > dado - 1) {
        destino = (destino + dado) % 68;
        dist_zanata1 -= dado;

        //si la casilla2 no esta en casilla segura y casilla 1 ha movido a casilla 2, casilla1 come a casilla2(si la casilla es segura se muestra 1 unico color)
        if (!esSeguro(casilla2)) {
            if (destino == casilla2) {
                casilla2 = -1;
                dist_zanata2 = 63;
                cout << "El jugador " << colorACadena(tColor(jugador2 - 1)) << " ha sido comido" << endl;
                //si la distancia a zanata de casilla1 despues de hacer el movimiento de dado es mayor o igual a 20 se mueve
                //si no la casilla1 se queda donde esta habiendo enviado a casa a casilla2
                if (dist_zanata1 > 19) {
                    destino = (destino + 20) % 68;
                    dist_zanata1 -= 20;
                }
                else cout << "No suma 20 porque se pasa de la zanata.";
            }
        }
        else if (destino == casilla2) {
            destino = (destino - dado + 68) % 68;
            dist_zanata1 += dado;
        }
    }
    else cout << "No puede mover(se pasa de la zanata)!";
    return destino;
}

int elegirColorJugador(int numJugador) {
    int jugador1;
    if(numJugador == 1)
        cout << "Colores: rojo(1), verde(2), amarillo(3), azul(4)" << endl << "elija el color del primer jugador: ";
    else
        cout << "Colores: rojo(1), verde(2), amarillo(3), azul(4)" << endl << "elija el color del segundo jugador: ";
    cin >> jugador1;

    while (jugador1 < 1 || jugador1 > 4) {
        cout << "Color invalido, elija uno valido: ";
        cin >> jugador1;
    }
    return jugador1;
}

int lanzarDado() {
    int dado;
    cout << "dado: ";
    cin >> dado;
    /*srand(time(NULL));
    dado = 1 + rand() %6;
    cout << "Ha salido un " << dado << endl;*/
    return dado;
}

//la casilla 38 es la salida roja, verde = rojo + 17 * 1(17 es la distancia entre casillas seguras rotadas 90 grados),
//amarillo = (rojo + 17 * 2) mod 68, azul = (rojo + 17 * 3) mod 68
int salidaJugador(int jugador) {
    return (39 + 17 * (jugador - 1)) % NUM_CASILLAS;
}

int zanataJugador(int jugador) {
    return (salidaJugador(jugador) - 5);
}

//FUNCION BOLEANA
bool esSeguro(int casilla) {
    bool seguro = false;
    int i = 0;
    while (!seguro && i < 4) {
        if (casilla == (22 + 17 * i) % NUM_CASILLAS)
            seguro = true;
        if (casilla == (17 + 17 * i) % NUM_CASILLAS)
            seguro = true;
        if (casilla == 12 + 17 * i)
            seguro = true;
        i++;
    }
    //si cogemos en el tablero las casillas seguras al lado de la subida a meta azul podemos poner en funcion de ellas todas las demás(la salida  roja es salida azul + 17 * 1)
    //y lo mismo con las otras(17 es la distancia entre salidas o zantas consecutivas)
    return seguro;
}

//DEVUELVEN CARACTERES
string colorACadena(tColor color) {
    string nombre;
    if (color == rojo)
        nombre = "Rojo";
    else if (color == verde)
        nombre = "Verde";
    else if (color == amarillo)
        nombre = "Amarillo";
    else nombre = "Azul";
    return nombre;
}

char letra(tColor color) {
    char c = 'R';
    if (color == verde)
        c = 'V';
    else if (color == azul)
        c = 'A';
    else if (color == amarillo)
        c = 'M';
    return c;
}