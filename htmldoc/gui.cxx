//
// "$Id: gui.cxx,v 1.1 1999/11/08 18:35:16 mike Exp $"
//
//   GUI routines for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-1999 by Michael Sweet.
//
//   HTMLDOC is distributed under the terms of the GNU General Public License
//   which is described in the file "COPYING-2.0".
//
// Contents:
//
//   GUI()             - Build the HTMLDOC GUI and load the indicated book as
//                       necessary.
//   ~GUI()            - Destroy the HTMLDOC GUI.
//   GUI::doGUI()      - Display the window and loop for events.
//   GUI::progress()   - Update the progress bar on the GUI.
//   GUI::title()      - Set the title bar of the window.
//   GUI::newBook()    - Clear out the current GUI settings for a new book.
//   GUI::loadBook()   - Load a book file from disk.
//   GUI::saveBook()   - Save a book to disk.
//   GUI::checkSave()  - Check to see if a save is needed.
//   docTypeCB()       - Handle input on the document type buttons.
//   inputFilesCB()    - Handle selections in the input files browser.
//   addFileCB()       - Add a file to the input files list.
//   editFilesCB()     - Edit one or more files in the input files list.
//   deleteFileCB()    - Delete one or more files from the input files list.
//   moveUpFileCB()    - Move one or more files up in the input files list.
//   moveDownFileCB()  - Move one or more files down in the input files list.
//   logoImageCB()     - Change the logo image file.
//   outputTypeCB()    - Set the output file type.
//   outputPathCB()    - Set the output path.
//   outputFormatCB()  - Set the output format.
//   changeCB()        - Mark the current book as changed.
//   jpegCB()          - Handle JPEG changes.
//   htmlEditorCB()    - Change the HTML editor.
//   bodyColorCB()     - Set the body color.
//   bodyImageCB()     - Set the body image.
//   newBookCB()       - Create a new book.
//   openBookCB()      - Open an existing book.
//   saveBookCB()      - Save the current book to disk.
//   saveAsBookCB()    - Save the current book to disk to a new file.
//   generateBookCB()  - Generate the current book.
//   closeBookCB()     - Close the current book.
//

#include "htmldoc.h"

#ifdef HAVE_LIBFLTK

//
// Include necessary headers.
//

#  include <ctype.h>
#  include <FL/fl_ask.H>
#  include <FL/fl_file_chooser.H>
#  include <FL/Fl_Color_Chooser.H>
#  include <FL/fl_draw.H>
#  include <FL/x.H>

#  ifdef WIN32
#    include <direct.h>
#    include <io.h>
#    include "icons.h"
#  else
#    include <unistd.h>
#    include "htmldoc.xbm"
#  endif // WIN32


//
// 'GUI()' - Build the HTMLDOC GUI and load the indicated book as necessary.
//

