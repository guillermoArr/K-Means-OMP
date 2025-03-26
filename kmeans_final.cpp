#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <omp.h>

using namespace std;
using namespace std::chrono;

/*
    CSV management functions

    Reading data from a CSV file
*/

void load_CSV(string file_name, double** points, long long int size) {
    ifstream in(file_name);
    if (!in) {
        cerr << "Couldn't read file: " << file_name << "\n";
        return;
    }

    long long int point_number = 0;
    string line;
    while (getline(in, line) && point_number < size) {
        istringstream iss(line);
        char delimiter;
        iss >> points[point_number][0] >> delimiter >> points[point_number][1];
        point_number++;
    }
    in.close();
}
/*
    Writing data to a CSV file
*/
void save_to_CSV(string file_name, double** data, long long int numPoints, int* clusterAssignment) {
    ofstream out(file_name);

    if (!out.is_open()){
        // Priting message of unsucessful open file
        cerr << "Couldn't write to file: " << file_name << "\n";
        return;
    } 

    for (int i = 0; i < numPoints; i++) {
        out << data[i][0] << "," << data[i][1] << "," << clusterAssignment[i] << "\n";
    }
    out.close();
    
}

// Function to append speedup results to a CSV file
void save_speedup_results(int data_size, int num_threads, double serial_time, double parallel_time) {
    ofstream out("output/speedups2.csv", ios::app); // Open file in append mode
    if (!out.is_open()) {
        cerr << "Error: Could not open speedup_results2.csv\n";
        return;
    }

    double speedup = serial_time / parallel_time;
    out << data_size << "," << num_threads << "," << serial_time << "," << parallel_time << "," << speedup << "\n";
    out.close();
}

/*
    Euclidean distance
*/
double euclideanDistance(double* a, double* b) {
    return sqrt(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2));
}
/* 
    K_MEANS 
*/

