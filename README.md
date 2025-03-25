# K-Means-OMP
K-Means algorithm implemented using Open MP. First project for ITAM's course Parallel Computing
![logo itam](image.png)
## Documentación de proyecto de apertura: comparación de implementaciones serial y paralela con K-Means

Elaborado por Guillermo Arredondo Renero. C.U. 197256
ITAM, Cómputo Paralelo bajo tutela del Profesor Octavio Gutiérrez


### Introducción 
En este proyecto se implementó un código del algortimo de K means que recibiera un conjunto de datos en dos dimensiones a partir de un archivo de *csv*. Los datos fuente se pueden encontrar dentro de la carpeta [data](https://github.com/guillermoArr/K-Means-OMP/tree/main/data), donde cada uno de  los archivos se genera al ejecutar las primeras dos celdas del notebook [synthetic_clusters](https://github.com/guillermoArr/K-Means-OMP/blob/main/synthetic_clusters.ipynb). A partir de estos datos, las funciones desarrolladas dentro de **kmeans_parallel.cpp**, **kmeans_serial.cpp** y **kmeans_pruebas.cpp** generaran un nuevo archivo *csv* con los datos y la correspondiente asignación de cluster resultado del algoritmo kmeans implementado. Los resultados se almacenan dentro de la carpeta **output**, pero se omiten con .gitignore para evitar una sobre carga de archivos. 

#### Descripción de archivos:
- synthetic_clusters.ipynb: notebook aportado por el profesor Octavio para generación de datos de prueba para el algoritmo de kmeans. Se realizaron algunas modificaciones para poder mostrar los resultados tanto del algoritmo serial como del paralelo.
- kmeans_serial.cpp: implementación serial de K-means utilizada como referencia para la implementación paralela. El main provee una forma rápida de probar los resultados del algoritmo.
- kmeans_parallel.cpp: implementación paralela de K-means utilizando la librería OMP. El main provee una forma rápida de probar los resultados del algoritmo.
- kmeans_pruebas.cpp: archivo de experimento para comparación de implementaciones. Dentro del main se ejecuta el experimento descrito en la siguiente sección.
- speedups_graph.ipynb: notebook diseñado para generar las gráficas de speedup que se muestran en este reporte.

### Descripción de experimento
Para realizar la comparación entre las implementaciones serial y paralela se diseñó un experimento que permitiera obtener los tiempos de ejecución de cada implementación considerando distintos tamaños de datos y distinto número de hilos a utilizar. Además se tomó el promedio de 10 iteraciones para cada una de las combinaciones entre ambos parámetros. Dado que la idea es obtener la aceleración (*speedup*) de la implementación de K-means con OpenMP respecto a la versión serial se considera la siguiente fórmula $$\text{Speedup}_{n,p} = \frac{\text{Tiempo serial}}{\text{Tiempo en paralelo}}$$ donde *n* = Número de puntos a clusterizar y *p* = Número de hilos utilizados. El tiempo de ejecución se obtiene a través del paquete de OpenMP con la función *omp_get_wtime()*. Además, para asegurar una buena comparación los métodos son llamados directamente dentro del **main** recibiendo la dirección de memoria del conjunto de datos, así como la dirección de memoria del arreglo en el que guardará los resultados (*clusterAssignment*). De esta forma es posible generar el output de cada método con una función de escritura en archivos *csv* que no se considere en la medición del tiempo de ejecución. 


**Metodología:**
1. Carga de datos: Se lee un archivo CSV con los puntos de datos de entrada.

2. Ejecución Serial: Se ejecuta el algoritmo K-Means en modo secuencial y se mide el tiempo de ejecución promedio de 10 repeticiones.

3. Ejecución Paralela: Se ejecuta la versión paralela de K-Means para cada configuración de hilos, midiendo el tiempo de ejecución promedio en 10 repeticiones.

4. Cálculo de Speedup y generación de resultados en archivo *csv* (speedups.csv), con el siguiente formato:  
```python-repl
DataSize,NumThreads,SerialTime,ParallelTime,Speedup
100000,1,0.52,0.50,1.04
100000,4,0.52,0.20,2.60
100000,8,0.52,0.15,3.47
100000,16,0.52,0.12,4.33
...
```
    Estos resultados se utilizan después para generar gráficas en función del número de threads.

5. Liberación de memoria para arreglos dinámicos utilizados durante el experimento (*data* y *clusterAssignment*).


**Parámetros del experimento:**
1. Tamaños de datos: Se utilizan conjuntos de datos de diferentes tamaños para analizar el impacto del volumen de datos en el rendimiento. Los tamaños considerados son:
    - 100,000 puntos
    - 200,000 puntos
    - 300,000 puntos
    - 400,000 puntos
    - 600,000 puntos
    - 800,000 puntos
    - 1,000,000 puntos
2. Número de threads en la ejecución paralela: Se varía la cantidad de hilos utilizados en OpenMP para analizar cómo influye el número de núcleos en la aceleración. Se prueban cuatro configuraciones:
    - 1 hilo
    - 4 hilos (mitad de los hilos disponibles en el sistema) 
    - 8 hilos (cantidad de hílos disponibles)
    - 16 hilos (duplicando el número de hilos disponibles para evaluar el *overhead* por exceso)
3. Número de iteraciones: Cada configuración se ejecuta 10 veces y se calcula el tiempo promedio para minimizar la variabilidad de los resultados.
4. Número de clusters (*k*): hiperparámetro de la implementación de K means (tanto serial como paralela) que debe asignarse como parámetro durante la ejecución del archivo ejecutable. Establece el número de clusters que buscará el algoritmo.
5. Número máximo de iteraciones (*maxIterations*): hiperparámetro de la implementación de K means (tanto serial como paralela) que debe asignarse como parámetro durante la ejecución del archivo ejecutable. Establece el tope de iteraciones antes de detener el algoritmo en caso de no converger.

#### Condiciones de experimento
El experimento se ejecuta con una Laptop en la que nos aseguramos de tener enchufada a la corriente eléctrica para evitar condiciones de ahorro de energía, así como evitar tener toda tarea no esencial en ejecución al mismo tiempo. El equipo consiste de las siguientes características:

* Hardware:
    - Modelo: Laptop HP EliteBook 840 G6
    - Procesador: Intel(R) Core(TM) i7-8665U CPU @ 1.90GHz 2.11 GHz
    - RAM instalada 16.0 GB (15.8 GB utilizable)
    - Tipo de sistema: Sistema operativo de 64 bits, procesador x64
    - Núcleos: 4
    - Procesadores lógicos: 8
    - Caché: 
        - L1: 256 kB
        - L2: 1.0 MB
        - L3: 8.0 MB
* Software:
    - Sistema operativo
        - Edición Windows 11 Home Single Language
        - Versión 24H2
        - Se instaló el 09/03/2025
        - Compilación del SO: 26100.3476
        - Experiencia: Paquete de experiencia de características de Windows 1000.26100.54.0
    - Compilador
        - Se utiliza el compilador g++ para C/C++ 
        - g++.exe (Rev2, Built by MSYS2 project) 14.2.0. Copyright (C) 2024 Free Software Foundation, Inc.

### Resultados




### Discusión

## Anexo 1
#### Código

``` C++
int main(int argc, char** argv) {
    // Create/Overwrite the CSV file and write headers
    ofstream out("output/speedups.csv");
    if (!out.is_open()) {
        cerr << "Error: Could not create speedup_results.csv\n";
        return 1;
    }
    out << "DataSize,NumThreads,SerialTime,ParallelTime,Speedup\n"; // Column headers
    out.close();

    // Experiment design
    int max_threads = omp_get_max_threads();
    int num_threads[4] = {1, max_threads/2, max_threads, max_threads*2};
    int num_points[7] = {100000, 200000, 300000, 400000, 600000, 800000, 1000000};
    double start, serial_time, parallel_time;

    // Converting the first command-line argument (argv[1]) into an integer for maximum number of iterations
    const int max_iterations = atoi(argv[1]);

    // Converting the second command-line argument (argv[2]) into an integer that represents the number of clusters (num_clusters) to be used in the K-means algorithm
    const int num_clusters = atoi(argv[2]);
    
    
    for (int data_size : num_points){
        string input = "data/" + to_string(data_size) + "_data.csv";
        char const *input_file_name = input.c_str();

        // Parameters for each k means function
        double** data = new double*[data_size];
        int* clusterAssignment = new int[data_size];

        for (long long int i = 0; i < data_size; i++) {
            data[i] = new double[3]{0.0, 0.0, -1};  // Memory allocation for 2D points and cluster assignments
        }

        load_CSV(input_file_name, data, data_size);

        // SERIAL execution
        cout << "Ejecutando kmeans serial para " << input_file_name << " de tamanio " << data_size << " buscando " << num_clusters << " clusters\n";
        string output_serial = "output/" + to_string(data_size) + "_results_serial_";

        // Run the serial experiment 10 times and calculate the average time
        double total_serial_time = 0.0;
        for (int i = 0; i < 10; i++) {
            start = omp_get_wtime();
            
            // Execute the K-means Serial Algorithm
            kmeans_serial(data, data_size, num_clusters, max_iterations, clusterAssignment);
            
            // Measure Execution Time for Serial
            total_serial_time += omp_get_wtime() - start;
            
            // Write results to csv
            save_to_CSV(output_serial + to_string(i) + ".csv", data, data_size, clusterAssignment);
            
        }
        serial_time = total_serial_time / 10.0;

        // Report Execution Time for Serial
        cout << "Tiempo de ejecucion promedio en serial: " << serial_time << " segundos.\n";

        string output_parallel = "output/" + to_string(data_size) + "_results_parallel_";

        for (int threads : num_threads) {
            omp_set_num_threads(threads);
            cout << "\nRunning with " << threads << " threads:" << endl;

            // Parallel execution with the same data
            double total_parallel_time = 0.0;
            for (int i = 0; i < 10; i++) {
                start = omp_get_wtime();

                // Execute the K-means Parallel Algorithm
                kmeans_paralelo(data, data_size, num_clusters, max_iterations, clusterAssignment);

                total_parallel_time += (omp_get_wtime() - start);

                // Write results to csv
                save_to_CSV(output_parallel + to_string(i) + ".csv", data, data_size, clusterAssignment);
            }

            parallel_time = total_parallel_time / 10.0;
            
            cout << "Tiempo de ejecucion promedio paralelo con " << threads << " threads: " << parallel_time << " segundos" << endl;
            cout << "Speedup: " << serial_time / parallel_time << "x\n";

            // Save speedup results to CSV
            save_speedup_results(data_size, threads, serial_time, parallel_time);
        }

        // Clean up dynamically allocated memory
        for (long long int i = 0; i < data_size; i++) {
            delete[] data[i];
        }
        delete[] data;
        delete[] clusterAssignment;
    }

    // Program exit
    return 0;
}
```