GUI::GUI(char *filename)		// Book file to load initially
{
  Fl_Tabs		*tabs;		// Tabs
  Fl_Group		*group;		// Group
  Fl_Button		*button;	// Push button
  Fl_Box		*label;		// Label box
  static char		*htmldoc[1] = { "htmldoc" };	// argv[] array
  static Fl_Menu	tocMenu[] =	// Menu items for TOC chooser
			{
			  {"None", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"1 level", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"2 levels", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"3 levels", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"4 Levels", 0,  0, 0, 0, 0, 3, 14, 0},
			  {0}
			};
  static Fl_Menu	formatMenu[] =	// Menu items for header/footer choosers
			{
			  {"Blank", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Title", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Chapter", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Heading", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Logo", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"1,2,3,...", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"i,ii,iii,...", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"I,II,III,...", 0,  0, 0, 0, 0, 3, 14, 0},
			  {0}
			};
  static Fl_Menu	fontMenu[] =	// Menu items for font choosers
			{
			  {"Courier", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Times", 0,  0, 0, 0, 0, 3, 14, 0},
			  {"Helvetica", 0,  0, 0, 0, 0, 3, 14, 0},
			  {0}
			};


  //
  // Create a dialog window...
  //

  window   = new Fl_Window(470, 390, "HTMLDOC " SVERSION);
  controls = new Fl_Group(0, 0, 465, 360);
  tabs     = new Fl_Tabs(10, 10, 445, 260);

  //
  // Input tab...
  //

  inputTab = new Fl_Group(10, 35, 450, 220, "Input");

  group = new Fl_Group(140, 45, 150, 20, "Document Type: ");
  group->align(FL_ALIGN_LEFT);
    typeBook = new Fl_Check_Button(140, 45, 60, 20, "Book");
    typeBook->type(FL_RADIO_BUTTON);
    typeBook->down_box(FL_DIAMOND_DOWN_BOX);
    typeBook->color2(FL_BLUE);
    typeBook->setonly();
    typeBook->callback((Fl_Callback *)docTypeCB, this);

    typeWebPage = new Fl_Check_Button(200, 45, 90, 20, "Web Page");
    typeWebPage->type(FL_RADIO_BUTTON);
    typeWebPage->down_box(FL_DIAMOND_DOWN_BOX);
    typeWebPage->color2(FL_BLUE);
    typeWebPage->callback((Fl_Callback *)docTypeCB, this);
  group->end();

  group = new Fl_Group(140, 70, 160, 20, "Input Files: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  inputFiles = new Fl_Multi_Browser(140, 70, 160, 125);
  inputFiles->callback((Fl_Callback *)inputFilesCB, this);
  inputFiles->when(FL_WHEN_RELEASE | FL_WHEN_NOT_CHANGED);

  addFile = new Fl_Button(300, 70, 95, 25, "Add File...");
  addFile->callback((Fl_Callback *)addFileCB, this);

  editFile = new Fl_Button(300, 95, 95, 25, "Edit File...");
  editFile->deactivate();
  editFile->callback((Fl_Callback *)editFilesCB, this);

  deleteFile = new Fl_Button(300, 120, 95, 25, "Delete File");
  deleteFile->deactivate();
  deleteFile->callback((Fl_Callback *)deleteFilesCB, this);

  moveUpFile = new Fl_Button(300, 145, 95, 25, "Move Up");
  moveUpFile->deactivate();
  moveUpFile->callback((Fl_Callback *)moveUpFilesCB, this);

  moveDownFile = new Fl_Button(300, 170, 95, 25, "Move Down");
  moveDownFile->deactivate();
  moveDownFile->callback((Fl_Callback *)moveDownFilesCB, this);

  logoImage = new Fl_Input(140, 205, 160, 25, "Logo Image: ");
  logoImage->callback((Fl_Callback *)logoImageCB, this);

  logoBrowse = new Fl_Button(300, 205, 95, 25, "Browse...");
  logoBrowse->callback((Fl_Callback *)logoImageCB, this);

  titleImage = new Fl_Input(140, 235, 160, 25, "Title Image: ");
  titleImage->callback((Fl_Callback *)titleImageCB, this);

  titleBrowse = new Fl_Button(300, 235, 95, 25, "Browse...");
  titleBrowse->callback((Fl_Callback *)titleImageCB, this);

  inputTab->end();
  inputTab->resizable(inputFiles);

  //
  // Output tab...
  //

  outputTab = new Fl_Group(10, 35, 450, 220, "Output");
  outputTab->hide();

  group = new Fl_Group(140, 45, 265, 20, "Output To: ");
  group->align(FL_ALIGN_LEFT);
    outputFile = new Fl_Check_Button(140, 45, 50, 20, "File");
    outputFile->type(FL_RADIO_BUTTON);
    outputFile->down_box(FL_DIAMOND_DOWN_BOX);
    outputFile->color2(FL_BLUE);
    outputFile->setonly();
    outputFile->callback((Fl_Callback *)outputTypeCB, this);

    outputDirectory = new Fl_Check_Button(185, 45, 105, 20, "Directory");
    outputDirectory->type(FL_RADIO_BUTTON);
    outputDirectory->down_box(FL_DIAMOND_DOWN_BOX);
    outputDirectory->color2(FL_BLUE);
    outputDirectory->callback((Fl_Callback *)outputTypeCB, this);
  group->end();

  outputPath = new Fl_Input(140, 70, 160, 25, "Output Path: ");
  outputPath->callback((Fl_Callback *)outputPathCB, this);

  outputBrowse = new Fl_Button(300, 70, 95, 25, "Browse...");
  outputBrowse->callback((Fl_Callback *)outputPathCB, this);

  group = new Fl_Group(140, 100, 255, 20, "Output Format: ");
  group->align(FL_ALIGN_LEFT);
    typeHTML = new Fl_Check_Button(140, 100, 50, 20, "HTML");
    typeHTML->type(FL_RADIO_BUTTON);
    typeHTML->down_box(FL_DIAMOND_DOWN_BOX);
    typeHTML->color2(FL_BLUE);
    typeHTML->setonly();
    typeHTML->callback((Fl_Callback *)outputFormatCB, this);

    typePS1 = new Fl_Check_Button(200, 100, 40, 20, "PS");
    typePS1->type(FL_RADIO_BUTTON);
    typePS1->down_box(FL_DIAMOND_DOWN_BOX);
    typePS1->color2(FL_BLUE);
    typePS1->callback((Fl_Callback *)outputFormatCB, this);

    typePS2 = new Fl_Check_Button(240, 100, 45, 20, "PS2");
    typePS2->type(FL_RADIO_BUTTON);
    typePS2->down_box(FL_DIAMOND_DOWN_BOX);
    typePS2->color2(FL_BLUE);
    typePS2->callback((Fl_Callback *)outputFormatCB, this);

    typePDF = new Fl_Check_Button(290, 100, 50, 20, "PDF");
    typePDF->type(FL_RADIO_BUTTON);
    typePDF->down_box(FL_DIAMOND_DOWN_BOX);
    typePDF->color2(FL_BLUE);
    typePDF->callback((Fl_Callback *)outputFormatCB, this);
  group->end();

  group = new Fl_Group(140, 125, 265, 20, "Output Options: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  grayscale = new Fl_Check_Button(140, 125, 90, 20, "Grayscale");
  grayscale->down_box(FL_ROUND_DOWN_BOX);
  grayscale->callback((Fl_Callback *)changeCB, this);

  compression = new Fl_Check_Button(230, 125, 110, 20, "Compression");
  compression->down_box(FL_ROUND_DOWN_BOX);
  compression->callback((Fl_Callback *)changeCB, this);

  titlePage = new Fl_Check_Button(140, 150, 90, 20, "Title Page");
  titlePage->down_box(FL_ROUND_DOWN_BOX);
  titlePage->callback((Fl_Callback *)changeCB, this);

  jpegCompress = new Fl_Check_Button(230, 150, 140, 20, "JPEG Big Images");
  jpegCompress->down_box(FL_ROUND_DOWN_BOX);
  jpegCompress->callback((Fl_Callback *)jpegCB, this);

  bodyColor = new Fl_Input(140, 175, 160, 25, "Body Color: ");
  bodyColor->callback((Fl_Callback *)bodyColorCB, this);

  bodyLookup = new Fl_Button(300, 175, 95, 25, "Lookup...");
  bodyLookup->callback((Fl_Callback *)bodyColorCB, this);

  bodyImage = new Fl_Input(140, 205, 160, 25, "Body Image: ");
  bodyImage->callback((Fl_Callback *)bodyImageCB, this);

  bodyBrowse = new Fl_Button(300, 205, 95, 25, "Browse...");
  bodyBrowse->callback((Fl_Callback *)bodyImageCB, this);

  outputTab->end();

  //
  // Page tab...
  //

  pageTab = new Fl_Group(10, 35, 450, 220, "Page");
  pageTab->hide();

  pageSize = new Fl_Input(140, 45, 120, 25, "Page Size: ");
  pageSize->callback((Fl_Callback *)changeCB, this);

  pageDuplex = new Fl_Check_Button(265, 48, 130, 20, "Double-sided");
  pageDuplex->down_box(FL_ROUND_DOWN_BOX);
  pageDuplex->callback((Fl_Callback *)changeCB, this);

  pageTop = new Fl_Input(225, 80, 60, 25, "Top");
  pageTop->callback((Fl_Callback *)changeCB, this);

  pageLeft = new Fl_Input(190, 110, 60, 25, "Left");
  pageLeft->callback((Fl_Callback *)changeCB, this);

  pageRight = new Fl_Input(255, 110, 60, 25, "Right");
  pageRight->align(FL_ALIGN_RIGHT);
  pageRight->callback((Fl_Callback *)changeCB, this);

  pageBottom = new Fl_Input(225, 140, 60, 25, "Bottom");
  pageBottom->callback((Fl_Callback *)changeCB, this);

  pageHeaderLeft = new Fl_Choice(140, 170, 80, 25, "Header: ");
  pageHeaderLeft->menu(formatMenu);
  pageHeaderLeft->callback((Fl_Callback *)changeCB, this);

  pageHeaderCenter = new Fl_Choice(225, 170, 80, 25);
  pageHeaderCenter->menu(formatMenu);
  pageHeaderCenter->callback((Fl_Callback *)changeCB, this);

  pageHeaderRight = new Fl_Choice(310, 170, 80, 25);
  pageHeaderRight->menu(formatMenu);
  pageHeaderRight->callback((Fl_Callback *)changeCB, this);

  pageFooterLeft = new Fl_Choice(140, 200, 80, 25, "Footer: ");
  pageFooterLeft->menu(formatMenu);
  pageFooterLeft->callback((Fl_Callback *)changeCB, this);

  pageFooterCenter = new Fl_Choice(225, 200, 80, 25);
  pageFooterCenter->menu(formatMenu);
  pageFooterCenter->callback((Fl_Callback *)changeCB, this);

  pageFooterRight = new Fl_Choice(310, 200, 80, 25);
  pageFooterRight->menu(formatMenu);
  pageFooterRight->callback((Fl_Callback *)changeCB, this);

  pageTab->end();

  //
  // TOC tab...
  //

  tocTab = new Fl_Group(10, 35, 450, 220, "TOC");
  tocTab->hide();

  tocLevels = new Fl_Choice(140, 45, 100, 25, "Table of Contents: ");
  tocLevels->menu(tocMenu);
  tocLevels->callback((Fl_Callback *)changeCB, this);

  numberedToc = new Fl_Check_Button(245, 47, 160, 20, "Numbered Headings");
  numberedToc->down_box(FL_ROUND_DOWN_BOX);
  numberedToc->callback((Fl_Callback *)changeCB, this);

  tocHeaderLeft = new Fl_Choice(140, 75, 80, 25, "Header: ");
  tocHeaderLeft->menu(formatMenu);
  tocHeaderLeft->callback((Fl_Callback *)changeCB, this);

  tocHeaderCenter = new Fl_Choice(225, 75, 80, 25);
  tocHeaderCenter->menu(formatMenu);
  tocHeaderCenter->callback((Fl_Callback *)changeCB, this);

  tocHeaderRight = new Fl_Choice(310, 75, 80, 25);
  tocHeaderRight->menu(formatMenu);
  tocHeaderRight->callback((Fl_Callback *)changeCB, this);

  tocFooterLeft = new Fl_Choice(140, 105, 80, 25, "Footer: ");
  tocFooterLeft->menu(formatMenu);
  tocFooterLeft->callback((Fl_Callback *)changeCB, this);

  tocFooterCenter = new Fl_Choice(225, 105, 80, 25);
  tocFooterCenter->menu(formatMenu);
  tocFooterCenter->callback((Fl_Callback *)changeCB, this);

  tocFooterRight = new Fl_Choice(310, 105, 80, 25);
  tocFooterRight->menu(formatMenu);
  tocFooterRight->callback((Fl_Callback *)changeCB, this);

  tocTab->end();

  fontsTab = new Fl_Group(10, 35, 395, 220, "Fonts");
  fontsTab->hide();

  fontBaseSize = new Fl_Counter(200, 45, 100, 25, "Base Font Size: ");
  fontBaseSize->callback((Fl_Callback *)changeCB, this);
  fontBaseSize->minimum(4.0);
  fontBaseSize->maximum(24.0);
  fontBaseSize->step(0.1);
  fontBaseSize->value(11.0);
  fontBaseSize->align(FL_ALIGN_LEFT);

  fontSpacing = new Fl_Counter(200, 75, 100, 25, "Line Spacing: ");
  fontSpacing->callback((Fl_Callback *)changeCB, this);
  fontSpacing->minimum(1.0);
  fontSpacing->maximum(3.0);
  fontSpacing->step(0.1);
  fontSpacing->value(1.2);
  fontSpacing->align(FL_ALIGN_LEFT);

  bodyFont = new Fl_Choice(200, 105, 120, 25, "Body Typeface: ");
  bodyFont->menu(fontMenu);
  bodyFont->callback((Fl_Callback *)changeCB, this);
  bodyFont->value(TYPE_TIMES);

  headingFont = new Fl_Choice(200, 135, 120, 25, "Heading Typeface: ");
  headingFont->menu(fontMenu);
  headingFont->callback((Fl_Callback *)changeCB, this);
  headingFont->value(TYPE_HELVETICA);

  headFootSize = new Fl_Counter(200, 165, 100, 25, "Header/Footer Size: ");
  headFootSize->callback((Fl_Callback *)changeCB, this);
  headFootSize->minimum(4.0);
  headFootSize->maximum(24.0);
  headFootSize->step(0.1);
  headFootSize->value(11.0);
  headFootSize->align(FL_ALIGN_LEFT);

  headFootFont = new Fl_Choice(200, 195, 120, 25, "Header/Footer Typeface: ");
  headFootFont->menu(fontMenu);
  headFootFont->callback((Fl_Callback *)changeCB, this);
  headFootFont->value(TYPE_HELVETICA);

  fontsTab->end();

  //
  // HTML tab...
  //

  htmlTab = new Fl_Group(10, 35, 450, 220, "HTML");
  htmlTab->hide();

  htmlTab->end();

  //
  // PS tab...
  //

  psTab = new Fl_Group(10, 35, 450, 220, "PS");
  psTab->hide();

  psTab->end();

  //
  // PDF tab...
  //

  pdfTab = new Fl_Group(10, 35, 450, 220, "PDF");
  pdfTab->hide();

  compressionLevel = new Fl_Slider(140, 45, 180, 25, "Compression: ");
  compressionLevel->align(FL_ALIGN_LEFT);
  compressionLevel->type(FL_HOR_NICE_SLIDER);
  compressionLevel->minimum(1.0);
  compressionLevel->maximum(9.0);
  compressionLevel->value(1.0);
  compressionLevel->step(1.0);
  compressionLevel->callback((Fl_Callback *)changeCB, this);

  label = new Fl_Box(140, 70, 40, 10, "Fast");
  label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  label->labelsize(10);

  label = new Fl_Box(280, 70, 40, 10, "Best");
  label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
  label->labelsize(10);

  pdfVersion = new Fl_Group(140, 85, 255, 20, "PDF Version: ");
  pdfVersion->align(FL_ALIGN_LEFT);

    pdf11 = new Fl_Check_Button(140, 85, 125, 20, "1.1 (Acrobat 2.x)");
    pdf11->type(FL_RADIO_BUTTON);
    pdf11->down_box(FL_DIAMOND_DOWN_BOX);
    pdf11->color2(FL_BLUE);
    pdf11->callback((Fl_Callback *)pdfCB, this);

    pdf12 = new Fl_Check_Button(270, 85, 125, 20, "1.2 (Acrobat 3.x)");
    pdf12->type(FL_RADIO_BUTTON);
    pdf12->down_box(FL_DIAMOND_DOWN_BOX);
    pdf12->color2(FL_BLUE);
    pdf12->callback((Fl_Callback *)pdfCB, this);

  pdfVersion->end();

  pdfTab->end();

  //
  // Options tab...
  //

  optionsTab = new Fl_Group(10, 35, 450, 220, "Options");
  optionsTab->hide();

  htmlEditor = new Fl_Input(140, 45, 160, 25, "HTML Editor: ");
  htmlEditor->value(HTMLEditor);
  htmlEditor->callback((Fl_Callback *)htmlEditorCB, this);

  htmlBrowse = new Fl_Button(300, 45, 95, 25, "Browse...");
  htmlBrowse->callback((Fl_Callback *)htmlEditorCB, this);

  jpegQuality = new Fl_Value_Slider(140, 75, 180, 25, "JPEG Quality: ");
  jpegQuality->align(FL_ALIGN_LEFT);
  jpegQuality->type(FL_HOR_NICE_SLIDER);
  jpegQuality->minimum(50.0);
  jpegQuality->maximum(100.0);
  jpegQuality->value(90.0);
  jpegQuality->step(1.0);
  jpegQuality->callback((Fl_Callback *)changeCB, this);

  label = new Fl_Box(175, 100, 40, 10, "Good");
  label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  label->labelsize(10);

  label = new Fl_Box(280, 100, 40, 10, "Best");
  label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
  label->labelsize(10);

  optionsTab->end();

  tabs->end();

  //
  // Button bar...
  //

  button = new Fl_Button(10, 330, 50, 25, "Help");
  button->callback((Fl_Callback *)helpCB, this);

  button = new Fl_Button(65, 330, 45, 25, "New");
  button->callback((Fl_Callback *)newBookCB, this);

  button = new Fl_Button(115, 330, 60, 25, "Open...");
  button->shortcut(FL_CTRL | 'o');
  button->callback((Fl_Callback *)openBookCB, this);

  bookSave = new Fl_Button(180, 330, 50, 25, "Save");
  bookSave->shortcut(FL_CTRL | 's');
  bookSave->callback((Fl_Callback *)saveBookCB, this);

  bookSaveAs = new Fl_Button(235, 330, 80, 25, "Save As...");
  bookSaveAs->shortcut(FL_CTRL | FL_SHIFT | 's');
  bookSaveAs->callback((Fl_Callback *)saveAsBookCB, this);

  bookGenerate = new Fl_Button(320, 330, 80, 25, "Generate");
  bookGenerate->shortcut(FL_CTRL | 'g');
  bookGenerate->callback((Fl_Callback *)generateBookCB, this);

  button = new Fl_Button(405, 330, 55, 25, "Close");
  button->shortcut(FL_CTRL | 'q');
  button->callback((Fl_Callback *)closeBookCB, this);

  controls->end();

  //
  // Copyright notice...
  //

  label = new Fl_Box(10, 275, 450, 50,
          "HTMLDOC " SVERSION " Copyright 1997-1999 Mike Sweet (mike@easysw.com). This "
	  "program is free software; you can redistribute it and/or modify it "
	  "under the terms of the GNU General Public License as published by "
	  "the Free Software Foundation."
#ifdef HAVE_LIBJPEG
          " This software is based in part on the work of the Independent JPEG "
	  "Group."
#endif // HAVE_LIBJPEG
	  );
  label->labelsize(10);
  label->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  //
  // Progress bar...
  //

  progressText = new Fl_Group(10, 360, 250, 20, "HTMLDOC " SVERSION " Ready.");
  progressText->box(FL_DOWN_BOX);
  progressText->labelsize(12);
  progressText->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  progressText->end();

  progressBar = new Fl_Slider(260, 360, 200, 20);
  progressBar->type(3);
  progressBar->color(15);
  progressBar->color2(10);
  progressBar->maximum(100);
  progressBar->value(0);

  window->end();

  // Set the class name to "htmldoc".
  window->xclass("htmldoc");

  // The "icon()" member function is only available in the latest
  // beta version (and/or production versions) of FLTK!
#  ifdef WIN32
  // Load the HTMLDOC icon image...
  window->icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
#  else // X11
  // Open the X display and load the HTMLDOC icon image...
  fl_open_display();
  window->icon((char *)XCreateBitmapFromData(fl_display,
               DefaultRootWindow(fl_display), htmldoc_bits,
	       htmldoc_width, htmldoc_height));
#  endif // WIN32

  window->resizable(tabs);
  window->size_range(465, 390);
  window->show(1, htmldoc);

  Fl::check();

  //
  // Load the given book or create a new one...
  //

  book_changed = 0;

  if (filename == NULL)
    newBookCB(NULL, this);
  else
    loadBook(filename);
}


//
// '~GUI()' - Destroy the HTMLDOC GUI.
//

GUI::~GUI(void)
{
  checkSave();

  delete window;
}


//
// 'GUI::doGUI()' - Display the window and loop for events.
//

int			// O - Exit status
GUI::doGUI(void)
{
  static char	*htmldoc[1] = { "htmldoc" };	// argv[] array


  window->show(1, htmldoc);

  return (Fl::run());
}


//
// 'GUI::progress()' - Update the progress bar on the GUI.
//

void
GUI::progress(int  percent,	// I - Percent complete
              char *text)	// I - Text prompt
{
  if (text != NULL)
  {
    progressText->label(text);
    progressText->redraw();
  }
  else if (percent == 0)
  {
    progressText->label("HTMLDOC " SVERSION " Ready.");
    progressText->redraw();
  }

  progressBar->value(percent);

  Fl::check();
}


//
// 'GUI::title()' - Set the title bar of the window.
//

void
GUI::title(char *filename,	// Name of file being edited
           int  changed)	// Whether or not the file is modified
{
  static char	title[1024];	// Title string


  book_changed = changed;

  if (filename == NULL || filename[0] == '\0')
  {
    book_filename[0] = '\0';
    strcpy(title, "NewBook");
  }
  else
  {
    strcpy(book_filename, filename);
    strcpy(title, file_basename(filename));
  }

  if (changed)
    strcat(title, "(modified) - ");
  else
    strcat(title, " - ");

  strcat(title, "HTMLDOC " SVERSION);

  window->label(title);
  if (window->visible())
    Fl::check();
}


//
// 'GUI::newBook()' - Clear out the current GUI settings for a new book.
//

int			// O - 1 on success, 0 on failure
GUI::newBook(void)
{
  if (!checkSave())
    return (0);

  typeBook->setonly();
  docTypeCB(typeBook, this);

  inputFiles->clear();
  inputFilesCB(inputFiles, this);

  logoImage->value("");
  titleImage->value("");

  outputFile->setonly();
  outputTypeCB(outputFile, this);

  outputPath->value("");

  typeHTML->setonly();
  outputFormatCB(typeHTML, this);

  grayscale->clear();
  compression->clear();
  compressionLevel->value(1.0);
  compressionLevel->deactivate();
  titlePage->set();
  jpegCompress->clear();
  jpegQuality->value(90.0);
  jpegQuality->deactivate();

  bodyColor->value(BodyColor);
  bodyImage->value(BodyImage);

  pageSize->value("Universal");
  pageLeft->value("1.0in");
  pageRight->value("0.5in");
  pageTop->value("0.5in");
  pageBottom->value("0.5in");
  pageDuplex->clear();

  pageHeaderLeft->value(0);	/* Blank */
  pageHeaderCenter->value(1);	/* Title */
  pageHeaderRight->value(0);	/* Blank */

  pageFooterLeft->value(2);	/* Heading */
  pageFooterCenter->value(0);	/* Blank */
  pageFooterRight->value(4);	/* 1,2,3,... */

  tocLevels->value(3);
  numberedToc->clear();

  tocHeaderLeft->value(0);	/* Blank */
  tocHeaderCenter->value(1);	/* Title */
  tocHeaderRight->value(0);	/* Blank */

  tocFooterLeft->value(2);	/* Heading */
  tocFooterCenter->value(0);	/* Blank */
  tocFooterRight->value(5);	/* i,ii,iii,... */

  pdf12->setonly();

  title(NULL, 0);

  return (1);
}


//
// 'GUI::loadBook()' - Load a book file from disk.
//

int				// O - 1 = success, 0 = fail
GUI::loadBook(char *filename)	// I - Name of book file
{
  int		i,
		count;
  FILE		*fp;
  char		line[10240],
		*lineptr,
		temp[1024],
		temp2[1024],
		*tempptr;
  static char	formats[256],
		first_time = 1;
  static char	*fonts[] =		// Font names...
		{ "Courier", "Times", "Helvetica" };


  //
  // Initialize the format character lookup table as needed...
  //

  if (first_time)
  {
    first_time = 0;

    memset(formats, 0, sizeof(formats));
    formats['t'] = 1;
    formats['h'] = 2;
    formats['l'] = 3;
    formats['1'] = 4;
    formats['i'] = 5;
    formats['I'] = 6;
  }

  //
  // If the filename contains a path, chdir to it first...
  //

  if ((tempptr = file_directory(filename)) != NULL)
  {
   /*
    * Filename contains a complete path - get the directory portion and do
    * a chdir()...
    */

    chdir(tempptr);

    filename = file_basename(filename);
  }

  //
  // Open the file...
  //

  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    fl_alert("Unable to open \"%s\"!", filename);
    return (0);
  }

  fgets(line, sizeof(line), fp);  /* Get header... */
  if (strncmp(line, "#HTMLDOC", 8) != 0)
  {
    fclose(fp);
    fl_alert("Bad or missing #HTMLDOC header:\n%-80.80s", line);
    return (0);
  }

  if (!newBook())
  {
    fclose(fp);
    return (0);
  }

  fgets(line, sizeof(line), fp);  /* Get input file count... */
  count = atoi(line);

  for (i = 0; i < count; i ++)
  {
    fgets(line, sizeof(line), fp);  /* Get input file... */
    line[strlen(line) - 1] = '\0';  /* Drop trailing newline */

    inputFiles->add(line);
  }

  inputFiles->topline(1);

  fgets(line, sizeof(line), fp);  /* Get options... */
  line[strlen(line) - 1] = '\0';  /* Drop trailing newline */

  for (lineptr = line; *lineptr != '\0';)
  {
    while (*lineptr == ' ')
      lineptr ++;

    for (tempptr = temp; *lineptr != '\0' && *lineptr != ' ';)
      *tempptr++ = *lineptr++;
    *tempptr = '\0';

    if (strcmp(temp, "--duplex") == 0)
    {
      pageDuplex->set();
      continue;
    }
    else if (strncmp(temp, "--jpeg", 6) == 0)
    {
      jpegCompress->set();
      jpegQuality->activate();
      if (strlen(temp) > 7)
        jpegQuality->value(atof(temp + 7));
      else
        jpegQuality->value(90.0);
      continue;
    }
    else if (strcmp(temp, "--grayscale") == 0)
    {
      grayscale->set();
      continue;
    }
    else if (strncmp(temp, "--compression", 13) == 0)
    {
      compression->set();
      if (strlen(temp) > 14)
        compressionLevel->value(atof(temp + 14));
      else
        compressionLevel->value(1.0);
      continue;
    }
    else if (strcmp(temp, "--no-compression") == 0)
    {
      compression->clear();
      continue;
    }
    else if (strcmp(temp, "--numbered") == 0)
    {
      numberedToc->set();
      continue;
    }
    else if (strcmp(temp, "--no-toc") == 0)
    {
      tocLevels->value(0);
      continue;
    }
    else if (strcmp(temp, "--no-title") == 0)
    {
      titlePage->clear();
      continue;
    }
    else if (strcmp(temp, "--webpage") == 0)
    {
      typeWebPage->setonly();
      docTypeCB(typeWebPage, this);
      continue;
    }
    else if (temp[0] != '-')
    {
      inputFiles->add(temp);
      continue;
    }

    while (*lineptr == ' ')
      lineptr ++;

    for (tempptr = temp2; *lineptr != '\0' && *lineptr != ' ';)
      *tempptr++ = *lineptr++;
    *tempptr = '\0';

    if (strcmp(temp, "-t") == 0)
    {
      if (strcmp(temp2, "html") == 0)
      {
        typeHTML->setonly();
	outputFormatCB(typeHTML, this);
      }
      else if (strcmp(temp2, "ps1") == 0)
      {
        typePS1->setonly();
	outputFormatCB(typePS1, this);
      }
      else if (strcmp(temp2, "ps2") == 0)
      {
        typePS2->setonly();
	outputFormatCB(typePS2, this);
      }
      else if (strcmp(temp2, "pdf") == 0)
      {
        typePDF->setonly();
	pdf12->setonly();
	outputFormatCB(typePDF, this);
      }
      else if (strcmp(temp2, "pdf11") == 0)
      {
        typePDF->setonly();
	pdf11->setonly();
	outputFormatCB(typePDF, this);
      }
    }
    else if (strcmp(temp, "--logo") == 0)
      logoImage->value(temp2);
    else if (strcmp(temp, "--title") == 0)
      titleImage->value(temp2);
    else if (strcmp(temp, "-f") == 0)
    {
      outputPath->value(temp2);
      outputFile->setonly();
      outputTypeCB(outputFile, this);
    }
    else if (strcmp(temp, "-d") == 0)
    {
      outputPath->value(temp2);
      outputDirectory->setonly();
      outputTypeCB(outputDirectory, this);
    }
    else if (strcmp(temp, "--size") == 0)
      pageSize->value(temp2);
    else if (strcmp(temp, "--left") == 0)
      pageLeft->value(temp2);
    else if (strcmp(temp, "--right") == 0)
      pageRight->value(temp2);
    else if (strcmp(temp, "--top") == 0)
      pageTop->value(temp2);
    else if (strcmp(temp, "--bottom") == 0)
      pageBottom->value(temp2);
    else if (strcmp(temp, "--header") == 0)
    {
      pageHeaderLeft->value(formats[temp2[0]]);
      pageHeaderCenter->value(formats[temp2[1]]);
      pageHeaderRight->value(formats[temp2[2]]);
    }
    else if (strcmp(temp, "--footer") == 0)
    {
      pageFooterLeft->value(formats[temp2[0]]);
      pageFooterCenter->value(formats[temp2[1]]);
      pageFooterRight->value(formats[temp2[2]]);
    }
    else if (strcmp(temp, "--bodycolor") == 0)
      bodyColor->value(temp2);
    else if (strcmp(temp, "--bodyimage") == 0)
      bodyImage->value(temp2);
    else if (strcmp(temp, "--toclevels") == 0)
      tocLevels->value(atoi(temp2));
    else if (strcmp(temp, "--tocheader") == 0)
    {
      tocHeaderLeft->value(formats[temp2[0]]);
      tocHeaderCenter->value(formats[temp2[1]]);
      tocHeaderRight->value(formats[temp2[2]]);
    }
    else if (strcmp(temp, "--tocfooter") == 0)
    {
      tocFooterLeft->value(formats[temp2[0]]);
      tocFooterCenter->value(formats[temp2[1]]);
      tocFooterRight->value(formats[temp2[2]]);
    }
    else if (strcmp(temp, "--fontsize") == 0)
      fontBaseSize->value(atof(temp2));
    else if (strcmp(temp, "--fontspacing") == 0)
      fontSpacing->value(atof(temp2));
    else if (strcmp(temp, "--headingfont") == 0)
    {
      for (i = 0; i < 3; i ++)
        if (strcasecmp(fonts[i], temp2) == 0)
	{
	  headingFont->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--bodyfont") == 0)
    {
      for (i = 0; i < 3; i ++)
        if (strcasecmp(fonts[i], temp2) == 0)
	{
	  bodyFont->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--headfootsize") == 0)
      headFootSize->value(atof(temp2));
    else if (strcmp(temp, "--headfootfont") == 0)
    {
      for (i = 0; i < 3; i ++)
        if (strcasecmp(fonts[i], temp2) == 0)
	{
	  headFootFont->value(i);
	  break;
	}
    }
  }

  fclose(fp);

  title(filename, 0);

  return (1);
}


//
// 'GUI::saveBook()' - Save a book to disk.
//

int				// O - 1 = success, 0 = fail
GUI::saveBook(char *filename)	// I - Name of book file
{
  int		i,			// Looping var
		count;			// Number of files
  FILE		*fp;			// Book file pointer
  static char	*formats = ".tchl1iI";	// Format characters
  static char	*fonts[] =		// Font names...
		{ "Courier", "Times", "Helvetica" };


  fp = fopen(filename, "w");
  if (fp == NULL)
  {
    fl_alert("Unable to create \"%s\"!", filename);
    return (0);
  }

  fputs("#HTMLDOC " SVERSION "\n", fp);

  count = inputFiles->size();
  fprintf(fp, "%d\n", count);

  for (i = 1; i <= count; i ++)
    fprintf(fp, "%s\n", inputFiles->text(i));

  if (typeHTML->value())
    fputs("-t html", fp);
  else if (typePS1->value())
    fputs("-t ps1", fp);
  else if (typePS2->value())
    fputs("-t ps2", fp);
  else if (pdf11->value())
    fputs("-t pdf11", fp);
  else
    fputs("-t pdf", fp);

  if (outputFile->value())
    fprintf(fp, " -f %s", outputPath->value());
  else
    fprintf(fp, " -d %s", outputPath->value());

  if (typeWebPage->value())
    fputs(" --webpage", fp);
  else
  {
    if (titlePage->value() == 0)
      fputs(" --no-title", fp);

    if (tocLevels->value() == 0)
      fputs(" --no-toc", fp);
    else
      fprintf(fp, " --toclevels %d", tocLevels->value());

    if (numberedToc->value())
      fputs(" --numbered", fp);
  }

  if (logoImage->size() > 0)
    fprintf(fp, " --logo %s", logoImage->value());

  if (titleImage->size() > 0)
    fprintf(fp, " --title %s", titleImage->value());

  if (bodyColor->size() > 0)
    fprintf(fp, " --bodycolor %s", bodyColor->value());

  if (bodyImage->size() > 0)
    fprintf(fp, " --bodyimage %s", bodyImage->value());

  if (!typeHTML->value())
  {
    if (pageSize->size() > 0)
      fprintf(fp, " --size %s", pageSize->value());

    if (pageLeft->size() > 0)
      fprintf(fp, " --left %s", pageLeft->value());

    if (pageRight->size() > 0)
      fprintf(fp, " --right %s", pageRight->value());

    if (pageTop->size() > 0)
      fprintf(fp, " --top %s", pageTop->value());

    if (pageBottom->size() > 0)
      fprintf(fp, " --bottom %s", pageBottom->value());

    fprintf(fp, " --header %c%c%c",
            formats[pageHeaderLeft->value()],
	    formats[pageHeaderCenter->value()],
	    formats[pageHeaderRight->value()]);

    fprintf(fp, " --footer %c%c%c",
            formats[pageFooterLeft->value()],
	    formats[pageFooterCenter->value()],
	    formats[pageFooterRight->value()]);

    fprintf(fp, " --tocheader %c%c%c",
            formats[tocHeaderLeft->value()],
	    formats[tocHeaderCenter->value()],
	    formats[tocHeaderRight->value()]);

    fprintf(fp, " --tocfooter %c%c%c",
            formats[tocFooterLeft->value()],
	    formats[tocFooterCenter->value()],
	    formats[tocFooterRight->value()]);

    if (pageDuplex->value())
      fputs(" --duplex", fp);

    if (grayscale->value())
      fputs(" --grayscale", fp);

    if (!compression->value())
      fputs(" --no-compression", fp);
    else
      fprintf(fp, " --compression=%.0f", compressionLevel->value());

    if (jpegCompress->value())
      fprintf(fp, " --jpeg=%.0f", jpegQuality->value());

    fprintf(fp, " --fontsize %.1f", fontBaseSize->value());
    fprintf(fp, " --fontspacing %.1f", fontSpacing->value());
    fprintf(fp, " --headingfont %s", fonts[headingFont->value()]);
    fprintf(fp, " --bodyfont %s", fonts[bodyFont->value()]);
    fprintf(fp, " --headfootsize %.1f", headFootSize->value());
    fprintf(fp, " --headfootfont %s", fonts[headFootFont->value()]);
  }

  fputs("\n", fp);
  fclose(fp);

  title(filename, 0);

  return (1);
}


//
// 'GUI::checkSave()' - Check to see if a save is needed.
//

int			// O - 1 if no save is needed, 0 if save is needed
GUI::checkSave(void)
{
  if (book_changed)
  {
    switch (fl_choice("The current book has been changed.\n"
                      "Do you wish to save it first?",
		      "Cancel", "Save", "Discard"))
    {
      case 0 : /* Cancel */
          return (0);

      case 1 : /* Save */          
	  if (book_filename[0] != '\0')
            return (saveBook(book_filename));
	  else
          {
	    saveAsBookCB(NULL, this);
	    return (!book_changed);
	  }

      case 2 : /* Discard */
          return (1);
    }
  }

  return (1);
}


//
// 'docTypeCB()' - Handle input on the document type buttons.
//

void
docTypeCB(Fl_Widget *w,		// I - Toggle button widget
          GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);

  if (w == gui->typeBook)
  {
    gui->typeHTML->activate();

    gui->titlePage->value(0);

    gui->tocLevels->value(3);
    gui->tocLevels->activate();
    gui->numberedToc->activate();

    gui->tocHeaderLeft->activate();
    gui->tocHeaderCenter->activate();
    gui->tocHeaderRight->activate();

    gui->tocFooterLeft->activate();
    gui->tocFooterCenter->activate();
    gui->tocFooterRight->activate();
  }
  else
  {
    if (gui->typeHTML->value())
      gui->typePS1->set();

    gui->typeHTML->deactivate();

    gui->titlePage->value(0);

    gui->tocLevels->value(0);
    gui->tocLevels->deactivate();

    gui->numberedToc->value(0);
    gui->numberedToc->deactivate();

    gui->tocHeaderLeft->deactivate();
    gui->tocHeaderCenter->deactivate();
    gui->tocHeaderRight->deactivate();

    gui->tocFooterLeft->deactivate();
    gui->tocFooterCenter->deactivate();
    gui->tocFooterRight->deactivate();
  }
}


//
// 'inputFilesCB()' - Handle selections in the input files browser.
//

void
inputFilesCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  int	i,			// Looping var
	num_items;		// Number of items in the file list


  REF(w);

  num_items = gui->inputFiles->size();

  for (i = 1; i <= num_items; i ++)
    if (gui->inputFiles->selected(i))
      break;

  if (i <= num_items)
  {
    gui->editFile->activate();
    gui->deleteFile->activate();
  }
  else
  {
    gui->editFile->deactivate();
    gui->deleteFile->deactivate();
  }

  if (gui->inputFiles->selected(1) || num_items == 0)
    gui->moveUpFile->deactivate();
  else
    gui->moveUpFile->activate();

  if (gui->inputFiles->selected(num_items) || num_items == 0)
    gui->moveDownFile->deactivate();
  else
    gui->moveDownFile->activate();

  if (Fl::event_clicks() && i <= num_items)
    editFilesCB(gui->editFile, gui);
}


//
// 'addFileCB()' - Add a file to the input files list.
//

void
addFileCB(Fl_Widget *w,		// I - Widget
          GUI       *gui)	// I - GUI
{
  char		*filename;		// New file to load
  static char	lastfile[1024] = "";	// Last file picked


  REF(w);

  if ((filename = fl_file_chooser("HTML File?", "*.htm*", lastfile)) != NULL)
  {
    gui->inputFiles->add(filename);
    gui->title(gui->book_filename, 1);

    //
    // Copy the filename over, then delete the filename component as needed.
    //

    strcpy(lastfile, file_directory(filename));
  }
}


//
// 'editFilesCB()' - Edit one or more files in the input files list.
//

void
editFilesCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  int			i,		// Looping var
			num_items;	// Number of items in the file list
  char			command[1024];	// Editor command
#ifdef WIN32
  STARTUPINFO		suInfo;		// Process startup information
  PROCESS_INFORMATION	prInfo;		// Process information
#endif // WIN32


  REF(w);

  num_items = gui->inputFiles->size();

  for (i = 1; i <= num_items; i ++)
    if (gui->inputFiles->selected(i))
    {
      sprintf(command, gui->htmlEditor->value(), gui->inputFiles->text(i));

#ifdef WIN32
      memset(&suInfo, 0, sizeof(suInfo));
      suInfo.cb = sizeof(suInfo);

      if (!CreateProcess(NULL, command, NULL, NULL, FALSE,
                         NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &prInfo))
        fl_alert("Unable to start editor!");
#else
      strcat(command, "&");
      if (system(command))
        fl_alert("Unable to start editor!");
#endif // !WIN32
    }
}


//
// 'deleteFileCB()' - Delete one or more files from the input files list.
//

void
deleteFilesCB(Fl_Widget *w,	// I - Widget
              GUI       *gui)	// I - GUI
{
  int	i,			// Looping var
	num_items;		// Number of items in the file list


  REF(w);

  num_items = gui->inputFiles->size();

  for (i = num_items; i > 0; i --)
    if (gui->inputFiles->selected(i))
    {
      gui->inputFiles->select(i, 0);
      gui->inputFiles->remove(i);
      gui->title(gui->book_filename, 1);
    }
}


//
// 'moveUpFileCB()' - Move one or more files up in the input files list.
//

void
moveUpFilesCB(Fl_Widget *w,	// I - Widget
              GUI       *gui)	// I - GUI
{
  int	i,			// Looping var
	num_items;		// Number of items in the file list
  char	*file;			// File to move up


  REF(w);

  num_items = gui->inputFiles->size();

  for (i = 1; i <= num_items; i ++)
    if (gui->inputFiles->selected(i))
    {
      file = (char *)gui->inputFiles->text(i);
      gui->inputFiles->insert(i - 1, file);
      gui->inputFiles->select(i - 1);
      gui->inputFiles->remove(i + 1);
      gui->inputFiles->select(i, 0);
      gui->title(gui->book_filename, 1);
    }

  if (gui->inputFiles->selected(1))
    gui->moveUpFile->deactivate();

  if (!gui->inputFiles->selected(num_items))
    gui->moveDownFile->activate();
}


//
// 'moveDownFileCB()' - Move one or more files down in the input files list.
//

void
moveDownFilesCB(Fl_Widget *w,	// I - Widget
                GUI       *gui)	// I - GUI
{
  int	i,			// Looping var
	num_items;		// Number of items in the file list
  char	*file;			// File to move down


  REF(w);

  num_items = gui->inputFiles->size();

  for (i = num_items; i > 0; i --)
    if (gui->inputFiles->selected(i))
    {
      file = (char *)gui->inputFiles->text(i);
      gui->inputFiles->insert(i + 2, file);
      gui->inputFiles->select(i + 2);
      gui->inputFiles->remove(i);
      gui->inputFiles->select(i, 0);
      gui->title(gui->book_filename, 1);
    }

  if (!gui->inputFiles->selected(1))
    gui->moveUpFile->activate();

  if (gui->inputFiles->selected(num_items))
    gui->moveDownFile->deactivate();
}


//
// 'logoImageCB()' - Change the logo image file.
//

void
logoImageCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  char	*filename;		// Filename from chooser


  if (w == gui->logoBrowse)
  {
    filename = fl_file_chooser("Logo Image?", "*.{gif|GIF|jpg|JPG|png|PNG}", NULL);

    if (filename != NULL)
    {
      gui->logoImage->value(filename);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'titleImageCB()' - Change the title image file.
//

void
titleImageCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  char	*filename;		// Filename from chooser


  if (w == gui->titleBrowse)
  {
    filename = fl_file_chooser("Title Image?", "*.{gif|GIF|jpg|JPG|png|PNG}", NULL);

    if (filename != NULL)
    {
      gui->titleImage->value(filename);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'outputTypeCB()' - Set the output file type.
//

void
outputTypeCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  if (w == gui->outputFile)
  {
    gui->outputBrowse->activate();
    gui->typePDF->activate();
  }
  else if (gui->typePDF->value())
    gui->outputFile->setonly();
  else
  {
    gui->outputBrowse->deactivate();
    gui->typePDF->deactivate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'outputPathCB()' - Set the output path.
//

void
outputPathCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  char	*filename;		// Filename from chooser


  if (w == gui->outputBrowse)
  {
    if (gui->typeHTML->value())
      filename = fl_file_chooser("Output Path?", "*.htm*", NULL);
    else if (gui->typePDF->value())
      filename = fl_file_chooser("Output Path?", "*.pdf", NULL);
    else
      filename = fl_file_chooser("Output Path?", "*.ps", NULL);

    if (filename != NULL)
    {
      gui->outputPath->value(filename);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'outputFormatCB()' - Set the output format.
//

void
outputFormatCB(Fl_Widget *w,
               GUI       *gui)	// I - GUI
{
  gui->title(gui->book_filename, 1);

  if (w == gui->typePDF)
  {
    if (gui->pdf12->value())
    {
      gui->compression->value(1);
      gui->compression->activate();
      gui->compressionLevel->activate();
    }

    gui->outputDirectory->deactivate();
  }
  else
  {
    gui->compression->value(0);
    gui->compression->deactivate();
    gui->compressionLevel->deactivate();
    gui->outputDirectory->activate();
  }

  if (w == gui->typeHTML)
  {
    gui->grayscale->value(0);
    gui->grayscale->deactivate();

    gui->pageTab->deactivate();

    gui->tocHeaderLeft->deactivate();
    gui->tocHeaderCenter->deactivate();
    gui->tocHeaderRight->deactivate();

    gui->tocFooterLeft->deactivate();
    gui->tocFooterCenter->deactivate();
    gui->tocFooterRight->deactivate();

    gui->fontsTab->deactivate();
  }
  else
  {
    gui->grayscale->activate();

    gui->pageTab->activate();

    gui->tocHeaderLeft->activate();
    gui->tocHeaderCenter->activate();
    gui->tocHeaderRight->activate();

    gui->tocFooterLeft->activate();
    gui->tocFooterCenter->activate();
    gui->tocFooterRight->activate();

    gui->fontsTab->activate();
  }

  if (w == gui->typeHTML || w == gui->typePS1)
  {
    gui->jpegCompress->value(0);
    gui->jpegCompress->deactivate();
    gui->jpegQuality->deactivate();
  }
  else
    gui->jpegCompress->activate();
}


//
// 'changeCB()' - Mark the current book as changed.
//

void
changeCB(Fl_Widget *w,		// I - Widget
         GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);
}


//
// 'jpegCB()' - Handle JPEG changes.
//

void
jpegCB(Fl_Widget *w,		// I - Widget
       GUI       *gui)		// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);

  if (gui->jpegCompress->value())
    gui->jpegQuality->activate();
  else
    gui->jpegQuality->deactivate();
}


//
// 'pdfCB()' - Handle PDF version changes.
//

void
pdfCB(Fl_Widget *w,		// I - Widget
      GUI       *gui)		// I - GUI
{
  REF(w);


  if (gui->pdf11->value())
  {
    gui->compression->deactivate();
    gui->compressionLevel->deactivate();
  }
  else
  {
    gui->compression->activate();
    gui->compressionLevel->activate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'htmlEditorCB()' - Change the HTML editor.
//

void
htmlEditorCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  char	*filename;		// New HTML editor file
  char	command[1024];		// Command string


  if (w == gui->htmlBrowse)
  {
#  ifdef WIN32
    filename = fl_file_chooser("HTML Editor?", "*.exe", NULL);
#  else
    filename = fl_file_chooser("HTML Editor?", "*", NULL);
#  endif // WIN32

    if (filename != NULL)
    {
      if (strstr(filename, "netscape") != NULL ||
          strstr(filename, "NETSCAPE") != NULL)
#ifdef WIN32
        sprintf(command, "%s -edit %%s", filename);
#else
        sprintf(command, "%s -remote \'editFile(%%s)\'", filename);
#endif // WIN32
      else
        sprintf(command, "%s %%s", filename);

      gui->htmlEditor->value(command);
    }
  }

  strcpy(HTMLEditor, gui->htmlEditor->value());
}


//
// 'bodyColorCB()' - Set the body color.
//

void
bodyColorCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  uchar	r, g, b;		// Color values
  int	color;			// Color from bar color
  char	newcolor[255];		// New color string


  if (w == gui->bodyLookup)
  {
    if (sscanf(gui->bodyColor->value(), "#%x", &color) == 1)
    {
      r = color >> 16;
      g = (color >> 8) & 255;
      b = color & 255;
    }
    else
    {
      r = 191;
      g = 191;
      b = 191;
    }

    if (fl_color_chooser("Body Color?", r, g, b))
    {
      sprintf(newcolor, "#%02x%02x%02x", r, g, b);
      gui->bodyColor->value(newcolor);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'bodyImageCB()' - Set the body image.
//

void
bodyImageCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  char	*filename;		// Filename from chooser


  if (w == gui->bodyBrowse)
  {
    filename = fl_file_chooser("Body Image?", "*", NULL);

    if (filename != NULL)
    {
      gui->bodyImage->value(filename);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'helpCB()' - Show on-line help...
//

void
helpCB(Fl_Widget *w,	// I - Widget
       GUI       *gui)	// I - GUI
{
  REF(w);
  REF(gui);
}


//
// 'newBookCB()' - Create a new book.
//

void
newBookCB(Fl_Widget *w,		// I - Widget
          GUI       *gui)	// I - GUI
{
  REF(w);

  gui->newBook();
}


//
// 'openBookCB()' - Open an existing book.
//

void
openBookCB(Fl_Widget *w,	// I - Widget
           GUI       *gui)	// I - GUI
{
  char	*filename;		// New book file


  REF(w);

  filename = fl_file_chooser("Book File?", "*.book", gui->book_filename);
  if (filename != NULL)
    gui->loadBook(filename);
}


//
// 'saveBookCB()' - Save the current book to disk.
//

void
saveBookCB(Fl_Widget *w,	// I - Widget
           GUI       *gui)	// I - GUI
{
  if (gui->book_filename[0] == '\0')
    saveAsBookCB(w, gui);
  else
    gui->saveBook(gui->book_filename);
}


//
// 'saveAsBookCB()' - Save the current book to disk to a new file.
//

void
saveAsBookCB(Fl_Widget *w,	// I - Widget
             GUI       *gui)	// I - GUI
{
  char	*filename;		// New book file


  REF(w);

  filename = fl_file_chooser("Book File?", "*.book", gui->book_filename);

  if (filename != NULL)
  {
    if (access(filename, 0) == 0)
      if (!fl_ask("File already exists!  OK to overwrite?"))
	return;

    gui->saveBook(filename);
  }
}


//
// 'generateBookCB()' - Generate the current book.
//

void
generateBookCB(Fl_Widget *w,	// I - Widget
               GUI       *gui)	// I - GUI
{
  int		i,		/* Looping var */
	        count;		/* Number of files */
  char	  	temp[1024];	/* Temporary string */
  FILE		*docfile;	/* Document file */
  tree_t	*document,	/* Master HTML document */
		*file,		/* HTML document file */
		*toc;		/* Table of contents */
  char		*filename,	/* HTML filename */
		base[1024],	/* Base directory of HTML file */
		bookbase[1024];	/* Base directory of book file */
  static char	*formats = ".tchl1iI";	// Format characters


  REF(w);

 /*
  * Disable the GUI while we generate...
  */

  gui->controls->deactivate();

 /*
  * Set global vars used for converting the HTML files to XYZ format...
  */

  strcpy(bookbase, file_directory(gui->book_filename));

  Verbosity = 1;

  strcpy(LogoImage, gui->logoImage->value());
  strcpy(TitleImage, gui->titleImage->value());
  strcpy(OutputPath, gui->outputPath->value());

  OutputFiles = gui->outputDirectory->value();

  set_page_size((char *)gui->pageSize->value());

  PageLeft     = get_measurement((char *)gui->pageLeft->value());
  PageRight    = get_measurement((char *)gui->pageRight->value());
  PageTop      = get_measurement((char *)gui->pageTop->value());
  PageBottom   = get_measurement((char *)gui->pageBottom->value());

  PageDuplex   = gui->pageDuplex->value();
  Compression  = gui->compression->value() ? (int)gui->compressionLevel->value() : 0;
  OutputColor  = !gui->grayscale->value();
  TocNumbers   = gui->numberedToc->value();
  TocLevels    = gui->tocLevels->value();
  TitlePage    = gui->titlePage->value();

  if (gui->jpegCompress->value())
    OutputJPEG = (int)gui->jpegQuality->value();
  else
    OutputJPEG = 0;

  TocHeader[0] = formats[gui->tocHeaderLeft->value()];
  TocHeader[1] = formats[gui->tocHeaderCenter->value()];
  TocHeader[2] = formats[gui->tocHeaderRight->value()];

  TocFooter[0] = formats[gui->tocFooterLeft->value()];
  TocFooter[1] = formats[gui->tocFooterCenter->value()];
  TocFooter[2] = formats[gui->tocFooterRight->value()];

  Header[0]    = formats[gui->pageHeaderLeft->value()];
  Header[1]    = formats[gui->pageHeaderCenter->value()];
  Header[2]    = formats[gui->pageHeaderRight->value()];

  Footer[0]    = formats[gui->pageFooterLeft->value()];
  Footer[1]    = formats[gui->pageFooterCenter->value()];
  Footer[2]    = formats[gui->pageFooterRight->value()];

  _htmlBodyFont    = (typeface_t)gui->bodyFont->value();
  _htmlHeadingFont = (typeface_t)gui->headingFont->value();
  htmlSetBaseSize(gui->fontBaseSize->value(), gui->fontSpacing->value());

  HeadFootFont = (typeface_t)gui->headFootFont->value();
  HeadFootSize = gui->headFootSize->value();

  if (gui->pdf11->value())
    PDFVersion = 1.1;
  else
    PDFVersion = 1.2;

 /*
  * Load the input files...
  */

  count    = gui->inputFiles->size();
  document = NULL;

  for (i = 1; i <= count; i ++)
  {
    filename = (char *)gui->inputFiles->text(i);

    if ((docfile = fopen(filename, "rb")) != NULL)
    {
     /*
      * Read from a file...
      */

      sprintf(temp, "Loading \"%s\"...", filename);
      gui->progress(100 * i / count, temp);

      strcpy(base, file_directory(filename));

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"FILENAME", (uchar *)filename);

      htmlReadFile(file, docfile, base);

      fclose(docfile);

      if (file->child != NULL)
      {
        if (document == NULL)
          document = file;
        else
        {
          while (document->next != NULL)
            document = document->next;

          document->next = file;
          file->prev     = document;
        }
      }
      else
        htmlDeleteTree(file);
    }
    else
      fl_alert("Unable to open \"%s\" for reading!",
               gui->inputFiles->text(i));
  }

 /*
  * We *must* have a document to process...
  */

  if (document == NULL)
  {
    gui->controls->activate();
    gui->progress(0);
    fl_alert("No HTML files to format, cannot generate!");
    return;
  }

 /*
  * Find the first one in the list...
  */

  while (document->prev != NULL)
    document = document->prev;

 /*
  * Build a table of contents for the documents...
  */

  if (TocLevels > 0)
    toc = toc_build(document);
  else
    toc = NULL;

 /*
  * Figure out the printable area of the output page...
  */

  PagePrintWidth  = PageWidth - PageLeft - PageRight;
  PagePrintLength = PageLength - PageTop - PageBottom;

 /*
  * Generate the output file(s).
  */

  if (gui->typeHTML->value())
    html_export(document, toc);
  else if (gui->typePS1->value())
    ps_export_level1(document, toc);
  else if (gui->typePS2->value())
    ps_export_level2(document, toc);
  else
    pdf_export(document, toc);

  htmlDeleteTree(document);
  htmlDeleteTree(toc);

  gui->controls->activate();
  gui->progress(0);

  fl_message("Document Generated!");
}


//
// 'closeBookCB()' - Close the current book.
//

void
closeBookCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  REF(w);

  delete gui;

  prefs_save();

  exit(0);
}


#endif // HAVE_LIBFLTK

//
// End of "$Id: gui.cxx,v 1.1 1999/11/08 18:35:16 mike Exp $".
//
