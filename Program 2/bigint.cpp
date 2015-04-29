// $Id: bigint.cpp,v 1.61 2014-06-26 17:06:06-07 - - $

//Programmers:
//Alex Vincent - avincent@ucsc.edu
//Nader Sleem - nsleem@ucsc.edu

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <locale>

#include "bigint.h"
#include "debug.h"

using namespace std;

//C-tor: Make from long
bigint::bigint (long that): negative(false) 
{
   //Read in the long as a string
   string digits;
   stringstream long_to_string;
   long_to_string << that;
   digits = long_to_string.str();
   //Initialize string into a new bigint
   initialize(digits);
}

//Modified from given code
//C-tor: Make from string
bigint::bigint (const string& that) 
{
   int shift = 0;
   string::const_reverse_iterator itor = that.rbegin();
   string::const_reverse_iterator end = that.rend();
   
   //Determine sign from string beginning
   if ( *(end-1) == '-' or *(end -1) == '_' ) 
   {
      negative = true;
      shift = 1;
   }
   
   //Push only the digits onto the vector
   for (; itor != end-shift; ++itor) 
   {
      if( not isdigit(*itor)) continue;
      big_value.push_back( *itor - '0');
   }
}

//Initialize C-tor: Sets the values for the new bigint object
void bigint::initialize(const string& s)
{
   int shift = 0;
   string::const_reverse_iterator itor = s.rbegin();
   string::const_reverse_iterator end = s.rend();
   
   //Determine the sign from string beginning
   if ( *(end-1) == '-' or *(end -1) == '_' ) 
   {
      negative = true;
      shift = 1;
   }
   
   //Push only the digits onto the vector
   for (; itor != end-shift; ++itor) 
   {
      if( not isdigit(*itor)) continue;
      big_value.push_back( *itor - '0');
   }
}

void bigint::do_bigadd(const bigvalue_t& left, const bigvalue_t& right,
                                      bigvalue_t& target) const
{
   //Declare iterators, variables
   
   bigvalue_t::const_iterator itor_left = left.begin();
   bigvalue_t::const_iterator end_left = left.end();
   bigvalue_t::const_iterator itor_right = right.begin();
   bigvalue_t::const_iterator end_right = right.end();
   int carry = 0;
   bool left_complete = false; 
   bool right_complete = false;
   
   //Special case: Left and Right are zero
   if ( left.size() == 0 ) left_complete = true;
   if ( right.size() == 0 ) right_complete = true;
   
   //Otherwise attempt addition
   for(;;)
   {
      //Sum the i'th digits
      int sum = (right_complete?0:*itor_right) + 
                (left_complete?0:*itor_left) + carry;
      carry = 0;
      
      //Handle carry then push
      if ( sum >= 10 ) 
      {
         carry = 1;
         sum -= 10;
      }
      target.push_back(sum);
      
      //Check for completion
      if( ++itor_left == end_left ) 
                     left_complete = true;
      if ( ++itor_right == end_right ) 
                     right_complete = true;
      
      //Break on completion - otherwise go to next iteration
      if ( left_complete && right_complete )
      {
         if ( carry > 0 ) target.push_back(carry);
         break;
      }
   }
   clean_zeroes(target);
}

void bigint::do_bigsub(const bigvalue_t& left, const bigvalue_t& right,
                                      bigvalue_t& target) const{
   //Declare iterators, variables
   
   bigvalue_t::const_iterator itor_left = left.begin();
   bigvalue_t::const_iterator end_left = left.end();
   bigvalue_t::const_iterator itor_right = right.begin();
   bigvalue_t::const_iterator end_right = right.end();
   int carry = 0;
   bool left_complete = false;
   bool right_complete = false;
   
   
   //Special case, the two are 0.
   if ( left.size() == 0 ) left_complete = true;
   if ( right.size() == 0 ) right_complete = true;
   
   //Otherwise attempt subtraction
   for(;;)
   {
      //Differentiate the i'th digits
      int diff = (left_complete?0:*itor_left) - 
                 (right_complete?0:*itor_right) + carry;
      carry = 0;
      
      //Handle carry then push
      if ( diff < 0 && !left_complete ) 
      {
         carry = -1;
         diff += 10;
      }
      target.push_back(diff);
      
      //Check for completion
      if( !left_complete && ++itor_left == end_left ) 
                               left_complete = true;
      if ( !right_complete && ++itor_right == end_right ) 
                               right_complete = true;
      if ( left_complete && right_complete )break;
   }
   clean_zeroes(target);
}



bigint operator+ (const bigint& left, const bigint& right) 
{
   bigint result = bigint();
   
   //Check for signs 
   //If the same do a straight add then set negative
   if(left.negative == right.negative) 
   {
      result.do_bigadd(left.big_value, right.big_value, 
                                       result.big_value);
      result.negative = right.negative; 
    }
    
   //Otherwise see which is bigger
    else 
    {
    int abs_compare = left.absolute_compare(right);
      //Case: the Left was bigger
      if ( abs_compare > 0 )
      {
         result.do_bigsub(left.big_value, right.big_value, 
                                          result.big_value);
         result.negative = left.negative;
      }
      //Case: the right was bigger
      else if ( abs_compare < 0 )
      {
         result.do_bigsub(right.big_value, left.big_value, 
                                           result.big_value);
         result.negative = right.negative;
      }
    }
    return result;
}

