#include "Manager.h"

bool Manager::validateId(int auxInt, std::vector<int> &ids) {
    if(auxInt <= 0)
        return false;
    for(int i(0); i<ids.size(); i++){
        if (ids[i] == auxInt)
            return false;  
    }
    return true;
}

void Manager::pop_front(std::vector<Process> &v) {
    if (!v.empty()) {
        v.erase(v.begin());
    }
}

void Manager::queueProcess() {
    if (!readyProcesses.empty()) {
        if (!readyProcesses[0].getResponse()) {
            // setamos su bandera para que no se vuelva a calcular el tiempo de respuesta
            readyProcesses[0].setResponse(true);
            // Tiempo de respuesta = contador global - tiempo de llegada
            readyProcesses[0].setTRes(globalCounter - readyProcesses[0].getTLL());
        }
        if (readyProcesses[0].getId() != 0){
            workingProcess = readyProcesses[0];
            workingProcess.setState(1);
            updateFrames(workingProcess, NEED_TO_UPDATE);
        }
    }
    else
        return;
    pop_front(readyProcesses);
}

void Manager::doneProcess() {
    if (workingProcess.getId()) {
        // setteamos el tiempo de finalización de cada proceso
        workingProcess.setTF(globalCounter);
        workingProcess.setTRet(workingProcess.getTF() - workingProcess.getTLL());
        workingProcess.setTE(workingProcess.getTRet() - workingProcess.getTT());
        updateFrames(workingProcess, REMOVE);
        doneProcesses.push_back(workingProcess);
        workingProcess.setId(0);
        workingProcess.setTR(1);
        if (!newProcesses.empty()) {
            // setteamos el tiempo de llegada de cada proceso nuevo
            newProcesses[0].setTLL(globalCounter);
            if (canFit(newProcesses[0])){
                updateFrames(newProcesses[0]);
                readyProcesses.push_back(newProcesses[0]);
                pop_front(newProcesses);
            }
        }
    }
}

void Manager::selectState(char character) {
    state = character;
    switch (character) {
        case 'i': // interrupción
            // solo se puede interrumpir si no esta en pausa
            // y el proceso que se esta trabajando no es nulo
            if(!pause && workingProcess.getId() != 0) {
                workingProcess.setQuantum(quantumLength);
                workingProcess.setTTB(5);
                blockedProcesses.push_back(workingProcess);
                workingProcess.setState(2);
                updateFrames(workingProcess, NEED_TO_UPDATE);
                workingProcess.setId(0);
                workingProcess.setTR(1);
                break;
            }
        case 'e': // error
            // solo se puede mandar error cuando no esta en pausa
            if(!pause) {
                doneProcess(); 
                queueProcess(); 
            }
            break;
        case 'p': case 'a': // tabla de paginas y pausa
            // levanta la bandera de pausa
            pause = true;
            // limpia la pantalla si no se esta mostrando los bcps
            if(!showBCP)
                CLEAR; 
            break;
        case 'c': // continuar
            // puede ser usado después de una 'b' o una 'p'
            pause = false;
            showBCP = false;
            CLEAR; 
            break;
        case 'n': // nuevo
            // solo se puede añadir cuando no esta en pausa
            if(!pause) 
                createNewProcess();
            break;
        case 'b': // bcp
            // levanta la bandera para mostrar bcps
            if(!pause)
                showBCP = true;
            // levanta la bandera de pausa
            pause = true;
            break;
        case 's': // suspender
            if (!pause) {
                if (!blockedProcesses.empty()){
                    suspendedProcess = blockedProcesses[0];
                    updateFrames(suspendedProcess, REMOVE);
                    pop_front(blockedProcesses);
                    suspended++;
                    suspendProcess();
                }
            }
            break;
        case 'r': // regresar
            if (!pause) {
                if (suspended != 0){
                    if (canFit(suspendedProcess)){
                        suspendedProcess.setState(2);
                        blockedProcesses.push_back(suspendedProcess);
                        updateFrames(suspendedProcess);
                        suspended--;
                    }
                }
            } 
            break;
    }
}

