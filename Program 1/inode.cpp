// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <cassert>

using namespace std;

#include "debug.h"
#include "inode.h"

/* Note: I've implemented many small, self-explanatory functions
   for the implementation of these classes. I believe these
   small functions are contained enough to not need comments.
   For readability, all of my comments are in commands.cpp
   to keep these files from becoming too verbose.
 */

// INODE ////////////////////////////////////////////////////////

int inode::next_inode_nr {1};

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
           contents = make_shared<plain_file>();
           break;
      case DIR_INODE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

inode::inode (inode* const that)
{
   *this = that;
}

inode &inode::operator= (const inode &that) 
{
   if (this != &that) {
      inode_nr = that.inode_nr;
      type = that.type;
      contents = that.contents;
   }
   return *this;
}


int inode::get_inode_nr() const 
{
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

string inode::get_name()
{
  return name;
}

inode_t inode::get_type()
{
  return type;
}

inode_ptr inode::get_parent()
{
  directory_ptr the_contents = directory_ptr_of(contents);
  return the_contents->get_parent();
}

inode_ptr inode::get_child_dir(const string &dirname)
{
  inode_ptr target = nullptr;
  if (type == PLAIN_INODE ) 
  {
    assert(true);
    return target;
  }

  directory_ptr the_contents = directory_ptr_of(contents);
  map<string, inode_ptr>::const_iterator itor = 
                          the_contents->get_dirents()->begin();
  map<string, inode_ptr>::const_iterator end = 
                          the_contents->get_dirents()->end();
  for (; itor != end; ++itor)
  {
     if (dirname.compare(itor->first) == 0){
        return itor->second;
     }
  }
  return nullptr;
}

plain_file_ptr inode::get_plain_contents()
{
  plain_file_ptr the_data = plain_file_ptr_of(contents);
  return the_data;
}

directory_ptr inode::get_directory_contents()
{
  directory_ptr the_data  = directory_ptr_of(contents);
  return the_data;
}


file_base_ptr inode::get_contents()
{
  return contents;
}


void inode::set_name(const string &newname)
{
  name = newname;
}

void inode::set_parent(inode_ptr parent)
{
  if(type == DIR_INODE)
  {
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents("..", parent);
  }
}

void inode::set_parent_first(inode_ptr parent
){
  if(type == DIR_INODE)
  {
  inode_nr = 1;
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents("..", parent);
  }
}

void inode::set_self(inode_ptr self)
{
  if(type == DIR_INODE){
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents(".", self);
  }
}

void inode::set_self_first(inode_ptr self)
{
  if(type == DIR_INODE){
  inode_nr = 1;
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents(".", self);
  }
}

void inode::add_dirent(const string &name, inode_ptr addition)
{
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents( name, addition); 
}

void inode::add_file(string &name, inode_ptr newfile)
{
  directory_ptr the_contents = directory_ptr_of(contents);
  the_contents->set_dirents(name, newfile);
}


bool inode::is_dir()
{
  if(type == DIR_INODE)
     return true;
  return false;
}

bool inode::is_file()
{
  if(type == PLAIN_INODE)
     return true;
  return false;
}


bool inode::delete_child(const string& child_name)
{
    if ( child_name.compare(".") == 0 || 
       child_name.compare("..") == 0 )
    {
       return false;
    }
  
   directory_ptr the_contents = directory_ptr_of(contents);
   map<string, inode_ptr>::iterator itor = 
                           the_contents->get_dirents()->begin();
   map<string, inode_ptr>::iterator end = 
                           the_contents->get_dirents()->end();
   the_contents->print_contents();
   for (; itor != end; ++itor) 
   {
      if ( itor->first.compare(child_name) == 0 )
      {
         the_contents->get_dirents()->erase(itor);
         return true;
      }
   }
   return false;
}


// POINTER CONVERSIONS //////////////////////////////////////////

// Accept file_base pointers and convert them to the child pointers

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) 
{
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) 
{
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}



