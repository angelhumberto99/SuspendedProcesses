#include "Process.h"
#include <iostream>

bool Process::validateOperation(const std::string &value) {
    const std::regex op("-?[0-9]+[[:space:]]*[+][[:space:]]*[0-9]+");
    const std::regex op2("-?[0-9]+[[:space:]]*-[[:space:]]*[0-9]+");
    const std::regex op3("-?[0-9]+[[:space:]]*[/|%|*][[:space:]]*-?[0-9]+");
    return std::regex_match(value, op) || std::regex_match(value, op2)
        || std::regex_match(value, op3);
}

// setters
void Process::setEMT(short value) { EMT = value; }

void Process::setId(short value) { id = value; }

void Process::setTT(short value) { TT = value; }

void Process::setTR(short value) { TR = value; }

void Process::setTTB(short value) { TTB = value; }

void Process::setTLL(short value) { TLL = value; }

void Process::setTF(short value) { TF = value; }

void Process::setTRet(short value) { TRet = value; }

void Process::setTRes(short value) { TRes = value; }

void Process::setTE(short value) { TE = value; }

void Process::setReady(bool value) { ready = value; }

void Process::setResponse(bool value) { response = value; }

void Process::setOperation(const std::string &value) {
    const std::regex num("[0-9]");
    const std::regex op("[+|*|/|%]");
    const std::regex op2("-");


    std::string num1 = "", 
                num2 = "", 
                oper = "", 
                aux = "";

    int k = 0;

    if (value[0] == '-') {
        k = 1;
        num1 = "-";
    }
    for (int i(k); i < value.length(); i++) { 
        aux = value[i];
        if (std::regex_match(aux, num) && oper == "")
            num1 += aux;
        else if ((oper == "/" || oper == "%" || oper == "*") && value[i] == '-')
            num2 += "-";
        else if (std::regex_match(aux, num))
            num2 += aux;
        else if (std::regex_match(aux, op) || std::regex_match(aux, op2))
            oper = aux;
        else
            continue;
    }

    // setteamos los valores
    if (num1[0] == '-') {
        aux = num1.substr(1);
        firstOperand = (atoi(aux.c_str())) * -1;
    }
    else
        firstOperand = atoi(num1.c_str());

    if (num2[0] == '-') {
        aux = num2.substr(1);
        secondOperand = (atoi(aux.c_str())) * -1;
    }
    else
        secondOperand = atoi(num2.c_str());

    operationSymbol = oper[0];
}

void Process::setQuantum(short value) { quantum = value; }

void Process::setWeight(short value) { weight = value; }

void Process::setState(short value) {state = value; }

// getters
const char Process::getOperationSymbol() const { return operationSymbol; }

const int Process::getSecondOperand() const { return secondOperand; }

const short Process::getEMT() const { return EMT; }

const short Process::getId() const { return id; }

const short Process::getTT() const { return TT; }

const short Process::getTR() const { return TR; }

const short Process::getTTB() const { return TTB; }

const short Process::getTLL() const { return TLL; }

const short Process::getTF() const { return TF; }

const short Process::getTRet() const { return TRet; }

const short Process::getTRes() const { return TRes; }

const short Process::getTE() const { return TE; }

const bool Process::getReady() const { return ready; }

const bool Process::getResponse() const { return response; }

const std::string Process::getOperation() const { 
    return std::to_string(firstOperand) 
            + operationSymbol 
            + std::to_string(secondOperand); 
}

const std::string Process::getResult() const {
    if (TR != 0)
        return "Error";
    switch(operationSymbol) { 
        case '+': return std::to_string(firstOperand + secondOperand);
        case '-': return std::to_string(firstOperand - secondOperand);
        case '*': return std::to_string(firstOperand * secondOperand);
        case '/': return std::to_string(firstOperand / secondOperand);
        case '%': return std::to_string(firstOperand % secondOperand);
        default: std::cout << "invalid operator" << std::endl; break;
    }
}

const short Process::getQuantum() const { return quantum; }

const short Process::getWeight() const { return weight; }

const short Process::getState() const { return state; }