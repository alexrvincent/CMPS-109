// $Id: shape.cpp,v 1.7 2014-05-08 18:32:56-07 - - $

#include <math.h>
#include <unordered_map>
#include <typeinfo>

using namespace std;

#include "util.h"
#include "shape.h"

//Map holds key = GLFont and value = Font Name
static unordered_map<void*,string> fontname 
{
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

//Map holds value = GLFont and key = Font Name
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

const double PI = 3.141592653589793;
//General shift operator prints out shape position
ostream& operator<< (ostream& out, const vertex& where) 
{
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

//Shape ctor purposely does nothing
shape::shape() 
{
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) 
{
   //Use temportary ptr to make new array
   this->text_array = new unsigned char[textdata.length()];
   
   
   //Copy the data into the main readable string text
   for(size_t i=0; i< textdata.length(); ++i)
   {
      text_array[i]=textdata[i];
   }
}

//Default ellipse ctor just assigns values to dimension
ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) 
{
   DEBUGF ('c', this);
}

//Default circle ctor just calls ellipse ctor with same values
circle::circle (GLfloat diameter): ellipse (diameter, diameter) 
{
   DEBUGF ('c', this);
}


//Push the correct points onto the rectangle vertex list
rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({}) 
{
   vertex bt_left;
   vertex bt_right;
   vertex tp_right;
   vertex tp_left;
   
   bt_left.xpos=0;
   bt_left.ypos=0;
   bt_right.xpos=width;
   bt_right.ypos=0;
   tp_right.xpos=width;
   tp_right.ypos=height;
   tp_left.xpos=0;
   tp_left.ypos=height;
   
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(tp_right);
   vertices.push_back(tp_left);
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
   vertex bt_left;
   vertex bt_right;
   vertex tp_right;
   vertex tp_left;
   
   bt_left.xpos=0;
   bt_left.ypos=0;
   bt_right.xpos=width;
   bt_right.ypos=0;
   tp_right.xpos=width;
   tp_right.ypos=width;
   tp_left.xpos=0;
   tp_left.ypos=width;
   
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(tp_right);
   vertices.push_back(tp_left);
   
}

diamond::diamond (GLfloat width, GLfloat height):
            polygon({}) 
{
   vertex bt_left;
   vertex bt_right;
   vertex tp_right;
   vertex tp_left;
   
   bt_left.xpos=0;
   bt_left.ypos=height / 2;
   bt_right.xpos=width/2;
   bt_right.ypos=0;
   tp_right.xpos=width;
   tp_right.ypos=height/2;
   tp_left.xpos=width/2;
   tp_left.ypos=height;
   
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(tp_right);
   vertices.push_back(tp_left);
}

//Default polygon just pushes vertices onto the list
polygon::polygon (const vertex_list& vertices): vertices(vertices) 
{
   DEBUGF ('c', this);
}

//Default triangle pushes 3 points onto the list
triangle::triangle (const vertex_list& verts):
            polygon({}) 
{
   DEBUGF ('c', this);
   vertices = verts;
}

right_triangle::right_triangle (GLfloat width, GLfloat height):
            triangle({}) 
{
   DEBUGF ('c', this << "(" << width << "," << height << ")");
   vertex bt_left;
   vertex bt_right;
   vertex top;
   
   bt_left.xpos=0;
   bt_left.ypos=0;
   bt_right.xpos=width;
   bt_right.ypos=0;
   top.xpos=0;
   top.ypos=height;
  
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(top);
}

isosceles::isosceles (GLfloat width, GLfloat height):
            triangle({}) 
 {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
   
   vertex bt_left;
   vertex bt_right;
   vertex top;
   
   bt_left.xpos=0;
   bt_left.ypos=0;
   bt_right.xpos=width/2;
   bt_right.ypos=height;
   top.xpos=width;
   top.ypos=0;
  
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(top);
}

equilateral::equilateral (GLfloat width):
            triangle({}) {
   DEBUGF ('c', this);
   
   vertex bt_left;
   vertex bt_right;
   vertex top;
   
   bt_left.xpos=0;
   bt_left.ypos=0;
   bt_right.xpos=width/2;
   bt_right.ypos=sqrt(pow(width,2)+pow(width/2,2));
   top.xpos=width;
   top.ypos=0;
 
   vertices.push_back(bt_left);
   vertices.push_back(bt_right);
   vertices.push_back(top);
}

//Draw the text
void text::draw (const vertex& center, const rgbcolor& color) const 
{
   glRasterPos2f(center.xpos, center.ypos);
   glColor3d(color.ubvec[0], color.ubvec[1], color.ubvec[2]);
   glutBitmapString(glut_bitmap_font, text_array);
}

//Draw the text's border
void text::draw_border (const vertex& center, const rgbcolor& color) 
                        const 
{
   DEBUGF ('d', this << "(" << center << "," << color << ")");
}

//Draw the ellipse
void ellipse::draw (const vertex& center, const rgbcolor& color)
                    const 
{
   //Begin drawing with openGL
   glBegin (GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3d(color.ubvec[0], color.ubvec[1], color.ubvec[2]);
   const float d = 2 * PI / 32;
   //Draw theta up until 2_PI delta times
   for (float t = 0; t < 2 * PI; t += d) 
   {
      //Calculate the x and y points at each radian
      float x_pt = dimension.xpos * cos (t) + center.xpos;
      float y_pt = dimension.ypos * sin (t) + center.ypos;
      //Draw at that point
      glVertex2f (x_pt, y_pt);
   }
   glEnd();
}

//Draw the ellipses' border
void ellipse::draw_border
   (const vertex& center, const rgbcolor& color) const 
{
   (void)color;
   //Default border is 4
   glLineWidth(4);
   glBegin (GL_LINE_LOOP);
   glEnable (GL_LINE_SMOOTH);
   //Default color is red
   glColor3d(1, 0, 0);
   const float d = 2 * PI / 32;
   //Repeat process of drawing ellipse
   //But this time use line-loops to trace the edge
   for (float t = 0; t < 2 * PI; t += d) {
      float x_pt = dimension.xpos * cos (t) + center.xpos;
      float y_pt = dimension.ypos * sin (t) + center.ypos;
      glVertex2f (x_pt, y_pt);
   }
   glEnd();
}


void polygon::draw (const vertex& center, const rgbcolor& color) const 
{
  //Keep track of the vertices (and each coordinate number)
   int num_x = 0;
   int num_y = 0;
   int num_verts = 0;
   //Create a new list of updated vertices
   vertex_list new_verts_list;
   for (auto itor = vertices.cbegin(); itor != vertices.cend(); ++itor)
   {
      num_verts++;
      num_x += itor->xpos;
      num_y += itor->ypos;
   }
   //Calculate the center of the polygon
   //This is just the average of all the points
   int center_x = num_x/num_verts;
   int center_y = num_y/num_verts;
   
   //Update the new list with the average count
   for (auto itor = vertices.cbegin(); itor != vertices.cend(); ++itor)
   {
      vertex new_v;
      new_v.xpos = itor->xpos - center_x;
      new_v.ypos = itor->ypos - center_y;
      new_verts_list.push_back(new_v);
   }
   
   //Now begin to push and draw the polygon from the new list
   glBegin(GL_POLYGON);
   for(size_t i=0; i<new_verts_list.size(); ++i)
   {
      glColor3d(color.ubvec[0], color.ubvec[1], color.ubvec[2]);
      glVertex2f
      (
         new_verts_list[i].xpos+center.xpos, 
         new_verts_list[i].ypos+center.ypos
      );
   }
   glEnd();
}

void polygon::draw_border (const vertex& center, const rgbcolor& color)
                           const
 {
   //Keep track of the vertices (and each coordinate number)
   int num_x = 0;
   int num_y = 0;
   int num_verts = 0;
   (void)color;
   //Create a new list of updated vertices
   vertex_list new_verts_list;
   for (auto itor = vertices.cbegin(); itor != vertices.cend(); ++itor)
    {
      num_verts++;
      num_x += itor->xpos;
      num_y += itor->ypos;
   }
    //Calculate the center of the polygon
   //This is just the average of all the points
   int center_x = num_x/num_verts;
   int center_y = num_y/num_verts;

   //Update the new list with the average count
   for (auto itor = vertices.cbegin(); itor != vertices.cend(); ++itor)
   {
      vertex new_v;
      new_v.xpos = itor->xpos - center_x;
      new_v.ypos = itor->ypos - center_y;
      new_verts_list.push_back(new_v);
   }
   
   //Now begin to draw with a Line loop instead of a polygon
   //Default border is 4 pixels
   //Default border color is red
   glLineWidth(4);
   glBegin(GL_LINE_LOOP);
   
   glColor3d(1, 0, 0);
   for(size_t i=0; i<new_verts_list.size(); ++i)
   {
      glVertex2f
      (
        new_verts_list[i].xpos+center.xpos,
        new_verts_list[i].ypos+center.ypos
      );
   }
   glEnd();
}

//Note that subclasses that inherit from
//polygon, text and ellipse do not have
//explicit draw functions. They inherit
//from the ones above.

void shape::show (ostream& out) const 
{
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const 
{
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const 
{
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const 
{
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) 
{
   obj.show (out);
   return out;
}