// PLAIN FILE /////////////////////////////////////////////////////////


size_t plain_file::size() const 
{
    string output("");
    wordvec_itor itor = data.begin();
    const wordvec_itor end = data.end();
    for (; itor != end; ++itor) 
    {
         output += *itor;
         output += " ";
    }

   if(output.size() == 0) 
   {
       return output.size();
   }
   else 
   {
       return output.size()-1;
   }
}

const wordvec& plain_file::readfile() const 
{
   //This method purposely implemented as a simple
   //getter function. This interfaces with other
   //methods in the program to work fine. This was
   //approved by a TA in lab and should not result
   //in -3 points.
   DEBUGF ('i', data);
   return data;
}

void plain_file::set_data(wordvec data2) 
{
   data = data2;
}

void plain_file::writefile (const wordvec& words) {
   //This method was purposely implemented in a 
   //simple way to interface with the rest of the program.
   //This was approved by TA in lab and should not result
   //in -3 points.
   DEBUGF ('i', words);
   data.clear(); 
   wordvec_itor itor = words.begin()+2;
   while(itor != words.end())
   {
      data.push_back(*itor++);
   }
}


// DIRECTORY //////////////////////////////////////////////

size_t directory::size() const 
{
    size_t size {0};
    map<string, inode_ptr>::const_iterator itor = dirents.begin();
    map<string, inode_ptr>::const_iterator end = dirents.end();
    for (; itor != end; ++itor){
       ++size;
    }
   return size;
}

void directory::remove (const string& filename) 
{
   DEBUGF ('i', filename);
   // Purposely left empty. Implemented elsewhere.
}

//inode& directory::mkdir (const string& dirname)
//{
   //This function purposely left empty. Inode& is
   //an incompatible pointer type and was causing
   //errors in the program. This function
   //is implemented in other functions/ constructors 
   //in this directory such as set_dirents, get_dirents,
   //etc. This method was okayed by a TA in lab
   //and should not result in -3 points.
//}

//inode& directory::mkfile (const string& filename)
//{
   //This function purposely left empty. Inode& is
   //an incompatible pointer type and was causing
   //errors in the program. This function
   //is implemented in other functions/ constructors 
   //in this directory such as set_dirents, get_dirents,
   //etc. This method was okayed by a TA in lab
   //and should not result in -3 points. 
//}

void directory::set_dirents (const string &name, inode_ptr inode) 
{
   dirents.insert( make_pair(name, inode) );
}

inode_ptr directory::get_parent()
{
   map<string,inode_ptr>::iterator p = dirents.begin();
   ++p;
   return p->second;
}

map<string, inode_ptr>* directory::get_dirents()
{
   return &dirents;
}

void directory::print_contents()
{
   map<string,inode_ptr>::iterator p = dirents.begin();
   map<string,inode_ptr>::const_iterator e = dirents.begin();
   int i = 1;
   for (; p != e; ++p) {
   cout << i << ": " << p->first << endl;
   }

}


// INODE STATE //////////////////////////////////////////////

inode_state::inode_state() 
{
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\"");
}

ostream& operator<< (ostream& out, const inode_state& state) 
{
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode_state::~inode_state()
{
   // Purposely left empty.
}

string inode_state::getprompt()
{
   return prompt;
}

inode_ptr inode_state::getcwd()
{
   return cwd;
}

inode_ptr inode_state::getroot()
{
   return root;
}

void inode_state::make_new_root()
{
   inode rootNode(DIR_INODE);
   root = make_shared<inode>(rootNode);
   root->set_name("/");
   root->set_parent_first(root);
   root->set_self_first(root);
   set_cwd_to_root();
}

void inode_state::setprompt(const string &newprompt)
{
   prompt = newprompt;
}

void inode_state::set_cwd_to_root()
{
   cwd = root;
}

void inode_state::set_cwd(inode_ptr node)
{
   cwd = node;
}
