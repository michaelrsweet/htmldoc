// generated by Fast Light User Interface Designer (fluid) version 1.0100

#ifndef HelpDialog_h
#define HelpDialog_h
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <string.h>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>

class HelpDialog {
  int index_;
  int max_;
  int line_[100];
  char file_[100][256];
public:
  HelpDialog();
private:
  Fl_Window *window_;
  Fl_Help_View *view_;
  inline void cb_view__i(Fl_Help_View*, void*);
  static void cb_view_(Fl_Help_View*, void*);
  Fl_Button *back_;
  inline void cb_back__i(Fl_Button*, void*);
  static void cb_back_(Fl_Button*, void*);
  Fl_Button *forward_;
  inline void cb_forward__i(Fl_Button*, void*);
  static void cb_forward_(Fl_Button*, void*);
  Fl_Button *smaller_;
  inline void cb_smaller__i(Fl_Button*, void*);
  static void cb_smaller_(Fl_Button*, void*);
  Fl_Button *larger_;
  inline void cb_larger__i(Fl_Button*, void*);
  static void cb_larger_(Fl_Button*, void*);
  static Fl_Menu_Item menu_[];
  inline void cb_Close_i(Fl_Menu_*, void*);
  static void cb_Close(Fl_Menu_*, void*);
public:
  int h();
  void hide();
  void load(const char *f);
  void position(int xx, int yy);
  void resize(int xx, int yy, int ww, int hh);
  void show();
  void textsize(uchar s);
  uchar textsize();
  void topline(const char *n);
  void topline(int n);
  int visible();
  int w();
  int x();
  int y();
};
#endif