bigint operator- (const bigint& left, const bigint& right) 
{
   bigint result = bigint();
   
   //Check the signs
   //If they're the same..
   if (left.negative == right.negative) 
   {
      //See which is bigger
      int abs_compare = left.absolute_compare(right);
      //Case: Left was bigger
      if ( abs_compare > 0) 
      {
          result.do_bigsub(left.big_value, right.big_value, 
                                           result.big_value);
          if (left.negative == true) 
             result.negative = true;
          else 
             result.negative = false;
      }
      //Case: Right was bigger  
      else if ( abs_compare < 0) 
      {
          result.do_bigsub(right.big_value, left.big_value, 
                                            result.big_value);
          if (left.negative == true) {
             result.negative = false;
          }
          else {
             result.negative = true;
          }
      }
   }
   //Otherwise the signs were different
   //So perform a straight addition and set negative   
   else {
      result.do_bigadd(left.big_value, right.big_value, 
                                       result.big_value);
      if (left.negative == true) result.negative = true;
    }
    result.clean_negative_zero();
    return result;   
}

bigint operator+ (const bigint& right) 
{
   bigint new_bigint = right;
   new_bigint.negative = false;
   return new_bigint;
}

bigint operator- (const bigint& right) 
{
   bigint new_bigint = right;
   new_bigint.negative = true;
   return new_bigint;
}

long bigint::to_long() const 
{
   if (*this <= bigint (numeric_limits<long>::min())
    or *this > bigint (numeric_limits<long>::max()))
               throw range_error ("bigint__to_long: out of range");
   int new_long = 0;
   bigvalue_t::const_iterator itor = big_value.begin();
   bigvalue_t::const_iterator end = big_value.end();
   for (int i = 1; itor != end; ++itor, i*=10)
   {
     new_long += i * (*itor);
   }
   if (negative)
      new_long = new_long * -1;
   return new_long;
}

//Don't use this. See other absolute comparison function.
bool abs_less (const long& left, const long& right) 
{
   return left < right;
}

static stackpair popstack (stack <stackpair> &mult_stack) 
{
   stackpair result = mult_stack.top ();
   mult_stack.pop();
   return result;
}
//
// Multiplication algorithm - Egyptian method
// note this deviates from source code
//
bigint operator* (bigint& left, bigint& right) 
{
   //Store the signs of the factors
   bool left_sign = left.negative;
   bool right_sign = right.negative;
   bigint left_copy = left;
   bigint right_copy = right;
   
   //Make positive numbers to avoid incorrect comparisons
   left_copy.negative = false;
   right_copy.negative = false;
   
   //Use a stack/stack pair to keep track of:
   //< < binary double > , < right double > >
   bigint count = 1;
   stack <stackpair> mult_stack;
   
   // Double the values and push them until values exceed each other
   while (count <= left_copy) 
   {
      mult_stack.push(stackpair(count, right_copy));
      right_copy = right_copy.multiply_by_2(right_copy);
      count = count.multiply_by_2(count);
   }
   bigint result;
   
   //Continuously add pairs to equal counter amount
   while ( !mult_stack.empty() )
   {
    stackpair pair = popstack(mult_stack);
     if(pair.first <= left_copy) 
     {
        left_copy = left_copy - pair.first;
        result = result + pair.second;
     }
    }
    //Use stored signs to determine new sign
    if ( left_sign != right_sign) 
    {
        result.negative = true;
    }
    result.clean_negative_zero();
    return result;
}


bigint bigint::multiply_by_2(bigint& factor) 
{
   bigint result = bigint();
   result.do_bigadd(factor.big_value,factor.big_value,result.big_value);
   return result;
}

//Purposely unimplemented. Not needed. See quot_rem.
void divide_by_2 (bigint::unumber& unumber_value) 
{
   unumber_value /= 2;
}

//
// Division algorithm.
//

bigint::quot_rem divide (const bigint& left, const bigint& right) 
{
   //Special case check for divide by 0
   if (right == 0) throw range_error ("cannot divide by 0");
   //Use quote_rem stack given in source code
   //consists of <bigint,bigint>
   stack <quot_rem> div_stack;
   //Copy values into temporary variables
   bigint quotient("0"), remainder = left;
   bigint top = bigint("1"), right_val = right;
   bigint left_copy = left;
   //Clean copied values to operate on positive integers
   left_copy.negative = false;
   right_val.negative = false;
   remainder.negative = false;
   
   //Create the stack with powers of 2 and value doubles
   while ( right_val <= left_copy )
   {
      div_stack.push(quot_rem(top,right_val));
      top = top.multiply_by_2(top);
      right_val = right_val.multiply_by_2(right_val);
   }
   //Pop and subtract from total when less than
   while ( !div_stack.empty() )
   {
      quot_rem pair = popstack(div_stack);
      if ( pair.second <= remainder ){
         remainder = remainder - pair.second;
         quotient = quotient + pair.first;
      }
   }
   //Set the negative flag if applicable
   if (left.negative != right.negative)
   {
      quotient.negative = true;
   }
   return quot_rem (quotient, remainder);
}

