# gs
1. To build test suite:  
   `g++ --std=c++14 tests.cpp -lgtest -lgtest_main -lpthread -o parser_GT`  
   - run:  
   `./parser_GT` //parametric performance test for 512MiB input file takes > 4min - should be commented out in the interest of time; 
   generally, file I/O are a huge bottleneck - processing could be done in parallel to some extent  
2. To build application:  
   `g++ --std=c++14 main.cpp -o parser`  
   - run:   
   `./parser`  
  
# Lucernam olet. 
General appraisal (from parser.hpp):

Desc: the underlying data structure for the problem at hand can be considered to be a tensor of rank 3 with dimensions [KxLxM],  
where K is the number of unique symbols, L the number of timestamps (not neccesarily unique) and M the number of fields. It
is also equivalent to a problem of K independent L by M  matirces - this approach is also what is implemented in the code below
as the data corresponding to different symbols is unrelated to each other - this helps to preserve locality of the data structure
during processing. The task ultimetely comes down to finding non-zero elements of a large [N x M] matrix. The key to a efficient
solution is effective cache management, as explained here:  
   https://lwn.net/Articles/255364/  
and also here: (Scott Meyers always fun to watch)  
   https://www.youtube.com/watch?v=WDIkqP4JbkE  
Due to the above mentioned facts, it is most advantageous to work on vectors (chosen here) or arrays.
The two required optimization options entail their specific considerations:  
-Oprint) the vector of vectors structure used should already be pretty friendly. The assembly of print buffers could be parallelized,
         though.  
-Oproduct) efficient use of cache requires matrix transposition to [M x N] form but also product calculations are easily parallelized
           into multiple threads (nice linear scaling) as long as thread local data is used for partial product calculation. SSE2
           instructions accessed via compiler intrinsics could also help boost performance of doubles - however, availabiliy of those
           is hardware specific.

