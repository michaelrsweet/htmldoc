//
// "$Id: gui.cxx,v 1.36.2.73 2004/05/27 20:13:13 mike Exp $"
//
//   GUI routines for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-2004 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   GUI()                 - Build the HTMLDOC GUI and load the indicated book
//                           as necessary.
//   ~GUI()                - Destroy the HTMLDOC GUI.
//   GUI::show()           - Display the window.
//   GUI::progress()       - Update the progress bar on the GUI.
//   GUI::title()          - Set the title bar of the window.
//   GUI::loadSettings()   - Load the current settings into the HTMLDOC globals.
//   GUI::newBook()        - Clear out the current GUI settings for a new book.
//   GUI::loadBook()       - Load a book file from disk.
//   GUI::parseOptions()   - Parse options in a book file...
//   GUI::saveBook()       - Save a book to disk.
//   GUI::checkSave()      - Check to see if a save is needed.
//   GUI::changeCB()       - Mark the current book as changed.
//   GUI::docTypeCB()      - Handle input on the document type buttons.
//   GUI::inputFilesCB()   - Handle selections in the input files browser.
//   GUI::addFileCB()      - Add a file to the input files list.
//   GUI::addURLCB()       - Add a URL to the input files list.
//   GUI::editFilesCB()    - Edit one or more files in the input files list.
//   GUI::deleteFileCB()   - Delete one or more files from the input files list.
//   GUI::moveUpFileCB()   - Move one or more files up in the input files list.
//   GUI::moveDownFileCB() - Move one or more files down in the input files list.
//   GUI::logoImageCB()    - Change the logo image file.
//   GUI::titleImageCB()   - Change the title image file.
//   GUI::outputTypeCB()   - Set the output file type.
//   GUI::outputPathCB()   - Set the output path.
//   GUI::outputFormatCB() - Set the output format.
//   GUI::jpegCB()         - Handle JPEG changes.
//   GUI::sizeCB()         - Change the page size based on the menu selection.
//   GUI::tocCB()          - Handle Table-of-Contents changes.
//   GUI::pdfCB()          - Handle PDF version changes.
//   GUI::effectCB()       - Handle PDF effect changes.
//   GUI::psCB()           - Handle PS language level changes.
//   GUI::htmlEditorCB()   - Change the HTML editor.
//   GUI::saveOptionsCB()  - Save preferences...
//   GUI::bodyColorCB()    - Set the body color.
//   GUI::bodyImageCB()    - Set the body image.
//   GUI::textColorCB()    - Set the text color.
//   GUI::linkColorCB()    - Set the link color.
//   GUI::helpCB()         - Show on-line help...
//   GUI::newBookCB()      - Create a new book.
//   GUI::openBookCB()     - Open an existing book.
//   GUI::saveBookCB()     - Save the current book to disk.
//   GUI::saveAsBookCB()   - Save the current book to disk to a new file.
//   GUI::generateBookCB() - Generate the current book.
//   GUI::closeBookCB()    - Close the current book.
//   GUI::errorCB()        - Close the error window.
//

#include "htmldoc.h"

#ifdef HAVE_LIBFLTK

//
// Include necessary headers.
//

#  include <ctype.h>
#  include <time.h>
#  include <math.h>
#  include <FL/fl_ask.H>
#  include <FL/Fl_Color_Chooser.H>
#  include <FL/fl_draw.H>
#  include <FL/x.H>

#  ifdef WIN32
#    include <direct.h>
#    include <io.h>
#    include "icons.h"
#  else
#    include <unistd.h>
#    ifdef HAVE_LIBXPM
#      include <X11/xpm.h>
#      include "htmldoc.xpm"
#    elif !defined(__APPLE__)
#      include "htmldoc.xbm"
#    endif // HAVE_LIBXPM
#  endif // WIN32


//
// Class globals...
//

const char	*GUI::help_dir = DOCUMENTATION;


//
// 'GUI()' - Build the HTMLDOC GUI and load the indicated book as necessary.
//