void Manager::suspendProcess() {
    std::fstream file;
    file.open("suspendidos.txt", std::ios::app);
    if (!file.is_open())
        std::cerr << "Error al abrir el archivo de texto" << std::endl;
    else {
        file << suspendedProcess;
        file.close();
    }
}

// actualiza el tiempo restante de todos los procesos bloqueados
void Manager::updateBlocked() {
    bool band = false;
    for (size_t i(0); i<blockedProcesses.size(); i++) {
        if (blockedProcesses[i].getTTB() == 1) {
            blockedProcesses[i].setTTB(0);
            blockedProcesses[i].setState(0);
            updateFrames(blockedProcesses[i], NEED_TO_UPDATE);
            readyProcesses.push_back(blockedProcesses[i]);
            band = true;
        }
        else
            blockedProcesses[i].setTTB(blockedProcesses[i].getTTB()-1);
    }
    if (band)
        pop_front(blockedProcesses);
}

void Manager::printData() {
    do{
        if (KBHIT())
            selectState(GETCH());
        if (!pause) {
            CLEAR;
            globalCounter++;
        }
        if (!showBCP) {
            printInProgress();
            printQueued();
            printBlocked();
            printFinished();
            printPageTable();
        }
        else
            printBCP();
        if (!pause) {
            updateBlocked();
            workingProcess.setTT(workingProcess.getTT()+1);
            workingProcess.setTR(workingProcess.getTR()-1);
            workingProcess.setQuantum(workingProcess.getQuantum()-1);
            if(workingProcess.getTR() == 0){
                doneProcess();
                queueProcess();
            }
            if (workingProcess.getQuantum() == 0){
                workingProcess.setQuantum(quantumLength);
                if (workingProcess.getId() != 0){
                    workingProcess.setState(0);
                    updateFrames(workingProcess, NEED_TO_UPDATE);
                    readyProcesses.push_back(workingProcess);
                }
                queueProcess();
            }
        }
        SLEEP(DELAY);
    }while(doneProcesses.size() != totalProcess || suspended != 0);
        CLEAR;
        std::cin.ignore();
        printQueued();
        printInProgress();
        printBlocked();
        printFinished();
        printPageTable();
}

void Manager::printPageTable() {
    int colPos = 3; 
    GOTO_XY(STATE_POS+31, colPos++);
    std::cout << "Tabla de paginas";
    GOTO_XY(STATE_POS+14, colPos);
    std::cout << "Marco";
    GOTO_XY(STATE_POS+23, colPos);
    std::cout << "espacio ocupado";
    GOTO_XY(STATE_POS+42, colPos);
    std::cout << "quien lo ocupa";
    GOTO_XY(STATE_POS+60, colPos++);
    std::cout << "estado";
    GOTO_XY(STATE_POS+14, colPos++);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    for (int i(0); i < MAX_MEM_SIZE/4; i++){
        GOTO_XY(STATE_POS+17, colPos);
        std::cout << i;
        GOTO_XY(STATE_POS+29, colPos);
        std::cout << frames[i].getOccupied() << "/4";
        GOTO_XY(STATE_POS+47, colPos); 
        std::cout << frames[i].getId();
        GOTO_XY(STATE_POS+60, colPos++); 
        std::cout << frames[i].getState();
    }

}

