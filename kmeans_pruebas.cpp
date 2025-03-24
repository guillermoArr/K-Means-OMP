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
int save_to_CSV(string file_name, double** points, long long int size) {
    ofstream fout(file_name);

    // Checking if the file was successfully opened for writing
    if (!fout.is_open()) {

        // Priting message of unsucessful open file
        cerr << "Couldn't write to file: " << file_name << "\n";

        // Exit the function
        return -1;

    }

    // For loop to iterate over each in the points array
    for (long long int i = 0; i < size; i++) {

        // Writing the x-coordindate, the y-coordinate and the cluster id
        fout << points[i][0] << "," << points[i][1] << "," << points[i][2] << "\n";
    }

    // Closing the file
    fout.close();
    return 0;
}

/*
    Euclidean distance
*/
double euclideanDistance(double* a, double* b) {
    return sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));
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

 void kmeans_serial(double** data, int numPoints, int k, int maxIterations, string output_file) {
    double** centroids = new double*[k];
    for (int i = 0; i < k; i++) {
        centroids[i] = new double[2];
    }
    int* clusterAssignment = new int[numPoints];
    
    srand(time(0));
    for (int i = 0; i < k; i++) {
        int randIndex = rand() % numPoints;
        centroids[i][0] = data[randIndex][0];
        centroids[i][1] = data[randIndex][1];
    }

    bool changed = true;
    int iter = 0;
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

        int* clusterSizes = new int[k]();
        double** newCentroids = new double*[k];
        for (int i = 0; i < k; i++) {
            newCentroids[i] = new double[2]{0.0, 0.0};
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

        for (int i = 0; i < k; i++) {
            delete[] newCentroids[i];
        }
        delete[] newCentroids;
        delete[] clusterSizes;
    }

    ofstream out(output_file);
    for (int i = 0; i < numPoints; i++) {
        out << data[i][0] << "," << data[i][1] << "," << clusterAssignment[i] << "\n";
    }
    out.close();

    for (int i = 0; i < k; i++) {
        delete[] centroids[i];
    }
    delete[] centroids;
    delete[] clusterAssignment;
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

 void kmeans_paralelo(double** data, int numPoints, int k, int maxIterations, string output_file) {
    double** centroids = new double*[k];
    #pragma omp parallel for
    for (int i = 0; i < k; i++) {
        centroids[i] = new double[2];
    }
    int* clusterAssignment = new int[numPoints];
    
    srand(time(0));
    for (int i = 0; i < k; i++) {
        int randIndex = rand() % numPoints;
        centroids[i][0] = data[randIndex][0];
        centroids[i][1] = data[randIndex][1];
    }

    bool changed = true;
    int iter = 0;
    while (changed && iter < maxIterations) {
        changed = false;
        iter++;

        #pragma omp parallel for schedule(static)
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

        int* clusterSizes = new int[k]();
        double** newCentroids = new double*[k];
        for (int i = 0; i < k; i++) {
            newCentroids[i] = new double[2]{0.0, 0.0};
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < numPoints; i++) {
            int cluster = clusterAssignment[i];
            #pragma omp atomic
            clusterSizes[cluster]++;
            
            #pragma omp critical
            {
                newCentroids[cluster][0] += data[i][0];
                newCentroids[cluster][1] += data[i][1];
            }
        }

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < k; i++) {
            if (clusterSizes[i] > 0) {
                centroids[i][0] = newCentroids[i][0] / clusterSizes[i];
                centroids[i][1] = newCentroids[i][1] / clusterSizes[i];
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < k; i++) {
            delete[] newCentroids[i];
        }
        delete[] newCentroids;
        delete[] clusterSizes;
    }

    ofstream out(output_file);
    for (int i = 0; i < numPoints; i++) {
        out << data[i][0] << "," << data[i][1] << "," << clusterAssignment[i] << "\n";
    }
    out.close();

    for (int i = 0; i < k; i++) {
        delete[] centroids[i];
    }
    delete[] centroids;
    delete[] clusterAssignment;
}



 int main(int argc, char** argv) {
    // Experiment design
    int max_threads = omp_get_max_threads();
    int num_threads[4] = {1, max_threads/2, max_threads, max_threads*2};
    int num_points[7] = {100000, 200000, 300000, 400000}; //, 600000, 800000, 1000000
    double start, serial_time, parallel_time;

    // Converting the first command-line argument (argv[1]) into an integer for maximum number of iterations
    const int max_iterations = atoi(argv[1]);

    // Converting the second command-line argument (argv[2]) into an integer that represents the number of clusters (num_clusters) to be used in the K-means algorithm
    const int num_clusters = atoi(argv[2]);
    
    for (int data_size : num_points){
        string input = "data/" + to_string(data_size) + "_data.csv";
        char const *input_file_name = input.c_str();

        double** data = new double*[data_size];

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
            kmeans_serial(data, data_size, num_clusters, max_iterations, output_serial + to_string(i) + ".csv");

            // Measure Execution Time for Serial
            total_serial_time += omp_get_wtime() - start;
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
                kmeans_paralelo(data, data_size, num_clusters, max_iterations, output_parallel + to_string(i) + ".csv");

                total_parallel_time += (omp_get_wtime() - start);
            }

            parallel_time = total_parallel_time / 10.0;
            
            cout << "Tiempo de ejecucion promedio paralelo con " << threads << " threads: " << parallel_time << " segundos" << endl;
            cout << "Speedup: " << serial_time / parallel_time << "x" << endl;
        }

        // Clean up dynamically allocated memory
        for (long long int i = 0; i < data_size; i++) {
            delete[] data[i];
        }
        delete[] data;
    }

    // Program exit
    return 0;
}