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

#include <vector>
#include <map>
#include <sys/stat.h>
#include <ctime>
#include <cstdlib>

#include <string>
#include <sstream>
#include <fstream> // to read the files
#include <filesystem>

using namespace std;

// ------------------------- Variables globales -------------------------
map<string, int> mapOfUsers;

map<string, int> mapOfPlayers;
map<string, int> mapOfSpectators;
int numJugadores = 0;
bool isInGame = false;
map<int, string> movimientos;   // 5 -> Angel
                                // 2 -> Diana
                                // 1 -> Angel

// ------------------------ Funciones auxiliares ------------------------
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

bool checkWinner() {
    // Combinaciones ganadoras
    int combinacionesGanadoras[8][3] = {
        {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, // Filas
        {1, 4, 7}, {2, 5, 8}, {3, 6, 9}, // Columnas
        {1, 5, 9}, {3, 5, 7}             // Diagonales
    };

    for (int i = 0; i < 8; i++) {
        int a = combinacionesGanadoras[i][0];
        int b = combinacionesGanadoras[i][1];
        int c = combinacionesGanadoras[i][2];

        if( movimientos.find(a) != movimientos.end() && 
            movimientos.find(b) != movimientos.end() && 
            movimientos.find(c) != movimientos.end() ){
                //cout << "ENTRO" << endl;
                if( movimientos[a] == movimientos[b] && movimientos[b] == movimientos[c]){
                    string nombreGanador = movimientos[a];
                    cout << "--> Protocolo de juego (J) - Ganador" << endl;
                    cout << "!" << nombreGanador << " ha ganado!" << endl;
                    return true;
                }
            }
        else{
            //cout << "NO ENTRO" << endl;
        }
    }

    return false; // No hay ganador todavía
}

// ----------------------- Threads para el chat -----------------------

void read_thread(int socketCLI){
    char buffer[1000];
    int n; // --> Para el socket
    string mensajeAux;
    bool isRegistered = false;
    bool quitChat = false;
    string nombreUsuario;

    //bool sendImage = false;
    //string nombreDestImage;


    // Bucle para el registro de usuarios
    while(isRegistered == false && quitChat == false){
        n = read(socketCLI, buffer, 1); // Para leer SOLO el protocolo (N)
        if( n < 0){perror("Error en el read");}
        buffer[n] = '\0'; 

        int protocolNumber = preProcessing(buffer);

        if( protocolNumber == 1 ){ // --> REGISTRO
            // --> Input: N05Angel

            cout << "--> Protocolo de registro (N)" << endl;

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "N"; // N
            n = read(socketCLI, buffer, 2); // Leemos el largo del nombre.
            if( n < 0){perror("Error en el read");}
            buffer[n] = '\0';
            int sizeNombre = stoi(buffer); // Convertimos el largo del nombre a int

            mensajeProcesado = mensajeProcesado + buffer; // N05

            n = read(socketCLI, buffer, sizeNombre); // Leemos el nombre
            if( n < 0){perror("Error en el read");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer); // Guardamos el nombre en un string

            mensajeProcesado = mensajeProcesado + mensajeAux; // N05Angel

            // -----------------------------------------------------------

            // --> Registramos en el mapa
            if( mapOfUsers.empty() ){
                mapOfUsers[mensajeAux] = socketCLI;
                cout << "Nuevo nombre: " << mensajeAux  << " - " << mensajeProcesado << endl;
                nombreUsuario = mensajeAux;

                isRegistered = true;

                // --> Le creamos al usuario un directorio personal
                const char *nombreCarpeta = mensajeAux.c_str();
                if(mkdir (nombreCarpeta, 0755) == -1){
                    cerr << "Error al crear la carpeta:  " << strerror(errno) << endl;
                }
                else{
                    cout << "Su directorio personal ha sido creado" << endl;
                }
            }
            else{
                // --> Verificamos si el nombre ya existe
                if( mapOfUsers.find(mensajeAux) == mapOfUsers.end() ){ // --> No existe
                    mapOfUsers[mensajeAux] = socketCLI;
                    cout << "Nuevo nombre: " << mensajeAux  << " - " << mensajeProcesado << endl;
                    nombreUsuario = mensajeAux;

                    isRegistered = true;

                    // --> Le creamos al usuario un directorio personal
                    const char *nombreCarpeta = mensajeAux.c_str();
                    if(mkdir (nombreCarpeta, 0755) == -1){
                        cerr << "Error al crear la carpeta:  " << strerror(errno) << endl;
                    }
                    else{
                        cout << "Su directorio personal ha sido creado" << endl;
                    }
                }
                else{ // --> Si existe
                    char alertMSG[1000] = "El nombre de usuario ya existe";
                    n = write(socketCLI, alertMSG, strlen(alertMSG));
                    if( n < 0){perror("Error en el write");}
                }
            }
            cout << endl;
        }

        else if ( protocolNumber == 6 ){ // --> QUIT
            quitChat = true;            
        }

        else{
            char alertMSG[50] = "Error en el protocolo - Registrate primero";
            n = write(socketCLI, alertMSG, strlen(alertMSG));
            if( n < 0){perror("Error en el write");}
        }
    }

    // --> Bucle para el chat
    while( quitChat == false ){        
        n = read(socketCLI, buffer, 1); // Para leer SOLO el protocolo (M, W, L, F, Q, I, J)
        if( n < 0){perror("Error en el read");}
        buffer[n] = '\0';

        int protocolNumber = preProcessing(buffer);

        if( protocolNumber == 2 ){ // --> MENSAJE
            // --> Input: M05Diana06Buenas
            // --> Output: M05Angel06Buenas

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "M"; // M

            if ( nombreUsuario.length() < 10 ){
                mensajeProcesado += "0" + to_string(nombreUsuario.size());  // M05
            }
            else{
                mensajeProcesado += to_string(nombreUsuario.size()); // M05
            }

            mensajeProcesado = mensajeProcesado + nombreUsuario; // M05Angel

            n = read(socketCLI, buffer, 2); // Leemos el largo del destinatario.
            if( n < 0){perror("Error en el read");}
            buffer[n] = '\0';
            int sizeDestinatario = stoi(buffer); 

            n = read(socketCLI, buffer, sizeDestinatario); // Leemos el destinatario
            if( n < 0){perror("Error en el read");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer); 
            string nombreDest = mensajeAux;

            if( mapOfUsers.find(mensajeAux) != mapOfUsers.end() ){ // --> Si existe el destinatario
                n = read(socketCLI, buffer, 2); // Leemos el largo del mensaje.
                if( n < 0){perror("Error en el read");}
                buffer[n] = '\0';
                mensajeAux.assign(buffer);

                mensajeProcesado = mensajeProcesado + mensajeAux; // M05Angel06

                int sizeMensaje = stoi(mensajeAux);
                n = read(socketCLI, buffer, sizeMensaje); // Leemos el mensaje
                if( n < 0){perror("Error en el read");}
                buffer[n] = '\0';
                mensajeAux.assign(buffer); // Guardamos el mensaje en un string

                mensajeProcesado = mensajeProcesado + mensajeAux; // M05Angel06Buenas
                // -----------------------------------------------------------

                // --> Enviamos el mensaje al destinatario
                cout << "--> Protocolo de mensaje (M)" << endl;
                cout << "Mensaje enviado de " << nombreUsuario << " a " << nombreDest << " es: " << mensajeAux << endl;
                cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;
                n = write(mapOfUsers[nombreDest], mensajeProcesado.c_str(), mensajeProcesado.length());
                if( n < 0){perror("Error en el write");}

            }
            else{ // --> No existe el destinatario
                cout << "El destinatario no existe" << endl;
                char alertMSG[100] = "El destinatario no existe";
                n = write(socketCLI, alertMSG, strlen(alertMSG));
                if(n < 0){perror("Error en el write");}
            }
        }

        else if ( protocolNumber == 3 ){ // --> MENSAJE A TODOS
            // --> Input: W10HolaATodos
            // --> Output: W05Angel10HolaATodos

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "W"; // W

            if(nombreUsuario.length() < 10){
                mensajeProcesado += "0" + to_string(nombreUsuario.size());  // W05
            }
            else{
                mensajeProcesado += to_string(nombreUsuario.size()); // W05
            }

            mensajeProcesado = mensajeProcesado + nombreUsuario; // W05Angel

            n = read(socketCLI, buffer, 2);
            buffer[n] = '\0';
            int sizeMensaje = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // W05Angel10

            n = read(socketCLI, buffer, sizeMensaje);
            buffer[n] = '\0';
            mensajeAux.assign(buffer);

            mensajeProcesado = mensajeProcesado + mensajeAux; // W05Angel10HolaATodos

            cout << "Protocolo de mensaje a todos (W)" << endl;
            cout << "Mensaje enviado de " << nombreUsuario << " a TODOS es: " << mensajeAux << endl;
            cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;

            map<string, int>::iterator itr;
            for (itr = mapOfUsers.begin(); itr != mapOfUsers.end(); ++itr) { 
                n = write(mapOfUsers[itr->first], mensajeProcesado.c_str(), mensajeProcesado.length());
            }
        }

        else if( protocolNumber == 4 ){ // --> LISTA DE USUARIOS
            // --> Input: L00
            // --> Output: L0305Angel05Diana04Jose

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "L"; // L
            if( mapOfUsers.size() < 10){
                mensajeProcesado = mensajeProcesado + "0" + to_string(mapOfUsers.size()); // L03
            }
            else{
                mensajeProcesado = mensajeProcesado + to_string(mapOfUsers.size()); // L03
            }

            map<string, int>::iterator itr;
            for (itr = mapOfUsers.begin(); itr != mapOfUsers.end(); ++itr) { 
                int sizeNombre = itr->first.size();
                if( sizeNombre < 10){
                    mensajeProcesado = mensajeProcesado + "0" + to_string(sizeNombre); // L0305
                }
                else{
                    mensajeProcesado = mensajeProcesado + to_string(sizeNombre); // L0305
                }
                mensajeProcesado = mensajeProcesado + itr->first; // L0305Angel
            }

            cout << "--> Protocolo de lista de usuarios (L)" << endl;
            cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;

            n = write(socketCLI, mensajeProcesado.c_str(), mensajeProcesado.length());
            if( n < 0){perror("Error en el write");}
        }

        else if( protocolNumber == 5 ){ // --> ENVIAR ARCHIVO
            // --> Input: F05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH
            // --> Output: F05Angel05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "F"; // F
            if(nombreUsuario.length() < 10){
                mensajeProcesado += "0" + to_string(nombreUsuario.size());  // F05
            }
            else{
                mensajeProcesado += to_string(nombreUsuario.size()); // F05
            }

            mensajeProcesado = mensajeProcesado + nombreUsuario; // F05Angel

            n = read(socketCLI, buffer, 2);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            int sizeDestinatario = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // F05Angel05

            n = read(socketCLI, buffer, sizeDestinatario);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer);
            string nombreDest = mensajeAux;

            mensajeProcesado = mensajeProcesado + mensajeAux; // F05Angel05Diana

            n = read(socketCLI, buffer, 2); 
            if( n < 0){perror("Error en el read de enviar archivo");} 
            buffer[n] = '\0';
            int sizeNombreArchivo = stoi(buffer); 

            mensajeProcesado = mensajeProcesado + buffer; // F05Angel05Diana07

            n = read(socketCLI, buffer, sizeNombreArchivo);
            if( n < 0){perror("Error en el read de enviar archivo");} 
            buffer[n] = '\0';
            mensajeAux.assign(buffer);
            string nombreArchivo = mensajeAux;

            mensajeProcesado = mensajeProcesado + mensajeAux; // F05Angel05Diana07msg.txt

            n = read(socketCLI, buffer, 2);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            int sizeMensaje = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // F05Angel05Diana07msg.txt25

            n = read(socketCLI, buffer, sizeMensaje);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer);

            string contenidoArchivo = mensajeAux;
            mensajeProcesado = mensajeProcesado + mensajeAux; // F05Angel05Diana07msg.txt25holaComoEstanDesdeArchivo

            n = read(socketCLI, buffer, 2);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            int sizeHash = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // F05Angel05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_

            n = read(socketCLI, buffer, sizeHash);
            if( n < 0){perror("Error en el read de enviar archivo");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer);

            mensajeProcesado = mensajeProcesado + mensajeAux; // F05Angel05Diana07msg.txt25holaComoEstanDesdeArchivo_sizeHash_HASH

            if( mapOfUsers.find(nombreDest) != mapOfUsers.end() ) // --> Si existe el destinatario
            {
                // --> Enviamos el mensaje al destinatario
                cout << "--> Protocolo de enviar archivo (F)" << endl;
                cout << "Mensaje enviado de " << nombreUsuario << " a " << nombreDest << " es: " << contenidoArchivo << endl;
                cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;
                n = write(mapOfUsers[nombreDest], mensajeProcesado.c_str(), mensajeProcesado.length());
                if( n < 0){perror("Error en el write");}
            }
            else{ // --> No existe el destinatario
                cout << "El destinatario no existe" << endl;
                char alertMSG[100] = "El destinatario no existe";
                n = write(socketCLI, alertMSG, strlen(alertMSG));
                if(n < 0){perror("Error en el write");}
            }
        }

        else if ( protocolNumber == 6 ){ // --> QUIT
            quitChat = true;
            mapOfUsers.erase(mapOfUsers.find(mensajeAux));

            // --> Eliminamos el directorio personal del usuario
            const char *nombreCarpeta = nombreUsuario.c_str();
            if(rmdir(nombreCarpeta) == -1){
                cerr << "Error al eliminar la carpeta:  " << strerror(errno) << endl;
            }
            else{
                cout << "Su directorio personal ha sido eliminado" << endl;
            }
        }
    
        else if(protocolNumber == 7){ // --> ENVIAR IMGAEN
            // --> Input: I05Diana10imagen.jpg
            // --> Output: I05Angel05Diana10imagen.jpg

            // ---------------- Procesamiento del mensaje ----------------
            string mensajeProcesado = "I"; // I
            if(nombreUsuario.length() < 10){
                mensajeProcesado += "0" + to_string(nombreUsuario.size());  // I05
            }
            else{
                mensajeProcesado += to_string(nombreUsuario.size()); // I05
            }

            mensajeProcesado = mensajeProcesado + nombreUsuario; // I05Angel

            n = read(socketCLI, buffer, 2);
            if( n < 0){perror("Error en el read de enviar imagen");}
            buffer[n] = '\0';
            int sizeDestinatario = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // I05Angel05

            n = read(socketCLI, buffer, sizeDestinatario);
            if( n < 0){perror("Error en el read de enviar imagen");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer);
            string nombreDest = mensajeAux;

            mensajeProcesado = mensajeProcesado + mensajeAux; // I05Angel05Diana

            n = read(socketCLI, buffer, 2);
            if( n < 0){perror("Error en el read de enviar imagen");}
            buffer[n] = '\0';
            int sizeNombreImagen = stoi(buffer);

            mensajeProcesado = mensajeProcesado + buffer; // I05Angel05Diana10

            n = read(socketCLI, buffer, sizeNombreImagen);
            if( n < 0){perror("Error en el read de enviar imagen");}
            buffer[n] = '\0';
            mensajeAux.assign(buffer);
            string nombreImagen = mensajeAux;

            mensajeProcesado = mensajeProcesado + mensajeAux; // I05Angel05Diana10imagen.jpg

            if( mapOfUsers.find(nombreDest) != mapOfUsers.end() ) // --> Si existe el destinatario
            {
                // --> Enviamos el mensaje al destinatario
                cout << "--> Protocolo de enviar imagen (I)" << endl;
                cout << "Mensaje enviado de " << nombreUsuario << " a " << nombreDest << endl;
                cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;
                n = write(mapOfUsers[nombreDest], mensajeProcesado.c_str(), mensajeProcesado.length());
                if( n < 0){perror("Error en el write");}
                
                // * Indicamos que vamos a recibir varios mensajes de la imagen y su destinatario
                //sendImage = true;
                //nombreDestImage = nombreDest;
            }
            else{ // --> No existe el destinatario
                cout << "El destinatario no existe" << endl;
                char alertMSG[100] = "El destinatario no existe";
                n = write(socketCLI, alertMSG, strlen(alertMSG));
                if(n < 0){perror("Error en el write");}
            }
        }
    
        else if(protocolNumber == 8){ // --> JUGAR AL 3 EN RAYA
                // --> Input: J...
                n = read(socketCLI, buffer, 1); // Para leer que protocolo del juego J es (N, M)
                if( n < 0){perror("Error en el read");}
                buffer[n] = '\0';  
                mensajeAux.assign(buffer);
                
                if(mensajeAux[0] == 'N'){ // --> Input: JN05Angel (Registro / nuevo juego)
                    string mensajeProcesado = "JN"; // JN
                    n = read(socketCLI, buffer, 2);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    int sizeNombre = stoi(buffer);
                    mensajeProcesado = mensajeProcesado + buffer; // JN05

                    n = read(socketCLI, buffer, sizeNombre);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    mensajeAux.assign(buffer);

                    string nombreJugador = mensajeAux;
                    mensajeProcesado = mensajeProcesado + mensajeAux; // JN05Angel

                    cout << "--> Protocolo de jugar (J) - Nuevo jugador" << endl;
                    cout << "Mensaje procesado: " << mensajeProcesado << endl;

                    if( numJugadores < 2){ // player
                        if( mapOfPlayers.find(nombreJugador) == mapOfPlayers.end()) { // Si no existe el jugador lo agregamos
                            mapOfPlayers[nombreJugador] = socketCLI;
                            cout << "La persona " << nombreJugador << " sera jugador" << endl;
                            numJugadores++;
                        }
                    }
                    else{ // spectator
                        if( mapOfSpectators.find(nombreJugador) == mapOfSpectators.end()) { // Si no existe el espectador lo agregamos
                            mapOfSpectators[nombreJugador] = socketCLI;
                            cout << "La persona" << nombreJugador << " sera espectador" << endl;
                        }
                    }

                    if( numJugadores == 2 && isInGame == false){ // --> Damos la señal de inicio de juego
                        cout << endl << "--------------------------------------------" << endl;
                        cout << "Enviando mensaje de inicio de juego" << endl;
                        cout << "--------------------------------------------" << endl << endl;
                        isInGame = true;

                        // Elegimos al azar quien empieza
                        srand(static_cast<unsigned int>(time(nullptr)));
                        int random = (rand() % 2) + 1;

                        if(random == 1){
                            string mensajeProcesado1 = "JI1"; // --> 1 de inicia primero
                            n = write(mapOfPlayers.begin()->second, mensajeProcesado1.c_str(), mensajeProcesado1.length()); // Apunta al primer jugador

                            string mensajeProcesado2 = "JI2"; // --> 2 de inicia segundo
                            n = write(mapOfPlayers.rbegin()->second, mensajeProcesado2.c_str(), mensajeProcesado2.length()); // Apunta al ultimo jugador
                        }
                        else if(random == 2){
                            string mensajeProcesado1 = "JI2"; // --> 2 de inicia segundo
                            n = write(mapOfPlayers.begin()->second, mensajeProcesado1.c_str(), mensajeProcesado1.length()); // Apunta al primer jugador

                            string mensajeProcesado2 = "JI1"; // --> 1 de inicia primero
                            n = write(mapOfPlayers.rbegin()->second, mensajeProcesado2.c_str(), mensajeProcesado2.length()); // Apunta al ultimo jugador
                        }
                        else{
                            cout << "Error en el random" << endl;
                        }
                    }
                    else if(isInGame == true){ // Damos la señal de espera de juego a todos los espectadores
                        cout << endl << "--------------------------------------------" << endl;
                        cout << "Enviando mensaje de espectador de juego" << endl;
                        cout << "--------------------------------------------" << endl << endl;
                        string mensajeProcesado = "JE"; // --> E de espectador
                        map<string, int>::iterator itr;
                        for (itr = mapOfSpectators.begin(); itr != mapOfSpectators.end(); ++itr) { 
                            n = write(mapOfSpectators[itr->first], mensajeProcesado.c_str(), mensajeProcesado.length());
                            if( n < 0){perror("Error en el write");}
                        }
                    }
                }
            
                else if(mensajeAux[0] == 'M'){ // --> Input: JM05Angel2X (Nuevo movimiento/nombre/posicion/simbolo)
                    // --> Obtenemos el nombre del jugador y su jugada
                    string nombreJugador;
                    int posicionJugada;
                    string simboloJugador;

                    n = read(socketCLI, buffer, 2);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    int sizeNombre = stoi(buffer);
                    string sizeNombreS = buffer;

                    n = read(socketCLI, buffer, sizeNombre);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    mensajeAux.assign(buffer);

                    nombreJugador = mensajeAux;

                    n = read(socketCLI, buffer, 1);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    posicionJugada = stoi(buffer);

                    n = read(socketCLI, buffer, 1);
                    if( n < 0){perror("Error en el read de jugar");}
                    buffer[n] = '\0';
                    simboloJugador = buffer;

                    // --> Guardamos el movimiento en el mapa
                    movimientos[posicionJugada] = nombreJugador;

                    // --> Verificamos si hay ganador
                    bool hayGanador = checkWinner();
                    if(hayGanador == true){
                        string mensajeProcesado = "JG" + sizeNombreS + nombreJugador + to_string(posicionJugada) + simboloJugador; // JG05Angel2X
                        cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;

                        map<string, int>::iterator itr;
                        for (itr = mapOfPlayers.begin(); itr != mapOfPlayers.end(); ++itr) { 
                            n = write(mapOfPlayers[itr->first], mensajeProcesado.c_str(), mensajeProcesado.length());
                            if( n < 0){perror("Error en el write");}
                        }
                        map<string, int>::iterator itr2;
                        for (itr2 = mapOfSpectators.begin(); itr2 != mapOfSpectators.end(); ++itr2) { 
                            n = write(mapOfSpectators[itr2->first], mensajeProcesado.c_str(), mensajeProcesado.length());
                            if( n < 0){perror("Error en el write");}
                        }
                    }
                    else{
                        // --> Enviamos el movimiento a todos los jugadores y espectadores
                        cout << endl << "--> Protocolo de jugar (J) - Nuevo movimiento" << endl;
                        string mensajeProcesado = "JM" + sizeNombreS + nombreJugador + to_string(posicionJugada) + simboloJugador; // JM05Angel2X
                        cout << "Mensaje procesado: " << mensajeProcesado << endl << endl;

                        map<string, int>::iterator itr;
                        for (itr = mapOfPlayers.begin(); itr != mapOfPlayers.end(); ++itr) { 
                            n = write(mapOfPlayers[itr->first], mensajeProcesado.c_str(), mensajeProcesado.length());
                            if( n < 0){perror("Error en el write");}
                        }
                        map<string, int>::iterator itr2;
                        for (itr2 = mapOfSpectators.begin(); itr2 != mapOfSpectators.end(); ++itr2) { 
                            n = write(mapOfSpectators[itr2->first], mensajeProcesado.c_str(), mensajeProcesado.length());
                            if( n < 0){perror("Error en el write");}
                        }
                    }
                }
            }
    }
    cout << "Usuario " << nombreUsuario << " se ha desconectado" << endl;
    shutdown(socketCLI, SHUT_RDWR);
    close(socketCLI);
}