// lote en espera
void Manager::printQueued() {
    // imprimimos el encabezado del programa
    GOTO_XY(ON_HOLD_X_POS, 1);
        std::cout << "No. Procesos nuevos: " << newProcesses.size();
    GOTO_XY(QUEUED_X_POS-10, 1);
    std::cout << "Contador Global: " << globalCounter;
    GOTO_XY(FINISHED_X_POS-25, 1);
    std::cout << "Procesos totales: " << totalProcess;
    GOTO_XY(FINISHED_X_POS+2, 1);
    if (doneProcesses.size() != totalProcess)
        std::cout << "Estado: " << (pause?"pausado":"en ejecucion");
    else
        std::cout << "Estado: Terminado";
    GOTO_XY(STATE_POS, 1);
    std::cout << "Entrada: " << state;
    GOTO_XY(ON_HOLD_X_POS, 2);
    std::cout << "Quantum: " << quantumLength;
    if (!newProcesses.empty()){
        GOTO_XY(QUEUED_X_POS-10, 2);
        std::cout << "Nuevo (ID: " << newProcesses[0].getId() 
        << " | Peso: " << newProcesses[0].getWeight() << ")";
    }

    GOTO_XY(FINISHED_X_POS-22, 2);
    std::cout << "Suspendidos: " << suspended;

    if (suspended != 0){
        GOTO_XY(FINISHED_X_POS+2, 2);
        std::cout << "Suspendido (ID: " << suspendedProcess.getId() 
        << " | Peso: " << suspendedProcess.getWeight() << ")";
    }

    // mostramos los procesos en la cola de LISTOS
    GOTO_XY(ON_HOLD_X_POS, 4);
    if (readyProcesses.empty()) {
        std::cout << "Cola de listos" << std::endl;
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-" << std::endl;
        return;
    }
    std::cout << "Cola de listos" << std::endl; 
    for(int i(0); i<readyProcesses.size(); i++) {
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-" << std::endl
                  << "ID: " << readyProcesses[i].getId() << std::endl
                  << "TME: " << readyProcesses[i].getEMT() << std::endl
                  << "TT: " << readyProcesses[i].getTT() << std::endl;
    }
    
}

// proceso en ejecución
void Manager::printInProgress() {
    if (doneProcesses.size() == totalProcess || workingProcess.getId() == 0) {
        GOTO_XY(QUEUED_X_POS, 4);
        std::cout << "Proceso en ejecucion";
        GOTO_XY(QUEUED_X_POS, 5);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
        return;
    }
    GOTO_XY(QUEUED_X_POS, 4);
    std::cout << "Proceso en ejecucion";
    GOTO_XY(QUEUED_X_POS, 5);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    GOTO_XY(QUEUED_X_POS, 6);
    std::cout << "ID: " << workingProcess.getId();
    GOTO_XY(QUEUED_X_POS, 7);
    std::cout << "Operacion: " << workingProcess.getOperation();
    GOTO_XY(QUEUED_X_POS, 8);
    std::cout << "TME: " << workingProcess.getEMT();
    GOTO_XY(QUEUED_X_POS, 9);
    std::cout << "TT: " << workingProcess.getTT();
    GOTO_XY(QUEUED_X_POS, 10);
    std::cout << "TR: " << workingProcess.getTR();
    GOTO_XY(QUEUED_X_POS, 11);
    std::cout << "QR: " << workingProcess.getQuantum();
    GOTO_XY(QUEUED_X_POS, 12);
    std::cout << "Peso: " << workingProcess.getWeight();
}

// procesos finalizados
void Manager::printFinished() {
    int pos = 4;
    GOTO_XY(FINISHED_X_POS, pos);
    std::cout << "Procesos finalizados";
    if (doneProcesses.empty()) {
        GOTO_XY(FINISHED_X_POS, ++pos);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    }
    for(int i(0); i<doneProcesses.size(); i++) {
        GOTO_XY(FINISHED_X_POS, ++pos);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
        GOTO_XY(FINISHED_X_POS, ++pos);
        std::cout << "ID: " << doneProcesses[i].getId();
        GOTO_XY(FINISHED_X_POS, ++pos);
        std::cout << "Operacion: " << doneProcesses[i].getOperation();
        GOTO_XY(FINISHED_X_POS, ++pos);
        std::cout << "Resultado: " << doneProcesses[i].getResult();
    }
}

