#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
using namespace std;

const int MAX_PRODUCTOS = 100;

struct tProducto {
	int id;
	float precio;
	int cantidad;
	string nombre;
};

struct tProductosComprados {
	int unidades;
	tProducto producto;
};

typedef tProducto tListaProductos[MAX_PRODUCTOS];

struct tProductos {
	int contador;
	tListaProductos listaProductos;
};

typedef tProductosComprados tListaProductosComprados[MAX_PRODUCTOS];

struct tCarrito {
	float precioTotal;
	tListaProductosComprados carrito;
	int contador;
};

int menu();
bool cargar(tProductos& productos);
void guardar(const tProductos& productos);
void generarFactura(tCarrito carrito);
bool codigoCorrecto(int codigo, const tProductos& productos);
bool cantidadCorrecta(int codigo, int cantidad, const tProductos& productos);
void elegirItem(tProductos& productos, tCarrito& carrito);
void comprar(int codigo, int cantidad, tProductos& productos, tCarrito& carrito);
void eliminarItem(tProductos& productos, tCarrito& carrito);
void mostrar(const tProductos& productos, const tCarrito& carrito);

int main() {
	bool ok = false;
	int opcion;
	tProductos productos;
	tCarrito carrito;
	carrito.contador = 0;
	carrito.precioTotal = 0;
	//Si se cargan bien el fichero, mostramos el menu() y un switch con las diferentes opciones.
	if (cargar(productos)) {
		do {
			mostrar(productos, carrito);
			opcion = menu();
			switch (opcion) {
			case 1: elegirItem(productos, carrito);
				break;
			case 2: eliminarItem(productos, carrito);
				break;
			case 3: guardar(productos); 
				generarFactura(carrito);
				break;
			}
		} while (opcion != 3);
	}
	else {
		cout << "No se ha podido cargar el fichero";
	}
}

int menu() {
	int opcion;
	do {
		cout << "1.Aniadir al carrito" << endl;
		cout << "2.Eliminar del carrito" << endl;
		cout << "3.Efectuar pedido y terminar" << endl;
		cout << "Elija una opcion: ";
		cin >> opcion;
	} while (opcion < 1 || opcion > 3);

	return opcion;
}

bool cargar(tProductos& productos) {
	int i = 0;
	productos.contador = 0;
	bool abierto = false;
	ifstream archivo;
	archivo.open("catalogo.txt");
	//Si el fichero se abre lo leemos hasta fin de fichero y guardamos cada dato en su campo correspondiente.
	if (archivo.is_open()) {
		abierto = true;
		while (!archivo.eof()) {
			productos.contador++;
			archivo >> productos.listaProductos[i].id >> productos.listaProductos[i].precio >> productos.listaProductos[i].cantidad;
			getline(archivo, productos.listaProductos[i].nombre);
			i++;
		}
	}
	archivo.close();
	return abierto;
}

void guardar(const tProductos& productos) {
	ofstream archivo;
	archivo.open("catalogo.txt");
	if (archivo.is_open()) {
		for (int i = 0; i < productos.contador - 1; i++) {
			archivo << productos.listaProductos[i].id << " " << productos.listaProductos[i].precio << " " << productos.listaProductos[i].cantidad << productos.listaProductos[i].nombre << endl;
		}
	}
	archivo.close();
}

void generarFactura(tCarrito carrito) {
	ofstream factura;
	factura.open("factura.txt");
	for (int i = 0; i < carrito.contador; i++) {
		factura << carrito.carrito[i].producto.nombre << " " << carrito.carrito[i].unidades * carrito.carrito[i].producto.precio << endl;
	}
	factura << "Total del pedido: " << carrito.precioTotal << endl;
	factura << "Total del pedido con IVA: " << carrito.precioTotal * 1.21;
}

