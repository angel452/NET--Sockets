#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>

#include <iostream>       // std::cout
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <string>
#include <sstream>
#include <fstream> // to read the files
#include <unistd.h>
#include <vector>
#include <map>

#include <openssl/sha.h>

using namespace std;

// ---------------------- VARIABLES GLOBALES ------------------------
bool isOffline = false; // El Flag sera verdadero cuando el cliente se desconecte
bool isRegistered = false; // El Flag sera verdadero cuando el cliente ya este registrado
string nombreUsuario; // Nombre del usuario

// * Para el juego 3 en raya
bool isReady = false;
bool isPlayer = false;
bool isSpectator = false;
bool isMyTurn;
char mySymbol;  
bool waitingMove;
bool mainGame = true; 
string nombreGanador;
vector<vector<char>> tablero = {{'1', '2', '3'}, 
                                {'4', '5', '6'}, 
                                {'7', '8', '9'}};
map<int, string> movimientos;   // 5 -> Angel
                                // 2 -> Diana
                                // 1 -> Angel

// ---------------------- FUNCIONES AUXILIARES -----------------------
int preProcessing (char mensaje[]){
    // --> Solo verificamos la primera posicion del mensaje
    char primerCaracter = mensaje[0];

    if(primerCaracter == 'N'){
        return 1;
    }
    else if(primerCaracter == 'M'){
        return 2;
    }
    else if(primerCaracter == 'W'){
        return 3;
    }
    else if(primerCaracter == 'L'){
        return 4;
    }
    else if(primerCaracter == 'F'){
        return 5;
    }
    else if(primerCaracter == 'Q'){
        return 6;
    }
    else if(primerCaracter == 'I'){
        return 7;
    }
    else if(primerCaracter == 'J'){
        return 8;
    }
    else{
        return -1;
    }
}

void printTablero(){
    cout << endl;
    cout << "     |     |     " << endl;
    cout << "  " << tablero[0][0] << "  |  " << tablero[0][1] << "  |  " << tablero[0][2] << endl;
    cout << "_____|_____|_____" << endl;
    cout << "     |     |     " << endl;
    cout << "  " << tablero[1][0] << "  |  " << tablero[1][1] << "  |  " << tablero[1][2] << endl;
    cout << "_____|_____|_____" << endl;
    cout << "     |     |     " << endl;
    cout << "  " << tablero[2][0] << "  |  " << tablero[2][1] << "  |  " << tablero[2][2] << endl;
    cout << "     |     |     " << endl << endl;
}

void updateBoard(int position, char symbol){
    if(position == 1){
        tablero[0][0] = symbol;
    }
    else if(position == 2){
        tablero[0][1] = symbol;
    }
    else if(position == 3){
        tablero[0][2] = symbol;
    }
    else if(position == 4){
        tablero[1][0] = symbol;
    }
    else if(position == 5){
        tablero[1][1] = symbol;
    }
    else if(position == 6){
        tablero[1][2] = symbol;
    }
    else if(position == 7){
        tablero[2][0] = symbol;
    }
    else if(position == 8){
        tablero[2][1] = symbol;
    }
    else if(position == 9){
        tablero[2][2] = symbol;
    }
    else{
        cout << "Error: Posicion no valida" << endl;
    }
}