void Manager::printBlocked() {
    int pos = 14;
    GOTO_XY(QUEUED_X_POS, pos);
    std::cout << "Procesos bloqueados";
    if (blockedProcesses.empty()) {
        GOTO_XY(QUEUED_X_POS, ++pos);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    }
    for(size_t i(0); i<blockedProcesses.size(); i++) {    
        GOTO_XY(QUEUED_X_POS, ++pos);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
        GOTO_XY(QUEUED_X_POS, ++pos);
        std::cout << "ID: " << blockedProcesses[i].getId();
        GOTO_XY(QUEUED_X_POS, ++pos);
        std::cout << "TTB: " << blockedProcesses[i].getTTB();
    }
}

// evalua que no se intente realizar una división entre cero
bool Manager::isOperationValid(Process &auxProcess) {
    char oper = auxProcess.getOperationSymbol();
    int num2 = auxProcess.getSecondOperand();
    if ((oper == '%' || oper == '/') && num2 == 0)
        return false;
    return true;
}

void Manager::createData() {
    int processQuantity = 0;

    // pedimos la cantidad de procesos
    std::cout << "Ingrese la cantidad de procesos que desea: ";
    std::cin >> processQuantity;
    
    // pedimos el tamaño del quantum
    std::cout << "Ingrese la longitud del quantum: ";
    std::cin >> quantumLength;

    do {
        createNewProcess();
        processQuantity--;
    } while(processQuantity != 0);
}

void Manager::printFinishedStatus() {
    std::cout << "Procesos" << std::endl << std::endl;

    for (size_t i(0); i < doneProcesses.size(); i++) { 
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-" << std::endl 
                  << "ID: " << doneProcesses[i].getId() << std::endl
                  << "Operacion: " << doneProcesses[i].getOperation() << std::endl
                  << "Resultado: " << doneProcesses[i].getResult() << std::endl
                  << "Tiempo maximo estimado: " << doneProcesses[i].getEMT() << std::endl
                  << "Tiempo de servicio: " << doneProcesses[i].getTT() << std::endl
                  << "Tiempo de llegada: " << doneProcesses[i].getTLL() << std::endl
                  << "Tiempo de respuesta: " << doneProcesses[i].getTRes() << std::endl
                  << "Tiempo de finalizacion: " << doneProcesses[i].getTF() << std::endl
                  << "Tiempo de retorno: " << doneProcesses[i].getTRet() << std::endl
                  << "Tiempo de espera: " << doneProcesses[i].getTE() << std::endl
                  << "Quantum sobrante: " << doneProcesses[i].getQuantum() << std::endl;
    }
}

void Manager::createNewProcess() {
    // variables auxiliares
    Process auxProcess;
    std::string auxString = "";
    int auxInt = 0;
    // operandos disponibles
    std::string operands  = "+-*/%"; 

    // ingresamos un id valido
    auxProcess.setId(uniqueKey);
    // el id será consecutivo
    uniqueKey++;
    // aumentamos la cantidad de procesos a ejecutar
    totalProcess++;

    // ingresamos una operación valida
    bool band1 = false, band2 = false;
    do{
        auxString = "";
        auxString += std::to_string(rand()%100); // random de 0 - 99
        auxString += operands[rand()%5]; // random de 0-4
        auxString += std::to_string(rand()%100); // random de 0 - 99
        // validamos que la operación sea correcta
        band1 = auxProcess.validateOperation(auxString);
        if(band1){
            auxProcess.setOperation(auxString);
            band2 = isOperationValid(auxProcess);
        }
    }while(!band2);

    // ingresamos el tiempo máximo estimado
    auxInt = 6 + (rand() % 10); // random de 6 - 15
    auxProcess.setEMT(auxInt);
    auxProcess.setTR(auxInt);

    // ingresamos el quantum
    auxProcess.setQuantum(quantumLength);

    // generamos un peso para el proceso (5 y 25)
    auxProcess.setWeight((5 + (rand() % 20)));

    // obtenemos la cantidad de procesos que estan en memoria
    int procInMem = readyProcesses.size() + blockedProcesses.size();
    workingProcess.getId() != 0? procInMem++:procInMem;

    // si hay espacio en memoria, podemos meterlos directamente a Listos
    if (canFit(auxProcess)) {
        auxProcess.setTLL(globalCounter); 
        auxProcess.setReady(true);
        updateFrames(auxProcess);
        readyProcesses.push_back(auxProcess);       
    }
    else{
        // si la memoria ya esta llena, entonces los metemos a Nuevos
        newProcesses.push_back(auxProcess);
    }
}

