#include "ooo_cpu.h"

#define TAKEN true
#define NOT_TAKEN false

#define TAM_TABLA_LOCAL 1024      // Tamaño de la tabla de predicción local
#define TAM_HISTORIA_LOCAL 10      // Tamaño de la historia local 
#define NUM_CONT 1024    // Tamaño de la tabla de predicciones global
#define TAM_HISTORIA_GLOBAL 10 //Tamaño de la historia global
#define MAX_COUNTER 3        // Valor máx del contador de 2 bits del metapredictor
#define MIN_COUNTER 0        // Valor mín del contador de 2 bits del metapredictor

//LBP
int historia_local[TAM_TABLA_LOCAL];  // Tabla de historiales locales (10 a 12 bits por entrada)
int tabla_prediccion[TAM_TABLA_LOCAL];  // Tabla de predicciones locales (2 bits por entrada)
//GSHARE
int contador_global[NUM_CONT];    //Tabla de predicciones globales (2 bits por entrada)
uint16_t historia_global;         //se supone que este valor está contenido en un registro del procesador 
                                  //cualquiera y no computa para el cálculo del tamaño, lo marco como uint16_t por
                                  //tener un tamaño de 2 bytes (historia)
                                  
//METAPREDICTOR
int meta_predictor[NUM_CONT];      // Metapredictor (2 bits/entrada)

void O3_CPU::initialize_branch_predictor()
{
    for (int i = 0; i < TAM_TABLA_LOCAL; i++) {
        historia_local[i] = 0;      // Inicializa la historia local a 0
        tabla_prediccion[i] = 2;     // Inicializa los contadores locales a "weakly taken"
    }
    for (int i = 0; i < NUM_CONT; i++) {
        contador_global[i] = 2;      // Inicializa cada contador global a "weakly taken"
        meta_predictor[i] = 1;       // Inicializa el metapredictor para usar P1 al comienzo
    }
    historia_global = 0;             // Inicializa la historia global a 0
}

uint8_t O3_CPU::predict_branch(uint64_t pc)
{
    // indices para historia local y global
    int ind_local = pc % TAM_TABLA_LOCAL;
    int ind_global = (pc ^ historia_global) % NUM_CONT;

    //Predicciones de P1 y P2
    int pred_local = (tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL] >= 2) ? TAKEN : NOT_TAKEN;
    int pred_global = (contador_global[ind_global] >= 2) ? TAKEN : NOT_TAKEN;

    // Seleccion de predictor (en base al contador de 2 bits)
    if (meta_predictor[ind_global] < 2)
        return pred_local; //P1 si 00 o 01
    else
        return pred_global; // P2 si 10 o 11
}

void O3_CPU::last_branch_result(uint64_t pc, uint8_t taken)
{

//cuidado! aquí entra la limitación de implementar este tipo de predictor en este entorno, se va a devolver
//si el salto se ha tomado o no, pero la prediccion antes calculada pudo venir de dos predictores distintos
//es necesario volver a calcular lo que se predijo (de nuevo, utilizando el pc) para poder actualizar en consecuencia
//ejemplo: si se escoge P1 y P1 falla, hemos de penalizar a P1, es decir, si fuese 01, actualizar a 10. si
//arbitrariamente hacemos contador-- pensando que "restar" penaliza, pasaríamos de 01 a 00, sería completamente inconsistente.
//si se modificase el programa de manera que se devolviese lo que se predijo este procedimiento no tendría que 
//recalcular todo y sería más eficiente.

    //indices
    int ind_local = pc % TAM_TABLA_LOCAL;
    int ind_global = (pc ^ historia_global) % NUM_CONT;

    // calculo de las predicciones de P1 y P2
    int pred_local = (tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL] >= 2) ? TAKEN : NOT_TAKEN;
    int pred_global = (contador_global[ind_global] >= 2) ? TAKEN : NOT_TAKEN;

    // Actualizacion de los predictores locales y globales
    //utilizamos si fue o no fue tomado porque AQUI no nos importa quien lo predijo, solo nos interesa 
    //almacenar el comportamiento del salto
    if (taken) {
        if (tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL] < MAX_COUNTER)
            tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL]++;
        if (contador_global[ind_global] < MAX_COUNTER)
            contador_global[ind_global]++;
    } else {
        if (tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL] > MIN_COUNTER)
            tabla_prediccion[historia_local[ind_local] % TAM_TABLA_LOCAL]--;
        if (contador_global[ind_global] > MIN_COUNTER)
            contador_global[ind_global]--;
    }

    // actualizar el metapredictor basado en las reglas establecidas (ver documentacion)
    //aquí es cuando se hace necesario usar las predicciones calculadas nuevamente
    if (pred_local != taken && pred_global == taken) { //P1 FALLA, P2 ACIERTA
        if (meta_predictor[ind_global] < MAX_COUNTER)
            meta_predictor[ind_global]++;
    } else if (pred_local == taken && pred_global != taken) { //P1 ACIERTA, P2 FALLA
        if (meta_predictor[ind_global] > MIN_COUNTER)
            meta_predictor[ind_global]--;
    }

    // actualizamos la historia local y global
    historia_local[ind_local] = ((historia_local[ind_local] << 1) | (taken ? 1 : 0)) & ((1 << TAM_HISTORIA_LOCAL) - 1);
    historia_global = ((historia_global << 1) | (taken ? 1 : 0)) & ((1 << TAM_HISTORIA_GLOBAL) - 1);
}

