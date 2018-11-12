#include "kenny_include.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <algorithm>
#include <cmath>
#include <vector>
#include <bitset>
#include <cerrno>
#define NUM_THREADS 8
// NUM_THREADS MUST NOT BE CHANGED! It must be 8, because quite a bit of logic depends on it being specifically 8. 

using namespace std;
pthread_mutex_t sol_found_lock;
bool SOLUTION_FOUND = 0;
int num_backtracks = 0;
pthread_mutex_t num_backtracks_lock;



pair<vector<vector<int> >, int> input_reader(char* filename){
    ifstream in_file;
    in_file.open(filename);
    string cur_line;

    vector<vector<int> > clause_vect;
    int num_vars = 0;
    pair<vector<vector<int> >, int> retval;
    
    if (in_file.is_open()){
        while (getline(in_file, cur_line)){
            char cstr_cur_line[cur_line.size() + 1];
            char* cstr_tok;
            vector<int> curr_clause;

            // Skip over commented lines 
            if (cur_line[0] == 'c'){
                continue;
            }

            // If the current line starts with 'p', then it is "p cnf ${num_vars} ${num_clauses}"
            if (cur_line[0] == 'p'){
                strcpy(cstr_cur_line, &cur_line[5]);
                cstr_tok = strtok(cstr_cur_line, " ");
                num_vars = atoi(cstr_tok);
                continue;
            }
            
            // If the current line doesn't start with 'c' or 'p', then it's a clause.
            else {
                // Copy the current line into a Cstring so that we can call strtok();
                strcpy(cstr_cur_line, &cur_line[0]);
                cstr_tok = strtok(cstr_cur_line, " ");
                int num = 0;
                while (cstr_tok != NULL){
                    num = atoi(cstr_tok);
                    if (num == 0){
                        break;
                    }
                    curr_clause.push_back(num);
                    cstr_tok = strtok(NULL, " ");
                }

                clause_vect.push_back(curr_clause);
            }
        }
    } else {
        cout << "\nThe file didn't open correctly oh god oh man" << endl;
    }
    
    retval = make_pair(clause_vect, num_vars);
    return retval;
}



int check_satisfied(vector<vector<int> > clause_vect, vector<int> sat_variables){
    // Return values:
    //    1 if satisfied
    //    0 if unknown
    //    -${clause_that_failed} if failed
    //    Note that the failed clause is only ONE of MANY POSSIBLE failed clauses.

    int retval = 15;

    for (int i = 0; i < clause_vect.size(); ++i){        
        // "retval" has a hierarchy, since it's an AND of ORs.
        // If a clause is false, the whole SAT is false.
        // Otherwise, if a clause is undecided, the whole SAT is undecided.
        // Otherwise, the whole SAT is true.
        int cur_row_satisfied = 15;

        for (int j = 0; j < clause_vect[i].size(); ++j){
            // "cur_row_satisfied" has a kind of hierarchy, since it's an AND of ORs.
            // If a clause has a literal that is true, the whole clause is true.
            // Otherwise, if a clause has variable that is undecided, the whole clause is undecided.
            // Otherwise, a clause is false.

            int cur_literal = clause_vect[i][j];
            int variable_in_literal = sat_variables[abs(cur_literal)];
            
            if (sgn(variable_in_literal) == sgn(cur_literal)){
                // If any variable has the same sign as its literal, the current row is definitely true.
                cur_row_satisfied = 1;
                break;
            } else if (variable_in_literal == 0){
                // If the variable is set to 0, the current row is undecided IFF all other literals are unsatisfied or undecided as well.
                // Seems like a lot of bullets to dodge for a failure case, but keep in mind that when we find a solution we're probably
                //      never going to hit this block, because the variables will have been decided.
                cur_row_satisfied = 0;
            }
        }

        if (cur_row_satisfied == 15){
            cur_row_satisfied = -1;
            retval = -1;
            break;
        } else if (cur_row_satisfied == 0){
            retval = 0;
        }
    }

    if (retval == 15){
        retval = 1;
    }

    return retval;
}


