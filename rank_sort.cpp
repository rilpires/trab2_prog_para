#include <iostream>
#include <cstdlib>
#include <omp.h>
using namespace std;

/**
 *
 * Para rodar o programa:
 * g++ rank_sort.cpp -o rank_sort -fopenmp
 * ./rank_sort VECTOR_SIZE [NUM_THREADS]
 * 
**/


void rank_sort( int* v , size_t v_size , int num_threads = 1){
    int* sorted_v = new int[v_size]();
    int i , j , pivot , rank;

    omp_set_num_threads( num_threads );

    
    #pragma omp parallel shared(sorted_v,v_size) private(i,j,pivot,rank)
    {
        // 'for' facilmente paralelizado, laços não possuem dependências entre si
        #pragma omp for
        for( i=0 ; i < v_size ; i++ ){
            pivot = v[i];
            if( pivot == 0 ) continue;
            rank = 0;
            // Este 'for' não é paralelizado!
            for( j=0 ; j < v_size ; j++ ){
                if( v[j] < pivot ){
                    rank ++;
                }
            }
            // Lida com valores iguais sendo organizado
            while( rank < v_size-1 && sorted_v[rank] == pivot ) rank++ ;
            sorted_v[rank] = pivot;
        }
    }

    for( i=0 ; i<v_size ; i++ ) v[i] = sorted_v[i];
    delete[] sorted_v;
}

int main( int argc , char** argv ){
    int i , *x , num_threads = 1 , TAM = 1000;
    if( argc >= 2 ) TAM = atoi( argv[1] );
    if( argc >= 3 ) num_threads = atoi( argv[2] );
    bool should_print = ( TAM <= 50 );
    x = new int[TAM];
    for( i = 0 ; i < TAM ; i++ ) x[i] = rand() ;
    
    // Printa antes de organizar
    if(should_print ){
        cout << "x (unsorted):  " << endl;
        for(i=0 ; i < TAM ; i++ ) cout << x[i] << ", ";
        cout << endl;
    }

    // Faz o sort, medindo o tempo
    double initial_t = omp_get_wtime();
    rank_sort( x , TAM , num_threads );
    cout << "Duracao: " << omp_get_wtime() - initial_t << "s" << endl;
    
    // Printa depois de organizar
    if( should_print ){
        cout << "x (sorted):  " << endl;
        for(i=0 ; i < TAM ; i++ ) cout << x[i] << ", ";
        cout << endl;
    }


}