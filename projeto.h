#ifndef SALA_CIN
#define Sala_CIN 

#include <string>
using namespace std;

class salas{
private:

string codigo;
int quantidade;

//projetor, laboratorio de hardware e laboratorio de software
bool tem_projetor, eh_laboratorio_hw, eh_laboratorio_sw;

public:

// criando um construto vazio para o .h
salas() = default;

//criando os set´s para armazenar informações no private
void setcodigo(string c){codigo = c;}
void setquantidade(int q){quantidade = q;}
void settem_projetor(bool t){tem_projetor = t;}
void seteh_laboratorio_hw(bool eh){eh_laboratorio_hw = eh;}
void seteh_laboratorio_sw(bool es){eh_laboratorio_sw = es;}

//criando os get´s para acessar informações do private
string getcodigo() const {return codigo;}
int getquantidade() const {return quantidade;}
bool gettemprojetor() const {return tem_projetor;}
bool geteh_laboratorio_hw() const {return eh_laboratorio_hw;}
bool getehlaboratorio_sw() const {return eh_laboratorio_sw;}

};

#endif