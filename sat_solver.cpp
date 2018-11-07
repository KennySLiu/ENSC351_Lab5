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

using namespace std;

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


int main(int argc, char *argv[]){
    if (argc != 2){
        cout << "\nTo use this program, call it along with the name of the input file. For example, ./sat_solve problem1\n";
        return 1;
    }
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
    cout << num_vars << endl; 
    */

    // sat_variables will be our vector which holds each variable's value. 0 means unset, -1 means false, 1 means true.
    // Index i will correspond to variable i in the input; hence sat_variables[0] doesn't represent anything.
    vector<int> sat_variables(num_vars + 1, 0);
    
    // Algorithm: for each variable, first set it to true. If that does not fail any clauses, then set the next variable to true. Continue on in this way until you get a TRUE or FALSE final result.
    //      If it's TRUE, great. you're done. 
    //      If it's FALSE, backtrack and switch your previous choice to false. Then continue on in the same way.
    for (int i = 1; i < sat_variables.size(); ++i){
        sat_variables[i] = 1;

        int satisfied = check_satisfied(clause_vect, sat_variables);
        if (satisfied == 1){
            /* success! */
            cout << "v ";
            for (int j = 1; j < sat_variables.size(); ++j){
                if (sgn(sat_variables[j]) == -1){ 
                    cout << -j << " ";
                } else {
                    cout << j << " ";
                }
            }
            cout << endl;
            break;
        } else if (satisfied == 0){
            continue;
        } else if (satisfied == -1){
            // If it's not satisfied, we backtrack until we find the last decision we made to set something true, and change that to false.
            // All other entries BEFORE then will be set true.
            for (int j = i; j > 0; --j){
                if (sat_variables[j] == -1){
                    sat_variables[j] = 1;
                }
                if (sat_variables[j] == 1){
                    sat_variables[j] = -1;
                    break;
                }
            }
        }
    }
}


    