void Manager::updateFrames(Process &proc, int band) {
    int auxSize = proc.getWeight();
    int ammount = 0;
    std::string state = "";
    if (proc.getState() == 0)
        state = "Listo";
    else if (proc.getState() == 1)
        state = "Ejecucion";
    else
        state = "Bloqueado";
    // si de debe remover, entonces establece los marcos como libres
    if (band == REMOVE){
        for (int i(0); i < MAX_MEM_SIZE/4; i++)
            if (frames[i].getId() == std::to_string(proc.getId()))
                frames[i] = Frame();
    }
    else{
        for (int i(0); i < MAX_MEM_SIZE/4; i++)
            if (frames[i].getId() == "Libre" && band == IS_FREE) {
                ammount = auxSize-4 >= 0? 4: auxSize;
                frames[i] = Frame(std::to_string(proc.getId()), ammount, state);
                auxSize -= 4;
                if (auxSize <= 0)
                    break;
            }
            else if (frames[i].getId() == std::to_string(proc.getId()) && band == NEED_TO_UPDATE)
                frames[i].setState(state);  
    }
}

bool Manager::canFit(Process &proc) {
    int count = 0;
    int framesNeeded = (proc.getWeight() / 4) + (proc.getWeight() % 4? 1:0);
    for (int i(0); i < MAX_MEM_SIZE/4; i++){
        if (frames[i].getId() == "Libre"){
            count++;
            if (count == framesNeeded)
                return true;
        }
    }
    return false;
}

