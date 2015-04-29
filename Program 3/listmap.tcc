// $Id: listmap.tcc,v 1.5 2014-07-09 11:50:34-07 - - $

#include "listmap.h"
#include "trace.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* n, node* p,
                                     const value_type& value):
            link (n, p), value (value) {
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   //Pop and delete items off the front
   while (!empty()) 
   {
      erase(begin());
   }
   TRACE ('l', (void*) this);
}

//
// listmap::empty()
//
template <typename Key, typename Value, class Less>
bool listmap<Key,Value,Less>::empty() const {
   //Fixed bug from starter code here
   return anchor_.next == &anchor_;
}

//
// listmap::iterator listmap::begin()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::begin() {
  //Return the node off of anchor
  //Because anchor cannot have a value_type
  return iterator (anchor_.next);
}

//
// listmap::iterator listmap::end()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::end() {
   //Return anchor itself as this is a cyclical list
   return iterator (anchor());
}

template<typename Key, typename Value, class Less>
const typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::cbegin() const{
   //Same as begin just with const
   return iterator(anchor_.next);
}

template<typename Key, typename Value, class Less>
const typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::cend() const{
   //Same as end just with const
   return iterator(anchor());
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
   Less less;
   listmap<Key,Value,Less>::iterator itor = begin();
   
  //Break if the inserted key is less than current value 
  while (itor!=end())
  {
    if(less(pair.first,itor->first))
    {
        break;
    }
  //Otherwise continue to find a place where it's less than
    else 
    {
      if( not less(pair.first,itor->first)) ++itor;
    }
    
  }
   //If it's in the map, just update the value
   if(itor!=end())
   {
      if(itor->first == pair.first) 
      {
          itor->second = pair.second;
          return itor;
      }
   }
   
   //Special Case: The first node to be inserted   
   if(itor.where == anchor() && itor.where->next == anchor()) 
   {
     node* temp = new node(itor.where->next, itor.where, pair);
     itor.where->next = temp;
     itor.where->prev = temp;
     return iterator(temp);
   }
   
   // Otherwise make a new node and reassign pointers.
   node* temp = new node(itor.where, itor.where->prev, pair);
   itor.where->prev->next = temp;
   itor.where->prev = temp;  
   return iterator(temp);
 
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) const {
   listmap<Key,Value,Less>::iterator itor = begin();
   //Iterate until either the end of the list or
   //until the current itor = that. Returns
   //end if it doesn't find that.
   while(itor!=end() && itor->first != that) 
   {
     ++itor;
   }
   TRACE ('l', that);
   return itor;
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
   listmap<Key,Value,Less>::iterator itor = position;
   //Iterate and erase at that position
   ++itor;
   position.erase();
   TRACE ('l', &*position);
   return itor;
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

// Most of these have already been implemented in starter code
//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   TRACE ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->(){
   TRACE ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   TRACE ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   TRACE ('l', where);
   where = where->prev;
   return *this;
}

//void listmap::iterator::erase()
template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::iterator::erase() {
   //Reassign pointer then delete contents at itor's pointer
   if (where != nullptr)
   {
      where->prev->next = where->next;
      where->next->prev = where->prev;
      delete where;
  }   
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}

