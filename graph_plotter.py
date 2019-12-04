from matplotlib import pyplot

num_procs = [ 1 , 2 , 4 , 8 , 16 ]

times_arrays = []
speedups_arrays = []
labels = []

times_arrays.append( [ 14.6109 , 7.31309 , 3.66512 , 1.84203 , 1.36338 ] ) , labels.append("1")
times_arrays.append( [ 32.7475 , 16.3677 , 8.2373 , 4.13542 , 2.6316 ] ) , labels.append("10")
times_arrays.append( [ 91.285 , 45.5806 , 22.7837 , 11.4367 , 6.70603 ] ) , labels.append("100")
times_arrays.append( [ 363.956 , 182.176 , 91.0162 , 45.7744 , 26.6109 ] ) , labels.append("1000")



for time_array in times_arrays:
    speedups_arrays.append( [ (time_array[0]/x) for x in time_array ] )


pyplot.xlabel("Processadores")
pyplot.ylabel("Speedup")
pyplot.xlim(0,20)
pyplot.ylim(0,20)
pyplot.xticks(range(0,20,1))
pyplot.yticks(range(0,20,1))
pyplot.plot( num_procs , num_procs , color=(0,0,0,0.3) )

for i in range(0,len(speedups_arrays)):
    speedup_array =  speedups_arrays[i]
    pyplot.plot( num_procs , speedup_array , alpha=0.6 , label=labels[i] )
    pyplot.scatter( num_procs , speedup_array , alpha=1.0 )

pyplot.legend()
pyplot.show()