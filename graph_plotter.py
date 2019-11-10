from matplotlib import pyplot




num_procs = [ 1 , 2 , 4 , 8 , 16 ]
t_40k = [ 14.6109 , 7.31309 , 3.66512 , 1.84203 , 1.36338 ]
t_60k = [ 32.7475 , 16.3677 , 8.2373 , 4.13542 , 2.6316 ]
t_100k = [ 91.285 , 45.5806 , 22.7837 , 11.4367 , 6.70603 ]
t_200k = [ 363.956 , 182.176 , 91.0162 , 45.7744 , 26.6109 ]

speedup_40k = [ (t_40k[0]/x) for x in t_40k ]
speedup_60k = [ (t_60k[0]/x) for x in t_60k ]
speedup_100k = [ (t_100k[0]/x) for x in t_100k ]
speedup_200k = [ (t_200k[0]/x) for x in t_200k ]

pyplot.xlabel("Processadores")
pyplot.ylabel("Speedup")
pyplot.xlim(0,20)
pyplot.ylim(0,20)
pyplot.xticks(range(0,20,1))
pyplot.yticks(range(0,20,1))
pyplot.plot( num_procs , num_procs , color=(0,0,0,0.3) )
pyplot.plot( num_procs , speedup_40k , alpha=0.6 , label='v_size = 40000' )
pyplot.scatter( num_procs , speedup_40k , alpha=1.0 )
pyplot.plot( num_procs , speedup_60k , alpha=0.6 , label='v_size = 60000' )
pyplot.scatter( num_procs , speedup_60k , alpha=1.0 )
pyplot.plot( num_procs , speedup_100k , alpha=0.6 , label='v_size = 100000' )
pyplot.scatter( num_procs , speedup_100k  , alpha=1.0 )
pyplot.plot( num_procs , speedup_200k , alpha=0.6 , label='v_size = 200000' )
pyplot.scatter( num_procs , speedup_200k  , alpha=1.0 )
pyplot.legend()
pyplot.show()