// ---------------------- THREADS -----------------------
void read_thread(int socketCLI){
    char buffer[1000];
    int n;
    string mensaje;

    while (isOffline == false){
        n = read(socketCLI, buffer, 1); // --> Leemos el primer caracter 
        if(n < 0){ perror("Error al leer(reed) del socket");}
        buffer[n] = '\0';
        mensaje.assign(buffer);

        if(mensaje[0] == '1'){
            cout << "Is already registed" << endl;
            isRegistered = false;
        }

        else if( mensaje[0] == 'M' ){ // --> Lecura de mensajes
            // --> Input: M05Angel06Buenas
            // --> Output: Angel:Buenas
            // ------------------------- Procesamiento de mensaje -----------------------
            string mensajeProcesado = "";
            n = read(socketCLI, buffer, 2); // --> Leemos el tamaño del nombre del emisor
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombre = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombre); // --> Leemos el nombre
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            mensajeProcesado = mensajeProcesado + mensaje + ":"; // Angel:

            n = read(socketCLI, buffer, 2); // --> Leemos el tamaño del mensaje
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeMensaje = stoi(mensaje);

            n = read(socketCLI, buffer, sizeMensaje); // --> Leemos el mensaje
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            mensajeProcesado = mensajeProcesado + mensaje; // --> Angel:Buenas

            //cout << "Mensaje recibido: " << endl;
            cout << mensajeProcesado << endl;
        }

        else if( mensaje[0] == 'W' ){ // --> Lectura de mensajes a todos...
            // --> Input: W05Angel10HolaATodos
            // --> Output: Angel(Difusion):HolaATodos
            // ------------------------- Procesamiento de mensaje -----------------------
            string mensajeProcesado = "";
            n = read(socketCLI, buffer, 2); // --> Leemos el tamaño del nombre del emisor
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombre = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombre); // --> Leemos el nombre
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            mensajeProcesado = mensajeProcesado + mensaje + "(Difusion):"; // Angel(Difusion):

            n = read(socketCLI, buffer, 2); // --> Leemos el tamaño del mensaje
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeMensaje = stoi(mensaje);

            n = read(socketCLI, buffer, sizeMensaje); // --> Leemos el mensaje
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            mensajeProcesado = mensajeProcesado + mensaje; // --> Angel(Difusion):HolaATodos

            //cout << "Mensaje recibido: " << endl;
            cout << mensajeProcesado << endl;
        }

        else if( mensaje[0] == 'L'){
            // --> Input: L0305Angel05Diana04Jose
            // --> Output: Numero de usuarios conectados: 3
            //             * Angel
            //             * Diana
            //             * Jose
            // ------------------------- Procesamiento de mensaje -----------------------
            string mensajeProcesado = "";
            n = read(socketCLI, buffer, 2); // --> Cantidad de usuarios
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int cantidadUsuarios = stoi(mensaje);

            cout << "Numero de usuarios conectados: " << cantidadUsuarios << endl; // --> Numero de usuarios conectados: 3
            for(int i = 0; i < cantidadUsuarios; i++){
                n = read(socketCLI, buffer, 2); 
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);
                int sizeNombre = stoi(mensaje);

                n = read(socketCLI, buffer, sizeNombre);
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);
                cout << "* " << mensaje << endl;
            }
            cout << endl;
        }

        else if( mensaje[0] == 'F' ){
            // --> Input: F05Angel05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH
            // --> Output: Angel(msg.txt):HolaComoEstanDesdeArchivo

            // ------------------------- Procesamiento de mensaje -----------------------
            string mensajeProcesado = "";
            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombre = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombre);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            
            mensajeProcesado = mensajeProcesado + mensaje + "("; // Angel:(

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeMensaje = stoi(mensaje);

            n = read(socketCLI, buffer, sizeMensaje);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string destinatario = mensaje;

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombreArchivo = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombreArchivo);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string nombreArchivo = mensaje;

            mensajeProcesado = mensajeProcesado + nombreArchivo + "):"; // Angel(msg.txt):

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeContenidoArchivo = stoi(mensaje);

            n = read(socketCLI, buffer, sizeContenidoArchivo);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string contenidoArchivo = mensaje;

            mensajeProcesado = mensajeProcesado + contenidoArchivo; // Angel(msg.txt):HolaComoEstanDesdeArchivo

            // --> LOGICA DEL HASH
            // * Calculamos el hash del contenido del archivo
            // * Guardamos el hash que nos pasaron
            // * Comparamos ambos hash
            // * Si son iguales, el archivo esta bien. Sino, el archivo esta corrupto y envia un mensaje de "Archivo corrupto"
        
            unsigned char hash[SHA256_DIGEST_LENGTH];

            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, contenidoArchivo.c_str(), contenidoArchivo.size());
            SHA256_Final(hash, &sha256);

            stringstream ss;
            for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
                ss << hex << (int)hash[i];
            }
            string hashString = ss.str();

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeHash = stoi(mensaje);

            n = read(socketCLI, buffer, sizeHash);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string hashRecibido = mensaje;

            if(hashString == hashRecibido){
                cout << mensajeProcesado << endl;
                cout << "* El contenido del archivo fue correctamente recibido sin errores" << endl;
            }
            else{
                cout << "Error: El archivo esta corrupto" << endl;
            }
        }

        else if( mensaje[0] == 'I' ){
            // --> Input: I05Angel05Diana10imagen.jpg
            // --> Output: Angel te envio un archivo llamado: imagen.jpg

            // ------------------------- Procesamiento de mensaje -----------------------
            string mensajeProcesado = "";
            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombre = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombre);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);

            mensajeProcesado = mensajeProcesado + mensaje + " te envio un archivo llamado: "; // Angel te envio un archivo llamado:

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombreDest = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombreDest);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string nombreDest = mensaje;

            n = read(socketCLI, buffer, 2);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            int sizeNombreArchivo = stoi(mensaje);

            n = read(socketCLI, buffer, sizeNombreArchivo);
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            string nombreArchivo = mensaje;
            
            mensajeProcesado = mensajeProcesado + nombreArchivo; // Angel te envio un archivo llamado: imagen.jpg

            cout << mensajeProcesado << endl;
        }

        else if( mensaje[0] == 'J' ){
            // --> Input: J...
            n = read(socketCLI, buffer, 1); // Leemos que comando del juego J es
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);

            if(mensaje[0] == 'I'){ // --> Iniciar el juego. (JI1, JI2)
                cout << "Iniciando el juego ..." << endl;
                n = read(socketCLI, buffer, 1); // Leemos si es el turno del jugador
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);

                if(mensaje[0] == '1'){
                    isMyTurn = true;
                    mySymbol = 'X';
                }
                else if(mensaje[0] == '2'){
                    isMyTurn = false;
                    mySymbol = 'Y';
                }
                else{
                    cout << "ERROR??" << endl;
                }

                isReady = true;
                isPlayer = true;
            }
            
            else if(mensaje[0] == 'E'){ // --> Espectador del juego (JE)
                cout << "Modo espectador" << endl;
                isReady = true;
                isSpectator = true;
            }

            else if(mensaje[0] == 'M'){ // --> Nuevo movimiento y persona (JM05Angel2X)
                // --> Leemos el nombre del jugador que jugo y la jugada que hizo y su simbolo
                string nombreJugador, jugada;
                char simbolo;

                n = read(socketCLI, buffer, 2); // Leemos el tamaño del nombre
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);
                int sizeNombre = stoi(mensaje);

                n = read(socketCLI, buffer, sizeNombre); // Leemos el nombre
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);

                nombreJugador = mensaje;

                n = read(socketCLI, buffer, 1); // Leemos la jugada
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);

                jugada = mensaje;

                n = read(socketCLI, buffer, 1); // Leemos el simbolo
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
            
                simbolo = buffer[0];

                // --> Guardamos la jugada en el mapa
                movimientos[stoi(jugada)] = nombreJugador;

                // --> Actualizamos el tablero
                updateBoard(stoi(jugada), simbolo);

                // --> Cambiamos de turno
                if( isMyTurn == true ){
                    isMyTurn = false;
                }
                else{
                    isMyTurn = true;
                }

                // --> Romper el ciclo de espera
                waitingMove = false;
            }
        
            else if(mensaje[0] == 'G'){ // --> Ya hay un ganador (JG05Angel2X)
                // --> Leemos el nombre del jugador que gano, la jugada que hizo y su simbolo
                string jugada;
                char simbolo;

                n = read(socketCLI, buffer, 2); // Leemos el tamaño del nombre
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);
                int sizeNombre = stoi(mensaje);

                n = read(socketCLI, buffer, sizeNombre); // Leemos el nombre
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);

                nombreGanador = mensaje;

                n = read(socketCLI, buffer, 1); // Leemos la jugada
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';
                mensaje.assign(buffer);

                jugada = mensaje;

                n = read(socketCLI, buffer, 1); // Leemos el simbolo
                if(n < 0){ perror("Error al leer(reed) del socket");}
                buffer[n] = '\0';

                simbolo = buffer[0];

                // --> Guardamos la jugada en el mapa
                movimientos[stoi(jugada)] = nombreGanador;

                // --> Actualizamos el tablero
                updateBoard(stoi(jugada), simbolo);

                // --> Cancelamos el juego
                mainGame = false;
                waitingMove = false;
            }
        }

        else{
            //cout << "Error: Protocolo no valido (Defoult)" << endl;
            cout << mensaje;

            n = read(socketCLI, buffer, 100); // --> Leemos el tamaño del mensaje
            if(n < 0){ perror("Error al leer(reed) del socket");}
            buffer[n] = '\0';
            mensaje.assign(buffer);
            cout << mensaje << endl;
        }
    }
    cout << "Sesion terminada" << endl;
    shutdown(socketCLI, SHUT_RDWR);
    close(socketCLI);
}

