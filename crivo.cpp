#include <iostream>
#include <omp.h>
#include <cstdlib>
using namespace std;

int conta_primos( int N , int num_threads = 1 ){
    short* crivo = new short[N+1]();
    int i,j,primos_encontrados,expected_size;
    omp_set_num_threads(num_threads);


    #pragma omp parallel private(j) shared(crivo,N)
    #pragma omp single
    {
        primos_encontrados = 0;
        for( i=2 ; i<N+1 ; i++ ){
            // Achou um primo
            if( crivo[i] == 0 ){
                primos_encontrados ++;

                #pragma omp taskloop firstprivate(i) grainsize(1000)
                for( j=i ; j<N+1 ; j += i ){
                    crivo[j] = 1;
                }
                #pragma omp taskwait

            }
        }
    }

    delete[] crivo;
    return primos_encontrados;
}

int main( int argc , char** argv ){
    int N=100 , num_threads=1;
    
    if( argc>= 2 ) N = atoi( argv[1] );
    if( argc>= 3 ) num_threads = atoi( argv[2] );

    double initial_t = omp_get_wtime();
    cout << "Quantidade de primos menores ou iguais a " << N << ": " << conta_primos( N , num_threads ) << endl;
    cout << "Duracao: " << omp_get_wtime() - initial_t << "s " << endl;

}