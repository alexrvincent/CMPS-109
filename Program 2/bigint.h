// $Id: bigint.h,v 1.16 2014-07-02 20:01:17-07 - - $

//Programmers:
//Alex Vincent - avincent@ucsc.edu
//Nader Sleem - nsleem@ucsc.edu

#ifndef __BIGINT_H__
#define __BIGINT_H__

#include <exception>
#include <iostream>
#include <utility>
#include <memory>
#include <vector>
using namespace std;

#include "debug.h"

class bigint;
using stackpair = pair <bigint,bigint>;
using quot_rem = pair<bigint,bigint>;

bigint operator+ (const bigint& left, const bigint& right);

//
// Define class bigint
//
class bigint {
      friend ostream &operator<< (ostream &, const bigint &);
   private:
      long long_value {};
      
      void initialize(const string& s);
      //BigInt Structure//
      using digit_t = unsigned char;
      using bigvalue_t = vector<digit_t>;
      bool negative = false; 
      bigvalue_t big_value; 
      
      using quot_rem = pair<bigint,bigint>;
      using unumber = unsigned long;
      friend quot_rem divide (const bigint&, const bigint&);
      
      //Arithmetic functions
      bigint multiply_by_2 (bigint&);
      friend void divide_by_2 (unumber&);
      void do_bigadd(const bigvalue_t&, const bigvalue_t&, 
                                        bigvalue_t&) const;
      void do_bigsub(const bigvalue_t&, const bigvalue_t&, 
                                        bigvalue_t&) const;
      
      //Clean and compare functions
      void clean_negative_zero();
      int compare (const bigint &that) const;
      int absolute_compare(const bigint &that) const;
      void clean_zeroes(bigvalue_t  &bigvalue) const;
   public:

      //
      // Ensure synthesized members are genrated.
      //
      bigint() = default;
      bigint (const bigint&) = default;
      bigint (bigint&&) = default;
      bigint& operator= (const bigint&) = default;
      bigint& operator= (bigint&&) = default;
      ~bigint() = default;

      //
      // Extra ctors to make bigints.
      //
      bigint (const long);
      bigint (const string&);

      //
      // Basic add/sub operators.
      //
      friend bigint operator+ (const bigint&, const bigint&);
      friend bigint operator- (const bigint&, const bigint&);
      friend bigint operator+ (const bigint&);
      friend bigint operator- (const bigint&);
      long to_long() const;

      //
      // Extended operators implemented with add/sub.
      //
      friend bigint operator* (bigint&, bigint&);
      friend bigint operator/ (const bigint&, const bigint&);
      friend bigint operator% (const bigint&, const bigint&);

      //
      // Comparison operators.
      //
      friend bool operator== (const bigint&, const bigint&);
      friend bool operator<  (const bigint&, const bigint&);
};

//
// The rest of the operators do not need to be friends.
// Make the comparisons inline for efficiency.
//

bigint pow (const bigint& base, const bigint& exponent);

inline bool operator!= (const bigint &left, const bigint &right) {
   return not (left == right);
}
inline bool operator>  (const bigint &left, const bigint &right) {
   return right < left;
}
inline bool operator<= (const bigint &left, const bigint &right) {
   return not (right < left);
}
inline bool operator>= (const bigint &left, const bigint &right) {
   return not (left < right);
}

#endif

