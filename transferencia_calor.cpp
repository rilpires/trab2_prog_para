#include <iostream>
#include <omp.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>


/**
 * Compilar com:        g++ -fopenpm transferencia_calor.cpp -o transferencia_calor -O3
 * Executar com:        ./transferencia_calor [NUM_THREADS]
**/

using namespace std;

struct heat_point {
    int x;
    int y;
    float temperature;
    heat_point( float temperature , int x=0 , int y=0 ):x(x),y(y),temperature(temperature){}
};

float   ALPHA           = 0.0;
size_t  MATRIX_SIZE     = 1024;
float   INITIAL_TEMP    = 20;
float   BORDER_TEMP     = 30;
size_t  MAX_STEPS       = 1024;
size_t  BLOCK_SIZE      = 16;
vector<heat_point> heat_points;

void sor_step( float*& current_matrix , float* next_matrix , size_t matrix_width , size_t matrix_height , 
    int initial_y , int final_y , vector<heat_point>& heat_points )
{

    float BETA = ((1.0-ALPHA)/4);
    
    if( initial_y <= 0 ) initial_y = 1;
    if( final_y >= matrix_height ) final_y = matrix_height-1;

    // Set temperature in each point a heat_point exists
    for( vector<heat_point>::iterator it = heat_points.begin() ; it != heat_points.end() ; it++ ){
        heat_point& hp = *it;
        if(hp.y >= initial_y && hp.y < final_y) current_matrix[ (hp.x) + (hp.y)*matrix_width ] = it->temperature;
    }
    for( int y = initial_y ; y < final_y ; y++ ){
        int temp_1 = (y-1)*matrix_width;
        int temp_2 = y*matrix_width;
        int temp_3 = (y+1)*matrix_width;
        for( int x = 1 ; x < matrix_width-1 ; x++ ){
            next_matrix[x+temp_2] = current_matrix[x+temp_2]*ALPHA + BETA*(current_matrix[x+temp_3]+current_matrix[x+temp_1]+current_matrix[x+1+temp_2]+current_matrix[x-1+temp_2]);
        }
    }
    // Set temperature in each point a heat_point exists for the next matrix
    for( vector<heat_point>::iterator it = heat_points.begin() ; it != heat_points.end() ; it++ ){
        heat_point& hp = *it;
        if(hp.y >= initial_y && hp.y < final_y) next_matrix[ (hp.x) + (hp.y)*matrix_width ] = it->temperature;
    }
    
}

int main( int argc , char** argv ){
    size_t num_threads = 1;
    if( argc > 1 ){
        num_threads = atoi(argv[1]);
        omp_set_num_threads(num_threads);
    }
    else
    #pragma omp parallel
    #pragma omp single
        num_threads = omp_get_num_threads();
    
    cout << "Numero de threads: " << num_threads << endl;
    
    heat_points.push_back( heat_point(100,800,800) );
    
    // Allocating memory to be used
    float**  mat_block = new float*[BLOCK_SIZE];
    for( int b=0 ; b<BLOCK_SIZE ; b++ ){
        mat_block[b] = new float[MATRIX_SIZE*MATRIX_SIZE];
    }
    
    // Setting initial temperatures
    for( int b=0 ; b<BLOCK_SIZE ; b++ ){
        float* mat = mat_block[b];
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
    }

    // Running until MAX_STEPS of SOR
    double initial_t = omp_get_wtime();
    
    //num_threads = 4;
    #pragma omp parallel
    {
        #pragma omp master
        {
            for( int step = 0 ; step < MAX_STEPS ; step += BLOCK_SIZE-1 )
            {
                size_t max_block_steps = min( BLOCK_SIZE-1 , MAX_STEPS-step );

                // Não existe taskloop na versão OMP do tesla !!!
                for( int t = 0 ; t < num_threads ; t++ ){
                    #pragma omp task
                    {
                        int initial_y = 1 + (MATRIX_SIZE-2) * (float(t)/num_threads);
                        int final_y   = 1 + (MATRIX_SIZE-2) * (float(t+1)/num_threads);
                        for( int i = 0 ; i < max_block_steps ; i++ ){
                            sor_step( mat_block[i] , mat_block[i+1] , MATRIX_SIZE , MATRIX_SIZE , initial_y+i , final_y-i ,heat_points );
                        }
                    }
                }
                #pragma omp taskwait

                // Não existe taskloop na versão OMP do tesla !!!
                for( int t = 0 ; t < num_threads+1 ; t++ ){
                    #pragma omp task
                    {
                        int center_y = MATRIX_SIZE * ( float(t) / (num_threads) ) ;
                        int initial_y = center_y-1;
                        int final_y = center_y+1;
                        for( int i = 0 ; i < max_block_steps-1 ; i++ ){
                            sor_step( mat_block[i+1] , mat_block[i+2] , MATRIX_SIZE , MATRIX_SIZE , initial_y-i , final_y+i ,heat_points );
                        }
                    }
                }
                #pragma omp taskwait
                
                memcpy( mat_block[0] , mat_block[max_block_steps] , sizeof(float)*MATRIX_SIZE*MATRIX_SIZE );
            }
        }
    }

    cout << "Tempo: " << omp_get_wtime() - initial_t << " segundos. " << endl; 
    cout << "Ponto(30,30) deve estar 21.277: " << (mat_block[0])[30+MATRIX_SIZE*30] << endl;
    cout << "Ponto(999,999) deve estar 21.5509: " << (mat_block[0])[999+MATRIX_SIZE*999] << endl;
    cout << "Ponto(795,788) deve estar 32.4487: " << (mat_block[0])[795+MATRIX_SIZE*788] << endl;
    cout << "Ponto(799,799) deve estar 67.3064: " << (mat_block[0])[799+MATRIX_SIZE*799] << endl;
    cout << "Ponto(799,800) deve estar 74.3187: " << (mat_block[0])[799+MATRIX_SIZE*800] << endl;

    for( int b = 0 ; b < BLOCK_SIZE ; b++ )
        delete[] mat_block[b];
    delete[] mat_block;
}
