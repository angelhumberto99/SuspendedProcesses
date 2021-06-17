#ifndef PROCESS_H
#define PROCESS_H 

#include <string>
#include <regex>
#include <stdlib.h>

class Process{
    private:
        // miembros de la clase
        char operationSymbol;
        int secondOperand;
        int firstOperand;
        short EMT; // tiempo máximo estimado
        short id; // identificador
        short TT; // tiempo transcurrido == tiempo de servicio
        short TR; // tiempo restante
        short TTB; // tiempo transcurrido bloqueado
        short TLL; // tiempo de llegada
        short TF; // tiempo de finalización
        short TRet; // tiempo de retorno
        short TRes; // tiempo de respuesta
        short TE; // tiempo de espera
        bool ready; // bandera para obtener el tiempo de llegada
        bool response; // bandera para obtener el tiempo de respuesta
        short quantum;
        short weight; // peso del proceso
        short state;

    public:
        Process() { 
            TT = 0; 
            TTB = 0;
            ready = false;
            response = false;
            quantum = 0;
            state = 0;
        }
        ~Process() {}

        bool validateOperation(const std::string &value);

        // setters
        void setOperation(const std::string &value);
        void setEMT(short value);
        void setId(short value);
        void setTT(short value);
        void setTR(short value);
        void setTTB(short value);
        void setTLL(short value);
        void setTF(short value);
        void setTRet(short value);
        void setTRes(short value); 
        void setTE(short value); 
        void setReady(bool value);
        void setResponse(bool value);
        void setQuantum(short value);
        void setWeight(short value);
        void setState(short value);
        
        // getters
        const std::string getOperation() const;
        const char getOperationSymbol() const;
        const int getSecondOperand() const;
        const std::string getResult() const;
        const short getEMT() const;
        const short getId() const;
        const short getTT() const;
        const short getTR() const;
        const short getTTB() const;
        const short getTLL() const;
        const short getTF() const;
        const short getTRet() const;
        const short getTRes() const; 
        const short getTE() const; 
        const bool getReady() const;
        const bool getResponse() const;
        const short getQuantum() const;
        const short getWeight() const;
        const short getState() const;

        // sobrecarga de operador
        const Process& operator=(const Process &other) {
            this->operationSymbol = other.operationSymbol;
            this->secondOperand = other.secondOperand;
            this->firstOperand = other.firstOperand;
            this->EMT = other.EMT;
            this->id = other.id;
            this->TT = other.TT;
            this->TR = other.TR;
            this->TTB = other.TTB;
            this->TLL = other.TLL;
            this->TF = other.TF;
            this->TRet = other.TRet;
            this->TRes = other.TRes;
            this->TE = other.TE;
            this->ready = other.ready;
            this->response = other.response;
            this->quantum = other.quantum;
            this->weight = other.weight;
            this->state = other.state;
            return *this;
        }
};

#endif //PROCESS_H