void writte_thread(int socketCLI){
    char mensaje[256];
    int n;  // --> For the socket

    while (isOffline == false){
        cout << endl;
        cin.getline(mensaje, sizeof(mensaje));
        int protocolNumber = preProcessing(mensaje);

        if(protocolNumber == 1){ // --> REGISTRO
            if (isRegistered == false){ 
                cout << "Registrando usuario..." << endl;

                // ------------------ Procesando el mensaje ----------------------
                // --> Input: N:Angel
                // --> Output: N05Angel

                string auxString(mensaje);
                string mensajeProcesado = "";
                
                // --> Outuput: N05Angel
                mensajeProcesado = mensajeProcesado + "N"; // N

                int sizeNombre = auxString.size() - 2; // Solo el size del nombre (Angel)
                if (sizeNombre < 10){
                    mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre);
                }
                else{
                    mensajeProcesado = mensajeProcesado + to_string(sizeNombre);
                }

                nombreUsuario = auxString.substr(2, sizeNombre); // Angel
                mensajeProcesado = mensajeProcesado + nombreUsuario; // N05Angel

                // --> Convertimos a char[]
                char newMenssage[mensajeProcesado.size() + 1];
                strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
                // ----------------------------------------------------------------

                isRegistered = true;
                n = write(socketCLI, newMenssage, strlen(newMenssage));
                if(n < 0){ perror("Error al escribir en el socket");}

                cout << "Bienvenido " << nombreUsuario << endl << endl;
            }
            else{
                // Para que el usuario no se vuelva a registrar una vez que ya lo hizo
                cout << "Ya te registraste... xd :)" << endl << endl;
            }
        }
        
        else if( protocolNumber == 2 ){ // --> MENSAJE
            // ------------------ Procesando el mensaje ----------------------
            // --> Input: MDiana:Buenas
            // --> Output: M05Diana06Buenas

            string auxString(mensaje);
            string mensajeProcesado = "";

            mensajeProcesado = mensajeProcesado + "M"; // M

            int separatorSimbol = auxString.find(":");
            int sizeNombre = separatorSimbol - 1;
            if(sizeNombre < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre); // M05
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeNombre); // M05
            }

            mensajeProcesado = mensajeProcesado + auxString.substr(1, sizeNombre); // M05Diana

            int sizeMensaje = auxString.size() - separatorSimbol - 1; 
            if(sizeMensaje < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeMensaje); // M05Diana06
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeMensaje); // M05Diana06
            }

            mensajeProcesado = mensajeProcesado + auxString.substr(separatorSimbol + 1, sizeMensaje); // M05Diana06Buenas

            //cout << "Mensaje procesado: " << mensajeProcesado << endl;
            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}
        }

        else if( protocolNumber == 3){ // --> MENSAJE A TODOS
            // ------------------ Procesando el mensaje ----------------------
            // --> Input: W:HolaATodos
            // --> Output: W10HolaATodos

            string auxString(mensaje);
            string mensajeProcesado = "";

            // --> Output: W04Hola
            mensajeProcesado = mensajeProcesado + "W"; // W
            int sizeMensaje = auxString.size() - 2;

            if(sizeMensaje < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeMensaje); // W10
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeMensaje); // W10
            }

            mensajeProcesado = mensajeProcesado + auxString.substr(2, sizeMensaje); // W10HolaATodos

            cout << "Mensaje procesado: " << mensajeProcesado << endl;

            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}
        }
        
        else if( protocolNumber == 4 ){ // --> LISTAR USUARIOS
            // ------------------ Procesando el mensaje ----------------------
            // --> Input: L
            // --> Output: L00
            
            cout << "Listando usuarios..." << endl;
            
            string auxString(mensaje);
            string mensajeProcesado = "";

            mensajeProcesado = mensajeProcesado + "L00"; // L00

            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}
        }
        
        else if(protocolNumber == 5){ // --> ENVIAR ARCHIVO
            // ------------------ Procesando el mensaje ----------------------
            // --> Input: FDiana:msg.txt
            // --> Output: F05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH

            string auxString(mensaje);
            string mensajeProcesado = "";

            // --> Verificamos si el archivo existe
            int separatorSimbol = auxString.find(":") - 1; // posicion del separador
            string nombreArchivo = auxString.substr(separatorSimbol + 2, auxString.size() - separatorSimbol - 2); // msg.txt
            ifstream file(nombreArchivo);
            if(file.good()){
                mensajeProcesado = mensajeProcesado + "F"; // F

                int sizeNombre = separatorSimbol; 
                if(sizeNombre < 10){
                    mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre); // F05
                }
                else{
                    mensajeProcesado = mensajeProcesado + to_string(sizeNombre); // F05
                }

                mensajeProcesado = mensajeProcesado + auxString.substr(1, sizeNombre); // F05Diana

                int sizeArchivo = auxString.size() - separatorSimbol - 2; // 7
                if(sizeArchivo < 10){
                    mensajeProcesado = mensajeProcesado + "0" + to_string(sizeArchivo); // F05Diana07
                }
                else{
                    mensajeProcesado = mensajeProcesado + to_string(sizeArchivo); // F05Diana07
                }

                mensajeProcesado = mensajeProcesado + auxString.substr(separatorSimbol + 2, sizeArchivo); // F05Diana07msg.txt

                // -- Leemos el archivo
                string line, contenidoArchivo = "";
                int size_file = 0;
                // * Tenemos que leer caracter por caracter
                while(getline(file, line)){
                    contenidoArchivo = contenidoArchivo + line;
                    size_file = line.size() + size_file;
                }

                if(size_file < 10){
                    mensajeProcesado = mensajeProcesado + "0" + to_string(size_file); // F05Diana07msg.txt25
                }
                else{
                    mensajeProcesado = mensajeProcesado + to_string(size_file); // F05Diana07msg.txt25
                }

                mensajeProcesado = mensajeProcesado + contenidoArchivo; // F05Diana07msg.txt25holaComoEstanDesdeArchivo

                // --> LOGICA DEL HASH
                // * Leemos el output y lo transformamos a un hash
                // * Enviamos todo el mensaje al servidor + el hash al final del mensaje
                // * El servidor recive el mensaje y se lo envia al destinatario
                // * El destinatario recive el mensaje y vuelve a calcular el hash
                // * Comprueba que ambos hash sean iguales (el que envio el emisor y el que calculo el destinatario)
                // * Si son iguales, el archivo esta bien. Sino, el archivo esta corrupto y envia un mensaje de "Archivo corrupto"

                unsigned char hash[SHA256_DIGEST_LENGTH];

                SHA256_CTX sha256;
                SHA256_Init(&sha256);
                SHA256_Update(&sha256, contenidoArchivo.c_str(), contenidoArchivo.size());
                SHA256_Final(hash, &sha256);

                stringstream ss;
                for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
                    ss << hex << (int)hash[i];
                }
                string hashString = ss.str();

                mensajeProcesado = mensajeProcesado + to_string(hashString.size()) + hashString; // F05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH

                //cout << "Mensaje procesado: " << endl << mensajeProcesado << endl;
                char newMenssage[mensajeProcesado.size() + 1];
                strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
                n = write(socketCLI, newMenssage, strlen(newMenssage));
                if(n < 0){ perror("Error al escribir(write) en el socket");}
            }
            else{
                cout << "Error: El archivo no existe" << endl;
                continue;
            }
        }

        else if( protocolNumber == 6 ){ // --> SALIR DE LA SESION
            cout << "Saliendo de la sesion..." << endl;

            // ------------------ Procesando el mensaje ----------------------
            string auxString(mensaje); // String: Q
            string mensajeProcesado = "";

            // --> Output: Q00
            mensajeProcesado = mensajeProcesado + "Q00"; // Q00

            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}

            isOffline = true;
        }

        else if( protocolNumber == 7){ // --> ENVIAR IMAGEN
            // LOGICA:
            // * Trasformar la imagen en su formato binario 
            // * Enviar el mensaje al servidor
            // * El servidor recibe el mensaje y lo envia al destinatario
            // * El destinatario recibe el mensaje y lo transforma nuevamente a imagen

            // ------------------ Procesando el mensaje ----------------------
            // --> Input: IDiana:imagen.jpg o IDiana:imagen.png
            // --> Output: I05Diana10imagen.jpg

            string auxString(mensaje);
            string mensajeProcesado = "";

            int separatorSimbol = auxString.find(":") - 1;
            string nombreArchivo = auxString.substr(separatorSimbol + 2, auxString.size() - separatorSimbol - 2); // imagen.jpg

            mensajeProcesado = mensajeProcesado + "I"; // I

            int sizeNombre = separatorSimbol;
            if(sizeNombre < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre); // I05
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeNombre); // I05
            }

            mensajeProcesado = mensajeProcesado + auxString.substr(1, sizeNombre); // I05Diana

            int sizeArchivo = auxString.size() - separatorSimbol - 2; // 10
            if(sizeArchivo < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeArchivo); // I05Diana10
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeArchivo); // I05Diana10
            }

            mensajeProcesado = mensajeProcesado + auxString.substr(separatorSimbol + 2, sizeArchivo); // I05Diana10imagen.jpg

            cout << "Mensaje procesado: " << mensajeProcesado << endl; // I05Diana10imagen.jpg

            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}
        }
        
        else if( protocolNumber == 8){ // --> JUGAR 3 EN RAYA
            // ------------------ Procesando el mensaje ----------------------
            // 1. Envia sus datos para registrarse al juego
            // --> Input: J
            // --> Output: JN05Angel     

            string auxString(mensaje);
            string mensajeProcesado = "";

            mensajeProcesado = mensajeProcesado + "JN"; // JN --> J por el protocolo y N por "nuevo jugador"

            int sizeNombre = nombreUsuario.size();
            if(sizeNombre < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre); // JN05
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(sizeNombre); // JN05
            }

            mensajeProcesado = mensajeProcesado + nombreUsuario; // JN05Angel
            cout << "Esperando opontente..." << endl;

            char newMenssage[mensajeProcesado.size() + 1];
            strcpy(newMenssage, mensajeProcesado.c_str()); // Convertimos a char array
            n = write(socketCLI, newMenssage, strlen(newMenssage));
            if(n < 0){ perror("Error al escribir(write) en el socket");}

            // 2. Espera a que se unan los 2 jugadores
            while(isReady == false){}

            // 3. Inicia el juego
            while(mainGame){
                // 4. Verifica si es un jugador o un espectador
                if(isPlayer){

                    printTablero();
                    waitingMove = true;

                    if(isMyTurn){
                        cout << "Ingresa la posicion a jugar: ";
                        cin.getline(mensaje, sizeof(mensaje));

                        // 4.1 Enviamos jugada al servidor
                        int posicion = stoi(mensaje);
                        string nuevoMovimiento;

                        if(nombreUsuario.size() < 10){ // * JM (J por el protocolo y M por "nuevo movimiento")
                            nuevoMovimiento = nuevoMovimiento + "JM0" + to_string(nombreUsuario.size()) + nombreUsuario + to_string(posicion) + mySymbol; // JM05Angel2X
                        }
                        else{
                            nuevoMovimiento = nuevoMovimiento + "JM" + to_string(nombreUsuario.size()) + nombreUsuario + to_string(posicion) + mySymbol; // JM05Angel2X
                        }

                        cout << "Nuevo movimiento: " << nuevoMovimiento << endl;

                        char newMenssage[nuevoMovimiento.size() + 1];
                        strcpy(newMenssage, nuevoMovimiento.c_str());
                        n = write(socketCLI, newMenssage, strlen(newMenssage));
                        if(n < 0){ perror("Error al escribir(write) en el socket");}

                        while(waitingMove == true){} // Esperamos a que el servidor nos envie el movimiento, se actualice el tablero y cambie turnos
                    }
                    else{ 
                        cout << "Esperando a que el otro jugador juegue..." << endl;
                        while(waitingMove == true){} // Esperamos a que el servidor nos envie el movimiento, se actualice el tablero y cambie turnos
                    }
                }
                else if(isSpectator){
                    cout << "Espectando" << endl;
                    printTablero();
                    sleep(1);
                }
            }
            
            if(nombreUsuario == nombreGanador){
                std::cout << "\n\n";
                std::cout << "                   .-=========-.\n";
                std::cout << "                   \\'-=======-'/\n";
                std::cout << "                   _|    .=.  |_\n";
                std::cout << "                  ((|  {{1}}  |))\n";
                std::cout << "                   \\|   /|\\   |/\n";
                std::cout << "                    \\__ '`' __/\n";
                std::cout << "                      _`) (`_\n";
                std::cout << "                    _/_______\\_\n";
                std::cout << "                   /___________\\\n\n";


                std::cout << "             You are the WINNER!!! \\O_0/\n";
            }
            cout << endl << "Fin del juego" << endl;
        }

        else{
            cout << "Error: Protocolo no valido" << endl;
        }
    }

    cout << "Sesion terminada" << endl;
    shutdown(socketCLI, SHUT_RDWR);
    close(socketCLI);
}

