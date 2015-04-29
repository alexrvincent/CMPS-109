// $Id: shape.h,v 1.7 2014-06-05 16:11:09-07 - - $

#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <utility>

using namespace std;

#include "rgbcolor.h"


//
// Shapes constitute a single-inheritance hierarchy, summarized
// here, with the superclass listed first, and subclasses indented
// under their immediate superclass.
//
// shape
//    text
//    ellipse
//       circle
//    polygon
//       rectangle
//          square
//       diamond
//       triangle
//          right_triangle
//          isosceles
//          equilateral
//

//Define basic class and using statements
class shape;
struct vertex {GLfloat xpos; GLfloat ypos; };
using vertex_list = vector<vertex>;
using shape_ptr = shared_ptr<shape>; 

//
// Abstract base class for all shapes in this system.
//

class shape 
{
   //Operator will print out the coords of any shape
   friend ostream& operator<< (ostream& out, const shape&);
   private:
      shape (const shape&) = delete; // Prevent copying.
      shape& operator= (const shape&) = delete; // Prevent copying.
      shape (shape&&) = delete; // Prevent moving.
      shape& operator= (shape&&) = delete; // Prevent moving.
   protected:
      inline shape(); // Only subclass may instantiate.
   public:
      virtual ~shape() {} //Default virtual Dtor
      
      //Accessor functions
      virtual string get_type() const = 0;
      
      //Draw and debug commands
      virtual void show (ostream&) const;
      virtual void draw (const vertex&, const rgbcolor&) const = 0;
      virtual void draw_border (const vertex&, 
                const rgbcolor&) const = 0;
};


//
// Class for printing text.
//
class text: public shape 
{
   protected:
      void* glut_bitmap_font = nullptr;
      // GLUT_BITMAP_8_BY_13
      // GLUT_BITMAP_9_BY_15
      // GLUT_BITMAP_HELVETICA_10
      // GLUT_BITMAP_HELVETICA_12
      // GLUT_BITMAP_HELVETICA_18
      // GLUT_BITMAP_TIMES_ROMAN_10
      // GLUT_BITMAP_TIMES_ROMAN_24
      string textdata;
      unsigned char* text_array = nullptr;//////
   public:
      //Ctors
      text (void* glut_bitmap_font, const string& textdata);
      
      //Accessor functions
      virtual string get_type() const override {return "text";} ////
      
      //Draw and debug functions
      virtual void show (ostream&) const override;
      virtual void draw (const vertex&, const rgbcolor&) const override;
      virtual void draw_border(const vertex&, const rgbcolor&) ////
                                                 const override;////
                                                 
};

//
// Classes for ellipse and circle.
//
class ellipse: public shape 
{
   protected:
      vertex dimension;
   public:
      //Ctor
      ellipse (GLfloat width, GLfloat height); 
      
      //Accessor functions
      virtual string get_type() const override {return "ellipse";} ///
      
      //Draw and debug functions
      virtual void show (ostream&) const override;
      virtual void draw (const vertex&, const rgbcolor&) const override;
      virtual void draw_border(const vertex&, const rgbcolor&) ///
                                                 const override;///
      
};

//Circle class just an ellipse with the same width/height
class circle: public ellipse 
{
   public:
      circle (GLfloat diameter);
};

//
// Class polygon.
//
class polygon: public shape 
{
   private:
      void center(); ////
   protected:
      //List to hold polygon vertices
      vertex_list vertices; ///////
   public:
      //Ctor
      polygon (const vertex_list& vertices);
      
      //Accessor functions
      vertex_list get_vertices() {return vertices;} /////
      virtual string get_type() const override {return "polygon";} ///
      
      //Draw and debug functions
      virtual void show (ostream&) const override;
      virtual void draw (const vertex&, const rgbcolor&) const override;
      virtual void draw_border (const vertex&, const rgbcolor&) 
                                                   const override;
};

//
// Classes that inherit from polygon and triangle..
//

//Rectangle is a polygon with specified 4 coordinates
class rectangle: public polygon 
{
   public:
      rectangle (GLfloat width, GLfloat height);
};

//Square is a rectangle with same width and height
class square: public rectangle 
{
   public:
      square (GLfloat width);
};

//Diamond is a rectangle with augmented height, same width
class diamond: public polygon 
{
   public:
      diamond (const GLfloat width, const GLfloat height);
};

//Triangle is a polygon with special list
class triangle: public polygon 
{
   protected:
      vertex_list verts;
   public:
      triangle (const vertex_list& verts);
};

//Ostream op prints coordinates of a shape
ostream& operator<< (ostream& out, const shape&);

//Equilateral triangle is a triangle with same widths
class equilateral: public triangle 
{
   public:
      equilateral (GLfloat width);
};

//Isosceles triangle is a triangle with width and height
class isosceles: public triangle 
{
   public:
      isosceles (const GLfloat width, const GLfloat height);
};

//Rt_triangle is a triangle with designated width/height
class right_triangle: public triangle 
{
   public:
      right_triangle (const GLfloat width, const GLfloat height);
};


#endif