bigint operator/ (const bigint& left, const bigint& right) 
{
   return divide (left, right).first;
}

bigint operator% (const bigint& left, const bigint& right) 
{
   return divide (left, right).second;
}

bool operator== (const bigint& left, const bigint& right) 
{
   if (left.compare(right) == 0) {
     return true;
   }
   else return false;
}

bool operator< (const bigint& left, const bigint& right) 
{
   if(left.compare(right) == -1) {
      return true;
   }
   else return false;
}

ostream &operator<< (ostream &out, const bigint &that) 
{
   bigint::bigvalue_t::const_reverse_iterator itor = 
        that.big_value.rbegin();
   bigint::bigvalue_t::const_reverse_iterator end =  
        that.big_value.rend();
   //Immediately print negatives
   if ( that.negative ) 
   {
      out << "_";
   }
   //Print and handle values exceeding 72 digits
   if ( itor != end ){
      for (int i = 1; itor != end; ++itor, ++i ){
         out << char(*itor+'0');
         if ( i == 69 ){
            out << "\\\n";
            i=0;
         }
      }
   }
   //Otherwise special case to 0   
   else
      out << '0';
   return out;
}

void bigint::clean_negative_zero()
{
   if (big_value.size() == 0) negative = false;
}

int bigint::compare (const bigint &that) const 
{
   //Signs are a quick determiner of which is bigger
   if ( negative == false && that.negative == true ) return 1;
   else if ( negative == true && that.negative == false ) return -1;
   
   bool sign = negative;
   unsigned long this_size, that_size;
   this_size = big_value.size();
   that_size = that.big_value.size();
   
   //Otherwise size can determine which is bigger
   
   //Left is bigger
   if ( this_size > that_size )
   {
      if( sign == false ) return 1;
      else return -1;
   
   //Right is bigger   
   }
   else if ( this_size < that_size )
   {
      if( sign == false ) return -1;
      else return 1;
   }
   //The bigints are the same size
   else 
   {
      bigvalue_t::const_reverse_iterator itor_this = 
         big_value.rbegin();
      bigvalue_t::const_reverse_iterator end_this = 
         big_value.rend();
      bigvalue_t::const_reverse_iterator itor_that = 
         that.big_value.rbegin();
      bigvalue_t::const_reverse_iterator end_that = 
         that.big_value.rend();
      //While iterating, return when one digit is bigger.
      while ( itor_this != end_this && itor_that != end_that )
      {
         if ( *itor_this > *itor_that )
         {
            if ( sign == false ) return 1;
            else return -1;
         }
         else if ( *itor_this < *itor_that )
         {
            if ( sign == false ) return -1;
            else return 1;
         }
         ++itor_this;
         ++itor_that;
      }
   }
   //Otherwise default to 0 as equal
   return 0;


}

int bigint::absolute_compare (const bigint &that) const 
{
   unsigned long this_size, that_size;
   bigvalue_t::const_reverse_iterator itor_this = big_value.rbegin();
   bigvalue_t::const_reverse_iterator end_this = big_value.rend();
   bigvalue_t::const_reverse_iterator itor_that =
      that.big_value.rbegin();
   bigvalue_t::const_reverse_iterator end_that =
      that.big_value.rend();
   //Store the sizes
   this_size = big_value.size();
   that_size = that.big_value.size();
   //Size is a quick determiner of which is bigger
   if ( this_size > that_size )
      return 1;
   else if ( this_size < that_size )
      return -1;
   //Otherwise iterate through the numbers
   else 
   {
      //While iterating, return when you hit a bigger number
      while ( itor_this != end_this && itor_that != end_that )
      {
         if ( *itor_this > *itor_that ) return 1;
         else if ( *itor_this < *itor_that ) return -1;
         ++itor_this;
         ++itor_that;
      }
   }
   return 0;
}

void bigint::clean_zeroes(bigvalue_t &bigvalue) const 
{
   bigvalue_t::const_reverse_iterator itor = bigvalue.rbegin();
   bigvalue_t::const_reverse_iterator end = bigvalue.rend();
   //Pop back 0 values
   while (*itor == 0 && itor != end){
      ++itor;
      bigvalue.pop_back();      
    }
}


bigint pow (const bigint& base, const bigint& exponent) 
{
   DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
   // Zero special case
   if (base == 0) return 0;
   bigint base_copy = base;
   long expt = exponent.to_long();
   bigint result = 1;
   //cout << base_copy << endl;
   
   if (expt < 0) {
      base_copy = 1 / base_copy;
      expt = - expt;
   }
   while (expt > 0) {
      //cout << expt << endl;
      if (expt & 1) { //odd
         result = result * base_copy;
         --expt;
      }else { //even
         base_copy = base_copy * base_copy;
         expt /= 2;
      }
   }
   DEBUGF ('^', "result = " << result);
   return result;
}