GUI::GUI(const char *filename)		// Book file to load initially
{
  Fl_Group		*group;		// Group
  Fl_Box		*label;		// Label box
  static Fl_Menu	sizeMenu[] =	// Menu items for page size button */
			{
			  {"A3",        0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"A4",        0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Legal",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Letter",    0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Tabloid",   0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Universal", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	tocMenu[] =	// Menu items for TOC chooser
			{
			  {"None",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1 level",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"2 levels", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"3 levels", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"4 Levels", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	formatMenu[] =	// Menu items for header/footer choosers
			{
			  {"Blank",         0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Title",         0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Chapter Title", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Heading",       0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Logo",          0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1,2,3,...",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"i,ii,iii,...",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"I,II,III,...",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"a,b,c,...",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"A,B,C,...",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Chapter Page",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1/N,2/N,...",   0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1/C,2/C,...",   0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Date",          0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Time",          0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Date + Time",   0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	nupMenu[] =	// Menu items for number-up chooser
			{
			  {"1",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"2",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"4",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"6",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"9",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"16", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	typefaceMenu[] = // Menu items for typeface choosers
			{
			  {"Courier",   0, 0, 0, 0, 0, FL_COURIER, 14, 0},
			  {"Times",     0, 0, 0, 0, 0, FL_TIMES, 14, 0},
			  {"Helvetica", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	fontMenu[] =	// Menu items for font choosers
			{
			  {"Courier",               0, 0, 0, 0, 0, FL_COURIER, 14, 0},
			  {"Courier-Bold",          0, 0, 0, 0, 0, FL_COURIER_BOLD, 14, 0},
			  {"Courier-Oblique",       0, 0, 0, 0, 0, FL_COURIER_ITALIC, 14, 0},
			  {"Courier-BoldOblique",   0, 0, 0, 0, 0, FL_COURIER_BOLD_ITALIC, 14, 0},
			  {"Times-Roman",           0, 0, 0, 0, 0, FL_TIMES, 14, 0},
			  {"Times-Bold",            0, 0, 0, 0, 0, FL_TIMES_BOLD, 14, 0},
			  {"Times-Italic",          0, 0, 0, 0, 0, FL_TIMES_ITALIC, 14, 0},
			  {"Times-BoldItalic",      0, 0, 0, 0, 0, FL_TIMES_BOLD_ITALIC, 14, 0},
			  {"Helvetica",             0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Helvetica-Bold",        0, 0, 0, 0, 0, FL_HELVETICA_BOLD, 14, 0},
			  {"Helvetica-Oblique",     0, 0, 0, 0, 0, FL_HELVETICA_ITALIC, 14, 0},
			  {"Helvetica-BoldOblique", 0, 0, 0, 0, 0, FL_HELVETICA_BOLD_ITALIC, 14, 0},
			  {0}
			};
  static Fl_Menu	charsetMenu[] =	// Menu items for charset chooser
			{
			  {"cp-874",      0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1250",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1251",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1252",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1253",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1254",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1255",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1256",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1257",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"cp-1258",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-1",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-2",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-3",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-4",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-5",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-6",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-7",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-8",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-9",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-14", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"iso-8859-15", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"koi8-r",      0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	modeMenu[] =	// Menu items for mode chooser
			{
			  {"Document",    0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Outline",     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Full-Screen", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	layoutMenu[] =	// Menu items for layout chooser
			{
			  {"Single",           0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"One Column",       0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Two Column Left",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Two Column Right", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	firstMenu[] =	// Menu items for first chooser
			{
			  {"Page 1",    0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"TOC",       0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Chapter 1", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	effectMenu[] =	// Menu items for effect chooser
			{
			  {"None",                     0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Box Inward",               0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Box Outward",              0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Dissolve",                 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Down",             0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Down+Right",       0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Right",            0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Blinds",        0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Sweep Inward",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Sweep Outward", 0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Blinds",          0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Sweep Inward",    0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Sweep Outward ",  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Down",                0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Left",                0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Right",               0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Up",                  0, 0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};


  // Enable/disable tooltips...
  Fl_Tooltip::enable(Tooltips);

  //
  // Create a dialog window...
  //

  window = new Fl_Window(505, 415, "HTMLDOC " SVERSION);
  window->callback((Fl_Callback *)closeBookCB, this);

  controls = new Fl_Group(0, 0, 505, 385);
  tabs     = new Fl_Tabs(10, 10, 485, 285);

  tabs->selection_color(FL_WHITE);

  //
  // Input tab...
  //

  inputTab = new Fl_Group(10, 35, 485, 260, "Input");

  group = new Fl_Group(140, 45, 250, 20, "Document Type: ");
  group->align(FL_ALIGN_LEFT);
    typeBook = new Fl_Round_Button(140, 45, 60, 20, "Book");
    typeBook->type(FL_RADIO_BUTTON);
    typeBook->setonly();
    typeBook->callback((Fl_Callback *)docTypeCB, this);
    typeBook->tooltip("Convert chapters into a book.");

    typeContinuous = new Fl_Round_Button(200, 45, 100, 20, "Continuous");
    typeContinuous->type(FL_RADIO_BUTTON);
    typeContinuous->callback((Fl_Callback *)docTypeCB, this);
    typeContinuous->tooltip("Convert web pages without page breaks.");

    typeWebPage = new Fl_Round_Button(300, 45, 90, 20, "Web Page");
    typeWebPage->type(FL_RADIO_BUTTON);
    typeWebPage->callback((Fl_Callback *)docTypeCB, this);
    typeWebPage->tooltip("Convert web pages with page breaks.");
  group->end();

  group = new Fl_Group(140, 70, 250, 20, "Input Files: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  inputFiles = new Fl_File_Browser(140, 70, 250, 150);
  inputFiles->iconsize(20);
  inputFiles->type(FL_MULTI_BROWSER);
  inputFiles->callback((Fl_Callback *)inputFilesCB, this);
  inputFiles->when(FL_WHEN_RELEASE | FL_WHEN_NOT_CHANGED);
  inputFiles->tooltip("This is the list of HTML files and URLs that will be converted.");

  addFile = new Fl_Button(390, 70, 95, 25, "Add Files...");
  addFile->callback((Fl_Callback *)addFileCB, this);
  addFile->tooltip("Add HTML files to the list.");

  addURL = new Fl_Button(390, 95, 95, 25, "Add URL...");
  addURL->callback((Fl_Callback *)addURLCB, this);
  addURL->tooltip("Add a URL to the list.");

  editFile = new Fl_Button(390, 120, 95, 25, "Edit Files...");
  editFile->deactivate();
  editFile->callback((Fl_Callback *)editFilesCB, this);
  editFile->tooltip("Edit HTML files in the list.");

  deleteFile = new Fl_Button(390, 145, 95, 25, "Delete Files");
  deleteFile->deactivate();
  deleteFile->callback((Fl_Callback *)deleteFilesCB, this);
  deleteFile->tooltip("Remove HTML files and URLs from the list.");

  moveUpFile = new Fl_Button(390, 170, 95, 25, "Move Up");
  moveUpFile->deactivate();
  moveUpFile->callback((Fl_Callback *)moveUpFilesCB, this);
  moveUpFile->tooltip("Move HTML files and URLs up in the list.");

  moveDownFile = new Fl_Button(390, 195, 95, 25, "Move Down");
  moveDownFile->deactivate();
  moveDownFile->callback((Fl_Callback *)moveDownFilesCB, this);
  moveDownFile->tooltip("Move HTML files and URLs down in the list.");

  logoImage = new Fl_Input(140, 230, 250, 25, "Logo Image: ");
  logoImage->when(FL_WHEN_CHANGED);
  logoImage->callback((Fl_Callback *)logoImageCB, this);
  logoImage->tooltip("The logo image for the navigation bar and header or footer.");

  logoBrowse = new Fl_Button(390, 230, 95, 25, "Browse...");
  logoBrowse->callback((Fl_Callback *)logoImageCB, this);
  logoBrowse->tooltip("Choose a logo image file.");

  titleImage = new Fl_Input(140, 260, 250, 25, "Title File/Image: ");
  titleImage->when(FL_WHEN_CHANGED);
  titleImage->callback((Fl_Callback *)titleImageCB, this);
  titleImage->tooltip("The title image or HTML file for the title page.");

  titleBrowse = new Fl_Button(390, 260, 95, 25, "Browse...");
  titleBrowse->callback((Fl_Callback *)titleImageCB, this);
  titleBrowse->tooltip("Choose a title file.");

  inputTab->end();
  inputTab->resizable(inputFiles);

  //
  // Output tab...
  //

  outputTab = new Fl_Group(10, 35, 485, 260, "Output");
  outputTab->hide();

  group = new Fl_Group(140, 45, 265, 20, "Output To: ");
  group->align(FL_ALIGN_LEFT);
    outputFile = new Fl_Round_Button(140, 45, 50, 20, "File");
    outputFile->type(FL_RADIO_BUTTON);
    outputFile->setonly();
    outputFile->callback((Fl_Callback *)outputTypeCB, this);
    outputFile->tooltip("Generate a single output file.");

    outputDirectory = new Fl_Round_Button(190, 45, 105, 20, "Directory");
    outputDirectory->type(FL_RADIO_BUTTON);
    outputDirectory->callback((Fl_Callback *)outputTypeCB, this);
    outputDirectory->tooltip("Generate multiple output files in a directory.");
  group->end();

  outputPath = new Fl_Input(140, 70, 250, 25, "Output Path: ");
  outputPath->when(FL_WHEN_CHANGED);
  outputPath->callback((Fl_Callback *)outputPathCB, this);
  outputPath->tooltip("The name of the output file or directory.");

  outputBrowse = new Fl_Button(390, 70, 95, 25, "Browse...");
  outputBrowse->callback((Fl_Callback *)outputPathCB, this);
  outputBrowse->tooltip("Choose an output file.");

  group = new Fl_Group(140, 100, 255, 20, "Output Format: ");
  group->align(FL_ALIGN_LEFT);
    typeHTML = new Fl_Round_Button(140, 100, 65, 20, "HTML");
    typeHTML->type(FL_RADIO_BUTTON);
    typeHTML->setonly();
    typeHTML->callback((Fl_Callback *)outputFormatCB, this);
    typeHTML->tooltip("Generate HTML file(s).");

    typeHTMLSep = new Fl_Round_Button(205, 100, 135, 20, "Separated HTML");
    typeHTMLSep->type(FL_RADIO_BUTTON);
    typeHTMLSep->callback((Fl_Callback *)outputFormatCB, this);
    typeHTMLSep->tooltip("Generate separate HTML files for each TOC heading.");

    typePS = new Fl_Round_Button(340, 100, 45, 20, "PS");
    typePS->type(FL_RADIO_BUTTON);
    typePS->callback((Fl_Callback *)outputFormatCB, this);
    typePS->tooltip("Generate Adobe PostScript(r) file(s).");

    typePDF = new Fl_Round_Button(385, 100, 55, 20, "PDF");
    typePDF->type(FL_RADIO_BUTTON);
    typePDF->callback((Fl_Callback *)outputFormatCB, this);
    typePDF->tooltip("Generate an Adobe Acrobat file.");
  group->end();

  group = new Fl_Group(140, 125, 265, 20, "Output Options: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  grayscale = new Fl_Check_Button(140, 125, 90, 20, "Grayscale");
  grayscale->callback((Fl_Callback *)changeCB, this);
  grayscale->tooltip("Check to produce grayscale output.");

  titlePage = new Fl_Check_Button(230, 125, 90, 20, "Title Page");
  titlePage->callback((Fl_Callback *)changeCB, this);
  titlePage->tooltip("Check to generate a title page.");

  jpegCompress = new Fl_Check_Button(320, 125, 140, 20, "JPEG Big Images");
  jpegCompress->callback((Fl_Callback *)jpegCB, this);
  jpegCompress->tooltip("Check to reduce the size of large images using the JPEG algorithm.");

  compGroup = new Fl_Group(140, 150, 345, 40, "Compression: \n ");
  compGroup->align(FL_ALIGN_LEFT);

    compression = new Fl_Slider(140, 150, 345, 20);
    compression->type(FL_HOR_NICE_SLIDER);
    compression->minimum(0.0);
    compression->maximum(9.0);
    compression->value(1.0);
    compression->step(1.0);
    compression->callback((Fl_Callback *)changeCB, this);
    compression->tooltip("Reduce the size of output files.");

    label = new Fl_Box(140, 170, 30, 10, "None");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(170, 170, 30, 10, "Fast");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(455, 170, 30, 10, "Best");
    label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    label->labelsize(10);

  compGroup->end();

  jpegGroup = new Fl_Group(140, 185, 345, 40, "JPEG Quality: \n ");
  jpegGroup->align(FL_ALIGN_LEFT);

    jpegQuality = new Fl_Value_Slider(140, 185, 345, 20);
    jpegQuality->type(FL_HOR_NICE_SLIDER);
    jpegQuality->minimum(50.0);
    jpegQuality->maximum(100.0);
    jpegQuality->value(90.0);
    jpegQuality->step(1.0);
    jpegQuality->callback((Fl_Callback *)changeCB, this);
    jpegQuality->tooltip("Set the quality of images using JPEG compression.\n"
                          "(lower quality produces smaller output)");

    label = new Fl_Box(175, 205, 40, 10, "Good");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(445, 205, 40, 10, "Best");
    label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    label->labelsize(10);

  jpegGroup->end();

  outputTab->end();

  //
  // Page tab...
  //

  pageTab = new Fl_Group(10, 35, 485, 260, "Page");
  pageTab->hide();

  pageSize = new Fl_Input(140, 45, 100, 25, "Page Size: ");
  pageSize->when(FL_WHEN_CHANGED);
  pageSize->callback((Fl_Callback *)changeCB, this);
  pageSize->tooltip("Enter the page size.");

  pageSizeMenu = new Fl_Menu_Button(240, 45, 25, 25, "");
  pageSizeMenu->menu(sizeMenu);
  pageSizeMenu->callback((Fl_Callback *)sizeCB, this);
  pageSizeMenu->tooltip("Click to choose a standard size.");

  pageDuplex = new Fl_Check_Button(270, 48, 70, 20, "2-Sided");
  pageDuplex->callback((Fl_Callback *)changeCB, this);
  pageDuplex->tooltip("Produce output suitable for double-sided printing.");

  landscape = new Fl_Check_Button(345, 48, 90, 20, "Landscape");
  landscape->callback((Fl_Callback *)changeCB, this);
  landscape->tooltip("Check to rotate the output to landscape orientation.");

  pageTop = new Fl_Input(225, 75, 60, 25, "Top");
  pageTop->when(FL_WHEN_CHANGED);
  pageTop->callback((Fl_Callback *)changeCB, this);
  pageTop->tooltip("Enter the top margin.");

  pageLeft = new Fl_Input(190, 105, 60, 25, "Left");
  pageLeft->when(FL_WHEN_CHANGED);
  pageLeft->callback((Fl_Callback *)changeCB, this);
  pageLeft->tooltip("Enter the left margin.");

  pageRight = new Fl_Input(255, 105, 60, 25, "Right");
  pageRight->when(FL_WHEN_CHANGED);
  pageRight->align(FL_ALIGN_RIGHT);
  pageRight->callback((Fl_Callback *)changeCB, this);
  pageRight->tooltip("Enter the right margin.");

  pageBottom = new Fl_Input(225, 135, 60, 25, "Bottom");
  pageBottom->when(FL_WHEN_CHANGED);
  pageBottom->callback((Fl_Callback *)changeCB, this);
  pageBottom->tooltip("Enter the bottom margin.");

  pageHeaderLeft = new Fl_Choice(140, 165, 110, 25, "Header: ");
  pageHeaderLeft->menu(formatMenu);
  pageHeaderLeft->callback((Fl_Callback *)changeCB, this);
  pageHeaderLeft->tooltip("Choose the left header.");

  pageHeaderCenter = new Fl_Choice(255, 165, 110, 25);
  pageHeaderCenter->menu(formatMenu);
  pageHeaderCenter->callback((Fl_Callback *)changeCB, this);
  pageHeaderCenter->tooltip("Choose the center header.");

  pageHeaderRight = new Fl_Choice(370, 165, 110, 25);
  pageHeaderRight->menu(formatMenu);
  pageHeaderRight->callback((Fl_Callback *)changeCB, this);
  pageHeaderRight->tooltip("Choose the right header.");

  pageFooterLeft = new Fl_Choice(140, 195, 110, 25, "Footer: ");
  pageFooterLeft->menu(formatMenu);
  pageFooterLeft->callback((Fl_Callback *)changeCB, this);
  pageFooterLeft->tooltip("Choose the left footer.");

  pageFooterCenter = new Fl_Choice(255, 195, 110, 25);
  pageFooterCenter->menu(formatMenu);
  pageFooterCenter->callback((Fl_Callback *)changeCB, this);
  pageFooterCenter->tooltip("Choose the center header.");

  pageFooterRight = new Fl_Choice(370, 195, 110, 25);
  pageFooterRight->menu(formatMenu);
  pageFooterRight->callback((Fl_Callback *)changeCB, this);
  pageFooterRight->tooltip("Choose the right header.");

  numberUp = new Fl_Choice(140, 225, 50, 25, "Number Up: ");
  numberUp->menu(nupMenu);
  numberUp->callback((Fl_Callback *)changeCB, this);
  numberUp->tooltip("Set the number of pages on each sheet.");

  pageTab->end();

  //
  // TOC tab...
  //

  tocTab = new Fl_Group(10, 35, 485, 260, "TOC");
  tocTab->hide();

  tocLevels = new Fl_Choice(140, 45, 100, 25, "Table of Contents: ");
  tocLevels->menu(tocMenu);
  tocLevels->callback((Fl_Callback *)tocCB, this);
  tocLevels->tooltip("Choose the number of table of contents levels.");

  numberedToc = new Fl_Check_Button(245, 47, 160, 20, "Numbered Headings");
  numberedToc->callback((Fl_Callback *)changeCB, this);
  numberedToc->tooltip("Check to number all of the headings in the document.");

  tocHeader = new Fl_Group(140, 75, 345, 25, "Header: ");
  tocHeader->align(FL_ALIGN_LEFT);

    tocHeaderLeft = new Fl_Choice(140, 75, 110, 25);
    tocHeaderLeft->menu(formatMenu);
    tocHeaderLeft->callback((Fl_Callback *)changeCB, this);
    tocHeaderLeft->tooltip("Choose the left header.");

    tocHeaderCenter = new Fl_Choice(255, 75, 110, 25);
    tocHeaderCenter->menu(formatMenu);
    tocHeaderCenter->callback((Fl_Callback *)changeCB, this);
    tocHeaderCenter->tooltip("Choose the center header.");

    tocHeaderRight = new Fl_Choice(370, 75, 110, 25);
    tocHeaderRight->menu(formatMenu);
    tocHeaderRight->callback((Fl_Callback *)changeCB, this);
    tocHeaderRight->tooltip("Choose the right header.");

  tocHeader->end();

  tocFooter = new Fl_Group(140, 105, 345, 25, "Footer: ");
  tocFooter->align(FL_ALIGN_LEFT);

    tocFooterLeft = new Fl_Choice(140, 105, 110, 25, "Footer: ");
    tocFooterLeft->menu(formatMenu);
    tocFooterLeft->callback((Fl_Callback *)changeCB, this);
    tocFooterLeft->tooltip("Choose the left footer.");

    tocFooterCenter = new Fl_Choice(255, 105, 110, 25);
    tocFooterCenter->menu(formatMenu);
    tocFooterCenter->callback((Fl_Callback *)changeCB, this);
    tocFooterCenter->tooltip("Choose the center footer.");

    tocFooterRight = new Fl_Choice(370, 105, 110, 25);
    tocFooterRight->menu(formatMenu);
    tocFooterRight->callback((Fl_Callback *)changeCB, this);
    tocFooterRight->tooltip("Choose the right footer.");

  tocFooter->end();

  tocTitle = new Fl_Input(140, 135, 345, 25, "Title: ");
  tocTitle->when(FL_WHEN_CHANGED);
  tocTitle->callback((Fl_Callback *)changeCB, this);
  tocTitle->tooltip("Enter the title of the table of contents.");

  tocTab->end();

  //
  // Colors tab...
  //

  colorsTab = new Fl_Group(10, 35, 485, 260, "Colors");
  colorsTab->hide();

  bodyColor = new Fl_Input(140, 45, 100, 25, "Body Color: ");
  bodyColor->when(FL_WHEN_CHANGED);
  bodyColor->callback((Fl_Callback *)bodyColorCB, this);
  bodyColor->tooltip("Enter the HTML color for the body (background).");

  bodyLookup = new Fl_Button(240, 45, 80, 25, "Lookup...");
  bodyLookup->callback((Fl_Callback *)bodyColorCB, this);
  bodyLookup->tooltip("Click to choose the HTML color for the body (background).");

  bodyImage = new Fl_Input(140, 75, 250, 25, "Body Image: ");
  bodyImage->when(FL_WHEN_CHANGED);
  bodyImage->callback((Fl_Callback *)bodyImageCB, this);
  bodyImage->tooltip("Enter the image file for the body (background).");

  bodyBrowse = new Fl_Button(390, 75, 95, 25, "Browse...");
  bodyBrowse->callback((Fl_Callback *)bodyImageCB, this);
  bodyBrowse->tooltip("Click to choose the image file for the body (background).");

  textColor = new Fl_Input(140, 105, 100, 25, "Text Color: ");
  textColor->when(FL_WHEN_CHANGED);
  textColor->callback((Fl_Callback *)textColorCB, this);
  textColor->tooltip("Enter the HTML color for the text.");

  textLookup = new Fl_Button(240, 105, 80, 25, "Lookup...");
  textLookup->callback((Fl_Callback *)textColorCB, this);
  textLookup->tooltip("Click to choose the HTML color for the text.");

  linkColor = new Fl_Input(140, 135, 100, 25, "Link Color: ");
  linkColor->when(FL_WHEN_CHANGED);
  linkColor->callback((Fl_Callback *)linkColorCB, this);
  linkColor->tooltip("Enter the HTML color for links.");

  linkLookup = new Fl_Button(240, 135, 80, 25, "Lookup...");
  linkLookup->callback((Fl_Callback *)linkColorCB, this);
  linkLookup->tooltip("Click to choose the HTML color for links.");

  linkStyle = new Fl_Choice(140, 165, 100, 25, "Link Style: ");
  linkStyle->add("Plain");
  linkStyle->add("Underline");
  linkStyle->callback((Fl_Callback *)changeCB, this);
  linkStyle->tooltip("Choose the appearance of links.");

  colorsTab->end();

  //
  // Fonts tab...
  //

  fontsTab = new Fl_Group(10, 35, 485, 260, "Fonts");
  fontsTab->hide();

  fontBaseSize = new Fl_Counter(200, 45, 150, 25, "Base Font Size: ");
  fontBaseSize->callback((Fl_Callback *)changeCB, this);
  fontBaseSize->minimum(4.0);
  fontBaseSize->maximum(24.0);
  fontBaseSize->step(0.1);
  fontBaseSize->value(11.0);
  fontBaseSize->align(FL_ALIGN_LEFT);
  fontBaseSize->tooltip("Set the default size of text.");

  fontSpacing = new Fl_Counter(200, 75, 150, 25, "Line Spacing: ");
  fontSpacing->callback((Fl_Callback *)changeCB, this);
  fontSpacing->minimum(1.0);
  fontSpacing->maximum(3.0);
  fontSpacing->step(0.1);
  fontSpacing->value(1.2);
  fontSpacing->align(FL_ALIGN_LEFT);
  fontSpacing->tooltip("Set the spacing between lines of text.");

  bodyFont = new Fl_Choice(200, 105, 100, 25, "Body Typeface: ");
  bodyFont->menu(typefaceMenu);
  bodyFont->callback((Fl_Callback *)changeCB, this);
  bodyFont->tooltip("Choose the default typeface (font) of text.");

  headingFont = new Fl_Choice(200, 135, 100, 25, "Heading Typeface: ");
  headingFont->menu(typefaceMenu);
  headingFont->callback((Fl_Callback *)changeCB, this);
  headingFont->tooltip("Choose the default typeface (font) of headings.");

  headFootSize = new Fl_Counter(200, 165, 150, 25, "Header/Footer Size: ");
  headFootSize->callback((Fl_Callback *)changeCB, this);
  headFootSize->minimum(4.0);
  headFootSize->maximum(24.0);
  headFootSize->step(0.1);
  headFootSize->value(11.0);
  headFootSize->align(FL_ALIGN_LEFT);
  headFootSize->tooltip("Set the size of header and footer text.");

  headFootFont = new Fl_Choice(200, 195, 220, 25, "Header/Footer Font: ");
  headFootFont->menu(fontMenu);
  headFootFont->callback((Fl_Callback *)changeCB, this);
  headFootFont->tooltip("Choose the font for header and footer text.");

  charset = new Fl_Choice(200, 225, 110, 25, "Character Set: ");
  charset->menu(charsetMenu);
  charset->callback((Fl_Callback *)changeCB, this);
  charset->tooltip("Choose the encoding of text.");

  group = new Fl_Group(200, 255, 285, 25, "Options: ");
  group->align(FL_ALIGN_LEFT);

    embedFonts = new Fl_Check_Button(200, 255, 110, 25, "Embed Fonts");
    embedFonts->callback((Fl_Callback *)changeCB, this);
    embedFonts->tooltip("Check to embed fonts in the output file.");

  group->end();

  fontsTab->end();

  //
  // PostScript tab...
  //

  psTab = new Fl_Group(10, 35, 485, 260, "PS");
  psTab->hide();

  psLevel = new Fl_Group(140, 45, 310, 20, "PostScript: ");
  psLevel->align(FL_ALIGN_LEFT);

    ps1 = new Fl_Round_Button(140, 45, 70, 20, "Level 1");
    ps1->type(FL_RADIO_BUTTON);
    ps1->callback((Fl_Callback *)psCB, this);
    ps1->tooltip("Produce PostScript Level 1 output.");

    ps2 = new Fl_Round_Button(210, 45, 70, 20, "Level 2");
    ps2->type(FL_RADIO_BUTTON);
    ps2->callback((Fl_Callback *)psCB, this);
    ps2->tooltip("Produce PostScript Level 2 output.\n"
                  "(most common)");

    ps3 = new Fl_Round_Button(280, 45, 70, 20, "Level 3");
    ps3->type(FL_RADIO_BUTTON);
    ps3->callback((Fl_Callback *)psCB, this);
    ps3->tooltip("Produce PostScript Level 3 output.");

  psLevel->end();

  psCommands = new Fl_Check_Button(140, 70, 310, 20, "Send Printer Commands");
  psCommands->callback((Fl_Callback *)changeCB, this);
  psCommands->tooltip("Include PostScript commands to set the media size, etc.");

  xrxComments = new Fl_Check_Button(140, 95, 310, 20, "Include Xerox Job Comments");
  xrxComments->callback((Fl_Callback *)changeCB, this);
  xrxComments->tooltip("Include Xerox job comments to set the media size, etc.");

  psTab->end();

  //
  // PDF tab...
  //

  pdfTab = new Fl_Group(10, 35, 485, 260, "PDF");
  pdfTab->hide();

  pdfVersion = new Fl_Group(140, 45, 310, 40, "PDF Version: \n ");
  pdfVersion->align(FL_ALIGN_LEFT);

    pdf11 = new Fl_Round_Button(140, 45, 125, 20, "1.1 (Acrobat 2.x)");
    pdf11->type(FL_RADIO_BUTTON);
    pdf11->callback((Fl_Callback *)pdfCB, this);
    pdf11->tooltip("Produce PDF files for Acrobat 2.x.");

    pdf12 = new Fl_Round_Button(270, 45, 125, 20, "1.2 (Acrobat 3.0)");
    pdf12->type(FL_RADIO_BUTTON);
    pdf12->callback((Fl_Callback *)pdfCB, this);
    pdf12->tooltip("Produce PDF files for Acrobat 3.0.");

    pdf13 = new Fl_Round_Button(140, 65, 125, 20, "1.3 (Acrobat 4.0)");
    pdf13->type(FL_RADIO_BUTTON);
    pdf13->callback((Fl_Callback *)pdfCB, this);
    pdf13->tooltip("Produce PDF files for Acrobat 4.0.");

    pdf14 = new Fl_Round_Button(270, 65, 125, 20, "1.4 (Acrobat 5.0)");
    pdf14->type(FL_RADIO_BUTTON);
    pdf14->callback((Fl_Callback *)pdfCB, this);
    pdf14->tooltip("Produce PDF files for Acrobat 5.0.");

  pdfVersion->end();

  pageMode = new Fl_Choice(140, 90, 120, 25, "Page Mode: ");
  pageMode->menu(modeMenu);
  pageMode->callback((Fl_Callback *)changeCB, this);
  pageMode->tooltip("Choose the initial viewing mode for the file.");

  pageLayout = new Fl_Choice(140, 120, 150, 25, "Page Layout: ");
  pageLayout->menu(layoutMenu);
  pageLayout->callback((Fl_Callback *)changeCB, this);
  pageLayout->tooltip("Choose the initial page layout for the file.");

  firstPage = new Fl_Choice(140, 150, 100, 25, "First Page: ");
  firstPage->menu(firstMenu);
  firstPage->callback((Fl_Callback *)changeCB, this);
  firstPage->tooltip("Choose the initial page that will be shown.");

  pageEffect = new Fl_Choice(140, 180, 210, 25, "Page Effect: ");
  pageEffect->menu(effectMenu);
  pageEffect->callback((Fl_Callback *)effectCB, this);
  pageEffect->tooltip("Choose the page transition effect.");

  pageDuration = new Fl_Value_Slider(140, 210, 345, 20, "Page Duration: ");
  pageDuration->align(FL_ALIGN_LEFT);
  pageDuration->type(FL_HOR_NICE_SLIDER);
  pageDuration->minimum(1.0);
  pageDuration->maximum(60.0);
  pageDuration->value(10.0);
  pageDuration->step(1.0);
  pageDuration->callback((Fl_Callback *)changeCB, this);
  pageDuration->tooltip("Set the amount of time each page is visible.");

  effectDuration = new Fl_Value_Slider(140, 235, 345, 20, "Effect Duration: ");
  effectDuration->align(FL_ALIGN_LEFT);
  effectDuration->type(FL_HOR_NICE_SLIDER);
  effectDuration->minimum(0.5);
  effectDuration->maximum(5.0);
  effectDuration->value(1.0);
  effectDuration->step(0.1);
  effectDuration->callback((Fl_Callback *)changeCB, this);
  effectDuration->tooltip("Set the amount of time to use for the page transition effect.");

  group = new Fl_Group(140, 260, 350, 25, "Options: ");
  group->align(FL_ALIGN_LEFT);

    links = new Fl_Check_Button(140, 260, 110, 25, "Include Links");
    links->callback((Fl_Callback *)changeCB, this);
    links->tooltip("Check to include hyperlinks in the output file.");

  group->end();

  pdfTab->end();

  //
  // Security tab...
  //

  securityTab = new Fl_Group(10, 35, 485, 260, "Security");
  securityTab->hide();

  encryption = new Fl_Group(140, 45, 310, 20, "Encryption: ");
  encryption->align(FL_ALIGN_LEFT);

    encryptionNo = new Fl_Round_Button(140, 45, 40, 20, "No");
    encryptionNo->type(FL_RADIO_BUTTON);
    encryptionNo->set();
    encryptionNo->callback((Fl_Callback *)encryptionCB, this);
    encryptionNo->tooltip("Select to disable encryption (scrambling) of the output file.");

    encryptionYes = new Fl_Round_Button(180, 45, 45, 20, "Yes");
    encryptionYes->type(FL_RADIO_BUTTON);
    encryptionYes->callback((Fl_Callback *)encryptionCB, this);
    encryptionYes->tooltip("Select to enable encryption (scrambling) of the output file.\n"
                            "(128-bit encryption for Acrobat 5.0, 40-bit for older versions.)");

  encryption->end();

  permissions = new Fl_Group(140, 70, 310, 40, "Permissions: ");
  permissions->align(FL_ALIGN_LEFT);

    permPrint    = new Fl_Check_Button(140, 70, 80, 20, "Print");
    permPrint->tooltip("Check to allow the user to print the output file.");
    permModify   = new Fl_Check_Button(220, 70, 80, 20, "Modify");
    permModify->tooltip("Check to allow the user to modify the output file.");
    permCopy     = new Fl_Check_Button(140, 90, 80, 20, "Copy");
    permCopy->tooltip("Check to allow the user to copy text and images from the output file.");
    permAnnotate = new Fl_Check_Button(220, 90, 80, 20, "Annotate");
    permAnnotate->tooltip("Check to allow the user to annotate the output file.");

  permissions->end();

  ownerPassword = new Fl_Secret_Input(140, 115, 150, 25, "Owner Password: ");
  ownerPassword->maximum_size(32);
  ownerPassword->tooltip("Enter the password required to modify the file.\n"
                          "(leave blank for a random password)");

  userPassword = new Fl_Secret_Input(140, 145, 150, 25, "User Password: ");
  userPassword->maximum_size(32);
  userPassword->tooltip("Enter the password required to open the file.\n"
                         "(leave blank for no password)");

  securityTab->end();

  //
  // Options tab...
  //

  optionsTab = new Fl_Group(10, 35, 485, 260, "Options");
  optionsTab->hide();

  htmlEditor = new Fl_Input(140, 45, 250, 25, "HTML Editor: ");
  htmlEditor->value(HTMLEditor);
  htmlEditor->when(FL_WHEN_CHANGED);
  htmlEditor->callback((Fl_Callback *)htmlEditorCB, this);
  htmlEditor->tooltip("Enter the command used to edit HTML files.\n"
                       "(use \"%s\" to insert the filename)");

  htmlBrowse = new Fl_Button(390, 45, 95, 25, "Browse...");
  htmlBrowse->callback((Fl_Callback *)htmlEditorCB, this);
  htmlBrowse->tooltip("Click to choose the HTML editor.");

  browserWidth = new Fl_Value_Slider(140, 75, 345, 20, "Browser Width: ");
  browserWidth->align(FL_ALIGN_LEFT);
  browserWidth->type(FL_HOR_NICE_SLIDER);
  browserWidth->minimum(400.0);
  browserWidth->maximum(1200.0);
  browserWidth->value(_htmlBrowserWidth);
  browserWidth->step(5.0);
  browserWidth->callback((Fl_Callback *)changeCB, this);
  browserWidth->tooltip("Set the target browser width in pixels.\n"
                         "(this determines the page scaling of images)");

  path = new Fl_Input(140, 100, 345, 25, "Search Path: ");
  path->value(Path);
  path->maximum_size(sizeof(Path) - 1);
  path->when(FL_WHEN_CHANGED);
  path->callback((Fl_Callback *)changeCB, this);
  path->tooltip("Enter one or more directories or URLs to search for files.\n"
                 "(separate each directory or URL with the ';' character)");

  proxy = new Fl_Input(140, 130, 345, 25, "HTTP Proxy URL: ");
  proxy->value(Proxy);
  proxy->maximum_size(sizeof(Proxy) - 1);
  proxy->when(FL_WHEN_CHANGED);
  proxy->callback((Fl_Callback *)changeCB, this);
  proxy->tooltip("Enter a URL for your HTTP proxy server.\n"
                  "(http://server:port)");

  group = new Fl_Group(140, 160, 350, 75, "GUI Options: \n\n\n\n");
  group->align(FL_ALIGN_LEFT);

    tooltips = new Fl_Check_Button(140, 160, 80, 25, "Tooltips");
    tooltips->callback((Fl_Callback *)tooltipCB, this);
    tooltips->value(Tooltips);
    tooltips->tooltip("Check to show tooltips.");

    modern_skin = new Fl_Check_Button(140, 185, 120, 25, "Modern Look");
    modern_skin->callback((Fl_Callback *)skinCB, this);
    modern_skin->value(ModernSkin);
    modern_skin->tooltip("Check to show the more modern look-n-feel.");

    strict_html = new Fl_Check_Button(140, 210, 120, 25, "Strict HTML");
    strict_html->value(StrictHTML);
    strict_html->tooltip("Check to require strict HTML conformance.");

  group->end();

  saveOptions = new Fl_Button(295, 260, 190, 25, "Save Options and Defaults");
  saveOptions->callback((Fl_Callback *)saveOptionsCB, this);
  saveOptions->tooltip("Click to save the current options.");

  optionsTab->end();

  tabs->end();

  //
  // Button bar...
  //

  bookHelp = new Fl_Button(10, 355, 55, 25, "Help");
  bookHelp->shortcut(FL_F + 1);
  bookHelp->callback((Fl_Callback *)helpCB, this);

  bookNew = new Fl_Button(70, 355, 50, 25, "New");
  bookNew->shortcut(FL_CTRL | 'n');
  bookNew->callback((Fl_Callback *)newBookCB, this);

  bookOpen = new Fl_Button(125, 355, 65, 25, "Open...");
  bookOpen->shortcut(FL_CTRL | 'o');
  bookOpen->callback((Fl_Callback *)openBookCB, this);

  bookSave = new Fl_Button(195, 355, 55, 25, "Save");
  bookSave->shortcut(FL_CTRL | 's');
  bookSave->callback((Fl_Callback *)saveBookCB, this);

  bookSaveAs = new Fl_Button(255, 355, 85, 25, "Save As...");
  bookSaveAs->shortcut(FL_CTRL | FL_SHIFT | 's');
  bookSaveAs->callback((Fl_Callback *)saveAsBookCB, this);

  bookGenerate = new Fl_Button(345, 355, 85, 25, "Generate");
  bookGenerate->shortcut(FL_CTRL | 'g');
  bookGenerate->callback((Fl_Callback *)generateBookCB, this);

  bookClose = new Fl_Button(435, 355, 60, 25, "Close");
  bookClose->shortcut(FL_CTRL | 'q');
  bookClose->callback((Fl_Callback *)closeBookCB, this);

  controls->end();

  //
  // Copyright notice...
  //

  label = new Fl_Box(10, 300, 485, 50,
          "HTMLDOC " SVERSION " Copyright 1997-2004 by Easy Software Products "
	  "(http://www.easysw.com). This program is free software; you can "
	  "redistribute it and/or modify it under the terms of the GNU General "
	  "Public License as published by the Free Software Foundation. This "
	  "software is based in part on the work of the Independent JPEG Group."
	  );
  label->labelsize(10);
  label->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

  //
  // Progress bar...
  //

  progressBar = new Fl_Progress(10, 385, 485, 20, "HTMLDOC " SVERSION " Ready.");

  window->end();

  // Set the class name to "htmldoc".
  window->xclass("htmldoc");

#  ifdef WIN32
  // Load the HTMLDOC icon image...
  window->icon((char *)LoadImage(fl_display, MAKEINTRESOURCE(IDI_ICON),
                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
#  elif defined(__APPLE__)
  // NEED TO DO MacOS stuff here...
#  elif defined(HAVE_LIBXPM) // X11 w/Xpm library
  Pixmap	pixmap, mask;	// Icon pixmaps
  XpmAttributes	attrs;		// Attributes of icon

  // Open the X display and load the HTMLDOC icon image...
  fl_open_display();

  memset(&attrs, 0, sizeof(attrs));

  XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display),
                          (char **)htmldoc_xpm, &pixmap, &mask, &attrs);
  window->icon((char *)pixmap);
#  else // X11 w/o Xpm library
  // Open the X display and load the HTMLDOC icon image...
  fl_open_display();
  window->icon((char *)XCreateBitmapFromData(fl_display,
               DefaultRootWindow(fl_display), htmldoc_bits,
	       htmldoc_width, htmldoc_height));
#  endif // WIN32

  window->resizable(tabs);
  window->size_range(470, 390);
  show();

  // File chooser, icons, help dialog, error window...
  fc = new Fl_File_Chooser(".", "*", Fl_File_Chooser::SINGLE, "Title");
  fc->iconsize(20);

  if (!Fl_File_Icon::first())
    Fl_File_Icon::load_system_icons();

  icon = Fl_File_Icon::find("file.html", Fl_File_Icon::PLAIN);

  help = new Fl_Help_Dialog();

  error_window = new Fl_Window(400, 300, "Errors");
  error_list   = new Fl_Browser(10, 10, 380, 245);
  error_ok     = new Fl_Button(335, 265, 55, 25, "Close");

  error_ok->callback((Fl_Callback *)errorCB, this);
  error_window->end();
  error_window->resizable(error_list);

  // Use cheesy hardcoded "style" stuff until FLTK 2.0...
  skinCB(0, this);

#  ifdef __sgi
  fc->color((Fl_Color)196);
  inputFiles->color((Fl_Color)196);
#  endif // __sgi

  while (window->damage())
    Fl::check();

  //
  // Load the given book or create a new one...
  //

  book_changed     = 0;
  book_filename[0] = '\0';

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
  delete window;

  delete fc;
  delete help;
  delete error_window;

  while (Fl_File_Icon::first())
    delete Fl_File_Icon::first();
}


//
// 'GUI::show()' - Display the window.
//

void
GUI::show(void)
{
  static char	*htmldoc[1] = { (char *)"htmldoc" };	// argv[] array


  window->show(1, htmldoc);
}


//
// 'GUI::progress()' - Update the progress bar on the GUI.
//

void
GUI::progress(int        percent,	// I - Percent complete
              const char *text)		// I - Text prompt
{
  if (text != NULL)
    progressBar->label(text);
  else if (percent == 0)
    progressBar->label("HTMLDOC " SVERSION " Ready.");

 if ((percent - (int)progressBar->value()) >= 5 ||
     percent < (int)progressBar->value())
    progressBar->value(percent);

  Fl::check();
}


//
// 'GUI::title()' - Set the title bar of the window.
//

void
GUI::title(const char *filename,// Name of file being edited
           int        changed)	// Whether or not the file is modified
{
  book_changed = changed;

  if (filename == NULL || filename[0] == '\0')
  {
    book_filename[0] = '\0';
    strlcpy(title_string, "NewBook", sizeof(title_string));
  }
  else
  {
    strlcpy(book_filename, filename, sizeof(book_filename));
    strlcpy(title_string, file_basename(filename), sizeof(title_string));
  }

  if (changed)
    strlcat(title_string, "(modified) - ", sizeof(title_string));
  else
    strlcat(title_string, " - ", sizeof(title_string));

  strlcat(title_string, "HTMLDOC " SVERSION, sizeof(title_string));

  window->label(title_string);
  if (window->visible())
    Fl::check();
}


//
// 'GUI::loadSettings()' - Load the current settings into the HTMLDOC globals.
//

void
GUI::loadSettings()
{
  char		temp[4];		// Format string
  static const char *formats = ".tchl1iIaAC/:dTD";
					// Format characters


  set_page_size((char *)pageSize->value());

  PageLeft     = get_measurement((char *)pageLeft->value());
  PageRight    = get_measurement((char *)pageRight->value());
  PageTop      = get_measurement((char *)pageTop->value());
  PageBottom   = get_measurement((char *)pageBottom->value());

  PageDuplex   = pageDuplex->value();
  Landscape    = landscape->value();
  Compression  = (int)compression->value();
  OutputColor  = !grayscale->value();
  TocNumbers   = numberedToc->value();
  TocLevels    = tocLevels->value();
  TitlePage    = titlePage->value();

  if (jpegCompress->value())
    OutputJPEG = (int)jpegQuality->value();
  else
    OutputJPEG = 0;

  strlcpy(TocTitle, tocTitle->value(), sizeof(TocTitle));

  temp[0] = formats[tocHeaderLeft->value()];
  temp[1] = formats[tocHeaderCenter->value()];
  temp[2] = formats[tocHeaderRight->value()];
  temp[3] = '\0';

  get_format(temp, TocHeader);

  temp[0] = formats[tocFooterLeft->value()];
  temp[1] = formats[tocFooterCenter->value()];
  temp[2] = formats[tocFooterRight->value()];

  get_format(temp, TocFooter);

  temp[0] = formats[pageHeaderLeft->value()];
  temp[1] = formats[pageHeaderCenter->value()];
  temp[2] = formats[pageHeaderRight->value()];

  get_format(temp, Header);

  temp[0] = formats[pageFooterLeft->value()];
  temp[1] = formats[pageFooterCenter->value()];
  temp[2] = formats[pageFooterRight->value()];

  get_format(temp, Footer);

  NumberUp = atoi(numberUp->text(numberUp->value()));

  _htmlBodyFont    = (typeface_t)bodyFont->value();
  _htmlHeadingFont = (typeface_t)headingFont->value();
  htmlSetBaseSize(fontBaseSize->value(), fontSpacing->value());

  HeadFootType  = (typeface_t)(headFootFont->value() / 4);
  HeadFootStyle = (style_t)(headFootFont->value() & 3);
  HeadFootSize  = headFootSize->value();

  if (pdf11->value())
    PDFVersion = 11;
  else if (pdf12->value())
    PDFVersion = 12;
  else if (pdf13->value())
    PDFVersion = 13;
  else
    PDFVersion = 14;

  PDFPageMode       = pageMode->value();
  PDFPageLayout     = pageLayout->value();
  PDFFirstPage      = firstPage->value();
  PDFEffect         = pageEffect->value();
  PDFPageDuration   = pageDuration->value();
  PDFEffectDuration = effectDuration->value();
  Links             = links->value();
  EmbedFonts        = embedFonts->value();

  Encryption  = encryptionYes->value();
  Permissions = -64;
  if (permPrint->value())
    Permissions |= PDF_PERM_PRINT;
  if (permModify->value())
    Permissions |= PDF_PERM_MODIFY;
  if (permCopy->value())
    Permissions |= PDF_PERM_COPY;
  if (permAnnotate->value())
    Permissions |= PDF_PERM_ANNOTATE;

  strlcpy(UserPassword, userPassword->value(), sizeof(UserPassword));
  strlcpy(OwnerPassword, ownerPassword->value(), sizeof(OwnerPassword));

  if (ps1->value())
    PSLevel = 1;
  else if (ps2->value())
    PSLevel = 2;
  else
    PSLevel = 3;

  PSCommands  = psCommands->value();
  XRXComments = xrxComments->value();

  strlcpy(BodyColor, bodyColor->value(), sizeof(BodyColor));
  strlcpy(BodyImage, bodyImage->value(), sizeof(BodyImage));

  htmlSetTextColor((uchar *)textColor->value());
  htmlSetCharSet(charset->text(charset->value()));

  strlcpy(LinkColor, linkColor->value(), sizeof(LinkColor));
  LinkStyle = linkStyle->value();

  _htmlBrowserWidth = browserWidth->value();

  strlcpy(Path, path->value(), sizeof(Path));

  strlcpy(Proxy, proxy->value(), sizeof(Proxy));

  StrictHTML = strict_html->value();
}


//
// 'GUI::newBook()' - Clear out the current GUI settings for a new book.
//

int				// O - 1 on success, 0 on failure
GUI::newBook(void)
{
  int		i;		// Looping var
  char		size[255];	// Page size string
  char		formats[256];	// Format characters
  const char	*fmt;		// Old format string


  prefs_load();

  switch (OutputType)
  {
    case OUTPUT_BOOK :
	typeBook->setonly();
	docTypeCB(typeBook, this);
	break;

    case OUTPUT_CONTINUOUS :
	typeContinuous->setonly();
	docTypeCB(typeContinuous, this);
	break;

    case OUTPUT_WEBPAGES :
	typeWebPage->setonly();
	docTypeCB(typeWebPage, this);
	break;
  }

  inputFiles->clear();
  inputFilesCB(inputFiles, this);

  logoImage->value("");
  titleImage->value("");

  outputFile->setonly();
  outputTypeCB(outputFile, this);

  outputPath->value("");

  typeHTML->setonly();
  outputFormatCB(typeHTML, this);

  grayscale->value(!OutputColor);
  titlePage->value(TitlePage);

  bodyColor->value(BodyColor);
  bodyImage->value(BodyImage);
  textColor->value((char *)_htmlTextColor);
  linkColor->value(LinkColor);
  linkStyle->value(LinkStyle);

  if (PageWidth == 595 && PageLength == 842)
    pageSize->value("A4");
  else if (PageWidth == 595 && PageLength == 792)
    pageSize->value("Universal");
  else if (PageWidth == 612 && PageLength == 792)
    pageSize->value("Letter");
  else
  {
    sprintf(size, "%.2fx%.2fin", PageWidth / 72.0f, PageLength / 72.0f);
    pageSize->value(size);
  }

  sprintf(size, "%.2fin", PageLeft / 72.0f);
  pageLeft->value(size);

  sprintf(size, "%.2fin", PageRight / 72.0f);
  pageRight->value(size);

  sprintf(size, "%.2fin", PageTop / 72.0f);
  pageTop->value(size);

  sprintf(size, "%.2fin", PageBottom / 72.0f);
  pageBottom->value(size);

  pageDuplex->value(PageDuplex);

  landscape->value(Landscape);

  memset(formats, 0, sizeof(formats));
  formats[(int)'t'] = 1;
  formats[(int)'c'] = 2;
  formats[(int)'h'] = 3;
  formats[(int)'l'] = 4;
  formats[(int)'1'] = 5;
  formats[(int)'i'] = 6;
  formats[(int)'I'] = 7;
  formats[(int)'a'] = 8;
  formats[(int)'A'] = 9;
  formats[(int)'C'] = 10;
  formats[(int)'/'] = 11;
  formats[(int)':'] = 12;
  formats[(int)'d'] = 13;
  formats[(int)'T'] = 14;
  formats[(int)'D'] = 15;

  fmt = get_fmt(Header);
  pageHeaderLeft->value(formats[fmt[0]]);
  pageHeaderCenter->value(formats[fmt[1]]);
  pageHeaderRight->value(formats[fmt[2]]);

  fmt = get_fmt(Footer);
  pageFooterLeft->value(formats[fmt[0]]);
  pageFooterCenter->value(formats[fmt[1]]);
  pageFooterRight->value(formats[fmt[2]]);

  if (NumberUp == 1)
    numberUp->value(0);
  else if (NumberUp == 2)
    numberUp->value(1);
  else if (NumberUp == 4)
    numberUp->value(2);
  else if (NumberUp == 6)
    numberUp->value(3);
  else if (NumberUp == 9)
    numberUp->value(4);
  else if (NumberUp == 16)
    numberUp->value(5);

  tocLevels->value(TocLevels);
  numberedToc->value(TocNumbers);

  fmt = get_fmt(TocHeader);
  tocHeaderLeft->value(formats[fmt[0]]);
  tocHeaderCenter->value(formats[fmt[1]]);
  tocHeaderRight->value(formats[fmt[2]]);

  fmt = get_fmt(TocFooter);
  tocFooterLeft->value(formats[fmt[0]]);
  tocFooterCenter->value(formats[fmt[1]]);
  tocFooterRight->value(formats[fmt[2]]);

  tocTitle->value(TocTitle);

  headingFont->value(_htmlHeadingFont);
  bodyFont->value(_htmlBodyFont);
  headFootFont->value(HeadFootType * 4 + HeadFootStyle);

  fontBaseSize->value(_htmlSizes[SIZE_P]);
  fontSpacing->value(_htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
  headFootSize->value(HeadFootSize);

  for (i = 0; i < (charset->size() - 1); i ++)
    if (strcasecmp(_htmlCharSet, charset->text(i)) == 0)
    {
      charset->value(i);
      break;
    }

  compression->value(Compression);
  compGroup->deactivate();

  jpegCompress->value(OutputJPEG > 0);
  jpegQuality->value(OutputJPEG > 0 ? OutputJPEG : 90);
  jpegGroup->deactivate();

  pdfTab->deactivate();

  if (PDFVersion < 12)
  {
    pdf11->setonly();
    pdfCB(pdf11, this);
  }
  else if (PDFVersion < 13)
  {
    pdf12->setonly();
    pdfCB(pdf12, this);
  }
  else if (PDFVersion < 14)
  {
    pdf13->setonly();
    pdfCB(pdf13, this);
  }
  else
  {
    pdf14->setonly();
    pdfCB(pdf14, this);
  }

  pageMode->value(PDFPageMode);

  pageLayout->value(PDFPageLayout);

  firstPage->value(PDFFirstPage);

  pageEffect->value(PDFEffect);
  effectCB(pageEffect, this);

  pageDuration->value(PDFPageDuration);

  effectDuration->value(PDFEffectDuration);

  links->value(Links);
  embedFonts->value(EmbedFonts);

  securityTab->deactivate();

  if (Encryption)
  {
    encryptionYes->setonly();
    encryptionCB(encryptionYes, this);
  }
  else
  {
    encryptionNo->setonly();
    encryptionCB(encryptionNo, this);
  }

  if (Permissions & PDF_PERM_PRINT)
    permPrint->set();
  else
    permPrint->clear();

  if (Permissions & PDF_PERM_MODIFY)
    permModify->set();
  else
    permModify->clear();

  if (Permissions & PDF_PERM_COPY)
    permCopy->set();
  else
    permCopy->clear();

  if (Permissions & PDF_PERM_ANNOTATE)
    permAnnotate->set();
  else
    permAnnotate->clear();

  ownerPassword->value(OwnerPassword);
  userPassword->value(UserPassword);

  if (PSLevel == 1)
    ps1->setonly();
  else if (PSLevel == 2)
    ps2->setonly();
  else
    ps3->setonly();

  if (PSLevel == 1)
    psCommands->deactivate();
  else
    psCommands->activate();

  psCommands->value(PSCommands);

  xrxComments->value(XRXComments);

  path->value(Path);
  proxy->value(Proxy);
  browserWidth->value(_htmlBrowserWidth);
  strict_html->value(StrictHTML);

  title(NULL, 0);

  return (1);
}


//
// 'GUI::loadBook()' - Load a book file from disk.
//

int					// O - 1 = success, 0 = fail
GUI::loadBook(const char *filename)	// I - Name of book file
{
  FILE		*fp;			// File to read from
  char		line[10240];		// Line from file
  const char	*dir;			// Directory
  char		basename[1024];		// Base filename


  // If the filename contains a path, chdir to it first...
  if ((dir = file_directory(filename)) != NULL)
  {
   /*
    * Filename contains a complete path - get the directory portion and do
    * a chdir()...
    */

    strlcpy(basename, file_basename(filename), sizeof(basename));
    filename = basename;

    chdir(dir);
    fc->directory(".");
  }

  // Open the file...
  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    fl_alert("Unable to open \"%s\"!", filename);
    return (0);
  }

  // Get the header...
  file_gets(line, sizeof(line), fp);
  if (strncmp(line, "#HTMLDOC", 8) != 0)
  {
    fclose(fp);
    fl_alert("Bad or missing #HTMLDOC header:\n%-80.80s", line);
    return (0);
  }

  // Reset the GUI...
  if (!newBook())
  {
    fclose(fp);
    return (0);
  }

  // Read the second line from the book file; for older book files, this will
  // be the file count; for new files this will be the options...
  do
  {
    file_gets(line, sizeof(line), fp);

    if (line[0] == '-')
      parseOptions(line);
  }
  while (!line[0]);			// Skip blank lines...

  // Get input files/options...
  while (file_gets(line, sizeof(line), fp) != NULL)
  {
    if (line[0] == '\0')
      continue;				// Skip blank lines
    else if (line[0] == '-')
      parseOptions(line);
    else if (line[0] == '\\')
      inputFiles->add(line + 1, icon);
    else
      inputFiles->add(line, icon);
  }

  // Close the book file and update the GUI...
  fclose(fp);

  inputFiles->topline(1);

  title(filename, 0);

  return (1);
}


//
// 'GUI::parseOptions()' - Parse options in a book file...
//

void
GUI::parseOptions(const char *line)	// I - Line from file
{
  int		i;			// Looping var
  const char	*lineptr;		// Pointer into line
  char		temp[1024],		// Option name
		temp2[1024],		// Option value
		*tempptr,		// Pointer into option
		formats[256];		// Header/footer formats
  static const char *types[] =		// Typeface names...
		{ "Courier", "Times", "Helvetica" };
  static const char *fonts[] =		// Font names...
		{
		  "Courier", "Courier-Bold", "Courier-Oblique",
		  "Courier-BoldOblique", "Times-Roman", "Times-Bold",
		  "Times-Italic", "Times-BoldItalic",
		  "Helvetica", "Helvetica-Bold",
		  "Helvetica-Oblique", "Helvetica-BoldOblique"
		};


  // Initialize the format character lookup table...
  memset(formats, 0, sizeof(formats));
  formats[(int)'t'] = 1;
  formats[(int)'c'] = 2;
  formats[(int)'h'] = 3;
  formats[(int)'l'] = 4;
  formats[(int)'1'] = 5;
  formats[(int)'i'] = 6;
  formats[(int)'I'] = 7;
  formats[(int)'a'] = 8;
  formats[(int)'A'] = 9;
  formats[(int)'C'] = 10;
  formats[(int)'/'] = 11;
  formats[(int)':'] = 12;
  formats[(int)'d'] = 13;
  formats[(int)'T'] = 14;
  formats[(int)'D'] = 15;

  // Parse the input line...
  for (lineptr = line; *lineptr != '\0';)
  {
    while (*lineptr == ' ')
      lineptr ++;

    for (tempptr = temp; *lineptr != '\0' && *lineptr != ' ';)
      *tempptr++ = *lineptr++;
    *tempptr = '\0';

    while (*lineptr == ' ')
      lineptr ++;

    if (strcmp(temp, "--duplex") == 0)
    {
      pageDuplex->set();
      continue;
    }
    else if (strcmp(temp, "--landscape") == 0)
    {
      landscape->set();
      continue;
    }
    else if (strcmp(temp, "--portrait") == 0)
    {
      landscape->clear();
      continue;
    }
    else if (strncmp(temp, "--jpeg", 6) == 0)
    {
      if (strlen(temp) > 7)
        jpegQuality->value(atof(temp + 7));
      else
        jpegQuality->value(90.0);

      if (jpegQuality->value() > 0.0)
      {
        jpegCompress->set();
        jpegGroup->activate();
      }
      else
      {
        jpegCompress->clear();
        jpegGroup->deactivate();
      }
      continue;
    }
    else if (strcmp(temp, "--grayscale") == 0)
    {
      grayscale->set();
      continue;
    }
    else if (strcmp(temp, "--color") == 0)
    {
      grayscale->clear();
      continue;
    }
    else if (strcmp(temp, "--links") == 0)
    {
      links->set();
      continue;
    }
    else if (strcmp(temp, "--no-links") == 0)
    {
      links->clear();
      continue;
    }
    else if (strcmp(temp, "--truetype") == 0 ||
             strcmp(temp, "--embedfonts") == 0)
    {
      embedFonts->set();
      continue;
    }
    else if (strcmp(temp, "--no-truetype") == 0 ||
             strcmp(temp, "--no-embedfonts") == 0)
    {
      embedFonts->clear();
      continue;
    }
    else if (strcmp(temp, "--pscommands") == 0)
    {
      psCommands->set();
      continue;
    }
    else if (strcmp(temp, "--no-pscommands") == 0)
    {
      psCommands->clear();
      continue;
    }
    else if (strcmp(temp, "--xrxcomments") == 0)
    {
      xrxComments->set();
      continue;
    }
    else if (strcmp(temp, "--no-xrxcomments") == 0)
    {
      xrxComments->clear();
      continue;
    }
    else if (strncmp(temp, "--compression", 13) == 0)
    {
      if (strlen(temp) > 14)
        compression->value(atof(temp + 14));
      else
        compression->value(1.0);
      continue;
    }
    else if (strcmp(temp, "--no-compression") == 0)
    {
      compression->value(0.0);
      continue;
    }
    else if (strcmp(temp, "--no-jpeg") == 0)
    {
      jpegCompress->clear();
      jpegGroup->deactivate();
      continue;
    }
    else if (strcmp(temp, "--numbered") == 0)
    {
      numberedToc->set();
      continue;
    }
    else if (strcmp(temp, "--no-numbered") == 0)
    {
      numberedToc->clear();
      continue;
    }
    else if (strcmp(temp, "--no-toc") == 0)
    {
      tocLevels->value(0);
      continue;
    }
    else if (strcmp(temp, "--title") == 0)
    {
      titlePage->set();
      continue;
    }
    else if (strcmp(temp, "--no-title") == 0)
    {
      titlePage->clear();
      continue;
    }
    else if (strcmp(temp, "--book") == 0)
    {
      typeBook->setonly();
      docTypeCB(typeBook, this);
      continue;
    }
    else if (strcmp(temp, "--continuous") == 0)
    {
      typeContinuous->setonly();
      docTypeCB(typeContinuous, this);
      continue;
    }
    else if (strcmp(temp, "--webpage") == 0)
    {
      typeWebPage->setonly();
      docTypeCB(typeWebPage, this);
      continue;
    }
    else if (strcmp(temp, "--encryption") == 0)
    {
      encryptionYes->setonly();
      encryptionCB(encryptionYes, this);
      continue;
    }
    else if (strcmp(temp, "--no-encryption") == 0)
    {
      encryptionNo->setonly();
      encryptionCB(encryptionNo, this);
      continue;
    }
    else if (temp[0] != '-')
    {
      inputFiles->add(temp, icon);
      continue;
    }

    if (*lineptr == '\"')
    {
      lineptr ++;

      for (tempptr = temp2; *lineptr != '\0' && *lineptr != '\"';)
	*tempptr++ = *lineptr++;

      if (*lineptr == '\"')
        lineptr ++;
    }
    else
    {
      for (tempptr = temp2; *lineptr != '\0' && *lineptr != ' ';)
	*tempptr++ = *lineptr++;
    }

    *tempptr = '\0';

    if (strcmp(temp, "-t") == 0)
    {
      if (strcmp(temp2, "html") == 0)
      {
        typeHTML->setonly();
	outputFormatCB(typeHTML, this);
      }
      else if (strcmp(temp2, "htmlsep") == 0)
      {
        typeHTMLSep->setonly();
	outputFormatCB(typeHTMLSep, this);
      }
      else if (strcmp(temp2, "ps1") == 0)
      {
        typePS->setonly();
	ps1->setonly();
	outputFormatCB(typePS, this);
	psCB(ps1, this);
      }
      else if (strcmp(temp2, "ps") == 0 ||
               strcmp(temp2, "ps2") == 0)
      {
        typePS->setonly();
	ps2->setonly();
	outputFormatCB(typePS, this);
	psCB(ps2, this);
      }
      else if (strcmp(temp2, "ps3") == 0)
      {
        typePS->setonly();
	ps3->setonly();
	outputFormatCB(typePS, this);
	psCB(ps3, this);
      }
      else if (strcmp(temp2, "pdf11") == 0)
      {
        typePDF->setonly();
	pdf11->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf11, this);
      }
      else if (strcmp(temp2, "pdf12") == 0)
      {
        typePDF->setonly();
	pdf12->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf12, this);
      }
      else if (strcmp(temp2, "pdf") == 0 ||
               strcmp(temp2, "pdf13") == 0)
      {
        typePDF->setonly();
	pdf13->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf13, this);
      }
      else if (strcmp(temp2, "pdf14") == 0)
      {
        typePDF->setonly();
	pdf14->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf14, this);
      }
    }
    else if (strcmp(temp, "--logo") == 0 ||
             strcmp(temp, "--logoimage") == 0)
      logoImage->value(temp2);
    else if (strcmp(temp, "--titlefile") == 0 ||
             strcmp(temp, "--titleimage") == 0)
    {
      titlePage->set();
      titleImage->value(temp2);
    }
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
    else if (strcmp(temp, "--browserwidth") == 0)
      browserWidth->value(atof(temp2));
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
    else if (strcmp(temp, "--nup") == 0)
    {
      i = atoi(temp2);

      if (i == 1)
        numberUp->value(0);
      else if (i == 2)
        numberUp->value(1);
      else if (i == 4)
        numberUp->value(2);
      else if (i == 6)
        numberUp->value(3);
      else if (i == 9)
        numberUp->value(4);
      else if (i == 16)
        numberUp->value(5);
    }
    else if (strcmp(temp, "--bodycolor") == 0)
      bodyColor->value(temp2);
    else if (strcmp(temp, "--bodyimage") == 0)
      bodyImage->value(temp2);
    else if (strcmp(temp, "--textcolor") == 0)
      textColor->value(temp2);
    else if (strcmp(temp, "--linkcolor") == 0)
      linkColor->value(temp2);
    else if (strcmp(temp, "--linkstyle") == 0)
    {
      if (strcmp(temp2, "plain") == 0)
        linkStyle->value(0);
      else
        linkStyle->value(1);
    }
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
    else if (strcmp(temp, "--toctitle") == 0)
      tocTitle->value(temp2);
    else if (strcmp(temp, "--fontsize") == 0)
      fontBaseSize->value(atof(temp2));
    else if (strcmp(temp, "--fontspacing") == 0)
      fontSpacing->value(atof(temp2));
    else if (strcmp(temp, "--headingfont") == 0)
    {
      for (i = 0; i < 3; i ++)
        if (strcasecmp(types[i], temp2) == 0)
	{
	  headingFont->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--bodyfont") == 0)
    {
      for (i = 0; i < 3; i ++)
        if (strcasecmp(types[i], temp2) == 0)
	{
	  bodyFont->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--headfootsize") == 0)
      headFootSize->value(atof(temp2));
    else if (strcmp(temp, "--headfootfont") == 0)
    {
      for (i = 0; i < 12; i ++)
        if (strcasecmp(fonts[i], temp2) == 0)
	{
	  headFootFont->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--charset") == 0)
    {
      for (i = 0; i < (charset->size() - 1); i ++)
        if (strcasecmp(temp2, charset->text(i)) == 0)
	{
	  charset->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--pagemode") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFModes) / sizeof(PDFModes[0])); i ++)
        if (strcasecmp(temp2, PDFModes[i]) == 0)
	{
	  pageMode->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--pagelayout") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFLayouts) / sizeof(PDFLayouts[0])); i ++)
        if (strcasecmp(temp2, PDFLayouts[i]) == 0)
	{
	  pageLayout->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--firstpage") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFPages) / sizeof(PDFPages[0])); i ++)
        if (strcasecmp(temp2, PDFPages[i]) == 0)
	{
	  firstPage->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--pageeffect") == 0)
    {
      for (i = 0; i < (int)(sizeof(PDFEffects) / sizeof(PDFEffects[0])); i ++)
        if (strcasecmp(temp2, PDFEffects[i]) == 0)
	{
	  pageEffect->value(i);
	  effectCB(pageEffect, this);
	  break;
	}
    }
    else if (strcmp(temp, "--pageduration") == 0)
      pageDuration->value(atof(temp2));
    else if (strcmp(temp, "--effectduration") == 0)
      effectDuration->value(atof(temp2));
    else if (strcmp(temp, "--permissions") == 0)
    {
      if (strcmp(temp2, "all") == 0)
      {
        permPrint->set();
	permModify->set();
	permCopy->set();
	permAnnotate->set();
      }
      else if (strcmp(temp2, "none") == 0)
      {
        permPrint->clear();
	permModify->clear();
	permCopy->clear();
	permAnnotate->clear();
      }
      else if (strcmp(temp2, "print") == 0)
        permPrint->set();
      else if (strcmp(temp2, "no-print") == 0)
        permPrint->clear();
      else if (strcmp(temp2, "modify") == 0)
	permModify->set();
      else if (strcmp(temp2, "no-modify") == 0)
	permModify->clear();
      else if (strcmp(temp2, "copy") == 0)
	permCopy->set();
      else if (strcmp(temp2, "no-copy") == 0)
	permCopy->clear();
      else if (strcmp(temp2, "annotate") == 0)
	permAnnotate->set();
      else if (strcmp(temp2, "no-annotate") == 0)
	permAnnotate->clear();
    }
    else if (strcmp(temp, "--user-password") == 0)
      userPassword->value(temp2);
    else if (strcmp(temp, "--owner-password") == 0)
      ownerPassword->value(temp2);
    else if (strcmp(temp, "--path") == 0)
      path->value(temp2);
    else if (strcmp(temp, "--proxy") == 0)
      proxy->value(temp2);
  }
}


//
// 'GUI::saveBook()' - Save a book to disk.
//

int					// O - 1 = success, 0 = fail
GUI::saveBook(const char *filename)	// I - Name of book file
{
  int		i,			// Looping var
		count;			// Number of files
  FILE		*fp;			// Book file pointer
  static const char *formats = ".tchl1iIaAC/:dTD";
					// Format characters
  static const char *types[] =		// Typeface names...
		{ "Courier", "Times", "Helvetica" };
  static const char *fonts[] =		// Font names...
		{
		  "Courier", "Courier-Bold", "Courier-Oblique",
		  "Courier-BoldOblique", "Times-Roman", "Times-Bold",
		  "Times-Italic", "Times-BoldItalic",
		  "Helvetica", "Helvetica-Bold",
		  "Helvetica-Oblique", "Helvetica-BoldOblique"
		};


  // Open the book file...
  fp = fopen(filename, "w");
  if (fp == NULL)
  {
    fl_alert("Unable to create \"%s\"!", filename);
    return (0);
  }

  // Write the standard header...
  fputs("#HTMLDOC " SVERSION "\n", fp);

  // Write the options...
  if (typeHTML->value())
    fputs("-t html", fp);
  else if (typeHTMLSep->value())
    fputs("-t htmlsep", fp);
  else if (typePS->value())
  {
    if (ps1->value())
      fputs("-t ps1", fp);
    else if (ps2->value())
      fputs("-t ps2", fp);
    else if (ps3->value())
      fputs("-t ps3", fp);
  }
  else if (pdf11->value())
    fputs("-t pdf11", fp);
  else if (pdf12->value())
    fputs("-t pdf12", fp);
  else if (pdf13->value())
    fputs("-t pdf13", fp);
  else
    fputs("-t pdf14", fp);

  if (outputFile->value())
    fprintf(fp, " -f %s", outputPath->value());
  else
    fprintf(fp, " -d %s", outputPath->value());

  if (typeWebPage->value())
    fputs(" --webpage", fp);
  else if (typeContinuous->value())
    fputs(" --continuous", fp);
  else
  {
    fputs(" --book", fp);

    if (tocLevels->value() == 0)
      fputs(" --no-toc", fp);
    else
      fprintf(fp, " --toclevels %d", tocLevels->value());

    if (numberedToc->value())
      fputs(" --numbered", fp);
    else
      fputs(" --no-numbered", fp);

    fprintf(fp, " --toctitle \"%s\"", tocTitle->value());
  }

  if (titlePage->value())
  {
    fputs(" --title", fp);

    if (titleImage->size() > 0)
      fprintf(fp, " --titleimage %s", titleImage->value());
  }
  else
    fputs(" --no-title", fp);

  if (logoImage->size() > 0)
    fprintf(fp, " --logoimage %s", logoImage->value());

  if (textColor->size() > 0)
    fprintf(fp, " --textcolor %s", textColor->value());

  if (linkColor->size() > 0)
    fprintf(fp, " --linkcolor %s", linkColor->value());

  if (linkStyle->value())
    fputs(" --linkstyle underline", fp);
  else
    fputs(" --linkstyle plain", fp);

  if (bodyColor->size() > 0)
    fprintf(fp, " --bodycolor %s", bodyColor->value());

  if (bodyImage->size() > 0)
    fprintf(fp, " --bodyimage %s", bodyImage->value());

  if (!typeHTML->value() && !typeHTMLSep->value())
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

    fprintf(fp, " --nup %s", numberUp->text(numberUp->value()));

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

    if (landscape->value())
      fputs(" --landscape", fp);
    else
      fputs(" --portrait", fp);

    if (grayscale->value())
      fputs(" --grayscale", fp);
    else
      fputs(" --color", fp);

    if (psCommands->value())
      fputs(" --pscommands", fp);
    else
      fputs(" --no-pscommands", fp);

    if (xrxComments->value())
      fputs(" --xrxcomments", fp);
    else
      fputs(" --no-xrxcomments", fp);

    if (compression->value() == 0.0f)
      fputs(" --no-compression", fp);
    else
      fprintf(fp, " --compression=%.0f", compression->value());

    if (jpegCompress->value())
      fprintf(fp, " --jpeg=%.0f", jpegQuality->value());
    else
      fputs(" --jpeg=0", fp);
  }

  fprintf(fp, " --fontsize %.1f", fontBaseSize->value());
  fprintf(fp, " --fontspacing %.1f", fontSpacing->value());
  fprintf(fp, " --headingfont %s", types[headingFont->value()]);
  fprintf(fp, " --bodyfont %s", types[bodyFont->value()]);
  fprintf(fp, " --headfootsize %.1f", headFootSize->value());
  fprintf(fp, " --headfootfont %s", fonts[headFootFont->value()]);
  fprintf(fp, " --charset %s", charset->text(charset->value()));

  if (typePDF->value())
  {
    if (links->value())
      fputs(" --links", fp);
    else
      fputs(" --no-links", fp);

    if (embedFonts->value())
      fputs(" --embedfonts", fp);
    else
      fputs(" --no-embedfonts", fp);

    fprintf(fp, " --pagemode %s", PDFModes[pageMode->value()]);
    fprintf(fp, " --pagelayout %s", PDFLayouts[pageLayout->value()]);
    fprintf(fp, " --firstpage %s", PDFPages[firstPage->value()]);
    fprintf(fp, " --pageeffect %s", PDFEffects[pageEffect->value()]);
    fprintf(fp, " --pageduration %.0f", pageDuration->value());
    fprintf(fp, " --effectduration %.1f", effectDuration->value());
    fprintf(fp, " --%sencryption", encryptionYes->value() ? "" : "no-");

    if (permPrint->value() && permModify->value() && permCopy->value() &&
        permAnnotate->value())
      fputs(" --permissions all", fp);
    else if (permPrint->value() || permModify->value() || permCopy->value() ||
             permAnnotate->value())
    {
      if (permPrint->value())
        fputs(" --permissions print", fp);
      else
        fputs(" --permissions no-print", fp);

      if (permModify->value())
        fputs(" --permissions modify", fp);
      else
        fputs(" --permissions no-modify", fp);

      if (permCopy->value())
        fputs(" --permissions copy", fp);
      else
        fputs(" --permissions no-copy", fp);

      if (permAnnotate->value())
        fputs(" --permissions annotate", fp);
      else
        fputs(" --permissions no-annotate", fp);
    }
    else
      fputs(" --permissions none", fp);

    fprintf(fp, "  --owner-password \"%s\"", ownerPassword->value());    
    fprintf(fp, "  --user-password \"%s\"", userPassword->value());    
  }

  fprintf(fp, " --browserwidth %.0f", browserWidth->value());

  if (path->value()[0])
    fprintf(fp, " --path \"%s\"", path->value());

  if (proxy->value()[0])
    fprintf(fp, " --proxy \"%s\"", proxy->value());

  fputs("\n", fp);

  // Output the files...
  count = inputFiles->size();

  for (i = 1; i <= count; i ++)
    if (inputFiles->text(i)[0] == '-')
      fprintf(fp, "\\%s\n", inputFiles->text(i));
    else
      fprintf(fp, "%s\n", inputFiles->text(i));

  // Close the file and update the GUI...
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
// 'GUI::changeCB()' - Mark the current book as changed.
//

void
GUI::changeCB(Fl_Widget *w,	// I - Widget
              GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);
}


//
// 'GUI::docTypeCB()' - Handle input on the document type buttons.
//

void
GUI::docTypeCB(Fl_Widget *w,	// I - Toggle button widget
               GUI       *gui)	// I - GUI
{
  gui->title(gui->book_filename, 1);

  if (w == gui->typeBook)
  {
    gui->typeHTML->activate();
    gui->typeHTMLSep->activate();

    gui->titlePage->value(1);

    gui->tocTab->activate();
    gui->tocLevels->value(3);

    gui->firstPage->activate();
    gui->pageMode->value(1);
  }
  else
  {
    gui->typeHTML->deactivate();
    gui->typeHTMLSep->deactivate();

    if (gui->typeHTML->value() || gui->typeHTMLSep->value())
    {
      gui->typePDF->setonly();
      outputFormatCB(gui->typePDF, gui);
    }

    gui->titlePage->value(0);

    gui->tocTab->deactivate();

    gui->firstPage->value(0);
    gui->firstPage->deactivate();
    gui->pageMode->value(0);
  }
}


//
// 'GUI::inputFilesCB()' - Handle selections in the input files browser.
//

void
GUI::inputFilesCB(Fl_Widget *w,		// I - Widget
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
// 'GUI::addFileCB()' - Add a file to the input files list.
//

void
GUI::addFileCB(Fl_Widget *w,	// I - Widget
               GUI       *gui)	// I - GUI
{
  int	i;			// Looping var


  REF(w);

  gui->fc->filter("WWW Files (*.{htm,html,shtml,book})");
  gui->fc->type(Fl_File_Chooser::MULTI);
  gui->fc->label("Add HTML Files?");
  gui->fc->show();
  while (gui->fc->shown())
    Fl::wait();

  if (gui->fc->count())
  {
    for (i = 1; i <= gui->fc->count(); i ++)
    {
      if (strcasecmp(file_extension(gui->fc->value(i)), "book") == 0)
      {
        // Import files from the book...
	FILE	*fp;
	char	line[1024];
	char	directory[1024];
	int	count;


        getcwd(directory, sizeof(directory));
	chdir(file_directory(gui->fc->value(i)));

	if ((fp = fopen(gui->fc->value(i), "rb")) == NULL)
	{
	  fl_alert("Unable to import %s:\n%s", gui->fc->value(i),
	           strerror(errno));
	  chdir(directory);
	  continue;
	}

        if (fgets(line, sizeof(line), fp) == NULL)
	{
	  fl_alert("Unable to import %s:\nShort file.", gui->fc->value(i));
	  fclose(fp);
	  chdir(directory);
	  continue;
	}

        if (strncmp(line, "#HTMLDOC", 8) != 0)
	{
	  fl_alert("Unable to import %s:\nBad header line.", gui->fc->value(i));
	  fclose(fp);
	  chdir(directory);
	  continue;
	}

        if (fgets(line, sizeof(line), fp) == NULL)
	{
	  fl_alert("Unable to import %s:\nNo file count.", gui->fc->value(i));
	  fclose(fp);
	  chdir(directory);
	  continue;
	}

        count = atoi(line);
	while (count > 0)
	{
	  count --;

          if (fgets(line, sizeof(line), fp) == NULL)
	  {
	    fl_alert("Unable to import %s:\nMissing file.", gui->fc->value(i));
	    fclose(fp);
	    chdir(directory);
	    continue;
	  }

          line[strlen(line) - 1] = '\0'; // strip newline
          gui->inputFiles->add(file_localize(line, directory), gui->icon);
	}

        fclose(fp);
	chdir(directory);
      }
      else
        gui->inputFiles->add(file_localize(gui->fc->value(i), NULL), gui->icon);
    }

    gui->title(gui->book_filename, 1);
  }
}


//
// 'GUI::addURLCB()' - Add a URL to the input files list.
//

void
GUI::addURLCB(Fl_Widget *w,	// I - Widget
              GUI       *gui)	// I - GUI
{
  const char	*url;		// New URL to add


  REF(w);

  if ((url = fl_input("URL?", "http://")) != NULL)
  {
    gui->inputFiles->add(url, gui->icon);
    gui->title(gui->book_filename, 1);
  }
}


//
// 'GUI::editFilesCB()' - Edit one or more files in the input files list.
//

void
GUI::editFilesCB(Fl_Widget *w,		// I - Widget
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
      snprintf(command, sizeof(command), gui->htmlEditor->value(),
               gui->inputFiles->text(i));

#ifdef WIN32
      memset(&suInfo, 0, sizeof(suInfo));
      suInfo.cb = sizeof(suInfo);

      if (!CreateProcess(NULL, command, NULL, NULL, FALSE,
                         NORMAL_PRIORITY_CLASS, NULL, NULL, &suInfo, &prInfo))
        fl_alert("Unable to start editor!");
#else
      strlcat(command, "&", sizeof(command));
      if (system(command))
        fl_alert("Unable to start editor!");
#endif // !WIN32
    }
}


//
// 'GUI::deleteFileCB()' - Delete one or more files from the input files list.
//

void
GUI::deleteFilesCB(Fl_Widget *w,	// I - Widget
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
// 'GUI::moveUpFileCB()' - Move one or more files up in the input files list.
//

void
GUI::moveUpFilesCB(Fl_Widget *w,	// I - Widget
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
      gui->inputFiles->insert(i - 1, file, gui->icon);
      gui->inputFiles->select(i - 1);
      gui->inputFiles->remove(i + 1);
      gui->inputFiles->select(i, 0);
      gui->title(gui->book_filename, 1);
    }

  for (i = 1; i <= num_items; i ++)
    if (gui->inputFiles->selected(i))
      break;

  gui->inputFiles->make_visible(i);

  if (gui->inputFiles->selected(1))
    gui->moveUpFile->deactivate();

  if (!gui->inputFiles->selected(num_items))
    gui->moveDownFile->activate();
}


//
// 'GUI::moveDownFileCB()' - Move one or more files down in the input files list.
//

void
GUI::moveDownFilesCB(Fl_Widget *w,	// I - Widget
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
      gui->inputFiles->insert(i + 2, file, gui->icon);
      gui->inputFiles->select(i + 2);
      gui->inputFiles->remove(i);
      gui->inputFiles->select(i, 0);
      gui->title(gui->book_filename, 1);
    }

  for (i = num_items; i >= 1; i --)
    if (gui->inputFiles->selected(i))
      break;

  gui->inputFiles->make_visible(i);

  if (!gui->inputFiles->selected(1))
    gui->moveUpFile->activate();

  if (gui->inputFiles->selected(num_items))
    gui->moveDownFile->deactivate();
}


//
// 'GUI::logoImageCB()' - Change the logo image file.
//

void
GUI::logoImageCB(Fl_Widget *w,		// I - Widget
                 GUI       *gui)	// I - GUI
{
  if (w == gui->logoBrowse)
  {
    gui->fc->filter("Image Files (*.{bmp,gif,jpg,png})");
    gui->fc->label("Logo Image?");
    gui->fc->type(Fl_File_Chooser::SINGLE);
    gui->fc->show();
    while (gui->fc->shown())
      Fl::wait();

    if (gui->fc->count())
    {
      gui->logoImage->value(file_localize(gui->fc->value(), NULL));
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::titleImageCB()' - Change the title image file.
//

void
GUI::titleImageCB(Fl_Widget *w,		// I - Widget
                  GUI       *gui)	// I - GUI
{
  if (w == gui->titleBrowse)
  {
    gui->fc->filter("Image Files (*.{bmp,gif,jpg,png})\tWWW Files (*.{htm,html,shtml})");
    gui->fc->label("Title Image?");
    gui->fc->type(Fl_File_Chooser::SINGLE);
    gui->fc->show();
    while (gui->fc->shown())
      Fl::wait();

    if (gui->fc->count())
    {
      gui->titleImage->value(file_localize(gui->fc->value(), NULL));
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::outputTypeCB()' - Set the output file type.
//

void
GUI::outputTypeCB(Fl_Widget *w,		// I - Widget
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
// 'GUI::outputPathCB()' - Set the output path.
//

void
GUI::outputPathCB(Fl_Widget *w,		// I - Widget
                  GUI       *gui)	// I - GUI
{
  char		filename[1024];		// Name of the output file
  const char	*extension;		// Extension of the output file


  if (w == gui->outputBrowse)
  {
    gui->fc->label("Output Path?");

    if (gui->outputFile->value())
    {
      gui->fc->type(Fl_File_Chooser::CREATE);

      if (gui->typeHTML->value())
	gui->fc->filter("WWW Files (*.htm*)");
      else if (gui->typePDF->value())
	gui->fc->filter("PDF Files (*.pdf)");
      else
	gui->fc->filter("PostScript Files (*.ps)");
    }
    else
    {
      gui->fc->type(Fl_File_Chooser::DIRECTORY | Fl_File_Chooser::CREATE);
      gui->fc->filter("*");
    }

    gui->fc->show();
    while (gui->fc->shown())
      Fl::wait();

    if (gui->fc->count())
    {
      // Get the selected file...
      strlcpy(filename, file_localize(gui->fc->value(), NULL), sizeof(filename));
      extension = file_extension(filename);

      if (extension[0])
      {
        // Have an extension - check it!
	if (strcasecmp(extension, "PS") == 0)
	{
	  gui->typePS->setonly();
	  outputFormatCB(gui->typePS, gui);
	}
	else if (strcasecmp(extension, "PDF") == 0)
	{
	  gui->typePDF->setonly();
	  outputFormatCB(gui->typePDF, gui);
	}
      }
      else if (gui->outputFile->value())
      {
        // No extension - add one!
	if (gui->typeHTML->value())
	  strlcat(filename, ".html", sizeof(filename));
	else if (gui->typePS->value())
	  strlcat(filename, ".ps", sizeof(filename));
	else
	  strlcat(filename, ".pdf", sizeof(filename));
      }

      gui->outputPath->value(filename);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::outputFormatCB()' - Set the output format.
//

void
GUI::outputFormatCB(Fl_Widget *w,	// I - Widget
                    GUI       *gui)	// I - GUI
{
  char		filename[1024],		// Output filename
		*ptr;			// Pointer to extension
  const char	*ext;			// Extension


  gui->title(gui->book_filename, 1);

  ext = NULL; // To make GCC happy...

  if (w == gui->typePDF)
  {
    gui->pdfTab->activate();
    gui->securityTab->activate();
    gui->outputDirectory->deactivate();
    gui->outputFile->setonly();

    ext = ".pdf";
  }
  else
  {
    gui->pdfTab->deactivate();
    gui->securityTab->deactivate();
    gui->outputDirectory->activate();
  }

  if (w == gui->typeHTMLSep)
  {
    gui->outputFile->deactivate();
    gui->outputDirectory->setonly();
  }
  else
    gui->outputFile->activate();

  if (w == gui->typeHTML || w == gui->typeHTMLSep)
  {
    gui->compression->value(0);

    gui->jpegCompress->value(0);
    gui->jpegCompress->deactivate();
    gui->jpegGroup->deactivate();

    gui->grayscale->value(0);
    gui->grayscale->deactivate();

    gui->pageTab->deactivate();

    gui->tocHeaderLeft->deactivate();
    gui->tocHeaderCenter->deactivate();
    gui->tocHeaderRight->deactivate();

    gui->tocFooterLeft->deactivate();
    gui->tocFooterCenter->deactivate();
    gui->tocFooterRight->deactivate();

    ext = ".html";
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

    if (w == gui->typePDF || !gui->ps1->value())
      gui->jpegCompress->activate();
    else
      gui->jpegCompress->deactivate();
  }

  if (w == gui->typePS)
  {
    gui->psTab->activate();

    if (gui->ps1->value())
      gui->psCommands->deactivate();
    else
      gui->psCommands->activate();

    ext = ".ps";
  }
  else
    gui->psTab->deactivate();

  if ((w == gui->typePDF && !gui->pdf11->value()) ||
      (w == gui->typePS && gui->ps3->value()))
    gui->compGroup->activate();
  else
    gui->compGroup->deactivate();

  // Update the output filename's extension if we are writing to a file
  // and the output filename is not blank...
  if (gui->outputFile->value() && gui->outputPath->value()[0])
  {
    strlcpy(filename, gui->outputPath->value(), sizeof(filename));

    if ((ptr = strrchr(filename, '/')) == NULL)
      ptr = filename;

    if ((ptr = strrchr(ptr, '.')) == NULL)
      strlcat(filename, ext, sizeof(filename));
    else
      strlcpy(ptr, ext, sizeof(filename) - (ptr - filename));

    gui->outputPath->value(filename);
  }
}


//
// 'GUI::jpegCB()' - Handle JPEG changes.
//

void
GUI::jpegCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);

  if (gui->jpegCompress->value())
    gui->jpegGroup->activate();
  else
    gui->jpegGroup->deactivate();
}


//
// 'GUI::sizeCB()' - Change the page size based on the menu selection.
//

void
GUI::sizeCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);
  gui->pageSize->value(gui->pageSizeMenu->text(gui->pageSizeMenu->value()));
}


//
// 'GUI::tocCB()' - Handle Table-of-Contents changes.
//

void
GUI::tocCB(Fl_Widget *w,	// I - Widget
           GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);

  if (gui->tocLevels->value())
  {
    gui->numberedToc->activate();
    gui->tocHeader->activate();
    gui->tocFooter->activate();
    gui->tocTitle->activate();
  }
  else
  {
    gui->numberedToc->deactivate();
    gui->tocHeader->deactivate();
    gui->tocFooter->deactivate();
    gui->tocTitle->deactivate();
  }
}


//
// 'GUI::pdfCB()' - Handle PDF version changes.
//

void
GUI::pdfCB(Fl_Widget *w,	// I - Widget
           GUI       *gui)	// I - GUI
{
  REF(w);


  if (gui->typePDF->value())
  {
    if (w == gui->pdf11)
      gui->compGroup->deactivate();
    else
      gui->compGroup->activate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'GUI::encryptionCB()' - Handle PDF encryption changes.
//

void
GUI::encryptionCB(Fl_Widget *w,		// I - Widget
                  GUI       *gui)	// I - GUI
{
  if (w == gui->encryptionYes)
  {
    gui->permissions->activate();
    gui->ownerPassword->activate();
    gui->userPassword->activate();
  }
  else if (w == gui->encryptionNo)
  {
    gui->permissions->deactivate();
    gui->ownerPassword->deactivate();
    gui->userPassword->deactivate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'GUI::effectCB()' - Handle PDF effect changes.
//

void
GUI::effectCB(Fl_Widget *w,	// I - Widget
              GUI       *gui)	// I - GUI
{
  REF(w);


  if (gui->pageEffect->value())
  {
    gui->pageDuration->activate();
    gui->effectDuration->activate();
  }
  else
  {
    gui->pageDuration->deactivate();
    gui->effectDuration->deactivate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'GUI::psCB()' - Handle PS language level changes.
//

void
GUI::psCB(Fl_Widget *w,		// I - Widget
          GUI       *gui)	// I - GUI
{
  if (w == gui->ps1)
  {
    gui->jpegCompress->deactivate();
    gui->psCommands->deactivate();
    gui->psCommands->value(0);
  }
  else
  {
    gui->jpegCompress->activate();
    gui->psCommands->activate();
  }

  if (gui->typePS->value())
  {
    if (w == gui->ps3)
      gui->compGroup->activate();
    else
      gui->compGroup->deactivate();
  }

  gui->title(gui->book_filename, 1);
}


//
// 'GUI::htmlEditorCB()' - Change the HTML editor.
//

void
GUI::htmlEditorCB(Fl_Widget *w,		// I - Widget
                  GUI       *gui)	// I - GUI
{
  const char	*filename;		// New HTML editor file
  char		command[1024];		// Command string


  if (w == gui->htmlBrowse)
  {
#  if defined(WIN32) || defined(__EMX__)
    gui->fc->filter("Program Files (*.exe)");
#  else
    gui->fc->filter("*");
#  endif // WIN32 || __EMX__
    gui->fc->label("HTML Editor?");
    gui->fc->type(Fl_File_Chooser::SINGLE);
    gui->fc->show();
    while (gui->fc->shown())
      Fl::wait();

    if (gui->fc->count())
    {
      filename = gui->fc->value();

      if (strstr(filename, "netscape") != NULL ||
          strstr(filename, "NETSCAPE") != NULL)
#if defined(WIN32) || defined(__EMX__)
        snprintf(command, sizeof(command), "%s -edit \"%%s\"", filename);
#else
        snprintf(command, sizeof(command), "%s -remote \'editFile(%%s)\'", filename);
#endif // WIN32 || __EMX__
      else
        snprintf(command, sizeof(command), "%s \"%%s\"", filename);

      gui->htmlEditor->value(command);
    }
  }

  strlcpy(HTMLEditor, gui->htmlEditor->value(), sizeof(HTMLEditor));
}


//
// 'GUI::tooltipCB()' - Enable or disable tooltips.
//

void
GUI::tooltipCB(Fl_Widget *w,	// I - Widget
               GUI       *gui)	// I - GUI interface
{
  REF(gui);

  Tooltips = ((Fl_Button *)w)->value();

  Fl_Tooltip::enable(Tooltips);
}


//
// 'GUI::skinCB()' - Enable or disable the modern "skin".
//

void
GUI::skinCB(Fl_Widget *,	// I - Widget
            GUI       *gui)	// I - GUI interface
{
  ModernSkin = gui->modern_skin->value();

  Fl::scheme(ModernSkin ? "plastic" : "");

  if (ModernSkin)
  {
    // Use alternate colors for the "modern" look-n-feel...
    gui->grayscale->color2(FL_RED);
    gui->titlePage->color2(FL_RED);
    gui->jpegCompress->color2(FL_RED);
    gui->pageDuplex->color2(FL_RED);
    gui->landscape->color2(FL_RED);
    gui->numberedToc->color2(FL_RED);
    gui->psCommands->color2(FL_RED);
    gui->xrxComments->color2(FL_RED);
    gui->links->color2(FL_RED);
    gui->embedFonts->color2(FL_RED);
    gui->permPrint->color2(FL_RED);
    gui->permModify->color2(FL_RED);
    gui->permCopy->color2(FL_RED);
    gui->permAnnotate->color2(FL_RED);
    gui->tooltips->color2(FL_RED);
    gui->modern_skin->color2(FL_RED);
    gui->strict_html->color2(FL_RED);

    gui->progressBar->color2(FL_BLUE);
    gui->progressBar->box(FL_UP_BOX);
  }
  else
  {
    // Use standard colors...
    gui->grayscale->color2(FL_BLACK);
    gui->titlePage->color2(FL_BLACK);
    gui->jpegCompress->color2(FL_BLACK);
    gui->pageDuplex->color2(FL_BLACK);
    gui->landscape->color2(FL_BLACK);
    gui->numberedToc->color2(FL_BLACK);
    gui->psCommands->color2(FL_BLACK);
    gui->xrxComments->color2(FL_BLACK);
    gui->links->color2(FL_BLACK);
    gui->embedFonts->color2(FL_BLACK);
    gui->permPrint->color2(FL_BLACK);
    gui->permModify->color2(FL_BLACK);
    gui->permCopy->color2(FL_BLACK);
    gui->permAnnotate->color2(FL_BLACK);
    gui->tooltips->color2(FL_BLACK);
    gui->modern_skin->color2(FL_BLACK);
    gui->strict_html->color2(FL_BLACK);

    gui->progressBar->color2(FL_YELLOW);
    gui->progressBar->box(FL_DOWN_BOX);
  }
}


//
// 'GUI::saveOptionsCB()' - Save preferences...
//

void
GUI::saveOptionsCB(Fl_Widget *w,
                   GUI       *gui)
{
  gui->loadSettings();

  prefs_save();
}


//
// 'GUI::bodyColorCB()' - Set the body color.
//

void
GUI::bodyColorCB(Fl_Widget *w,		// I - Widget
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
// 'GUI::bodyImageCB()' - Set the body image.
//

void
GUI::bodyImageCB(Fl_Widget *w,		// I - Widget
                 GUI       *gui)	// I - GUI
{
  if (w == gui->bodyBrowse)
  {
    gui->fc->filter("Image Files (*.{bmp,gif,jpg,png})");
    gui->fc->label("Body Image?");
    gui->fc->type(Fl_File_Chooser::SINGLE);
    gui->fc->show();
    while (gui->fc->shown())
      Fl::wait();

    if (gui->fc->count())
    {
      gui->bodyImage->value(file_localize(gui->fc->value(), NULL));
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::textColorCB()' - Set the text color.
//

void
GUI::textColorCB(Fl_Widget *w,		// I - Widget
                 GUI       *gui)	// I - GUI
{
  uchar	r, g, b;		// Color values
  int	color;			// Color from bar color
  char	newcolor[255];		// New color string


  if (w == gui->textLookup)
  {
    if (sscanf(gui->textColor->value(), "#%x", &color) == 1)
    {
      r = color >> 16;
      g = (color >> 8) & 255;
      b = color & 255;
    }
    else
    {
      r = 0;
      g = 0;
      b = 0;
    }

    if (fl_color_chooser("Text Color?", r, g, b))
    {
      sprintf(newcolor, "#%02x%02x%02x", r, g, b);
      gui->textColor->value(newcolor);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::linkColorCB()' - Set the link color.
//

void
GUI::linkColorCB(Fl_Widget *w,		// I - Widget
                 GUI       *gui)	// I - GUI
{
  uchar	r, g, b;		// Color values
  int	color;			// Color from bar color
  char	newcolor[255];		// New color string


  if (w == gui->linkLookup)
  {
    if (sscanf(gui->linkColor->value(), "#%x", &color) == 1)
    {
      r = color >> 16;
      g = (color >> 8) & 255;
      b = color & 255;
    }
    else
    {
      r = 0;
      g = 0;
      b = 255;
    }

    if (fl_color_chooser("Link Color?", r, g, b))
    {
      sprintf(newcolor, "#%02x%02x%02x", r, g, b);
      gui->linkColor->value(newcolor);
      gui->title(gui->book_filename, 1);
    }
  }
  else
    gui->title(gui->book_filename, 1);
}


//
// 'GUI::helpCB()' - Show on-line help...
//

void
GUI::helpCB(Fl_Widget *w,	// I - Widget
            GUI       *gui)	// I - GUI
{
  char	link[1024];	// filename#link


  REF(w);

  snprintf(link, sizeof(link), "%s/help.html#%s", help_dir,
           gui->tabs->value()->label());
  gui->help->load(link);
  gui->help->show();
}


//
// 'GUI::newBookCB()' - Create a new book.
//

void
GUI::newBookCB(Fl_Widget *w,	// I - Widget
               GUI       *gui)	// I - GUI
{
  REF(w);

  if (!gui->checkSave())
    return;

  gui->newBook();
}


//
// 'GUI::openBookCB()' - Open an existing book.
//

void
GUI::openBookCB(Fl_Widget *w,	// I - Widget
                GUI       *gui)	// I - GUI
{
  REF(w);

  if (!gui->checkSave())
    return;

  gui->fc->filter("Book Files (*.book)");
  gui->fc->label("Book File?");
  gui->fc->type(Fl_File_Chooser::SINGLE);
  gui->fc->show();
  while (gui->fc->shown())
    Fl::wait();

  if (gui->fc->count())
    gui->loadBook(gui->fc->value());
}


//
// 'GUI::saveBookCB()' - Save the current book to disk.
//

void
GUI::saveBookCB(Fl_Widget *w,	// I - Widget
                GUI       *gui)	// I - GUI
{
  if (gui->book_filename[0] == '\0')
    saveAsBookCB(w, gui);
  else
    gui->saveBook(gui->book_filename);
}


//
// 'GUI::saveAsBookCB()' - Save the current book to disk to a new file.
//

void
GUI::saveAsBookCB(Fl_Widget *w,		// I - Widget
                  GUI       *gui)	// I - GUI
{
  const char	*filename;	// Book filename
  char		realname[1024];	// Real filename
  const char	*extension;	// Filename extension
  const char	*newfile;	// New filename
  const char	*dir;		// Book directory


  REF(w);

  gui->fc->filter("Book Files (*.book)");
  gui->fc->label("Book File?");
  gui->fc->type(Fl_File_Chooser::CREATE);
  gui->fc->show();
  while (gui->fc->shown())
    Fl::wait();

  if (gui->fc->count())
  {
    filename = gui->fc->value();

    if (access(filename, 0) == 0)
      if (!fl_ask("File already exists!  OK to overwrite?"))
	return;

    extension = file_extension(filename);
    if (!extension[0])
    {
      // No extension!  Add .book to the name...
      snprintf(realname, sizeof(realname), "%s.book", filename);
      filename = realname;
    }
    else if (strcasecmp(extension, "pdf") == 0 ||
             strcasecmp(extension, "html") == 0 ||
             strcasecmp(extension, "ps") == 0)
    {
      gui->tabs->value(gui->outputTab);

      gui->outputPath->value(file_localize(filename, NULL));
      gui->outputFile->setonly();
      outputTypeCB(gui->outputFile, gui);

      if (strcasecmp(extension, "pdf") == 0)
      {
        gui->typePDF->setonly();
	outputFormatCB(gui->typePDF, gui);
      }
      else if (strcasecmp(extension, "html") == 0)
      {
        gui->typeHTML->setonly();
	outputFormatCB(gui->typeHTML, gui);
      }
      else
      {
        gui->typePS->setonly();
	outputFormatCB(gui->typePS, gui);
      }

      fl_alert("To generate a HTML, PDF, or PS file you must click on "
               "the GENERATE button.  The SAVE and SAVE AS buttons "
	       "save the current book file.");

      return;
    }

    dir = file_directory(filename);

    for (int i = 1; i <= gui->inputFiles->size(); i ++)
    {
      newfile = file_localize(gui->inputFiles->text(i), dir);
      gui->inputFiles->text(i, newfile);
    }

    newfile = file_localize(gui->logoImage->value(), dir);
    gui->logoImage->value(newfile);

    newfile = file_localize(gui->titleImage->value(), dir);
    gui->titleImage->value(newfile);

    newfile = file_localize(gui->bodyImage->value(), dir);
    gui->bodyImage->value(newfile);

    newfile = file_localize(gui->outputPath->value(), dir);
    gui->outputPath->value(newfile);

    chdir(dir);

    gui->saveBook(filename);
  }
}


//
// 'GUI::generateBookCB()' - Generate the current book.
//

void
GUI::generateBookCB(Fl_Widget *w,	// I - Widget
                    GUI       *gui)	// I - GUI
{
  int		i,		// Looping var
	        count;		// Number of files
  char	  	temp[1024];	// Temporary string
  FILE		*docfile;	// Document file
  tree_t	*document,	// Master HTML document
		*file,		// HTML document file
		*toc;		// Table of contents
  const char	*filename;	// HTML filename
  char		base[1024],	// Base directory of HTML file
		bookbase[1024];	// Base directory of book file


  REF(w);

  // Do we have an output filename?
  if (gui->outputPath->size() == 0)
  {
    gui->tabs->value(gui->outputTab);
    gui->outputPath->take_focus();

    fl_alert("You must specify an output directory or filename before "
             "you click on GENERATE.");

    return;
  }

  // Disable the GUI while we generate...
  gui->controls->deactivate();
  gui->window->cursor(FL_CURSOR_WAIT);

  // Set global vars used for converting the HTML files to XYZ format...
  strlcpy(bookbase, file_directory(gui->book_filename), sizeof(bookbase));

  Verbosity = 1;

  gui->loadSettings();

  strlcpy(LogoImage, gui->logoImage->value(), sizeof(LogoImage));
  strlcpy(TitleImage, gui->titleImage->value(), sizeof(TitleImage));
  strlcpy(OutputPath, gui->outputPath->value(), sizeof(OutputPath));

  OutputFiles = gui->outputDirectory->value();

  if (gui->typeBook->value())
    OutputType = OUTPUT_BOOK;
  else if (gui->typeContinuous->value())
    OutputType = OUTPUT_CONTINUOUS;
  else
    OutputType = OUTPUT_WEBPAGES;

  strlcpy(UserPassword, gui->userPassword->value(), sizeof(UserPassword));
  strlcpy(OwnerPassword, gui->ownerPassword->value(), sizeof(OwnerPassword));

  if (gui->typePDF->value())
    PSLevel = 0;
  else if (gui->ps1->value())
    PSLevel = 1;
  else if (gui->ps2->value())
    PSLevel = 2;
  else
    PSLevel = 3;

  _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);

  file_proxy(gui->proxy->value());

  Errors = 0;
  gui->error_list->clear();

 /*
  * Load the input files...
  */

  count    = gui->inputFiles->size();
  document = NULL;

  for (i = 1; i <= count; i ++)
  {
    filename = file_find(Path, gui->inputFiles->text(i));

    if (filename != NULL &&
        (docfile = fopen(filename, "rb")) != NULL)
    {
     /*
      * Read from a file...
      */

      snprintf(temp, sizeof(temp), "Loading \"%s\"...", filename);
      gui->progress(100 * i / count, temp);

      strlcpy(base, file_directory(gui->inputFiles->text(i)), sizeof(base));

      file = htmlAddTree(NULL, MARKUP_FILE, NULL);
      htmlSetVariable(file, (uchar *)"_HD_FILENAME",
                      (uchar *)file_basename(filename));
      htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)base);

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
      progress_error(HD_ERROR_FILE_NOT_FOUND,
                     "Unable to open \"%s\" for reading!",
                     gui->inputFiles->text(i));
  }

 /*
  * We *must* have a document to process...
  */

  if (document == NULL)
    progress_error(HD_ERROR_NO_FILES,
                   "No HTML files to format, cannot generate document!");
  else
  {
   /*
    * Find the first one in the list...
    */

    while (document->prev != NULL)
      document = document->prev;

    // Fix links...
    htmlFixLinks(document, document);

    // Show debug info...
    htmlDebugStats("Document Tree", document);

   /*
    * Build a table of contents for the documents...
    */

    if (OutputType == OUTPUT_BOOK && TocLevels > 0)
      toc = toc_build(document);
    else
      toc = NULL;

   /*
    * Generate the output file(s).
    */

    if (gui->typeHTML->value())
      html_export(document, toc);
    else if (gui->typeHTMLSep->value())
      htmlsep_export(document, toc);
    else
      pspdf_export(document, toc);

    htmlDeleteTree(document);
    htmlDeleteTree(toc);

    file_cleanup();
    image_flush_cache();
  }

  if (Errors == 0)
    fl_message("Document generated successfully!");
  else if (fl_ask("%d error%s occurred while generating document.\nWould you like to see the list?",
                  Errors, Errors == 1 ? "" : "s"))
  {
    gui->error_window->show();

    while (gui->error_window->shown())
      Fl::wait();
  }

  gui->controls->activate();
  gui->window->cursor(FL_CURSOR_DEFAULT);
  gui->progress(0);
}


//
// 'GUI::closeBookCB()' - Close the current book.
//

void
GUI::closeBookCB(Fl_Widget *w,		// I - Widget
                 GUI       *gui)	// I - GUI
{
  REF(w);

  if (gui->checkSave())
    gui->hide();
}


//
// 'GUI::errorCB()' - Close the error window.
//

void
GUI::errorCB(Fl_Widget *w,		// I - Widget
             GUI       *gui)		// I - GUI
{
  REF(w);

  gui->error_window->hide();
}


#endif // HAVE_LIBFLTK

//
// End of "$Id: gui.cxx,v 1.36.2.73 2004/05/27 20:13:13 mike Exp $".
//
