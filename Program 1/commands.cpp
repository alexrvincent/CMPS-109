// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr    },
}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

void preorder_traversal(inode_ptr curr_inode, string path);
void postorder_traversal(inode_ptr curr_inode);

inode_ptr go_to_path(inode_state& state, const wordvec &words,
                     int destination, int control);



string wordvec_to_string(wordvec &words);


void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error;
   // Check for no input
   if ( words.size() == 1 )
   {
      error += ": No such file or directory";
   }
   // Attempt to cat the file
   else
   {
      wordvec_itor itor = words.begin() + 1;
      const wordvec_itor end = words.end();
      for (int i = 1; itor != end; ++itor, ++i)
      {
       inode_ptr target = go_to_path(state, words, i, 0);
        // Check to see if the file exists
        if ( target == nullptr) 
        {
          error +=  "cat: " + words[i] +
          ": No such file or directory";
          throw yshell_exn(error);
           return;
        }
        //Double check to make sure it's a file
        else if( target->is_dir())
        {
           error += "cd: Invalid argument";
           throw yshell_exn(error);
           return;
        }
        // Final check, then out the file contents
        else if ( target != nullptr )
        {
           cout << target->get_plain_contents()->readfile() << endl;
        } 
        // All checks failed, so throw an error. 
        else
        {
           error += "cat: " + words[i] + 
           ": No such file or directory";
        }
      }
   }
   // Throw error
   if ( error.size() > 0)
   {
      throw yshell_exn(error);
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error;
   // If no arguments, move to root
   if ( words.size() == 1 )
   {
       state.set_cwd_to_root();
   }
    
   //Case: one argument
   else if ( words.size() == 2)
   {
      inode_ptr target = go_to_path(state, words, 1, 0);
  // Check if the directory exists
      if ( target == nullptr ) 
      {
        error += "cd: " + words[1] + 
            ": No such file or directory";
      }
  // If the target is a file, error out.
  else if (target->is_file())
  {
   error += "cd: Invalid argument";
   throw yshell_exn(error);
   return;
  }
  // Everything passed, so move to that directory
  else
  {
        state.set_cwd(target);
      }
   }
   //Case: More than one argument
   else 
   {
       error += "cd:";
       wordvec_itor itor = words.begin() + 1;
       while ( itor != words.end() ) error += " " + *itor++;
       error += ": No such file or directory";
   }
   // Throw error
   if ( error.size() > 0)
   {
      throw yshell_exn(error);
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string output("");
   // Check to make sure there are arguments
   if ( words.size() > 1 )
   {
      // Iterate, add words to a string
      wordvec_itor itor = words.begin() + 1;
      const wordvec_itor end = words.end();
      for (; itor != end; ++itor) {
         output += *itor;
         output += " ";
      }
      output.erase(output.end()-1);
   }
   //Output the string of collected words
   cout << output << endl;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   int code;
   //Check if an exit code is requested
   if ( words.size() > 1 )
   {
      code = atoi(words[1].c_str());
      exit_status::set(code);
   }
   throw ysh_exit_exn();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string err;
   //Case: No arguments
   if ( words.size() == 1 )
   {
    inode_ptr cwd = state.getcwd();
    cout << cwd->get_name() << ":" << endl;
  
    inode_ptr currDir = state.getcwd();
    directory_ptr the_contents = directory_ptr_of(
                                 currDir->get_contents());
    map<string, inode_ptr>::iterator itor = 
                            the_contents->get_dirents()->begin();
    map<string, inode_ptr>::const_iterator end = 
                            the_contents->get_dirents()->end();
    // Iterate through this inode's map
    for (; itor != end; ++itor)
    {
        //Print the inode number
        cout << itor->second->get_inode_nr() << "  " ;

        //Print either the directory or file size
        if(itor->second->is_dir())
        {
          cout << itor->second->get_directory_contents()->size() 
          << "  " ;
        }
        else
        {
          cout << itor->second->get_plain_contents()->size()
          << "  ";
        }

        //Print the "/" next to a directory, not a file.
        if(itor->second->is_dir() && (itor->first.compare(".") != 0)
                            && (itor->first.compare("..") != 0) )
        {
              cout << itor->first << "/" <<endl;
        }
        else
        {
             cout << itor->first <<endl;
        }
    } 
 }

   // Case: More than one argument
   else
   {
      inode_ptr target;
      wordvec_itor itor = words.begin();
      ++itor;
      wordvec_itor end = words.end();
      // Iterate to the path
      for (int i = 1; itor != end; ++itor, ++i)
      {
        target = go_to_path(state, words, i, 0);
 
        // Check to make sure the target is there
        if ( target == nullptr )
        {
             err += "ls: Invalid file or directory: " 
               + words[i];
            throw yshell_exn(err);
            return;
        }
        // Check to make sure you're printing a directory
        if( target->is_file()) 
        {
            err += "ls: Invalid argument: " + target->get_name();
            throw yshell_exn(err);
            return;
        }
        // All checks passed attempt to ls the directory
         else
         {
          // Iterate through this inode's map
           directory_ptr the_contents = directory_ptr_of(target->
                                           get_contents());
           map<string, inode_ptr>::iterator itor = 
                        the_contents->get_dirents()->begin();
           map<string, inode_ptr>::const_iterator end = 
                         the_contents->get_dirents()->end();
            cout << words[i] << ":" << endl;

           for (; itor != end; ++itor)
                {
                 //Print the inode number
                 cout << itor->second->get_inode_nr() << "  " ;
                 //Print either the directory or file size
                 if(itor->second->is_dir())
                 {
                   cout << itor->second->get_directory_contents()->
                                                  size() << "  " ;
                 }
                 else
                 {
                  cout << itor->second->get_plain_contents()->
                                               size() << "  ";
                 }

                 //Print the directory "/" next to a directory
                 if(itor->second->is_dir() && 
                (itor->first.compare(".") != 0) && 
                (itor->first.compare("..") != 0) )
                {
                       cout << itor->first << "/" <<endl;
                }
                else
                {
                      cout << itor->first <<endl;
                }

          }
        }
      }
   }
   if ( err.size() > 0)
   {
      throw yshell_exn(err);
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error("");
   inode_ptr head;
   
   //Case: No arguments
   if ( words.size() == 1 )
   {
      head = state.getcwd();
      string n;
      preorder_traversal(head, n);
   }
   //Case: More than one argument
   else {
      wordvec_itor itor = words.begin();
      ++itor;
      wordvec_itor end = words.end();
       // Iterate through this inode's map
      for (int i = 1; itor!=end; ++itor, ++i)
      {
         head = go_to_path(state, words, i, 0);

         //Check if the directory exists
         if ( head != NULL )
         {
          // Check if the directory is a directory
          if( head->is_file()) 
          {
            error += "lsr: Invalid argument: " + head->get_name();
            throw yshell_exn(error);
            return;
          }
           string n;
           preorder_traversal(head, n);
        }
         //Otherwise there was an error
         else 
         {
           error += "lsr: No such file or directory: " +
               words[i];
        }
      }
   }
   if ( error.size() > 0)
   {
      throw yshell_exn(error);
   }
}


void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error("");
   // Case: No arguments
   if ( words.size() == 1 )
   {
      error += "make: Please specify a filename";
   }
   // Case: Arguments
   else 
   {
      inode_ptr target_parent = go_to_path(state, words, 1, 1);
      //Check to make sure the traverse was successful
      if ( target_parent != nullptr )
      {
         bool newfile = false;
         wordvec path = split(words[1], "/" );
         string dirname(path[path.size()-1]);
         inode_ptr targetfile = target_parent->get_child_dir(dirname);
         //If a file doesn't exist, create it
         if ( targetfile == nullptr )
         {
            inode temp(PLAIN_INODE);
            inode_ptr newFil = make_shared<inode>(temp);
            newFil->set_name(wordvec_to_string(path));
            newfile = true;
            newFil->get_plain_contents()->writefile(words);
            if ( newfile == true )
            {
               string name(words[1]);
               target_parent->add_file(dirname, newFil);
            }
         }
        //Otherwise the file exists, so error out.
        else
        {
            error+="make: directory/file already exists with name: " 
                 +  words[1];
        }
      }
         // Otherwise there was an error
        else 
        {
         error += "make: Invalid file or directory";
      }
   }
   // Alert the error
   if ( error.size() > 0 )
   {
      throw yshell_exn(error);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error("");
   // Case: Bad arguments
   if ( words.size() == 1 || words.size() > 2 )
   {
      error += "mkdir: Invalid directory name";
      //cout << error << endl;
   }
   //Attempt to create the file
   else if ( words.size() == 2)
   {
      inode_ptr target = go_to_path(state, words, 1, 1);
      if ( target != nullptr )
      {
        //Check if the directory exists
        wordvec path = split(words.at(1),"/");
        if ( target->get_child_dir(path[path.size()-1]) != nullptr )
        {
            error += 
            "mkdir: Directory or File already exists with that name";
        }
        //Otherwise success, so make the directory
        else 
        {
        inode_ptr newDir = make_shared<inode>(DIR_INODE);
        target->add_dirent(path.at(path.size()-1), newDir);
        newDir->set_self(newDir);
        newDir->set_parent(target);
        newDir->set_name(wordvec_to_string(path));
        }
      }
       //Otherwise there was an error
      else 
      {
         error += "mkdir: Invalid path";
      }
   }
   if ( error.size() > 0 )
   {
      throw yshell_exn(error);
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
  
   string error;
   //Case: No arguments
   if (words.size() < 1)
   {
      error += "Invalid prompt";
   }
   
   //Initialize the new prompt
   wordvec prompt_vec = words;
   prompt_vec.erase (prompt_vec.begin());
   string prompt_string;
   
   //Iterate through the word and put it into a string
  for(unsigned int vec_itor=0;vec_itor<prompt_vec.size();vec_itor++)
  {
    prompt_string.append(prompt_vec[vec_itor]);
    if (!(vec_itor+1 >= prompt_vec.size()))
    {
      prompt_string.append(" ");
    }
  }
  
  //Case: More than one argument
  if (words.size() < 1)
  {
    error += "Invalid prompt";
  }
  // Otherwise it works, so update prompt
  else
  {  
    state.setprompt(prompt_string);
  }
  if ( error.size() > 0 )
   {
      throw yshell_exn(error);
   }
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words); 
   
   string path("");
   path += state.getcwd()->get_name();
   inode_ptr parent = state.getcwd()->get_parent();
   //Doesn't use words, so go ahead and begin making path
   while (parent->get_inode_nr() !=
          state.getroot()->get_inode_nr() )
   {
     path = parent->get_name() + "/" + path;
     parent = parent->get_parent();
   }
   //Insert proper '/' character
   if (path.compare("/") != 0)
   {
     path = "/" + path;
   }
   //Out the path
   cout << path << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error("");
   //Case: Bad arguments
   if ( words.size() == 1 || words.size() > 2 )
   {
      error += "rm: Invalid file or directory";
   }
   //Case: One argument
   else if ( words.size() == 2)
   {
      inode_ptr target = go_to_path(state, words, 1, 0);
      //Check to make sure target is there
      if ( target != nullptr )
      {
        //Make sure target is a directory, has size < 2
        if ( ( target->is_dir() && 
             target->get_directory_contents()->size() <= 2 ) || 
             !target->is_dir() )
            {
              inode_ptr target_parent = go_to_path(
                                        state, words, 1, 1);
              wordvec path = split(words[1],"/");
              //Make sure you can't delete . or .. 
              if ( target_parent->delete_child( path[path.size()-1] ) 
                 == false )
              {
                error += "rm: Cannot delete '.' or '..'";
              }
              //Otherwise, success.
              else
              {
               // Do nothing, success.
               //cout << "Sucessfully deleted the child node" << endl;
              }
      
           }
       //Otherwise, alert user of error
       else 
       {
            error +=  "rm: Cannot delete a non-empty directory";
        }
      }
      //Otherwise, alert user of error
      else 
      {
         error += "rm: Invalid file or directory";
      }
   }
   if ( error.size() > 0 ) 
   {
      throw yshell_exn(error);
   }  
}


void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string error("");
   //Case: Bad arguments
   if ( words.size() == 1 || words.size() > 2)
   {
      error += "rmr: Invalid file or directory";
   }
   //Otherwise attempt recursive remove
   else if ( words.size() == 2)
   {
      inode_ptr target_head = go_to_path(state, words, 1, 0);
      inode_ptr parent = go_to_path(state, words, 1, 1);
      
      if (target_head == nullptr )
      {
          error += "rmr: Invalid file or directory";
          throw yshell_exn(error);
          return;
      }
      if ( target_head->is_dir() )
      {
         postorder_traversal(target_head);
      }
      if ( parent->get_inode_nr() != target_head->get_inode_nr() )
      {
         parent->delete_child(target_head->get_name());
      }
   }
   if ( error.size() > 0 )
   {
      throw yshell_exn(error);
   }
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

//Private function: Gets the designated path
inode_ptr go_to_path(inode_state& state, const wordvec &words,
                     int destination, int control){
   bool start_at_root = false;
   wordvec path = split(words[destination],"/");
   
   //Determine whether to start at the root
   if (words[destination].at(0)=='/')
   {
      start_at_root = true;
   }
   
   //Begin traversal
   inode_ptr head;
   if ( start_at_root == true )
   {
      head = state.getroot();
   } 
   else
   {
      head = state.getcwd();
   }
   
   //For every inode in tree, go to..
   int size = path.size()-control;
   for ( int i = 0; i < size; ++i )
   {
      if ( head->is_dir() )
      {
         head = head->get_child_dir(path[i]);
      } 
      else
      {
        head = nullptr;
      }
      if ( head == nullptr ) 
      {
        return nullptr;
      }
   }
   return head;
}

//Private function: Performs the printing lsr function
void preorder_traversal(inode_ptr curr_inode, string path){
   const string name = curr_inode->get_name();
   path += curr_inode->get_name();
   
   cout << path << ":" << endl;
   inode_ptr currDir = curr_inode;
   directory_ptr the_contents = directory_ptr_of(
                                currDir->get_contents());
   map<string, inode_ptr>::iterator itor1 = 
                                the_contents->get_dirents()->begin();
   map<string, inode_ptr>::const_iterator end1 = 
                                the_contents->get_dirents()->end();
   // Iterate through the map and print..
   for (; itor1 != end1; ++itor1)
   {
    //Inode number
    cout << itor1->second->get_inode_nr() << "  " ;

    //Size
    if(itor1->second->is_dir())
    {
        cout << itor1->second->get_directory_contents()->
                                     size() << "  " ;
    }
    else
    {
    cout << itor1->second->get_plain_contents()->
                                 size() << "  ";
    }
    //Name
    if(itor1->second->is_dir() && 
    (itor1->first.compare(".") != 0) && 
    (itor1->first.compare("..") != 0) )
    cout << itor1->first << "/" <<endl;
    else
    cout << itor1->first << endl;
   }
   
   //Start another iteration, this time for traversal
   directory_ptr directory = curr_inode->get_directory_contents();
   map<string, inode_ptr>::const_iterator itor = 
                           directory->get_dirents()->begin();
   ++itor; ++itor;
   map<string, inode_ptr>::const_iterator end = 
                           directory->get_dirents()->end();
   if ( path.compare("/") != 0 )
      path += "/";
   //Recursive traversal loop
   for (; itor != end; ++itor) 
   {
      if ( itor->second->is_dir() )
      {
         preorder_traversal(itor->second, path);
      }
   }
}

void postorder_traversal(inode_ptr curr_inode){
   //Iteration for traversal
   directory_ptr directory = curr_inode->get_directory_contents();
   map<string, inode_ptr>::iterator itor = 
                           directory->get_dirents()->begin();
   ++itor; ++itor;
   map<string, inode_ptr>::const_iterator end = 
                           directory->get_dirents()->end();
   //Recursive removal loop
   for (; itor != end;) {
      if ( itor->second->is_dir() )
         postorder_traversal(itor->second);
      directory->get_dirents()->erase(itor++);
   }
 }

//Private function: Just a helper function
string wordvec_to_string(wordvec &words){
   string output("");
   wordvec_itor itor = words.begin();
   const wordvec_itor end = words.end();
   for (; itor != end; ++itor) {
      output += *itor;
      output += " ";
   }
  return output;
}

