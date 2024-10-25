#include <stdlib.h>
#include <stdio.h>
#include "malloc.h"
#include "raylib.h"
#include <math.h>
#include <string.h>
#include "debug.h"

///DEFICINIONES
#define DIM 30
#define SCREENWIDTH 1920
#define SCREENHEIGHT 1080
///INDEX STATE MACHINE Y ESCENARIO
#define HABITACION 0
#define COCINA 1
#define JARDIN 2
#define MENU 3
#define PAUSA 4
#define SAVE 5
#define SCORE 6
#define CREDITOS 7
///INDEX OBJETOS HABITACION 1
#define MUNIECA 0
#define ALFOMBRA 1
#define ESPEJO 2
#define ZAPATOS 3
#define GUITARRA 4
#define CAMA 5
#define VENTANA 6
#define CAJON1 7
#define ALMOHADA 8
#define CAJON2 10
///INDEX OBJETOS COCINA
#define CUADRO 9
#define TACHO 10
#define HELADERA 11
#define CUCHILLO 12
#define PARED 13
#define MESADA 14
#define PUERTA 15
///INDEX OBJETOS PATIO

///BOTONES DEL MENU
#define PLAY 0
#define BOTON 1
///BOTONES DE CREDITOS
#define BACK 0
///ESTRUCTURAS
typedef struct stSonidos {
    Sound musica;
    Sound efectos[DIM];
} stSonidos;

typedef struct stObjeto {
    Texture2D sprite[DIM];
    Rectangle hitbox;           ///El hitbox contiene la posición
    int existe;
} stObjeto;

typedef struct stEvento {
    int idObjeto;
    int idProgreso;
    int idEvento;             ///Indice de nodo de los objetos fijo para buscar
    int activo;                 ///Existe 0 o 1
    char texto[DIM*10];
} stEvento;                     ///camino dentro del arbol

typedef struct stNodo {
    stEvento evento;
    struct stNodo *anterior;
    struct stNodo *siguiente;
} stNodo;

typedef struct stFila {
    stNodo *primero;
    stNodo *ultimo;
} stFila;

typedef struct stArbol {
    stFila fila;
    struct stArbol *izquierda;
    struct stArbol *derecha;
} stArbol;

typedef struct stEscenario {
    Texture2D fondoEstatico;
    Texture2D subEscenarios[3];
    Texture2D botones[DIM];
    stArbol arbolDelEscenario;
    stObjeto objetos[DIM];
} stEscenario;

typedef struct stFlag {
    int idProgreso;
    int idEvento;
} stFlag;

typedef struct stJugador {
    stFlag flags[DIM]; //Booleanos relacionados a eventos
    int idObjeto;
    stArbol *posicionSubArbol; //posicion actual en el arbol
    int temporizador; //Tiempo máximo para completar el juego
    int escenario; //Número de escenario o subescenario (state machine)
} stJugador;

///VARIABLES GLOBALES
int i, j, k;

///MAIN
int main() {
    srand(time(NULL));
    stSonidos sonidos;
    stJugador *player = (stJugador *)malloc(sizeof(stJugador));
    stEscenario escenario[7];

    Debug herramientaDebug;

    inicializaciones(&sonidos, player, escenario, &herramientaDebug);

    ///-------------------------------------------------------------------------------------
    ///BUCLE PRINCIPAL
    ///-------------------------------------------------------------------------------------
    while( !WindowShouldClose() ) {
        actualizar(escenario, player, &herramientaDebug);
        dibujar(escenario, &herramientaDebug);
    }
    ///-------------------------------------------------------------------------------------
    ///DE-INICIALIZACION
    ///-------------------------------------------------------------------------------------
    deinicializaciones();
    return 0;
}

