#include <iostream>
#include <omp.h>

#define TAM 20

using namespace std;


void rank_sort( int* v , size_t v_size ){
    int* sorted_v = new int[v_size]();
    int i , j , pivot , rank;

    omp_set_num_threads( 2 );

    #pragma omp parallel for shared(sorted_v)
    for( i=0 ; i < v_size ; i++ ){
        // cout << " i = " << i << "( pego pela thread " << omp_get_thread_num() << ")" << endl;
        pivot = v[i];
        if( pivot == 0 ) continue;
        rank = 0;
        for( j=0 ; j<v_size ; j++ ){
            rank += ( v[j] < pivot );
        }
        sorted_v[rank] = pivot;
    }

    for( i=0 ; i<v_size ; i++ ) v[i] = sorted_v[i];
    delete[] sorted_v;
}

int main(){
    int i;
    int *x;
    bool should_print = ( TAM < 50 );
    x = new int[TAM];
    for( i = 0 ; i < TAM ; i++ ) x[i] = rand() % 1000;
    
    if(should_print){
        cout << "x (unsorted):  " << endl;
        for(i=0 ; i < TAM ; i++ ) cout << x[i] << ", ";
        cout << endl;
    }

    double initial_t = omp_get_wtime();
    rank_sort( x , TAM );
    cout << "Duracao: " << omp_get_wtime() - initial_t << "s" << endl;
    
    if(should_print){
        cout << "x (sorted):  " << endl;
        for(i=0 ; i < TAM ; i++ ) cout << x[i] << ", ";
        cout << endl;
    }


}