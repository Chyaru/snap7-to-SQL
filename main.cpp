#include "snap7.h"
#include "mysql_connection.h"

#include <iostream>
#include <queue>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <thread>
#include <chrono>
using namespace std;

 const int N = 9;
string columnas[N] = {
    "num_ticket",        // 0
    "num_equipo",        // 1
    "usuario",           // 2
    "tiempo_ciclo",      // 3 
    "temp_elegida",      // 4
    "hora_inicio",       // 5
    "hora_fin", // 6
    "gas_abierto", // 7   
    "natural_cancelado", // 8
};

// i -> int
// s -> string 
// ...
char tipo_de_valor[N] = {
    'i',
    'i',
    's',
    'i',
    'i',
    'd',
    'd',
    'i',
    'i'
};
int offset[N+1]={
    0, 4, 6, 262, 264, 266, 278, 290, 294, 296
};

int valoresInt[N];

string valoresString[N];


sql::Driver *driver;
sql::Connection *conexion;
sql::Statement *st;
sql::ResultSet *cod_s;
sql::ConnectOptionsMap propiedades;

bool subir(){
    try {

        driver = get_driver_instance();

        propiedades["hostName"] = "tcp://x.x.x.x:3306";
        propiedades["userName"] = "your_username";
        propiedades["password"] = "your_password";
        propiedades["OPT_CONNECT_TIMEOUT"] = 1;

        conexion = driver->connect(propiedades);
        conexion->setSchema("name1__");

        string s = "INSERT IGNORE INTO name2__ (";
        for(int i = 0; i < N ; i++){
            s += columnas[i]; 
            s += (i+1 < N ? ", " : ")");
        }
        s+= " VALUES (";
        for(int i = 0; i < N ; i++){
            s += '?';
            s += (i+1 < N ? ", " : ")");
        }
        cout<<"\033[92m" <<"Enviando Instruccion:\n"<< "\033[0m" << s <<"\n";

        sql::PreparedStatement* paquete = conexion->prepareStatement(s);
        for(int i = 0; i < N; i++){
            if(tipo_de_valor[i]=='i'){
                paquete->setInt(i+1,valoresInt[i]);
            }else{
                paquete->setString(i+1,valoresString[i]);
            }
        }
        
        paquete->executeUpdate();

//        delete cod_s;
//        delete st;
//        delete conexion;

        return 1;

    } catch (sql::SQLException &e) {
        cout << "\033[31m" << e.what() << "\033[0m" << "\n";
        return 0;
    }

}

const int M = 296;
char buffer[M];
char lstbuffer[M];
queue<vector<char>> cola_de_instrucciones;

void convertir(){
    for(int i = 0; i < M; i++) buffer[i] = cola_de_instrucciones.front().at(i);
    valoresString[2] = "";
    // usuario,la posicion 7 tiene el tamano de la cadena
    for(int i = 0; i < buffer[7]; i++){
        valoresString[2] += buffer[i+8];
    }
    for(int i = 0; i < N; i++){
        if(tipo_de_valor[i]=='i'){
            int &x = valoresInt[i];
            x = 0;
            for(int j = offset[i+1]-1, c = 0; j >= offset[i]; j--, c+=8){
                x += buffer[j]<<c;
            }
         //   cout<<x<<" ";
        }else if(tipo_de_valor[i] == 'd'){
            string &s = valoresString[i];
            s = "20";
            int j = offset[i];
            int y = (256 + buffer[j+1] + buffer[j] * 256)-2000;
            s += y/10 + '0';
            s += y%10 + '0';
            s += '-';
            y = buffer[j+2];
            s += y/10 + '0';
            s += y%10 + '0';
            s += '-';
            y = buffer[j+3];
            s += y/10 + '0';
            s += y%10 + '0';
            s += ' ';

            y = buffer[j+5];
            s += y/10 + '0';
            s += y%10 + '0';
            s += ':';
            y = buffer[j+6];
            s += y/10 + '0';
            s += y%10 + '0';
            s += ':';
            y = buffer[j+7];
            s += y/10 + '0';
            s += y%10 + '0';

            //cout<<s<<" ";
        }
    }
    return;
}


bool diferentes(){
    for(int i = 0; i < M; i++){
        if(buffer[i]!=lstbuffer[i]) 
            return true;
    }
    return false;
}
int main(int argc, char* argv[]){

    vector<char> aux(M,0);

    TS7Client Client;
    cout<< "\033[94m"<<"Hola, Inicializando...\n" << "\033[0m";

    int sz = sizeof(buffer);

    while(true){
        int cod_s = Client.ConnectTo("x.x.x.x","slot","rack");
        if(cod_s != 0) {
            cout<<"\033[31m" <<"Error\n" << "\033[0m"; 
        }else{
            cod_s = Client.DBRead(5,0x0, sz, buffer);
            if(cod_s != 0){
                cout<< "\033[31m" <<"Error DB\n" << "\033[0m";
                break;
            }
    /*        if(diferentes()){
                for(int i  = 0; i < M; i++){
                    cout<<(int)buffer[i]<<" ";
                }
                cout<<"\n";
            }*/
            if((buffer[294]|buffer[295]) > 0 &&  diferentes()){
                for(int i = 0; i < M; i++) aux[i] =lstbuffer[i]= buffer[i];
                cola_de_instrucciones.push(aux);
                cout<<"\033[95m" << "Ciclo recibido, guardando ...\n"<<"\033[0m";
            }
        }
        while(cola_de_instrucciones.size()){
            convertir();
            if(subir()==0) break;
            cola_de_instrucciones.pop();
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }

//    delete Client;

    return 0; 

}
