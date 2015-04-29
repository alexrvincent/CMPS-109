// $Id: interp.cpp,v 1.15 2014-07-22 20:03:19-07 - - $

#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;

#include <GL/freeglut.h>

#include "shape.h"
#include "interp.h"
#include "debug.h"
#include "util.h"

//Map contains key font names and value GLU bitmaps
static unordered_map<string,void*> fontcode 
{
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

//Map contains key function names and value void * functions
map<string,interpreter::interpreterfn> interpreter::interp_map 
{
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border },
   {"moveby" , &interpreter::do_moveby },
};

//Map contains shape type names and value shape constructor functions
map<string,interpreter::factoryfn> interpreter::factory_map 
{
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
   {"diamond"  , &interpreter::make_diamond  },
   {"triangle" , &interpreter::make_triangle },
   {"isosceles", &interpreter::make_isosceles},
   {"equilateral", &interpreter::make_equilateral},
   {"right_triangle" , &interpreter::make_right_triangle },
};

interpreter::shape_map interpreter::objmap;

//Interpreter dtor prints out objmap on destruction
interpreter::~interpreter() 
{
   for (const auto& itor: objmap) 
   {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

//Interpreter ctor splits line up into parameters
void interpreter::interpret (const parameters& params) 
{
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.insert ({name, make_shape (++begin, end)});
}


void interpreter::do_draw (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   //Make sure draw command has exactly 5 words
   if (end - begin != 4) throw runtime_error ("do_draw: syntax err"); 
   
   //Get the name of the item and find it in the map
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) 
   {
      throw runtime_error (name + ": no such shape");
   }
   
   //Create the vertex and color varibles
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   rgbcolor color {begin[0]};
   
   //Create and push the new shape onto the object list
   object new_shape(itor->second, where, color);
   window::push_back(new_shape);
 
}

void interpreter::do_border(param begin, param end) 
{
   if(end-begin != 2) throw runtime_error ("do_border syntax > 2");  
}

void interpreter::do_moveby(param begin, param end) 
{
   if (end-begin != 1) throw runtime_error ("do_moveby syntax > 1");

}
shape_ptr interpreter::make_shape (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   //Attempt to find the shape
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) 
   {
      throw runtime_error (type + ": no such shape");
   }
   //If found call its constructor function
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   //Grab the font from the map and the text from the params
   void* the_font = fontcode.at(*itor++);
   string the_text = *itor++;
   //Append space the the end of text
   for(; itor != end; ++itor)
   {
      the_text.append(" " + *itor);
   }
   return make_shared<text> (the_font, the_text);
}

shape_ptr interpreter::make_ellipse (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Grab the width and height specified from the parameters
   float width = stof(*itor++);
   float height = stof(*itor);
   
   //Push the obtained width, height values onto the ellipse ctor.
   return make_shared<ellipse> (GLfloat(width), GLfloat(height));
}

shape_ptr interpreter::make_circle (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   
   //Same as the ellipse, just push the width onto the ctor
   return make_shared<circle> (GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_polygon (param begin, param end) 
{
   DEBUGF ('f', range (begin, end));
   //Create a list to hold all the polygon vertices
   vertex_list poly_list;

   //Push all the parameter vertices onto the poly list
   for (auto itor = begin; itor != end; ++itor)
   {
      vertex poly_v;
      poly_v.xpos = stof(*itor++);
      poly_v.ypos = stof(*itor);
      poly_list.push_back(poly_v);
   }
   
   //Create the polygon with the new list
   return make_shared<polygon> (poly_list);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Grab the width and height from the parameter input
   float width = stof(*itor++);
   float height = stof(*itor);
   
   //Simply construct a rectangle with the obtained w/h
   return make_shared<rectangle> (GLfloat(width), GLfloat(height));
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Just grab the width from the param list
   float width = stof(*itor++);
   
   //Construct a rectangle (square) with only the width
   return make_shared<square> (GLfloat(width));
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Grab the width and height from the param list
   float width = stof(*itor++);
   float height = stof(*itor);
   
   //Make the diamond with the obtained width and height by
   //passing it to the diamonds ctor
   return make_shared<diamond> (GLfloat(width), GLfloat(height));
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   
   //Create a list to hold just the 3 triangle vertices
   vertex_list tri_verts;

   //Iterate and collect/push the vertices onto the list
   for (auto itor = begin; itor != end; ++itor)
   {
      vertex tri_v;
      tri_v.xpos = stof(*itor++);
      tri_v.ypos = stof(*itor);
      tri_verts.push_back(tri_v);
   }

   //Create the triangle with the 3 vertices on the list
   return make_shared<triangle> (tri_verts);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   
   //Just push the new width onto the equilateral ctor
   return make_shared<equilateral> (GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_isosceles (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Grab the width and height from the param list
   float width = stof(*itor++);
   float height = stof(*itor);
   
   //Create the new iso with the obtained w/h
   return make_shared<isosceles> (GLfloat(width), GLfloat(height));
}

shape_ptr interpreter::make_right_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = begin;
   
   //Grab the width and height from the param list
   float width = stof(*itor++);
   float height = stof(*itor);
   
   //Create the new rt_triangle with the obtained w/h
   return make_shared<right_triangle> (GLfloat(width),GLfloat(height));
}
