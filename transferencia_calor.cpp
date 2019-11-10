#include <iostream>
#include <omp.h>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

enum {INCREASING , DECREASING};

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
size_t  MAX_STEPS       = 1024;
size_t  BLOCK_SIZE      = 17;
vector<heat_point> heat_points;

void sor_step( float** matrix_block , size_t current_step , size_t next_step , 
    size_t matrix_width , size_t matrix_height , 
    int initial_y , int final_y , 
    short increasing_or_decreasing , size_t last_step ,
    vector<heat_point>& heat_points )
{

    //cout << "stesp: " << current_step << "->" << next_step << endl;
    //cout << "last_step: " << last_step << endl;
    
    float BETA = ((1.0-ALPHA)/4);
    float* current_matrix = matrix_block[current_step];
    float* next_matrix = matrix_block[next_step];
    
    initial_y = max( 1,initial_y);
    final_y = min( int(matrix_height-1) , final_y);
    final_y = max(final_y,initial_y+1);
    // cout << initial_y << "," << final_y << " [" << omp_get_thread_num() << "]" << endl;

    // Set temperature in each point a heat_point exists
    for( vector<heat_point>::iterator it = heat_points.begin() ; it != heat_points.end() ; it++ ){
        int y = it->y;
        if( y >= initial_y && y < final_y )
            current_matrix[it->x+y*matrix_height] = it->temperature;
    }

    for( int y = initial_y ; y < final_y ; y++ )
    for( int x = 1 ; x < matrix_width-1 ; x++ ){
        next_matrix[x+y*matrix_width] = current_matrix[x+y*matrix_width]*ALPHA + BETA*(current_matrix[x+(y+1)*matrix_width]+current_matrix[x+(y-1)*matrix_width]+current_matrix[x+1+y*matrix_width]+current_matrix[x-1+y*matrix_width]);
    }
    
    if( next_step < last_step ){
        if( increasing_or_decreasing == INCREASING ){
            {
                sor_step( matrix_block , next_step , next_step + 1 , matrix_width , matrix_height , initial_y-1 , final_y+1 , INCREASING , last_step , heat_points );
            }
        } else if( increasing_or_decreasing == DECREASING ) {
            {
                sor_step( matrix_block , next_step , next_step + 1 , matrix_width , matrix_height , initial_y+1 , final_y-1 , DECREASING , last_step , heat_points );
            }
        }
    }

}

int main( int argc , char** argv ){
    
    size_t num_threads = 1;
    if( argc > 1 ){
        num_threads = atoi(argv[1]);
        omp_set_num_threads(num_threads);
    }
    else
    {
        #pragma omp parallel
        {
            #pragma omp single
            {
                num_threads = omp_get_num_threads();
            }
        }
    }
    
    cout << "Numero de threads: " << num_threads << endl;
    
    heat_points.push_back( heat_point(100,8,8) );
    
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
    
    #pragma omp parallel
    {
        #pragma omp master
        {
            for( int step = 0 ; step < MAX_STEPS ; step += BLOCK_SIZE-1 )
            {
                size_t max_block_steps = min( BLOCK_SIZE-1 , MAX_STEPS-step );

                for( int t = 0 ; t < num_threads ; t++ ){
                    #pragma omp task
                    {
                        int initial_y = MATRIX_SIZE * (float(t)/num_threads);
                        int final_y   = MATRIX_SIZE * (float(t+1)/num_threads);
                        size_t last_step = max_block_steps;
                        sor_step( mat_block , 0 , 1 , MATRIX_SIZE , MATRIX_SIZE , initial_y , final_y , DECREASING , last_step , heat_points );
                    }
                }
                #pragma omp taskwait

                for( int t = 0 ; t < num_threads+1 ; t++ ){
                    #pragma omp task
                    {
                        int initial_y = MATRIX_SIZE * ( float(t) / (num_threads+1) );
                        int final_y = initial_y+1;
                        size_t last_step = max_block_steps;
                        sor_step( mat_block , 0 , 1 , MATRIX_SIZE , MATRIX_SIZE , initial_y , final_y , INCREASING , last_step , heat_points );
                    }
                }
                #pragma omp taskwait

                for( int t = 0 ; t < num_threads+1 ; t++ ){
                    int initial_y = MATRIX_SIZE * (float(t)/num_threads);
                    int final_y   = MATRIX_SIZE * (float(t+1)/num_threads);
                    memcpy( (mat_block[0])+initial_y , (mat_block[max_block_steps])+initial_y , sizeof(float)*MATRIX_SIZE*(final_y-initial_y) );
                }
                #pragma omp taskwait
            }
        }
    }
    cout << "Tempo: " << omp_get_wtime() - initial_t << " segundos. "; 
    cout << "Ponto(8,8) deve estar 63.9926: " << mat_block[0][7+MATRIX_SIZE*7] << endl;

    for( int b = 0 ; b < BLOCK_SIZE ; b++ )
        delete[] mat_block[b];
    delete[] mat_block;
}
