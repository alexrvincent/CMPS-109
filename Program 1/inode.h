// $Id: inode.h,v 1.13 2014-06-12 18:10:25-07 - - $

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

//
// inode_t -
//    An inode is either a directory or a plain file.
//

enum inode_t {PLAIN_INODE, DIR_INODE};
class inode;
class file_base;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using file_base_ptr = shared_ptr<file_base>;
using plain_file_ptr = shared_ptr<plain_file>;
using directory_ptr = shared_ptr<directory>;

//
// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.
//

class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_state (const inode_state&) = delete; // delete copy ctor
      inode_state& operator= (const inode_state&) = delete; 
      inode_ptr root {nullptr};
      inode_ptr cwd {nullptr};
      string prompt {"% "};
   public:
      //Constructors and Destructors//
      inode_state();
      ~inode_state();

      //Accessors//
     string getprompt();
     inode_ptr getcwd();
     inode_ptr getroot();
  
     //Mutators//
     void make_new_root();
     void setprompt(const string &newprompt);
     void set_cwd_to_root();
     void set_cwd(inode_ptr node);
};


//
// class inode -
//
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//    

class inode {
   friend class inode_state;
   //friend ostream &operator<< (ostream &out, inode_ptr inode);
   private:
      int inode_nr;
      inode_t type;
      file_base_ptr contents;
      static int next_inode_nr;
      string name;
   public:
      //Constructor//
      inode (inode_t init_type);
      inode (inode* const that); 
      inode &operator= ( const inode &from); 
  
       //Accessors//
       int get_inode_nr() const;
       string get_name();
       inode_t get_type();
       inode_ptr get_parent();
       inode_ptr get_child_dir(const string& childname);
       plain_file_ptr get_plain_contents();
       directory_ptr get_directory_contents();
       file_base_ptr get_contents();
  
       //Setters//
       void set_name(const string &newname);
       void set_parent(inode_ptr parent);
       void set_parent_first(inode_ptr parent);
       void set_self(inode_ptr self);
       void set_self_first(inode_ptr self);
       void add_dirent(const string &name, inode_ptr addition);
       void add_file(string &name, inode_ptr newfile);
  
       //Booleans//
       bool is_dir();
       bool is_file();
       bool delete_child(const string& child_name);
 
};

//
// class file_base -
//
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.
//

class file_base {
   protected:
      file_base () = default;
      file_base (const file_base&) = default;
      file_base (file_base&&) = default;
      file_base& operator= (const file_base&) = default;
      file_base& operator= (file_base&&) = default;
      virtual ~file_base () = default;
      virtual size_t size() const = 0;
   public:
      friend plain_file_ptr plain_file_ptr_of (file_base_ptr);
      friend directory_ptr directory_ptr_of (file_base_ptr);
};


//
// class plain_file -
//
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
//    Throws an yshell_exn for a directory.
// writefile -
//    Replaces the contents of a file with new contents.
//    Throws an yshell_exn for a directory.
//

class plain_file: public file_base {
   private:
      wordvec data;
   public:
      size_t size() const override;
      const wordvec& readfile() const;
      void writefile (const wordvec& newdata);
      void set_data(wordvec data2);
};

//
// class directory -
//
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Purposely left blank due to errors. See implementation.
// mkdir -
//    Purposely left blank due to errors. See implementation.
// mkfile -
//    Purposely left blank due to errors. See implementation.

class directory: public file_base {
   private:
      map<string,inode_ptr> dirents;
   public:
      size_t size() const override;
      void remove (const string& filename);
      inode& mkdir (const string& dirname);
      inode& mkfile (const string& filename);
      void set_dirents(const string& name, inode_ptr node);
      inode_ptr get_parent();
      map<string, inode_ptr>* get_dirents();
      void print_contents();
  
};

#endif

