#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>


using namespace std;
int number_rows;
int number_cells;
int **mesh; //The array to be sorted
int phase;  //Amount of phases we need to go through
int phase_counter; //Current phase number
pthread_t *threads; //Collection of threads
int *thread_phase; //Tracks the phase of the threads
sem_t *semaphores; //Collection of semaphores coresponding to threads

/*
This function checks to see if every thread is on the same phase as is exected
Returns a false boolean if one or more threads is not on the ideal phase
Doesn't take any parameters
 */
bool check_threads()
{
    for(int i = 0; i < number_rows; i++) {
        if(thread_phase[i] != phase_counter) {
            return false;
        }
    }
    return true;
}
/*
A simple printing function. Displays the array to the screen.

 */
void print_array(int **mesh, int number_rows)
{
    for(int i = 0; i < number_rows; i++) {
        for(int j = 0; j < number_rows; j++) {
            std::cout<<mesh[i][j];
            std::cout<<" ";
        }
        std::cout<<"\n";
    }
}
/*Sorts the arrays that need to face left.
Very simply done, no fuss necessary. No value is returned
row_num is the row that is to be sorted. The phase is the current phase
This variable is largely unnecessary but was useful for testing
 */

void row_sort_left(int row_num, int phase, int **mesh)
{
    int i, j;
    int n = number_rows;
    int temp;
    bool swapped;
    for (i = 0; i < n-1; i++) {
        swapped = false;
        for (j = 0; j < n-i-1; j++) {
            if (mesh[row_num][j] > mesh[row_num][j+1]) {
                temp = mesh[row_num][j];
                mesh[row_num][j] = mesh[row_num][j+1];
                mesh[row_num][j+1] = temp;
                swapped = true;
            }
        }
        if (swapped == false) {
            break;
        }
    }


}
/*Sorts the arrays that need to face right. 
Very simply done, no fuss necessary. No value is returned
row_num is the row that is to be sorted. The phase is the current phase
This variable is largely unnecessary but was useful for testing
 */
void row_sort_right(int row_num, int phase, int **mesh)
{
    int i, j;
    int n = number_rows;
    int temp;
    bool swapped;
    for (i = 0; i < n-1; i++) {
        swapped = false;
        for (j = 0; j < n-i-1; j++) {
            if (mesh[row_num][j] < mesh[row_num][j+1]) {
                temp = mesh[row_num][j];
                mesh[row_num][j] = mesh[row_num][j+1];
                mesh[row_num][j+1] = temp;
                swapped = true;
            }
        }
        if (swapped == false) {
            break;
        }
    }


}
/*Very simply sorts the columns
  it sorts only the given column number, here represented by row_num
The phase variable is not used, it was previously used for testing
 */
void col_sort(int row_num, int phase, int **mesh)
{
    int i, j;
    int n = number_rows;
    int temp;
    bool swapped;
    for (i = 0; i < n-1; i++) {
        swapped = false;
        for (j = 0; j < n-i-1; j++) {
            if (mesh[j][row_num] > mesh[j+1][row_num]) {
                temp = mesh[j][row_num];
                mesh[j][row_num] = mesh[j+1][row_num];
                mesh[j+1][row_num] = temp;
                swapped = true;
            }
        }
        if (swapped == false) {
            break;
        }
    }
}
/*This function does all of the real work. runs until the phase counter matches the needed number of phases
It only takes in a row number to be sorted.
This row number corresponds to the thread number
I was unable to get semaphores working but used busy waiting to achieve the same outcome, though less efficiently
 */
void *sort(void *row_num)
{
  int a = *((int*)&row_num);
    while(phase_counter < phase) {
        
        while(thread_phase[a] == phase_counter){
	  //Busy wait until all threads are done
        }


        if(phase_counter%2 == 0) {
            //Sort Columns on even phases
            col_sort(a, phase_counter, mesh);
        }
	//Sort rows on odd phases
        else if(a%2 == 0) {
            row_sort_left(a, phase_counter, mesh);
        }
        else if(a%2 != 0){
            row_sort_right(a, phase_counter, mesh);
        }
	//Incrementing thread phases individually
        thread_phase[a]++;
	//check to see if all threads are on the same phase
        if(check_threads()) {
	  //If they are, increment the phase counter
            phase_counter++;
            for(int i = 0; i < number_rows; i++) {
                sem_post(&semaphores[i]);
            }
        }
    }
    pthread_exit(NULL);

}

int main( int argc, const char* argv[] )
{
  phase_counter = 1; //This counts the phase we are on in the program
    int a;
    number_cells = 0;
    
    ifstream myfile ("input.txt");
    while(myfile >> a) {
        number_cells++;                     // Counting the number of necessary cells
    }
    myfile.close();

    
    phase = log2((double)number_cells)+1.0; //calculating number of necessary phases
    number_rows = sqrt(number_cells);       //calculating number of rows based on number of cells given

    mesh = new int *[number_rows];          //creating mesh of the correct size
    for(int i = 0; i <number_rows; i++) {
        mesh[i] = new int[number_rows];
    }

    ifstream newfile ("input.txt");
    for(int i = 0; i <number_rows; i++) {
        for(int j = 0; j <number_rows; j++) {
            newfile >> mesh[i][j];          //filling mesh with input file. Probably could have been prettier.
        }
    }
    newfile.close();
    
    threads = new pthread_t[number_rows];
    semaphores = new sem_t[number_rows];    //Creating arrays for the threads, semaphors and phase trackers

    thread_phase = new int[number_rows];

    for(int i = 0; i < number_rows; i++) {
      thread_phase[i] = 0;                  //Setting base thread phases to 0

    }


    int rc;
    long t;
    for(t = 0; t < number_rows; t++) {
      
      rc = pthread_create(&threads[t], NULL, sort, (void *)t); //Creating threads
      if (rc) {
	cout<<"error in thread creation"<<endl;
	exit(-1);
      }
    }
    for(t = 0; t < number_rows; t++){
      pthread_join(threads[t], NULL); //Joining threads to main thread
    }

    print_array(mesh, number_rows);


    delete mesh;
    return 0;
}
