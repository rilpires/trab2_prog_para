#include <iostream>
#include <omp.h>
#include <vector>
#include <cstring>

using namespace std;

struct heat_point {
    int x;
    int y;
    float temperature;
    heat_point( float temperature , int x=0 , int y=0 ):x(x),y(y),temperature(temperature){}
};

void print_matrix( float* matrix , size_t w , size_t h ){
    cout << "==== MATRIX =====" << endl;
    for( size_t y = 0 ; y < w ; y++ )
    {
        for( size_t x = 0 ; x < w ; x++ )
            cout << matrix[x + y*w] << "\t";
        cout << endl;
    }
    cout << "===============" << endl;
}

float   ALPHA           = 0.2;
size_t  MATRIX_SIZE     = 1024;
float   INITIAL_TEMP    = 20;
float   BORDER_TEMP     = 30;
size_t  MAX_STEPS       = 512;
vector<heat_point> heat_points = { heat_point(100,8,8) };

void sor_step( float* mat , float* aux , 
    size_t matrix_width , size_t matrix_height , 
    size_t initial_y , size_t final_y , 
    vector<heat_point>& heat_points )
{
    float BETA = ((1.0-ALPHA)/4);

    initial_y = max( (size_t)1,initial_y);
    final_y = min( (size_t)(matrix_height-1) ,final_y);
    //cout << initial_y << "," << final_y << endl;

    // Set temperature in each point a heat_point exists
    for( vector<heat_point>::iterator it = heat_points.begin() ; it != heat_points.end() ; it++ ){
        int y = it->y;
        if( y >= initial_y && y < final_y )
            mat[it->x+y*matrix_height] = it->temperature;
    }

    for( int y = initial_y ; y < final_y ; y++ )
    for( int x = 1 ; x < matrix_width-1 ; x++ ){
        aux[x+y*matrix_width] = mat[x+y*matrix_width]*ALPHA + BETA*(mat[x+(y+1)*matrix_width]+mat[x+(y-1)*matrix_width]+mat[x+1+y*matrix_width]+mat[x-1+y*matrix_width]);
    }
    
}

int main( int argc , char** argv ){
    
    size_t num_threads = 1;
    if( argc > 1 )
        num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);
    cout << "Numero de threads: " << num_threads << endl;

    // Allocating memory to be used
    float* mat  = new float[MATRIX_SIZE*MATRIX_SIZE];
    float* aux  = new float[MATRIX_SIZE*MATRIX_SIZE];

    // Setting initial temperatures around the middle
    for( int y = 1 ; y < MATRIX_SIZE-1 ; y++ )
    for( int x = 1 ; x < MATRIX_SIZE-1 ; x++ )
        mat[x+y*MATRIX_SIZE] = INITIAL_TEMP;

    // Setting initial temperature around the borders
    for( int x = 0 ; x < MATRIX_SIZE ; x++ )
        mat[x] = BORDER_TEMP;
    for( int x = 0 ; x < MATRIX_SIZE ; x++ )
        mat[x+(MATRIX_SIZE-1)*MATRIX_SIZE] = BORDER_TEMP;
    for( int y = 0 ; y < MATRIX_SIZE ; y++ )
        mat[y*MATRIX_SIZE] = BORDER_TEMP;
    for( int y = 0 ; y < MATRIX_SIZE ; y++ )
        mat[MATRIX_SIZE-1+y*MATRIX_SIZE] = BORDER_TEMP;
    
    // aux gotta be the same as mat initially
    memcpy( aux , mat , sizeof(float)*MATRIX_SIZE*MATRIX_SIZE );

    // Running until MAX_STEPS of SOR
    double initial_t = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp master
        {
            for( int step = 0 ; step < MAX_STEPS ; step++ ){
                #pragma omp taskgroup
                {
                    for( int thread_id = 0 ; thread_id < num_threads ; thread_id++ ){
                        #pragma omp task
                        {    
                            size_t initial_y = MATRIX_SIZE * (float(thread_id)/num_threads);
                            size_t final_y   = MATRIX_SIZE * (float(thread_id+1)/num_threads);
                            sor_step( mat , aux , MATRIX_SIZE , MATRIX_SIZE , initial_y , final_y , heat_points );
                        }
                    }
                }
                #pragma omp taskgroup
                {
                    for( int thread_id = 0 ; thread_id < num_threads ; thread_id++ ){
                        #pragma omp task
                        {    
                            size_t initial_y = MATRIX_SIZE * (float(thread_id)/num_threads);
                            size_t final_y   = MATRIX_SIZE * (float(thread_id+1)/num_threads);
                            memcpy( mat , aux , sizeof(float)*MATRIX_SIZE*(final_y-initial_y) );
                        }
                    }
                }
                
            }
        }
    }
    cout << "Tempo: " << omp_get_wtime() - initial_t << " segundos. "; 
    cout << "Ponto(8,8) deve estar 63.6078: " << mat[7+MATRIX_SIZE*7] << endl;

    delete[] mat;
    delete[] aux;
}