int main(void)
{
    //--------------------- Configuracion de la INFORMACION DEL SOCKET ---------------------
    // --> Mediante una estructura, le indicamos al socket que puerto, IP y protocolo vamos a usar
    struct sockaddr_in stSockAddr; // Estructura

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(50001);

    // --------------------- Configuracion del Socket ---------------------
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Verificar si es correcto el SoketFD
    if (-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    // --------------------- Configuracion de la IP ---------------------
    // --> Convertimos la IP de string a binario y la guardamos en la estructura
    int ip_string; 
    ip_string = inet_pton(AF_INET, "127.0.0.0", &stSockAddr.sin_addr); //192.168.1.33

    if (0 > ip_string){ // Verificacion address family
        perror("Error: Address Family no validos (primer parametro))");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    else if (0 == ip_string){ // Verificacion de la IP
        perror("char string: IP no valida (segundo parametro)");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    // #################################################################################################
    // --> Establecemos la conexion con el servidor con puerto y IP especificados en la estructura
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    // ############################# LOGICA DEL CLIENTE ###############################################
    // --> Llamamos 2 threads para leer y escribir en el socket
    // --> Para mantener la terminal del cliente, creamos un bucle infinito hasta que el mismo cliente salga de sesion con el protocolo Q
    // ###############################################################################################
    
    cout << "Instrucciones: " << endl;
    cout << "*** Es importante que sigas la sintaxis de los comandos ***" << endl;
    cout << " - Registrar tu usuario................................. N:<nombre>" << endl;
    cout << " - Enviar un mensaje a un usuario....................... M<nombre>:<mensaje>" << endl;
    cout << " - Enviar un mensaje a todos los usuarios............... W:<mensaje>" << endl;
    cout << " - Listar todos los usuarios............................ L" << endl;
    cout << " - Enviar contenido de un ARCHIVO a un usuario.......... F<nombre>:<nombre_archivo>" << endl;
    cout << " - Enviar una IMAGEN a un usuario (NO DISPONIBLE)....... I<nombre>:<nombre_archivo>" << endl;
    cout << " - Jugar la 3 en raya................................... J" << endl;
    cout << " - Salir de la sesion................................... Q" << endl;
    cout << endl;
    cout << "*** Primero usa el comando REGISTRO para iniciar" << endl;

    std::thread ( read_thread, SocketFD ).detach();
    std::thread ( writte_thread, SocketFD ).detach();

    while( isOffline == false ){}

    return 0;
}