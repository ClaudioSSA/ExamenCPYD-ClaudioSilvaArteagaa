#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>               //Libreria utilizada para paralelizar funciones
#define MAX_CHAR_PER_LINE 100
#define MAX_NUM_DATES 500000 // Definicion de MAX_NUM_DATES, basicamente define hasta que tamaño de arreglo se podria crear

// Estructura para almacenar cada una de las fechas registradas
struct Fecha {
    int year;
    int month;
    int day;
    int grupo;
};
// Funcion para calcular la edad basada en la fecha actual (2023-07-16)
long long int calcularEdad(struct Fecha fecha) {
    long long int edad = 2023 - fecha.year;

    // Restar un año si aun no se ha cumplido el mes o el dia de la fecha actual
    if (fecha.month > 7 || (fecha.month == 7 && fecha.day > 16)) {
        edad--;
    }

    return edad;
}



void asignarGrupo(struct Fecha *fecha) {
    long long int edad = calcularEdad(*fecha);
//Utilizamos la siguiente directiva para paralelizar el código de la función asignarGrupo
//Esta función recibe como argumento la fecha, almacenada en el struct definido al comienzo.
// La directiva pragma omp single se utiliza para asegurar que este bloque de código dentro de ella se ejecute solo una vez
// y que solo un hilo realice las asignaciones en función de la edad.
    #pragma omp parallel
    {
        #pragma omp single
        {
            if (edad < 5) {
                fecha->grupo = 1; // Bebes
            } else if (edad < 12) {
                fecha->grupo = 2; // Niños
            } else if (edad < 18) {
                fecha->grupo = 3; // Adolescentes
            } else if (edad < 25) {
                fecha->grupo = 4; // Jovenes
            } else if (edad < 40) {
                fecha->grupo = 5; // Adultos jovenes
            } else if (edad < 55) {
                fecha->grupo = 6; // Adultos
            } else if (edad < 65) {
                fecha->grupo = 7; // Adultos mayores
            } else if (edad < 75) {
                fecha->grupo = 8; // Ancianos
            } else {
                fecha->grupo = 9; // Longevos
            }
        }
    }
}

const char *nombresGrupos[] = {
        "Bebes",
        "Ninos",
        "Adolescentes",
        "Jovenes",
        "Adultos jovenes",
        "Adultos",
        "Adultos mayores",
        "Ancianos",
        "Longevos"
    };

int main() {
    // Ruta del archivo CSV
    const char *file_path = "nacimientos.csv";
    long long int Contadores[9] = {0};
    // Abrir el archivo en modo lectura
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        return 1;
    }

    char line[MAX_CHAR_PER_LINE];
    long long int count = 0;
    long long int block_count = 0; // Contador para el bloque actual
    long long int current_block_count = 0; // Contador para los registros dentro del bloque actual

    struct Fecha *fechas = (struct Fecha *)malloc(sizeof(struct Fecha) * 500000); // Inicializar memoria para el primer bloque
    if (fechas == NULL) {
        printf("Error al asignar memoria para el arreglo de fechas.\n");
        fclose(file);
        return 1;
    }

    // leer y omitir la primera fila que contiene el encabezado "fecha"
    if (fgets(line, MAX_CHAR_PER_LINE, file) == NULL) {
        printf("Error al leer el archivo.\n");
        free(fechas); // Liberar memoria antes de salir en caso de error
        fclose(file);
        return 1;
    }

    // leer e imprimir los valores
    // Se dividen los registros con el fin de optimizar el proceso y liberar memoria una vez llegada a la mitad de registros.
    while (fgets(line, MAX_CHAR_PER_LINE, file) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (sscanf(line, "\"%d-%d-%d\"", &fechas[current_block_count].year, &fechas[current_block_count].month, &fechas[current_block_count].day) == 3) {
            asignarGrupo(&fechas[current_block_count]);
            Contadores[fechas[current_block_count].grupo-1]++;
            count++;
            current_block_count++;
            // Verificar si se lleg� a un bloque de 500,000 registros
            if (current_block_count == 500000) {
                //printf("Se ha alcanzado el bloque de 500,000 registros. Contador actual: %lld\n", count);
                block_count++;

                // Liberar la memoria para el bloque actual
                free(fechas);
                fechas = NULL;

                // Asignar memoria nuevamente para el siguiente bloque
                fechas = (struct Fecha *)malloc(sizeof(struct Fecha) * 500000);
                if (fechas == NULL) {
                    printf("Error al asignar memoria para el siguiente bloque de fechas.\n");
                    fclose(file);
                    return 1;
                }

                current_block_count = 0; // Reiniciar el contador para el siguiente bloque
            }
        } else {
            printf("Error al procesar la fecha: %s\n", line);
        }
    }

    fclose(file);

    // Imprimir el resultado final
    printf("Se han leido en total %lld registros.\n", count);
    //printf("Se han procesado %d bloques.\n", block_count);
    //for (int i=0; i<9;i++){
       // double porcentaje = (double)Contadores[i] / count * 100;
        //printf("Grupo %s: Registros:%lld (%.2f%%)\n", nombresGrupos[i], Contadores[i], porcentaje);
    //}
    // Liberar la memoria asignada para el ultimo bloque
    free(fechas);
    // resultados finales

    int year, month, day;
    char fecha_str[11];
    int fecha_valida = 0;

    do {
        // Obtener la fecha desde el usuario en formato ISO 8601 (AAAA-MM-DD)
        printf("Ingrese la fecha en formato AAAA-MM-DD: ");
        if (scanf("%10s", fecha_str) != 1) {
            printf("Error al leer la fecha.\n");
            return 1;
        }

        // Verificar si la fecha tiene el formato correcto
        if (sscanf(fecha_str, "%d-%d-%d", &year, &month, &day) != 3) {
            printf("Error: Fecha invalida. Formato debe ser AAAA-MM-DD\n");
        } else {
            // Verificar si los valores de mes y d�a son v�lidos
            if (month < 1 || month > 12 || day < 1 || day > 31) {
                printf("Error: Fecha invalida. Valores de mes y dia fuera de rango.\n");
            } else {
                fecha_valida = 1;
            }
        }
    } while (!fecha_valida);

    // Verificar el grupo etario de la fecha proporcionada
    struct Fecha fecha;
    fecha.year = year;
    fecha.month = month;
    fecha.day = day;
    asignarGrupo(&fecha);

    // Obtener el porcentaje del grupo
    double porcentaje = (double)Contadores[fecha.grupo - 1] / count * 100;

    // Imprimir el resultado
    printf("Grupo Etario %s: \nPorcentaje: %.2f%%\n", nombresGrupos[fecha.grupo - 1], porcentaje);

    return 0;
}

