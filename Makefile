singlethread:
	g++ -std=c++11 -g -o SATsolver ./sat_solver.cpp 
	cp SATsolver ./test_files

clean:
	rm SATsolver
