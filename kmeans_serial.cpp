#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>

using namespace std;

// Function to calculate Euclidean distance
double euclideanDistance(double* a, double* b) {
    return sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));
}

// Function to load CSV data
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

// K-means clustering function
void kmeans(double** data, int numPoints, int k, int maxIterations, string output_file) {
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

int main() {
    int numPoints = 400000;
    int k = 5;
    int maxIterations = 100000;
    double start, serial_time;
    
    double** data = new double*[numPoints];
    for (int i = 0; i < numPoints; i++) {
        data[i] = new double[2];
    }

    string input_file = "data/" + to_string(numPoints) + "_data.csv";
    string output_file = "output/" + to_string(numPoints) + "_results_serial.csv";
    
    load_CSV(input_file, data, numPoints);
    // Starting time measurement
    start = omp_get_wtime();
    kmeans(data, numPoints, k, maxIterations, output_file);

    // Measuring Execution Time
    serial_time = omp_get_wtime() - start;
    //Reporting Execution Time
    cout << "Tiempo de ejecucion en serial: " << serial_time << "\n";
    
    
    for (int i = 0; i < numPoints; i++) {
        delete[] data[i];
    }
    delete[] data;
    
    return 0;
}
