// $Id: main.cpp,v 1.6 2014-07-09 11:50:34-07 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_pair = xpair<const string,string>;
using str_str_map = listmap<string,string>;
using lmap_str_itor = listmap<string, string>::iterator;

string trim_spaces(string str){
   size_t first = str.find_first_not_of (" \t");
   if (first == string::npos) return "";
   size_t last = str.find_last_not_of (" \t");
   return str.substr (first, last - first + 1);
}

str_str_pair splitline(string line){
    string key;
    string value;
    
    //If the line does not contain an = just use the key
    size_t pos = line.find_first_of ("=");
      if (pos == string::npos) 
      {
         key = trim_spaces(line.substr (0, pos == 0 ? 0 : pos));
      }
      //Otherwise split at the = to make key and value strings
      else {
         key = trim_spaces(line.substr (0, pos == 0 ? 0 : pos));
         value = trim_spaces(line.substr (pos + 1));
         //Fill "NO_VALUE" for key= cases
         if(value == "") value = "NO_VALUE";
      }
      
    //Construct the pair and return it
    str_str_pair the_pair (key, value);
    return ( the_pair );
}

void print_line(const string &file, int line, const string &line_str){
   cout << file <<": " <<line<<": "<< line_str << endl;
}

void run_file(const string &read_file, istream &input_file){
     //listmap <string, string> test;
     
     //Create the new map
     str_str_map *tester = new str_str_map();
   
   for(int line_num = 1;; ++line_num) 
   {
      //Try to parse the file
      try 
      {
      
         //Get the input line, reset variables
         bool found = false;
         string line;
         getline (input_file, line);
         if(input_file.eof()) break;
         
         //Case: Return key hit or empty line
         if(line.size() == 0)
         { 
            print_line(read_file,line_num,"");
            continue;
         }
         
         //Handle the case of \\ indicators for big lines
         for (;;) {
            int end_spot = line.size() - 1;
            // Only break if \\ indicator is not there
            if (line[end_spot] != '\\') 
            {
               break;
            }
            line[end_spot] = ' ';
            
            //Get the rest of the input
            //From the next line
            string the_rest;
            getline (input_file, the_rest);
            if (input_file.eof()) 
            {
               break;
            }
            //And append it to the line to be read
            line = line + the_rest;
         }
         
         //Now that we've got the entire input
         //Trim and print the cleaned line
         line = trim_spaces(line); 
         print_line(read_file,line_num,line);
         
         //Special Case: Comment
         if (line.at(0) == '#' || line.size() == 0)
         {
            continue;
         }
         
         //Case: Equals sign - Print the map
         if (line.compare("=")==0)
         { 
            for (str_str_map::iterator itor = tester->begin();
                               itor != tester->end(); ++itor) 
            {
                cout << *itor << endl;
            }                    
            continue;
         }
         
         //Split the line into the key and value
          str_str_pair k_v = splitline(line);
                   
          //Case: key - when there is a key and no value
          if(k_v.first.size() > 0 && k_v.second.size() == 0)
          {
             for (str_str_map::iterator itor = tester->begin();
                   itor != tester->end(); ++itor) 
             {
                       if(itor->first == k_v.first) 
                       { 
                          found = true;
                          cout << *itor << endl;
                       }
             }
             if(found == false) cout << k_v.first << ": " 
                               << "key not found" << endl; 
            continue;
         }
         
         //Case: key= - when there is a key and an equals sign
         // This is a little messy, but it works.
         if(k_v.first.size() > 0 && k_v.second == "NO_VALUE")
         {
            for (str_str_map::iterator itor = tester->begin();
                               itor != tester->end(); ++itor) 
            {
                if(itor->first == k_v.first) 
                {
                    found = true;
                    tester->erase(itor);
                    break;
                }               
            }
            if(found == false) cout << k_v.first << ": " 
                             << "key not found" << endl;
            continue;
         }
         
         //Case: =value - no key but there is a value
          if(k_v.first.size() == 0 && k_v.second.size() > 0)
          {
            for (str_str_map::iterator itor = tester->begin();
                               itor != tester->end(); ++itor) 
            {
                if(itor->second == k_v.second) 
                {
                    found = true;
                    cout << *itor << endl;
                }               
            }
            if(found == false) cout << k_v.second << ": " 
                             << "value not found" << endl; 
            continue;
         }
         
         //Case: key=value - there is a key and value
         if(k_v.first.size() > 0 && k_v.second.size() > 0)
         {
             for (str_str_map::iterator itor = tester->begin();
                               itor != tester->end(); ++itor) 
             {
                 //Case: It's in the map
                 if(itor->first == k_v.first) 
                 {
                    itor->second = k_v.second;
                    found = true;
                }
            }
            if(found == false) tester->insert(k_v);
            cout << k_v.first << " = " << k_v.second << endl;
            continue;
         }
         
      }
      //Catch any errors and push to error output
      catch(runtime_error err){
         complain() << read_file << ":" << line_num << ": "
                    << err.what() <<endl;
      }
    }
    delete tester;
}

//Wesley Mackey's given scan function
void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   int EXIT_STATUS = 0;
   
   //Run the main program using cin if no arguments
   if (optind == argc) 
   {
     run_file("-", cin);
   }

   //Otherwise iterate over each file and run the program on it.
   for (int argi = optind; argi < argc; ++argi) 
   {
      const string read_file = argv[argi];
      ifstream input_file (read_file.c_str());
      
      //Error through if the file doesn't exist or throws error
      if (input_file.fail()) 
      {
          syscall_error(read_file);
          EXIT_STATUS = 1;
          continue;
       }
       
      //Run the program on it then close it
      run_file (read_file,input_file);
      input_file.close();
   }
   //return exit status;
   delete sys_info::execname;
   return EXIT_STATUS;
}