void* threaded_backtracker(void* vp_input){

    pair<vector<int>, vector<vector<int> > > input = *static_cast<pair<vector<int>, vector<vector<int> > >* >(vp_input);
    vector<int> sat_variables = input.first;
    vector<vector<int> > clause_vect = input.second;

    // Algorithm: for each variable, first set it to true. If that does not fail any clauses, then set the next variable to true. Continue on in this way until you get a TRUE or FALSE final result.
    //      If it's TRUE, great. you're done. 
    //      If it's FALSE, backtrack and switch your previous choice to false. Then continue on in the same way.
    // We start at i = 4 because we we assume 8 threads. Thus, the first three choices are predefined for each thread.
    for (int i = 4; i < sat_variables.size(); ++i){
        if (sat_variables[i] == 0){
            sat_variables[i] = 1;
        }

        int satisfied = check_satisfied(clause_vect, sat_variables);
        if (satisfied == 1){
            // SUCCESS!
            // If we found a solution, we don't want any other threads that may have found a solution to output concurrently. 
            //      So to solve this, we lock the solution found lock and just don't release it (since the program will exit shortly)
            pthread_mutex_lock(&sol_found_lock);
            cout << "\n\n\n\nv ";
            for (int j = 1; j < sat_variables.size(); ++j){
                if (sgn(sat_variables[j]) == -1){ 
                    cout << -j << " ";
                } else {
                    cout << j << " ";
                }
            }
            cout << endl;
            // Set the SOLUTION_FOUND flag AFTER we cout everything, because otherwise we might kill the program early
            SOLUTION_FOUND = 1;
            return 0;
        } else if (satisfied == 0){
            continue;
        } else if (satisfied == -1){
            // If it's not satisfied, we backtrack until we find the last decision we made to set something true, and change that to false.
            // All other entries we have decided on before then will be set true.
            for (int j = sat_variables.size() - 1; j > 3; --j){
                if (sat_variables[j] == 0){
                    // If the variable is 0, then we haven't seen it yet. Just leave it alone.
                    continue;
                } else if (sat_variables[j] == -1){
                    sat_variables[j] = 1;
                    continue;
                }
                if (sat_variables[j] == 1){
                    // Set i = j - 1 because it'll get incremented in the next loop step. If we set i = j, it could return early (i.e. if i = last entry)
                    i = j - 1;
                    sat_variables[j] = -1;
                    
                    // We just backtracked once, so increment the number of backtracks: 
                    pthread_mutex_lock(&num_backtracks_lock);
                    ++num_backtracks;
                    pthread_mutex_unlock(&num_backtracks_lock);
                    

                    // We also need to check if we are done traversing all possibilities, because that means this current thread was unable to find a solution.
                    // To do this, check if j = end of vector AND if everything is equal (besides the first three predefined vals, and the dummy at [0]).
                    if ( j = sat_variables.size() - 1){
                        bool no_solutions = 1;
                        for (int k = 4; k < sat_variables.size(); ++k){
                            if (sat_variables[k] != sat_variables[4]){
                                no_solutions = 0;
                                break;
                            }
                        }
                        if (no_solutions){
                            cout << "NO SOLUTIONS FOUND!" << endl;
                            return 0;
                        }
                    }

                    // Break, because we backtracked successfully.
                    break;
                }
            }
        }
    }
    
}


int main(int argc, char *argv[]){
    if (argc != 2){
        cout << "\nTo use this program, call it along with the name of the input file. For example, ./sat_solve problem1\n" << "Input files must be in DIMACS format. For help, see: https://www.satcompetition.org/2009/format-benchmarks2009.html\n" << endl;
        return 1;
    }

    SOLUTION_FOUND = 0;
    char* filename = argv[1];
    pair<vector<vector<int> >, int> inputs = input_reader(filename);

    vector<vector<int> > clause_vect = inputs.first;
    int num_vars = inputs.second;

    /*
    // DEBUGGING THE INPUT READER: 
    for (int i = 0; i < clause_vect.size(); ++i){
        for (int j = 0; j < clause_vect[i].size(); ++j){
            cout << clause_vect[i][j] << ", ";
        }
        cout << endl;
    }
    cout << "and num_vars = " << num_vars << endl << endl; 
    // END OF DEBUGGING THE INPUT READER CODE 
    */


    pthread_t threads[NUM_THREADS];

    // sat_variables will be our vector which holds each variable's value. 0 means unset, -1 means false, 1 means true.
    // Index i will correspond to variable i in the input; hence sat_variables[0] doesn't represent anything.
    vector<vector<int> > sat_variables;

    for (int i = 0; i < NUM_THREADS; ++i){
        vector<int> curr_thread_sat_variables;

        // To figure out the first three hard-coded values of each thread's sat variables, we convert i to binary.
        bitset<3> bset(i);

        // Push back the dummy in the first position:
        curr_thread_sat_variables.push_back(0);

        // Push back the first three characters
        for (int j = 2; j >= 0; --j){
            if (bset[j] == 0){
                curr_thread_sat_variables.push_back( -1 );
            } else {
                curr_thread_sat_variables.push_back( bset[j] );
            }
        }

        // Push back the remaining values as zeros. A total of num_vars + 1 values, as specified 2 lines above.
        for (int j = 4; j < num_vars + 1; ++j){
            curr_thread_sat_variables.push_back(0);
        }

        sat_variables.push_back(curr_thread_sat_variables);
    }

    for (int i = 0; i < NUM_THREADS; ++i){
        
        // DEBUGGING: making sure the sat_variables were built correctly
        cout << "i = " << i << endl;
        for (int j = 0; j < sat_variables[i].size(); ++j){
            cout << sat_variables[i][j];
        }
        cout << endl;
        pair< vector<int>, vector< vector< int> > >* threaded_backtracer_input = new pair< vector<int>, vector< vector< int> > >(sat_variables[i], clause_vect);
        pthread_create( &threads[i], NULL, threaded_backtracker, static_cast<void*>(threaded_backtracer_input) );
    }
    cout << "PARENT: All threads created" << endl;
    
    
    int num_threads_joined = 0;
    vector<int> joined_thread_ids;
    while (!SOLUTION_FOUND){
        for (int i = 0; i < NUM_THREADS; ++i){
            if ( find( joined_thread_ids.begin(), joined_thread_ids.end(), i ) != joined_thread_ids.end() ){
                continue;
            }

            int join = pthread_tryjoin_np(threads[i], NULL);
            if (join != EBUSY && join != EINVAL && join != ESRCH){
                ++num_threads_joined;
                joined_thread_ids.push_back(i);
                cout << "PARENT: Someone joined" << endl;
            }
        }

        if (num_threads_joined == NUM_THREADS){
            cout << "PARENT: No solutions found." << endl;
            exit(0);
        }
    }
    // If solution_found was set to true, the thread that found a solution will have outputted everything already.
    // So we're done! :) 
}


    