int main(void)
{
    //--------------------- Configuracion de la INFORMACION DEL SOCKET ---------------------
    // --> Mediante una estructura, le indicamos al socket que puerto, IP y protocolo vamos a usar
    struct sockaddr_in stSockAddr; // Estructura

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in)); // Llenamos de 0's la estructura stSockAddr

    stSockAddr.sin_family = AF_INET; // Especificamos que vamos a usar un Socket para comunicar en 2 computadoras distintas
    stSockAddr.sin_port = htons(50001); // Es el puerto en donde se abrira el servidor. Antes tenia 1100 
    stSockAddr.sin_addr.s_addr = INADDR_ANY; // Le decimos que tenga cualquier IP

    // --------------------- Configuracion del Socket ---------------------
    int Server_SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // Inicializar el socket ( puerto, tipo de socket, protocolo)
    if(Server_SocketFD == -1){
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // #################################################################################################

    // --------------------- BIND  ---------------------
    // --> Unir el socket con el puerto y la IP de la estructura
    if(-1 == bind(Server_SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("Error al hacer el bind");
        close(Server_SocketFD);
        exit(EXIT_FAILURE);
    }

    // --------------------- LISTEN ---------------------
    // --> Habilitamos el socket para recibir conexiones
    if(-1 == listen(Server_SocketFD, 10)) { // Le pasamos el socket y el numero de conexiones que puede tener en cola 
        perror("Error en el listen");
        close(Server_SocketFD);
        exit(EXIT_FAILURE);
    }

    // #################################################################################################

    // --------------------- ACCEPT ---------------------
    // --> Bucle infinito para aceptar conexiones
    for(;;) {
        // Aceptamos todas las conexiones entrantes en el socket Server_SocketFD
        int Client_SocketFD = accept(Server_SocketFD, NULL, NULL);
        if(Client_SocketFD < 0)  // Verificamos la coneccion
        {
            perror("Error en aceptar la conexion");
            close(Server_SocketFD);
            exit(EXIT_FAILURE);
        }

        cout << endl << "Nuevo usuario conectado" << endl;
        cout << "ID del socket: " << Client_SocketFD << endl <<  endl;

        std::thread ( read_thread, Client_SocketFD).detach();
    }

    close(Server_SocketFD);
    return 0;
}