///FUNCIONES
///INICIALIZACIONES
void inicializaciones(stSonidos *sonidos, stJugador *player, stEscenario *escenario, Debug *herramientaDebug) {
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Escape Alive");
    SetTargetFPS(60);
    InitAudioDevice();
    inicializarSonidos(sonidos);
    inicializarJugador(player);
    printf("\ntestInicializaciones1\n"); ///debug
    inicializarEscenarios(escenario);
    printf("\ntestInicializaciones2\n"); ///debug
    inicializarDebug(herramientaDebug);
    printf("\ntestInicializaciones3\n"); ///debug
}
void inicializarSonidos(stSonidos *sonidos) {
    //sonidos->musica = loadSoundRaylib();
    //sonidos->efectos[DIM] = loadEffectRaylib();
}
void inicializarJugador(stJugador *player) {
    for( i = 0 ; i < DIM ; i++ ){ ///Inicializamos todos los flags en 0
        (*player).flags[i].idProgreso = 0;
        (*player).flags[i].idEvento = 0; ///PROBABLEMENTE NO LO USEMOS CUANDO HAGAMOS LAS FUNCIONES DE FILAS
        printf("mostrando I de inicializar Jugador %i\n", i);       ///DEBUGGGG
        printf("\nINICIALIZAR JUGADOR ID PROGRESO %i\n", player->flags[i].idProgreso);  ///DEBUGGG
    }

    player->idObjeto = -1;
    player->posicionSubArbol = NULL;
    player->temporizador = 54000; ///15 min a 60 frames
    player->escenario = 0; /// por ahora 0 para debuguear el escenario
}
void inicializarEscenarios(stEscenario *escenario) {
    inicializarMenu(escenario);
    inicializarHabitacion(escenario);
    inicializarCocina(escenario);
    inicializarJardin(escenario);
    inicializarCreditos(escenario);
}
///Habitación
void inicializarFila(stFila *fila) {
    fila->primero = NULL;
    fila->ultimo = NULL;
}
void inicializarArbol(stArbol *arbol) {
    arbol->derecha = NULL;
    arbol->izquierda = NULL;
}
void inicializarHabitacion(stEscenario *escenario) {
    escenario[HABITACION].fondoEstatico = LoadTexture("habitacion.png");
    escenario[HABITACION].subEscenarios[0] = LoadTexture("espejosubescenario.png");
    escenario[HABITACION].subEscenarios[1] = LoadTexture("ventanasubescenario.png");
    printf("\n Inicializar Eventos Habitacion"); ///debug
    inicializarEventosHabitacion(escenario);
    printf("\n Inicializar Objetos Habitacion"); ///debug
    inicializarObjetosHabitacion(escenario);
}
void inicializarEventosHabitacion(stEscenario *escenario) {
    inicializarEventosHabitacionMunieca(escenario);
    inicializarEventosHabitacionAlfombra(escenario);
    inicializarEventosHabitacionEspejo(escenario);
}
void inicializarEventosHabitacionMunieca(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///LA REFERENCIA APUNTA A LA RAIZ

    printf("\nPRINT REFERENCIA SUB ARBOL %p\n", referenciaSubArbol); ///BORRAR

    ///NIVEL 0 RAIZ MUNIECA
    inicializarArbol(referenciaSubArbol);

    inicializarEventosHabitacionMunieca0(referenciaSubArbol); ///Inerte

    //printf("\n NIVEL 0 RAIZ MUNIECA"); ///debug
    //mostrarFila((*referenciaSubArbol).fila); ///debug

    ///NIVEL 1 DERECHA MUNIECA
    stArbol inerte1Munieca;
    nuevoSubArbol = &inerte1Munieca;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca1(nuevoSubArbol); ///INERTE

    //printf("\n NIVEL 1 DERECHA MUNIECA");///debug
    //mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ DEL ARBOL AL SUBARBOL NIVEL 1 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 MUNIECA

    printf("\nPRINT REFERENCIA SUB ARBOL EN DERECHA %p\n", referenciaSubArbol); ///BORRAR

    ///NIVEL 2 IZQUIERDA MUNIECA
    stArbol progreso0Munieca;
    nuevoSubArbol = &progreso0Munieca;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMuniecaProgreso0(nuevoSubArbol);

    //printf("\n NIVEL 2 IZQUIERDA MUNIECA");///debug
    //mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 IZQUIERDA MUNIECA

    ///NIVEL 2 DERECHA MUNIECA
    stArbol inerte2Munieca;
    nuevoSubArbol = &inerte2Munieca;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca6(nuevoSubArbol); ///inerte    progreso 1

    //printf("\n NIVEL 2 DERECHA MUNIECA"); ///debug
    //mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 1 AL SUBARBOL NIVEL 2 DERECHA MUNIECA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 MUNIECA

    ///NIVEL 3 IZQUIERDA MUNIECA
    stArbol progreso1Munieca;
    nuevoSubArbol = &progreso1Munieca;

    crearNodoArbol(&nuevoSubArbol);

    inicializarEventosHabitacionMunieca7(nuevoSubArbol); ///progreso 1
    inicializarEventosHabitacionMunieca8(nuevoSubArbol); ///progreso 1
    inicializarEventosHabitacionMunieca9(nuevoSubArbol); ///progreso 1


    //printf("\n NIVEL 3 IZQUIERDA MUNIECA");///debug
    //mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ NIVEL 2 AL SUBARBOL NIVEL 3 IZQUIERDA MUNIECA

}
void inicializarEventosHabitacionMuniecaProgreso0(stArbol *subArbol) {
    inicializarEventosHabitacionMunieca2(subArbol);
    inicializarEventosHabitacionMunieca3(subArbol);
    inicializarEventosHabitacionMunieca4(subArbol);
    inicializarEventosHabitacionMunieca5(subArbol);
}
void inicializarEventosHabitacionMunieca0(stArbol *subArbol) { ///INACTIVO
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 0;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca1(stArbol *subArbol) { ///INACTIVO
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;
    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 1;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca2(stArbol *subArbol) { ///PRIMERA SECUENCIA DE EVENTOS
    inicializarFila(&(subArbol->fila));

    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 2;
    eventoMunieca.activo = 1;
    char texto[] = { "A simple vista la munieca parece estar inerte..." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca3(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 3;
    eventoMunieca.activo = 1;
    char texto[] = { "La munieca parece tener un hueco en la espalda donde pueden entrar las pilas." };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca4(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 4;
    eventoMunieca.activo = 1;
    char texto[] = { "Revivela!"};
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca5(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 0;
    eventoMunieca.idEvento = 5;
    eventoMunieca.activo = 1;
    char texto[] = { "Sin las pilas no pareciera funcionar."};
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca6(stArbol *subArbol) { ///INACTIVO
    inicializarFila(&(subArbol->fila));
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 6;
    eventoMunieca.activo = 0;
    char texto[] = { "" };
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca7(stArbol *subArbol) { ///SEGUNDA SECUENCIA DE EVENTOS
    inicializarFila(&(subArbol->fila));
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 7;
    eventoMunieca.activo = 1;
    char texto[] = { "Ubicaste las pilas en la espalda de la muñeca"};
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca8(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 8;
    eventoMunieca.activo = 1;
    char texto[] = { "La munieca comienza a hablar..."};
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionMunieca9(stArbol *subArbol) {
    stEvento eventoMunieca;

    eventoMunieca.idObjeto = 0;
    eventoMunieca.idProgreso = 1;
    eventoMunieca.idEvento = 9;
    eventoMunieca.activo = 1;
    char texto[] = { "Hablando..."};
    strcpy(eventoMunieca.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), eventoMunieca);
}
void inicializarEventosHabitacionAlfombra(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///APUNTA A LA RAIZ PRINCIPAL
    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ PRINCIPAL IZQUIERDA AL SUBARBOL NIVEL 1 RAIZ ALFOMBRA

    ///NIVEL 1 RAIZ ALFOMBRA
    stArbol raizAlfombra;
    nuevoSubArbol = &raizAlfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra0(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 1 RAIZ ALFOMBRA"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 RAIZ ALFOMBRA

    ///NIVEL 2 DERECHA ALFOMBRA
    stArbol inerte1Alfombra;
    nuevoSubArbol = &inerte1Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra1(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 2 DERECHA ALFOMBRA"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ ALFOMBRA AL SUBARBOL NIVEL 1 RAIZ ALFOMBRA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 1 RAIZ ALFOMBRA

    ///NIVEL 3 IZQUIERDA ALFOMBRA
    stArbol progreso0Alfombra;
    nuevoSubArbol = &progreso0Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombraProgreso0(nuevoSubArbol);

    printf("\n NIVEL 3 IZQUIERDA ALFOMBRA"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR NIVEL 1 RAIZ ALFOMBRA IZQUIERDA AL SUBARBOL NIVEL 1 RAIZ ALFOMBRA

    ///NIVEL 3 DERECHA ALFOMBRA
    stArbol progreso1Alfombra;
    nuevoSubArbol = &progreso1Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra5(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 3 DERECHA ALFOMBRA"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR NIVEL 2 DERECHA ALFOMBRA A NIVEL 3 DERECHA ALFOMBRA
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 3 DERECHA ALFOMBRA

    ///NIVEL 4 IZQUIERDA ALFOMBRA
    stArbol progreso2Alfombra;
    nuevoSubArbol = &progreso2Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra6(nuevoSubArbol);
    inicializarEventosHabitacionAlfombra7(nuevoSubArbol);
    inicializarEventosHabitacionAlfombra8(nuevoSubArbol);

    printf("\n NIVEL 4 IZQUIERDA ALFOMBRA"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR NIVEL 3 DERECHA ALFOMBRA IZQUIERDA AL SUBARBOL NIVEL 4 IZQUIERDA ALFOMBRA

}
void inicializarEventosHabitacionAlfombra0(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 10;
    nodoAlfombra.activo = 0;
    char texto[] = { "" };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra1(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 11;
    nodoAlfombra.activo = 0;
    char texto[] = { "" };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra2(stArbol *subArbol) { ///PROGRESO 0
    inicializarFila(&(subArbol->fila));

    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 12;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 1 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra3(stArbol *subArbol) {
    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 13;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 2 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra4(stArbol *subArbol) {
    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 14;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 3 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra5(stArbol *subArbol) { ///INERTE
    inicializarFila(&(subArbol->fila));

    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 15;
    nodoAlfombra.activo = 0;
    char texto[] = { "" };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra6(stArbol *subArbol) { ///PROGRESO 1
    inicializarFila(&(subArbol->fila));

    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 16;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 4 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra7(stArbol *subArbol) {
    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 0;
    nodoAlfombra.idEvento = 17;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 5 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombra8(stArbol *subArbol) {
    stEvento nodoAlfombra;

    nodoAlfombra.idObjeto = 1;
    nodoAlfombra.idProgreso = 1;
    nodoAlfombra.idEvento = 18;
    nodoAlfombra.activo = 1;
    char texto[] = { " TEST ALFOMBRA 6 " };
    strcpy(nodoAlfombra.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoAlfombra);
}
void inicializarEventosHabitacionAlfombraProgreso0(stArbol *subArbol){
    inicializarEventosHabitacionAlfombra2(subArbol);
    inicializarEventosHabitacionAlfombra3(subArbol);
    inicializarEventosHabitacionAlfombra4(subArbol);
}
void inicializarEventosHabitacionEspejo(stEscenario *escenario) {
    stArbol *nuevoSubArbol;
    stArbol *referenciaSubArbol;

    ///REFERENCIA
    referenciaSubArbol = &(escenario[HABITACION].arbolDelEscenario); ///APUNTA A LA RAIZ PRINCIPAL
    referenciaSubArbol = referenciaSubArbol->izquierda; ///APUNTA A LA RAIZ ALFOMBRA

    ///NIVEL 2 RAIZ ESPEJO
    stArbol raizEspejo;
    nuevoSubArbol = &raizEspejo;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionEspejo0(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 2 RAIZ ESPEJO"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR LA RAIZ ALFOMBRA A LA RAIZ ESPEJO
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 2 RAIZ ESPEJO

    ///NIVEL 3 DERECHA ESPEJO
    stArbol inerte1Espejo;
    nuevoSubArbol = &inerte1Espejo;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionEspejo1(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 2 DERECHA ESPEJO"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR LA RAIZ ESPEJO AL SUBARBOL NIVEL 3 DERECHA ESPEJO
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA A NIVEL 3 DERECHA ESPEJO

    ///NIVEL 4 IZQUIERDA ESPEJO
    stArbol progreso0Alfombra;
    nuevoSubArbol = &progreso0Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionEspejo2(nuevoSubArbol);
    inicializarEventosHabitacionEspejo3(nuevoSubArbol);
    inicializarEventosHabitacionEspejo4(nuevoSubArbol);

    printf("\n NIVEL 4 IZQUIERDA ESPEJO"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR NIVEL 1 RAIZ ESPEJO IZQUIERDA AL SUBARBOL NIVEL 1 RAIZ ESPEJO

    ///NIVEL 4 DERECHA ESPEJO
    stArbol inerte2Espejo;
    nuevoSubArbol = &inerte2Espejo;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra5(nuevoSubArbol); ///INERTE

    printf("\n NIVEL 4 DERECHA ESPEJO"); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->derecha = nuevoSubArbol; ///HACE APUNTAR NIVEL 3 DERECHA ESPEJO A NIVEL 4 DERECHA ESPEJO
    referenciaSubArbol = nuevoSubArbol; ///MUEVE LA REFERENCIA AL NUEVO SUBARBOL NIVEL 3 DERECHA ESPEJO

    ///NIVEL 5 IZQUIERDA ESPEJO
    stArbol progreso1Alfombra;
    nuevoSubArbol = &progreso1Alfombra;
    inicializarArbol(nuevoSubArbol);

    inicializarEventosHabitacionAlfombra6(nuevoSubArbol);
    inicializarEventosHabitacionAlfombra7(nuevoSubArbol);
    inicializarEventosHabitacionAlfombra8(nuevoSubArbol);

    printf("\n NIVEL 5 IZQUIERDA ESPEJO "); ///debug
    mostrarFila((*nuevoSubArbol).fila); ///debug

    referenciaSubArbol->izquierda = nuevoSubArbol; ///HACE APUNTAR NIVEL 4 DERECHA ESPEJO IZQUIERDA AL NIVEL 5 IZQUIERDA ESPEJO

}
void inicializarEventosHabitacionEspejo0(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 19;
    nodoEspejo.activo = 0;
    char texto[] = { "" };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo1(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 20;
    nodoEspejo.activo = 0;
    char texto[] = { "" };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo2(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 21;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 1 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo3(stArbol *subArbol) {
    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 22;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 2 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo4(stArbol *subArbol) {
    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 23;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 3 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo5(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 24;
    nodoEspejo.activo = 0;
    char texto[] = { "" };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo6(stArbol *subArbol) {
    inicializarFila(&(subArbol->fila));

    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 25;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 4 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo7(stArbol *subArbol) {
    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 26;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 5 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}
void inicializarEventosHabitacionEspejo8(stArbol *subArbol) {
    stEvento nodoEspejo;

    nodoEspejo.idObjeto = 2;
    nodoEspejo.idEvento = 27;
    nodoEspejo.activo = 1;
    char texto[] = { " TEST ESPEJO 6 " };
    strcpy(nodoEspejo.texto, texto);

    agregarNodoEnFila(&(subArbol->fila), nodoEspejo);
}


void inicializarObjetosHabitacion(stEscenario *escenario) {
    inicializarObjetoMunieca(escenario);
    inicializarObjetoAlfombra(escenario);
    inicializarObjetoEspejo(escenario);
    inicializarObjetoZapatos(escenario);
    inicializarObjetoGuitarra(escenario);
    inicializarObjetoCama(escenario);
    inicializarObjetoVentana(escenario);
    inicializarObjetoCajon1(escenario);
    inicializarObjetoCajon2(escenario);
}
void inicializarObjetoMunieca(stEscenario *escenario){
    escenario[HABITACION].objetos[MUNIECA].existe = 1; //Muñeca WHITE
    escenario[HABITACION].objetos[MUNIECA].hitbox.x = 1398;
    escenario[HABITACION].objetos[MUNIECA].hitbox.y = 420; //el eje y esta invertido
    escenario[HABITACION].objetos[MUNIECA].hitbox.width = 61; //el eje y esta invertido
    escenario[HABITACION].objetos[MUNIECA].hitbox.height = 190; //el eje y esta invertido
}
void inicializarObjetoAlfombra(stEscenario *escenario){
   escenario[HABITACION].objetos[ALFOMBRA].existe = 1;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.x = 1272;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.y = 866;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.width = 80;
    escenario[HABITACION].objetos[ALFOMBRA].hitbox.height = 77;
}
void inicializarObjetoEspejo(stEscenario *escenario){
    escenario[HABITACION].objetos[ESPEJO].existe = 1;
    escenario[HABITACION].objetos[ESPEJO].hitbox.x = 567;
    escenario[HABITACION].objetos[ESPEJO].hitbox.y = 149;
    escenario[HABITACION].objetos[ESPEJO].hitbox.width = 115;
    escenario[HABITACION].objetos[ESPEJO].hitbox.height = 430;
}
void inicializarObjetoZapatos(stEscenario *escenario){
    escenario[HABITACION].objetos[ZAPATOS].existe = 1;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.x = 1126;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.y = 855;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.width = 64;
    escenario[HABITACION].objetos[ZAPATOS].hitbox.height = 100;
}
void inicializarObjetoGuitarra(stEscenario *escenario){
    escenario[HABITACION].objetos[GUITARRA].existe = 1;
    escenario[HABITACION].objetos[GUITARRA].hitbox.x = 854;
    escenario[HABITACION].objetos[GUITARRA].hitbox.y = 643;
    escenario[HABITACION].objetos[GUITARRA].hitbox.width = 63;
    escenario[HABITACION].objetos[GUITARRA].hitbox.height = 91;
}
void inicializarObjetoCama(stEscenario *escenario){
    escenario[HABITACION].objetos[CAMA].existe = 1;
    escenario[HABITACION].objetos[CAMA].hitbox.x = 862;
    escenario[HABITACION].objetos[CAMA].hitbox.y = 430;
    escenario[HABITACION].objetos[CAMA].hitbox.width = 168;
    escenario[HABITACION].objetos[CAMA].hitbox.height = 119;
}
void inicializarObjetoVentana(stEscenario *escenario){
    escenario[HABITACION].objetos[VENTANA].existe = 1;
    escenario[HABITACION].objetos[VENTANA].hitbox.x = 944;
    escenario[HABITACION].objetos[VENTANA].hitbox.y = 185;
    escenario[HABITACION].objetos[VENTANA].hitbox.width = 84;
    escenario[HABITACION].objetos[VENTANA].hitbox.height = 214;
}
void inicializarObjetoCajon1(stEscenario *escenario){
    escenario[HABITACION].objetos[CAJON1].existe = 1;
    escenario[HABITACION].objetos[CAJON1].hitbox.x = 626;
    escenario[HABITACION].objetos[CAJON1].hitbox.y = 653;
    escenario[HABITACION].objetos[CAJON1].hitbox.width = 47;
    escenario[HABITACION].objetos[CAJON1].hitbox.height = 46;
}
void inicializarObjetoCajon2(stEscenario *escenario){
    escenario[HABITACION].objetos[CAJON2].existe = 1;
    escenario[HABITACION].objetos[CAJON2].hitbox.x = 641;
    escenario[HABITACION].objetos[CAJON2].hitbox.y = 738;
    escenario[HABITACION].objetos[CAJON2].hitbox.width = 47;
    escenario[HABITACION].objetos[CAJON2].hitbox.height = 46;
}
///COCINA
void inicializarCocina(stEscenario *escenario) {
    escenario[COCINA].fondoEstatico = LoadTexture("cocina.png");
    ///FUNCIONES A DESARROLLAR
    //inicializarEventosCocina(escenario);
    //inicializarArbolCocina(escenario);
    inicializarObjetosCocina(escenario);
}
void inicializarEventosCocina(stEscenario *escenario) {  /// FUNCION A REALIZAR CUANDO TENGAMOS LOS PUZZLES
//    inicializarEventosHabitacionCuadro(escenario);
//    inicializarEventosHabitacionTacho(escenario);
//    inicializarEventosHabitacionHeladera(escenario);
//    inicializarEventosHabitacionCuchillo(escenario);
//    inicializarEventosHabitacionPared(escenario);
//    inicializarEventosHabitacionMesada(escenario);
//    inicializarEventosHabitacionPuerta(escenario);
}
void inicializarObjetosCocina(stEscenario *escenario) {
    inicializarObjetoCuadro(escenario);
    inicializarObjetoTacho(escenario);
    inicializarObjetoHeladera(escenario);
    inicializarObjetoCuchillo(escenario);
    inicializarObjetoPared(escenario);
    inicializarObjetoMesada(escenario);
    inicializarObjetoPuerta(escenario);
}
void inicializarObjetoCuadro(stEscenario *escenario){
    escenario[COCINA].objetos[CUADRO].existe = 1;
    escenario[COCINA].objetos[CUADRO].hitbox.x = 542;
    escenario[COCINA].objetos[CUADRO].hitbox.y = 40;
    escenario[COCINA].objetos[CUADRO].hitbox.width = 105;
    escenario[COCINA].objetos[CUADRO].hitbox.height = 228;
}
void inicializarObjetoTacho(stEscenario *escenario){
    escenario[COCINA].objetos[TACHO].existe = 1;
    escenario[COCINA].objetos[TACHO].hitbox.x = 849;
    escenario[COCINA].objetos[TACHO].hitbox.y = 521;
    escenario[COCINA].objetos[TACHO].hitbox.width = 100;
    escenario[COCINA].objetos[TACHO].hitbox.height = 177;
}
void inicializarObjetoHeladera(stEscenario *escenario){
    escenario[COCINA].objetos[HELADERA].existe = 1;
    escenario[COCINA].objetos[HELADERA].hitbox.x = 811;
    escenario[COCINA].objetos[HELADERA].hitbox.y = 205;
    escenario[COCINA].objetos[HELADERA].hitbox.width = 119;
    escenario[COCINA].objetos[HELADERA].hitbox.height = 240;
}
void inicializarObjetoCuchillo(stEscenario *escenario){
    escenario[COCINA].objetos[CUCHILLO].existe = 1;
    escenario[COCINA].objetos[CUCHILLO].hitbox.x = 809;
    escenario[COCINA].objetos[CUCHILLO].hitbox.y = 760;
    escenario[COCINA].objetos[CUCHILLO].hitbox.width = 64;
    escenario[COCINA].objetos[CUCHILLO].hitbox.height = 150;
}
void inicializarObjetoPared(stEscenario *escenario){
    escenario[COCINA].objetos[PARED].existe = 1;
    escenario[COCINA].objetos[PARED].hitbox.x = 1200;
    escenario[COCINA].objetos[PARED].hitbox.y = 340;
    escenario[COCINA].objetos[PARED].hitbox.width = 224;
    escenario[COCINA].objetos[PARED].hitbox.height = 47;
}
void inicializarObjetoMesada(stEscenario *escenario){
    escenario[COCINA].objetos[MESADA].existe = 1;
    escenario[COCINA].objetos[MESADA].hitbox.x = 553;
    escenario[COCINA].objetos[MESADA].hitbox.y = 464;
    escenario[COCINA].objetos[MESADA].hitbox.width = 261;
    escenario[COCINA].objetos[MESADA].hitbox.height = 197;
}
void inicializarObjetoPuerta(stEscenario *escenario){
    escenario[COCINA].objetos[PUERTA].existe = 1;
    escenario[COCINA].objetos[PUERTA].hitbox.x = 998;
    escenario[COCINA].objetos[PUERTA].hitbox.y = 135;
    escenario[COCINA].objetos[PUERTA].hitbox.width = 162;
    escenario[COCINA].objetos[PUERTA].hitbox.height = 402;
}

///PATIO
void inicializarJardin(stEscenario *escenario) {
    escenario[JARDIN].fondoEstatico = LoadTexture("jardin.png");
    ///FUNCIONES A DESARROLLARã
    //inicializarEventosJardin(escenario);
    //inicializarArbolJardin(escenario);
    //inicializarObjetosJardin(escenario);
}


///MENU
void inicializarMenu(stEscenario *escenario){
    escenario[MENU].fondoEstatico = LoadTexture("menu.png");
    dibujarBotonesMenu(escenario);
}
void dibujarBotonesMenu(stEscenario *escenario){
    dibujarBotonPlay(escenario);
    dibujarBotonLoad(escenario);
    dibujarBotonSave(escenario);
    dibujarBotonScore(escenario);
    dibujarBotonCredits(escenario);
}
void dibujarBotonPlay(stEscenario *escenario){
    escenario[MENU].botones[PLAY] = LoadTexture("play.png");
    DrawText("PLAY",935,829,60, BLACK);
}
void dibujarBotonLoad(stEscenario *escenario){
    escenario[MENU].botones[BOTON] = LoadTexture("boton.png");
    DrawText("LOAD",605,712,40, BLACK);
}
void dibujarBotonScore(stEscenario *escenario){
    escenario[MENU].botones[BOTON] = LoadTexture("boton.png");
    DrawText("SCORE",590,810,40, BLACK);
}
void dibujarBotonSave(stEscenario *escenario){
    escenario[MENU].botones[BOTON] = LoadTexture("boton.png");
    DrawText("SAVE",1310,712,40, BLACK);
}
void dibujarBotonCredits(stEscenario *escenario){
    escenario[MENU].botones[BOTON] = LoadTexture("boton.png");
    DrawText("CREDITS",1280,810,39, BLACK);
}
///CREDITOS
void inicializarCreditos(stEscenario *escenario){
    escenario[CREDITOS].fondoEstatico = LoadTexture("creditos.png");
    dibujarScrollCreditos(escenario);
    dibujarBotonBackCreditos(escenario);
}
void dibujarCreditos(stEscenario *escenario){
    dibujarScrollCreditos(escenario);
    dibujarBotonBackCreditos(escenario);
}
void dibujarBotonBackCreditos(stEscenario *escenario){
    escenario[CREDITOS].botones[BACK] = LoadTexture("boton.png");
    DrawText("BACK",930,960,39, BLACK);
}
void dibujarScrollCreditos(stEscenario *escenario) {
    DrawText("Desarrolladores", 700, 100, 60, YELLOW);
    DrawText("Ortiz Rocio", 700, 200, 60, WHITE);
    DrawText("Ortiz Brisa", 700, 300, 60, WHITE);
    DrawText("Olmos Fernando", 700,400, 60, WHITE);
    DrawText("Docente y Ayudantes",700, 500, 60, YELLOW);
    DrawText("Adrian Aroca", 700, 600, 60, WHITE);
    DrawText("Hilario Monti", 700, 700, 60, WHITE);
    DrawText("Nicanor Dondero", 700, 800, 60 , WHITE);
}

///ACTUALIZAR
void actualizar(stEscenario *escenario, stJugador *player, Debug *herramientaDebug) {

    if( player->escenario == HABITACION ) {
        actualizarInputHabitacion(escenario, player); ///Actualiza donde clickeaste con el mouse
        actualizarArbolHabitacion(escenario, player, player->flags[player->idObjeto].idProgreso); ///Actualiza los flags pero no los acciona. Los acciona el jugador
        actualizarEventosHabitacion(escenario, player);
    }

//     if( player->escenario == COCINA ) {
//        actualizarInputCocina(escenario, player); ///Actualiza donde clickeaste con el mouse
//        actualizarArbolCocina(escenario, player, player->flags[player->idObjeto].idProgreso); ///Actualiza los flags pero no los acciona. Los acciona el jugador
//        actualizarEventosCocina(escenario, player);
//    }

    if( herramientaDebug->mostrarHerramienta == 1 ) {
        actualizarInputDebug(herramientaDebug);
        actualizarTestDebug(escenario, herramientaDebug);
    }

}
///PRUEBA TEST DE CLICK DE ALGUNOS INPUT (OBJETOS A INTERACTUAR)
void actualizarInputHabitacion(stEscenario *escenario, stJugador *player){

    if( seClickeaLaMunieca(escenario) )  interactuarConLaMunieca(escenario, player);
    if( seClickeaElEspejo(escenario) )   interactuarConElEspejo(escenario, player);
    if( seClickeaLaVentana(escenario) )  interactuarConLaVentana(escenario, player);
    if( seClickeaLaCama(escenario) )     interactuarConLaCama(escenario, player);
    if( seClickeaLaAlfombra(escenario) ) interactuarConLaAlfombra(escenario, player);
}
int seClickeaLaMunieca(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[MUNIECA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElEspejo(stEscenario *escenario) { ///SubEscenario
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ESPEJO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaLaVentana(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[VENTANA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaCama(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[CAMA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaAlfombra(stEscenario *escenario) { ///SubEscenario
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario->objetos[ALFOMBRA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
void interactuarConLaMunieca(stEscenario *escenario, stJugador *player) {
    player->idObjeto = MUNIECA;
    printf("\nMUNIECA CLICKEADO\n");
}
void interactuarConElEspejo(stEscenario *escenario, stJugador *player) {
    player->idObjeto = ESPEJO;
    printf("\nESPEJO CLICKEADO\n");
}
void interactuarConLaVentana(stEscenario *escenario, stJugador *player) {
    player->idObjeto = VENTANA;
    printf("\nVENTANA CLICKEADA\n");
}
void interactuarConLaCama(stEscenario *escenario, stJugador *player) {
    player->idObjeto = CAMA;
    printf("\nCAMA CLICKEADA\n");
}
void interactuarConLaAlfombra(stEscenario *escenario, stJugador *player) {
    player->idObjeto = ALFOMBRA;
    printf("\nALFOMBRA CLICKEADA\n");
}
void actualizarArbolHabitacion(stEscenario *escenario, stJugador *player, int progreso) {
    int izquierda = 0;
    int derecha = 0;

    if( player->idObjeto == -1 ) printf("");

    else {

        if( IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) ) ///DEBUGGG
            player->flags[0].idProgreso = 0; ///DEBUGGG

        stArbol * buscador = &(escenario[HABITACION].arbolDelEscenario);

        while( buscador != NULL && player->idObjeto != (buscador->fila.primero)->evento.idObjeto ) { ///BUSCANDO EL ID DEL OBJETO. el 3 debe coincidir con cantidad de objetos
            printf("\n izquierda %i", izquierda);
            buscador = buscador->izquierda;
            izquierda++;
        }
        if( buscador == NULL ) printf("\nNo se encontro el id objeto.\n");
        if( buscador != NULL ) buscador = buscador->derecha;


        while( buscador != NULL && player->flags[player->idObjeto].idProgreso != (buscador->fila.primero)->evento.idProgreso ) {
            printf("\n derecha %i", derecha);
            buscador = buscador->derecha;
            derecha++;
        }
        if( buscador == NULL ) printf("\nNo se encontro el id progreso.\n");
        if( buscador != NULL ) buscador = buscador->izquierda;

        int clicks = 0;
        if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ) { ///debug
            printf("\n click nro %i", clicks);
            player->flags[player->idObjeto].idEvento = (buscador->fila.primero)->evento.idEvento;
            printf("\n actualizar arbol idEvento: %i", player->flags[player->idObjeto].idEvento);
            clicks++;
        }

        player->posicionSubArbol = buscador;

        ///printf("\n\n String: %s \n", (buscador->fila.primero)->evento.texto); ///debug
        ///quitarPrimerNodoFila(&(buscador->fila)); ///debug
    }
}
void actualizarEventosHabitacion(stEscenario *escenario, stJugador *player) {
//    stNodo *aux = buscador->fila.primero;
//    stFila fila = player.posicionSubArbol.fila;
//    printf("\nID ENVENTO CHOCLITO %i\n", fila.primero->evento.idEvento);
//    if( player->flags[player->idObjeto].idEvento == fila.primero->evento.idEvento ) { //aux->evento.idEvento
//        printf("\n\n String: %s \n", fila.primero->evento.texto); ///debug
//        quitarPrimerNodoFila(&((*player).posicionSubArbol->fila)); ///debug
//    }n
}
void actualizarInputCocina(stEscenario *escenario, stJugador *player){

    if( seClickeaElCuadro(escenario) )  interactuarConElCuadro(escenario, player);
    if( seClickeaElTacho(escenario) )   interactuarConElEspejo(escenario, player);
    if( seClickeaLaHeladera(escenario) )  interactuarConLaHeladera(escenario, player);
    if( seClickeaElCuchillo(escenario) )     interactuarConElCuchillo(escenario, player);
    if( seClickeaLaPared(escenario) ) interactuarConLaPared(escenario, player);
    if( seClickeaLaMesada(escenario) ) interactuarConLaMesada(escenario, player);
    if( seClickeaLaPuerta(escenario) ) interactuarConLaPuerta(escenario, player);
}
int seClickeaElCuadro(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[CUADRO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaElTacho(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[TACHO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaLaHeladera(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[HELADERA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaElCuchillo(stEscenario *escenario) {
    int esClickeado = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[CUCHILLO].hitbox) )
        esClickeado = 1;
    return esClickeado;
}
int seClickeaLaPared(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[PARED].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaMesada(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[MESADA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
int seClickeaLaPuerta(stEscenario *escenario) {
    int esClickeada = 0;
    Vector2 posicionDelMouse = GetMousePosition();
    if( IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(posicionDelMouse, escenario[COCINA].objetos[PUERTA].hitbox) )
        esClickeada = 1;
    return esClickeada;
}
void interactuarConElCuadro(stEscenario *escenario, stJugador *player) {
    player->idObjeto = CUADRO;
    printf("\nCUADRO CLICKEADO\n");
}
void interactuarConElTacho(stEscenario *escenario, stJugador *player) {
    player->idObjeto = TACHO;
    printf("\nTACHO CLICKEADO\n");
}
void interactuarConLaHeladera(stEscenario *escenario, stJugador *player) {
    player->idObjeto = HELADERA;
    printf("\nHELADERA CLICKEADO\n");
}
void interactuarConElCuchillo(stEscenario *escenario, stJugador *player) {
    player->idObjeto = CUCHILLO;
    printf("\nCUCHILLO CLICKEADO\n");
}
void interactuarConLaPared(stEscenario *escenario, stJugador *player) {
    player->idObjeto = PARED;
    printf("\nPARED CLICKEADO\n");
}
void interactuarConLaMesada(stEscenario *escenario, stJugador *player) {
    player->idObjeto = MESADA;
    printf("\nMESADA CLICKEADO\n");
}
void interactuarConLaPuerta(stEscenario *escenario, stJugador *player) {
    player->idObjeto = PUERTA;
    printf("\nPUERTA CLICKEADO\n");
}

///DIBUJAR
void dibujar(stEscenario *escenario, Debug *herramientaDebug, stJugador *player) {
    BeginDrawing(); ///Inicializa las funciones para dibujar (lo abre)
    ClearBackground(BLACK);
    DrawTexture(escenario[HABITACION].fondoEstatico, 500, 0, WHITE);
    DrawText("TEST", 600, 800, 80, YELLOW);
   // DrawTexture(escenario[HABITACION].subEscenarios[0], 500, 0, WHITE); ///Subescenario espejo
  // DrawTexture(escenario[HABITACION].subEscenarios[1], 500, 0, WHITE); ///Subescenario ventana
    //DrawTexture(escenario[COCINA].fondoEstatico, 500, 0, WHITE);
//    DrawTexture(escenario[JARDIN].fondoEstatico, 500, 0, WHITE);
//    DrawTexture(escenario[CREDITOS].fondoEstatico, 500, 0, WHITE);
//    DrawTexture(escenario[CREDITOS].botones[BACK], 880, 950, WHITE);
//    dibujarCreditos(escenario);
//    DrawTexture(escenario[MENU].fondoEstatico, 500, 0, WHITE);
//    DrawTexture(escenario[MENU].botones[PLAY], 880, 750, WHITE);//PLAY
//    DrawTexture(escenario[MENU].botones[BOTON], 550, 700, WHITE);//LOAD
//    DrawTexture(escenario[MENU].botones[BOTON], 550, 800, WHITE);//SCORE
//    DrawTexture(escenario[MENU].botones[BOTON], 1260, 700, WHITE);//SAVE
//    DrawTexture(escenario[MENU].botones[BOTON], 1260, 800, WHITE);//CREDITS
//    dibujarBotonesMenu(escenario);


    if( herramientaDebug->mostrarHerramienta == 1 ) {
        dibujarVentanaDebug(herramientaDebug);
    }

    actualizarTestDebug(escenario, herramientaDebug); //para que no quede atras del escenario

    EndDrawing(); ///Cierra las funciones para dibujar
}
void deinicializaciones() {
    CloseAudioDevice();
    CloseWindow();
}
///Funciones que deben de estar en librería
void crearNodoFila(stNodo **nuevoNodo, stEvento evento){
    (*nuevoNodo)->evento = evento;
    (*nuevoNodo)->siguiente = NULL;
    (*nuevoNodo)->anterior = NULL;
}
void crearNodoArbol(stArbol **arbol){
    stArbol *nuevoArbol = (stArbol *)malloc(sizeof(stArbol));
    nuevoArbol->derecha = NULL;
    nuevoArbol->izquierda = NULL;
    *arbol = nuevoArbol;
}
void agregarNodoEnFila(stFila *fila, stEvento evento){
///Siempre insertamos los datos al final
    stNodo *nuevoNodo = (stNodo*) malloc(sizeof(stNodo));
    crearNodoFila(&nuevoNodo, evento);
    if(fila->primero == NULL){///Si la fila esta vacia lo agrega al principio
            fila->primero = nuevoNodo;
            fila->ultimo = nuevoNodo;
    }else{
        fila->ultimo->siguiente = nuevoNodo;
        nuevoNodo->anterior = fila->ultimo;
        fila->ultimo = nuevoNodo;
    }
}

void mostrarNodo(stNodo *nodo) {
    printf("\n--------------------------------------------------");
    printf("\n idObjeto: %i", nodo->evento.idObjeto);
    printf("\n idEvento: %i", nodo->evento.idEvento);
    printf("\n activo: %i", nodo->evento.activo);
    printf("\n texto: %s", nodo->evento.texto);
    printf("\n anterior: %p", nodo->anterior);
    printf("\n actual: %p", nodo);
    printf("\n siguiente: %p", nodo->siguiente);
    printf("\n--------------------------------------------------");
}

void mostrarFila(stFila fila) {
    stNodo *indiceFila = fila.primero;
    if( indiceFila == NULL )
        printf("\nLa fila esta vacia.");
    if( indiceFila != NULL ) {
        while( indiceFila != NULL ) {
            mostrarNodo(indiceFila);
            indiceFila = indiceFila->siguiente;
        }
    }
}

void quitarPrimerNodoFila(stFila *fila){
    stNodo *nodoBorrar = (*fila).primero;

    fila->primero = (*fila).primero->siguiente;
    (*fila).primero->anterior = NULL;

    free(nodoBorrar);
    nodoBorrar = NULL;

    if((*fila).primero == NULL) (*fila).ultimo = NULL;
}

void actualizarTestDebug(stEscenario *escenario, Debug *herramientaDebug) {
    ///ZONA DE TESTEO DE ACTUALIZACION--------------------------------------

    ///las funciones de dibujado estan en actualizar porque en dibujar no funcionan, teniendo el mismo formato y funciones
    DrawRectangleLines(escenario[HABITACION].objetos[MUNIECA].hitbox.x, escenario[HABITACION].objetos[MUNIECA].hitbox.y, escenario[HABITACION].objetos[MUNIECA].hitbox.width, escenario[HABITACION].objetos[MUNIECA].hitbox.height, BLUE);
    DrawRectangleLines(escenario[HABITACION].objetos[ALFOMBRA].hitbox.x, escenario[HABITACION].objetos[ALFOMBRA].hitbox.y, escenario[HABITACION].objetos[ALFOMBRA].hitbox.width, escenario[HABITACION].objetos[ALFOMBRA].hitbox.height, RED);
    DrawRectangleLines(escenario[HABITACION].objetos[ESPEJO].hitbox.x, escenario[HABITACION].objetos[ESPEJO].hitbox.y, escenario[HABITACION].objetos[ESPEJO].hitbox.width, escenario[HABITACION].objetos[ESPEJO].hitbox.height, YELLOW);
    DrawRectangleLines(escenario[HABITACION].objetos[ZAPATOS].hitbox.x, escenario[HABITACION].objetos[ZAPATOS].hitbox.y, escenario[HABITACION].objetos[ZAPATOS].hitbox.width, escenario[HABITACION].objetos[ZAPATOS].hitbox.height, ORANGE);
    DrawRectangleLines(escenario[HABITACION].objetos[GUITARRA].hitbox.x, escenario[HABITACION].objetos[GUITARRA].hitbox.y, escenario[HABITACION].objetos[GUITARRA].hitbox.width, escenario[HABITACION].objetos[GUITARRA].hitbox.height, GREEN);
    DrawRectangleLines(escenario[HABITACION].objetos[CAMA].hitbox.x, escenario[HABITACION].objetos[CAMA].hitbox.y, escenario[HABITACION].objetos[CAMA].hitbox.width, escenario[HABITACION].objetos[CAMA].hitbox.height, PURPLE);
    DrawRectangleLines(escenario[HABITACION].objetos[VENTANA].hitbox.x, escenario[HABITACION].objetos[VENTANA].hitbox.y, escenario[HABITACION].objetos[VENTANA].hitbox.width, escenario[HABITACION].objetos[VENTANA].hitbox.height, YELLOW);
    DrawRectangleLines(escenario[HABITACION].objetos[CAJON1].hitbox.x, escenario[HABITACION].objetos[CAJON1].hitbox.y, escenario[HABITACION].objetos[CAJON1].hitbox.width, escenario[HABITACION].objetos[CAJON1].hitbox.height, YELLOW);
    DrawRectangleLines(escenario[HABITACION].objetos[CAJON2].hitbox.x, escenario[HABITACION].objetos[CAJON2].hitbox.y, escenario[HABITACION].objetos[CAJON2].hitbox.width, escenario[HABITACION].objetos[CAJON2].hitbox.height, BLUE);
//    DrawRectangleLines(escenario[HABITACION].objetos[CAJONERA].hitbox.x, escenario[HABITACION].objetos[CAJONERA].hitbox.y, escenario[HABITACION].objetos[CAJONERA].hitbox.width, escenario[HABITACION].objetos[CAJONERA].hitbox.height, PINK);
//    DrawRectangleLines(escenario[COCINA].objetos[CUADRO].hitbox.x, escenario[COCINA].objetos[CUADRO].hitbox.y, escenario[COCINA].objetos[CUADRO].hitbox.width, escenario[COCINA].objetos[CUADRO].hitbox.height, YELLOW);
//    DrawRectangleLines(escenario[COCINA].objetos[TACHO].hitbox.x, escenario[COCINA].objetos[TACHO].hitbox.y, escenario[COCINA].objetos[TACHO].hitbox.width, escenario[COCINA].objetos[TACHO].hitbox.height, RED);
//    DrawRectangleLines(escenario[COCINA].objetos[HELADERA].hitbox.x, escenario[COCINA].objetos[HELADERA].hitbox.y, escenario[COCINA].objetos[HELADERA].hitbox.width, escenario[COCINA].objetos[HELADERA].hitbox.height, GREEN);
//    DrawRectangleLines(escenario[COCINA].objetos[CUCHILLO].hitbox.x, escenario[COCINA].objetos[CUCHILLO].hitbox.y, escenario[COCINA].objetos[CUCHILLO].hitbox.width, escenario[COCINA].objetos[CUCHILLO].hitbox.height, BLUE);
//    DrawRectangleLines(escenario[COCINA].objetos[PARED].hitbox.x, escenario[COCINA].objetos[PARED].hitbox.y, escenario[COCINA].objetos[PARED].hitbox.width, escenario[COCINA].objetos[PARED].hitbox.height, WHITE);
//    DrawRectangleLines(escenario[COCINA].objetos[MESADA].hitbox.x, escenario[COCINA].objetos[MESADA].hitbox.y, escenario[COCINA].objetos[MESADA].hitbox.width, escenario[COCINA].objetos[MESADA].hitbox.height, VIOLET);
//    DrawRectangleLines(escenario[COCINA].objetos[PUERTA].hitbox.x, escenario[COCINA].objetos[PUERTA].hitbox.y, escenario[COCINA].objetos[PUERTA].hitbox.width, escenario[COCINA].objetos[PUERTA].hitbox.height, ORANGE);

    DrawRectangleLines(herramientaDebug->valorVariable[1] * 100, herramientaDebug->valorVariable[2] * 100, herramientaDebug->valorVariable[3] * 100, herramientaDebug->valorVariable[4] * 100, WHITE);

    ///-------------------------------------------------------------------
}

/*DibujarTestDebug y DibujarHitboxDebug nos rompen(cuando pasamos un valor de la herramienta debug por parámetro) el programa ? será problema del compilador?

void dibujarTestDebug(stEscenario *escenario, Debug *herramientaDebug) {
    ///ZONA DE TESTEO DE DIBUJO----------------------------------------

    dibujarHitboxDebug(escenario, herramientaDebug);

    ///----------------------------------------------------------------
}

void dibujarHitboxDebug(stEscenario *escenario, Debug *herramientaDebug) {

}
*/
///Ortiz Rocio, Ortiz Brisa, Olmos Fernando
///TP Programacion y Laboratorio 2, UTN Mar Del Plata 2023
