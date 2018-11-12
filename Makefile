singlethread:
	g++ -std=c++11 -g -o singlethreaded_SATsolver ./singlethreaded_sat_solver.cpp 
	cp singlethreaded_SATsolver ./test_files

multithread:
	g++ -std=c++11 -g -o SATsolver -pthread ./sat_solver.cpp
	cp SATsolver ./test_files

clean:
	rm SATsolver
	rm singlethreaded_SATsolver
