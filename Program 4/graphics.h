// $Id: graphics.h,v 1.9 2014-05-15 16:42:55-07 - - $

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <vector>
#include <memory>

using namespace std;

#include <GL/freeglut.h>

#include "shape.h"
#include "rgbcolor.h"

class object 
{
   private:
      rgbcolor color;
      //rgbcolor b_color = rgbcolor("red"); ////////
   public:
      vertex center;
      shared_ptr<shape> pshape;
      
      // Default copiers, movers, dtor all OK.
      // Default ctor
      object
      (  const shared_ptr<shape>& shape, 
         vertex& where,
         rgbcolor color ): color(color), center(where), pshape(shape){}
         
      //Mutators
      void move (GLfloat delta_x, GLfloat delta_y) {
         center.xpos += delta_x;
         center.ypos += delta_y;
      }
      
      //Draw and debug functions
      void draw() { pshape->draw (center, color); }
      void draw_border() { pshape->draw_border (center, color); }
};

class mouse {
      friend class window;
   private:
      int xpos {0};
      int ypos {0};
      int entered {GLUT_LEFT};
      int left_state {GLUT_UP};
      int middle_state {GLUT_UP};
      int right_state {GLUT_UP};
   private:
      void set (int x, int y) { xpos = x; ypos = y; }
      void state (int button, int state);
      void draw();
};


class window {
      friend class mouse;
   private:
      static int width;         // in pixels
      static int height;        // in pixels
      static vector<object> objects;
      static size_t selected_obj;
      static object border;
      static mouse mus;
   private:
      static void close();
      static void entry (int mouse_entered);
      static void display();
      static void reshape (int width, int height);
      static void keyboard (GLubyte key, int, int);
      static void special (int key, int, int);
      static void motion (int x, int y);
      static void passivemotion (int x, int y);
      static void mousefn (int button, int state, int x, int y);
   public:
      static void push_back (const object& obj) {
                  objects.push_back (obj); }
      static void setwidth (int width_) { width = width_; }
      static void setheight (int height_) { height = height_; }
      static void main();
      static void move_selected_object (float delta_x, float delta_y){
                  objects[selected_obj].move(delta_x, delta_y);}    
      static void select_object (size_t index){
                  if(index >= window::objects.size()) { return; }
                  else selected_obj=index; } 
};

#endif

