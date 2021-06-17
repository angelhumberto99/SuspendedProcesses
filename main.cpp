#include <iostream>
#include "Manager.h"

int main() {
    Manager manager;
    CLEAR; // limpiamos pantalla
    //manager.getData(); // pedimos los datos
    manager.createData();
    CLEAR; // limpiamos pantalla

    HIDE_CURSOR; // escondemos el cursor
    manager.printData(); // mostramos los datos
    SHOW_CURSOR; // mostramos el cursor
    
    // esperamos la entrada de una tecla para terminar el programa
    std::cin.get();
    CLEAR;
    manager.printFinishedStatus();
    std::cin.get();
    CLEAR;
    return 0;
}