/** SERIAL VERSION
 *  Performs the k-means algorithm for the given data points and returns the assigned cluster id.
 *  Array of data where each row represents a point with "x", "y" coordinates and its cluster assignment
 *  @param data  
 *  Number of desired clusters
 *  @param k 
 *  Data set size (number of points).    
 *  @param numPoints
 *  Maximum number of iterations allowed for the algorithm           
 *  @param maxIterations
 *  String file name for output result
 *  @param output_file
 */

 void kmeans_serial(double** data, int numPoints, int k, int maxIterations, int* clusterAssignment) {
    double** centroids = new double*[k];
    for (int i = 0; i < k; i++) {
        centroids[i] = new double[2];
    }

    for (int i = 0; i < k; i++) {
        int randIndex = rand() % numPoints;
        centroids[i][0] = data[randIndex][0];
        centroids[i][1] = data[randIndex][1];
    }

    bool changed = true;
    int iter = 0;

    int* clusterSizes = new int[k];
    double** newCentroids = new double*[k];
    for (int i = 0; i < k; i++) {
        newCentroids[i] = new double[2];
    }

    while (changed && iter < maxIterations) {
        changed = false;
        iter++;

        for (int i = 0; i < numPoints; i++) {
            double minDist = euclideanDistance(data[i], centroids[0]);
            int bestCluster = 0;
            for (int j = 1; j < k; j++) {
                double dist = euclideanDistance(data[i], centroids[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = j;
                }
            }
            if (clusterAssignment[i] != bestCluster) {
                clusterAssignment[i] = bestCluster;
                changed = true;
            }
        }

        if (!changed) break;

        for (int i = 0; i < k; i++) {
            clusterSizes[i] = 0;
            newCentroids[i][0] = 0.0;
            newCentroids[i][1] = 0.0;
        }

        for (int i = 0; i < numPoints; i++) {
            int cluster = clusterAssignment[i];
            clusterSizes[cluster]++;
            newCentroids[cluster][0] += data[i][0];
            newCentroids[cluster][1] += data[i][1];
        }

        for (int i = 0; i < k; i++) {
            if (clusterSizes[i] > 0) {
                centroids[i][0] = newCentroids[i][0] / clusterSizes[i];
                centroids[i][1] = newCentroids[i][1] / clusterSizes[i];
            }
        }
    }
    
    for (int i = 0; i < k; i++) {
        delete[] centroids[i];
        delete[] newCentroids[i];
    }
    delete[] newCentroids;
    delete[] clusterSizes;
    delete[] centroids;
}


 /** PARALLEL VERSION
 *  Performs the k-means algorithm using OMP.
 *  Array of data points where each row represents a point with "x", "y" coordinates and its assigned cluster
 *  @param data  
 *  Number of desired clusters
 *  @param k 
 *  Data set size (number of rows/points).    
 *  @param numPoints
 *  Maximum number of iterations allowed for the algorithm           
 *  @param maxIterations
 *  String file name for output result
 *  @param output_file
 */

 void kmeans_paralelo(double** data, int numPoints, int k, int maxIterations, int* clusterAssignment) {
    // Initialize centroids (random)
    double** centroids = new double*[k];
    // #pragma omp parallel for
    for (int i = 0; i < k; i++) {
        centroids[i] = new double[2];
    }
    
    for (int i = 0; i < k; i++) {
        int randIndex = rand() % numPoints;
        centroids[i][0] = data[randIndex][0];
        centroids[i][1] = data[randIndex][1];
    }
    
    // Pre-allocate memory for cluster updates
    int* clusterSizes = new int[k];
    double** newCentroids = new double*[k];
    for (int i = 0; i < k; i++) {
        newCentroids[i] = new double[2];
    }

    bool changed = true;
    int iter = 0;

    // Main loop - until convergance or max iterations are reached
    while (changed && iter < maxIterations) {
        changed = false;
        iter++;

        #pragma omp parallel for reduction(||:changed) schedule(static) //dynamic, 1000
        for (int i = 0; i < numPoints; i++) {
            double minDist = euclideanDistance(data[i], centroids[0]);
            int bestCluster = 0;
            for (int j = 1; j < k; j++) {
                double dist = euclideanDistance(data[i], centroids[j]);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = j;
                }
            }
            if (clusterAssignment[i] != bestCluster) {
                clusterAssignment[i] = bestCluster;
                changed = true;
            }
        }

        if (!changed) break;

        // (Re)set accumulators
        for (int i = 0; i < k; i++) {
            clusterSizes[i] = 0;
            newCentroids[i][0] = 0.0;
            newCentroids[i][1] = 0.0;
        }

        #pragma omp parallel
        {
            // Thread-local accumulators
            int* localSizes = new int[k]();
            double** localSums = new double*[k];
            for (int i = 0; i < k; i++) {
                localSums[i] = new double[2]{0.0, 0.0};
            }

            // Accumulate local sums
            #pragma omp for schedule(dynamic, 1000)
            for (int i = 0; i < numPoints; i++) {
                int cluster = clusterAssignment[i];
                localSizes[cluster]++;
                localSums[cluster][0] += data[i][0];
                localSums[cluster][1] += data[i][1];
            }

            // Combine results
            #pragma omp critical
            {
                for (int i = 0; i < k; i++) {
                    clusterSizes[i] += localSizes[i];
                    newCentroids[i][0] += localSums[i][0];
                    newCentroids[i][1] += localSums[i][1];
                }
            }

            // Clean up thread-local memory
            for (int i = 0; i < k; i++) {
                delete[] localSums[i];
            }
            delete[] localSums;
            delete[] localSizes;
        }
        
        // update centroids
        //#pragma omp parallel for
        for (int i = 0; i < k; i++) {
            if (clusterSizes[i] > 0) {
                centroids[i][0] = newCentroids[i][0] / clusterSizes[i];
                centroids[i][1] = newCentroids[i][1] / clusterSizes[i];
            }
        }

    }

    for (int i = 0; i < k; i++) {
        delete[] centroids[i];
        delete[] newCentroids[i];
    }
    delete[] newCentroids;
    delete[] clusterSizes;
    delete[] centroids;
}

 int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <max_iterations> <num_clusters> <seed>\n";
        return 1;
    }
    // // Create/Overwrite the CSV file and write headers
    // ofstream out("output/speedups2.csv");
    // if (!out.is_open()) {
    //     cerr << "Error: Could not create speedup_results2.csv\n";
    //     return 1;
    // }
    // out << "DataSize,NumThreads,SerialTime,ParallelTime,Speedup\n"; // Column headers
    // out.close();

    // Experiment design
    int max_threads = omp_get_max_threads();
    int num_threads[1] = {max_threads/2};
    int num_points[1] = {980000};
    double start, serial_time, parallel_time;

    // Converting the first command-line argument (argv[1]) into an integer for maximum number of iterations
    const int max_iterations = atoi(argv[1]);

    // Converting the second command-line argument (argv[2]) into an integer that represents the number of clusters (num_clusters) to be used in the K-means algorithm
    const int num_clusters = atoi(argv[2]);
    
    // User provided seed 
    const int seed = atoi(argv[3]);  

    // Set the seed for reproducibility
    srand(seed);
    
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
        for (int i = 0; i < 1; i++) {
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
        //save_speedup_results(data_size, 0, serial_time, serial_time);

        string output_parallel = "output/" + to_string(data_size) + "_results_parallel_";

        for (int threads : num_threads) {
            omp_set_num_threads(threads);
            cout << "\nRunning with " << threads << " threads:" << endl;

            // Parallel execution with the same data
            double total_parallel_time = 0.0;
            for (int i = 0; i < 1; i++) {
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
            //save_speedup_results(data_size, threads, serial_time, parallel_time);
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