void mostrar(const tProductos& productos, const tCarrito& carrito) {
	//Mostramos los productos disponibles:
	cout << "Productos disponibles" << endl;
	cout << "Codigo " << "Precio " << "Cantidad " << "Nombre" << endl;
	for (int i = 0; i < 35; i++)
		cout << "=";
	cout << endl;
	for (int j = 0; j < productos.contador - 1; j++) {
		//Si la cantidad es mayor a 0 lo mostramos entre los disponibles.
		if (productos.listaProductos[j].cantidad > 0) {
			//Con setw() establecemos el ancho que ocupa en la terminal el dato, y con right/left lo colocamos.
			cout << right << setw(3) << productos.listaProductos[j].id << " ";
			cout << right << setw(7) << productos.listaProductos[j].precio << " ";
			cout << right << setw(7) << productos.listaProductos[j].cantidad << " ";
			cout << right << setw(15) << productos.listaProductos[j].nombre << endl;
		}
	}
	//Mostramos los datos del carrito:
	cout << endl << "Estado del carrito:" << endl << endl;
	//Si el precio del carrito es mayor a 0 mostramos los datos del mismo, sino indicamos que está vacio.
	if (carrito.precioTotal > 0) {
		cout << "Codigo " << "Nombre " << setw(10) << "Precio" << endl;
		for (int i = 0; i < 25; i++)
			cout << "=";
		cout << endl;
		for (int z = 0; z < carrito.contador; z++) {
			cout << right << setw(3) << carrito.carrito[z].producto.id << " ";
			cout << right << setw(12) << carrito.carrito[z].producto.nombre << " ";
			cout << right << setw(5) << carrito.carrito[z].producto.precio * carrito.carrito[z].unidades << endl;
		}
		cout << endl << "Total del pedido: " << carrito.precioTotal << endl << endl;
	}
	else {
		cout << "Carrito vacio" << endl << endl;
	}
}

bool codigoCorrecto(int codigo, const tProductos& productos) {
	bool correcto = false;
	//Recorremos la lista de productos y si encontramos un producto con id igual al que buscamos ponemos a true correcto.
	for (int i = 0; i < productos.contador; i++) {
		if (productos.listaProductos[i].id == codigo) {
			correcto = true;
		}
	}
	return correcto;
}

bool cantidadCorrecta(int codigo, int cantidad, const tProductos& productos) {
	bool correcto = false;
	//Primero miramos si el codigo es correcto, y si lo es, comprobamos que la cantidad está disponible.
	if (codigoCorrecto(codigo, productos)) {
		if (productos.listaProductos[codigo].cantidad >= cantidad) {
			correcto = true;
		}
	}
	return correcto;
}

void elegirItem(tProductos& productos, tCarrito& carrito) {
	int id;
	int unidades;
	cout << "Que producto deseas comprar?: ";
	cin >> id;
	cout << "Cuantas unidades quiere comprar?: ";
	cin >> unidades;
	//Si los valores introducidos son validos a la hora de seleccionar el producto, llamamos a comprar().
	if (cantidadCorrecta(id, unidades, productos)) {
		comprar(id, unidades, productos, carrito);
	}
}

void comprar(int codigo, int cantidad, tProductos& productos, tCarrito& carrito) {
	//Restamos la cantidad de la lista de productos:
	productos.listaProductos[codigo].cantidad = productos.listaProductos[codigo].cantidad - cantidad;
	//Añadimos al carrito las unidades, el codigo de producto, el precio y el nombre del producto: 
	carrito.carrito[carrito.contador].unidades = cantidad;
	carrito.carrito[carrito.contador].producto.id = codigo;
	carrito.carrito[carrito.contador].producto.precio = productos.listaProductos[codigo].precio;
	carrito.carrito[carrito.contador].producto.nombre = productos.listaProductos[codigo].nombre;
	//Actualizamos el precio total del carrito y aumentamos en uno el contador:
	carrito.precioTotal = carrito.precioTotal + cantidad * productos.listaProductos[codigo].precio;	
	carrito.contador++;
	mostrar(productos, carrito);
}

void eliminarItem(tProductos& productos, tCarrito& carrito) {
	bool encontrado = false;
	int id, i = 0;
	cout << "Que producto deseas eliminar?: ";
	cin >> id;
	//Mientras que no lo encontremos o el contador del carrito se exceda buscamos si algún producto coindice con el id que queremos eliminar.
	while (!encontrado || i <= carrito.contador) {
		if (carrito.carrito[i].producto.id == id) {
			encontrado = true;
			//Añadimos de nuevo a la lista de productos los elementos del carrito que hemos eliminado:
			productos.listaProductos[id].cantidad = productos.listaProductos[id].cantidad + carrito.carrito[i].unidades;
			//Restamos el precio de los elementos seleccionados del precio final:
			carrito.precioTotal = carrito.precioTotal - carrito.carrito[i].unidades * carrito.carrito[i].producto.precio;
			carrito.carrito[i].unidades = 0;
			//Restamos 1 al contador del carrito:
			carrito.contador--;
		}
		i++;
	}

}