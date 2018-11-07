#include "kenny_include.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <algorithm>
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
                while (cstr_tok != NULL){
                    int num = atoi(cstr_tok);
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


int main(int argc, char *argv[]){
    if (argc != 2){
        cout << "\nTo use this program, call it along with the name of the input file. For example, ./sat_solve problem1\n";
    }
    char* filename = argv[2];
    pair<vector<vector<int> >, int> inputs = input_reader(filename);


}