void Manager::printBCP() {
    int colPos = 1; 
    CLEAR;
    GOTO_XY(ON_HOLD_X_POS, colPos++);
    std::cout << "Entrada: " << state;
    GOTO_XY(ON_HOLD_X_POS, colPos++);
    std::cout << "Tabla de procesos (BCP)";
    colPos++;

    // mostramos el bcp de los procesos Nuevos
    GOTO_XY(ON_HOLD_X_POS, colPos++);
    std::cout << "Nuevos";
    GOTO_XY(ON_HOLD_X_POS, colPos++);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    for(size_t i(0); i<newProcesses.size(); i++) {
        GOTO_XY(ON_HOLD_X_POS, colPos++);
        std::cout << "ID: " << newProcesses[i].getId();
        GOTO_XY(ON_HOLD_X_POS, colPos++);
        std::cout << "operacion: " << newProcesses[i].getOperation();
        GOTO_XY(ON_HOLD_X_POS, colPos++);
        std::cout << "Tiempo maximo estimado: " << newProcesses[i].getEMT() ;
        colPos++;
    }

    // mostramos el bcp de los procesos listos
    colPos = 4;
    GOTO_XY(QUEUED_X_POS-3, colPos++);
    std::cout << "Listos";
    GOTO_XY(QUEUED_X_POS-3, colPos++);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    for(size_t i(0); i<readyProcesses.size(); i++) {
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "ID: " << readyProcesses[i].getId();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "operacion: " << readyProcesses[i].getOperation();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Tiempo maximo estimado: " << readyProcesses[i].getEMT();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Tiempo de llegada: " << readyProcesses[i].getTLL();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Tiempo restante: " << readyProcesses[i].getTR();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Tiempo de servicio: " << readyProcesses[i].getTT();
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Tiempo de espera: " 
        << (globalCounter - readyProcesses[i].getTLL()) - readyProcesses[i].getTT(); 
                  

        if (readyProcesses[i].getResponse()) {
            GOTO_XY(QUEUED_X_POS-3, colPos++);
            std::cout << "Tiempo de respuesta: " << readyProcesses[i].getTRes(); 
        }
        GOTO_XY(QUEUED_X_POS-3, colPos++);
        std::cout << "Quantum restante: " << readyProcesses[i].getQuantum();
        colPos++;
    }

    // mostramos el bcp del proceso en ejecución (si aplica)
    colPos = 4;
    if(workingProcess.getId() != 0) {
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "En ejecucion";
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "ID: " << workingProcess.getId();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "operacion: " << workingProcess.getOperation();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo maximo estimado: " << workingProcess.getEMT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de llegada: " << workingProcess.getTLL();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo restante: " << workingProcess.getTR();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de servicio: " << workingProcess.getTT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de espera: " 
        << (globalCounter - workingProcess.getTLL()) - workingProcess.getTT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de respuesta: " << workingProcess.getTRes();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Quantum restante: " << workingProcess.getQuantum();
        colPos++;
    }

    // mostramos el bcp de los procesos Finalizados
    colPos = 4;
    GOTO_XY(STATE_POS+5, colPos++);
    std::cout << "Finalizados";
    GOTO_XY(STATE_POS+5, colPos++);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    for (size_t i(0); i < doneProcesses.size(); i++) { 
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "ID: " << doneProcesses[i].getId();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Operacion: " << doneProcesses[i].getOperation();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Resultado: " << doneProcesses[i].getResult();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo maximo estimado: " << doneProcesses[i].getEMT();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de servicio: " << doneProcesses[i].getTT();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de llegada: " << doneProcesses[i].getTLL();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de respuesta: " << doneProcesses[i].getTRes();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de finalizacion: " << doneProcesses[i].getTF();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de retorno: " << doneProcesses[i].getTRet();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Tiempo de espera: " << doneProcesses[i].getTE();
        GOTO_XY(STATE_POS+5, colPos++);
        std::cout << "Quantum restante: " << doneProcesses[i].getQuantum();
        colPos++;
    }

    // mostramos el bcp del proceso bloquados (si aplica)
    colPos = 17;
    GOTO_XY(FINISHED_X_POS-6, colPos++);
    std::cout << "Bloqueados";
    GOTO_XY(FINISHED_X_POS-6, colPos++);
    std::cout << ".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-";
    for (size_t i(0); i < blockedProcesses.size(); i++) {
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "ID: " << blockedProcesses[i].getId();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "operacion: " << blockedProcesses[i].getOperation();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo maximo estimado: " << blockedProcesses[i].getEMT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de llegada: " << blockedProcesses[i].getTLL();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo restante: " << blockedProcesses[i].getTR();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de servicio: " << blockedProcesses[i].getTT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de espera: " 
        << (globalCounter - blockedProcesses[i].getTLL()) - blockedProcesses[i].getTT();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo de respuesta: " << blockedProcesses[i].getTRes();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Tiempo bloqueado: " << 5 - blockedProcesses[i].getTTB();
        GOTO_XY(FINISHED_X_POS-6, colPos++);
        std::cout << "Quantum restante: " << blockedProcesses[i].getQuantum();
        colPos++;
    }

    // esperamos a que el usuario presione la tecla 'c' para continuar
    char c = ' ';
    do{
        GOTO_XY(ON_HOLD_X_POS, 1);
        std::cout << "Entrada: " << state;
        if (KBHIT()){   
            c = GETCH();
            selectState(c);
        }
    }while(c != 'c');
}