//
// "$Id: gui.cxx,v 1.32 2000/06/05 03:18:23 mike Exp $"
//
//   GUI routines for HTMLDOC, an HTML document processing program.
//
//   Copyright 1997-2000 by Easy Software Products.
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
//       Hollywood, Maryland 20636-3111 USA
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
//   GUI::newBook()        - Clear out the current GUI settings for a new book.
//   GUI::loadBook()       - Load a book file from disk.
//   GUI::saveBook()       - Save a book to disk.
//   GUI::checkSave()      - Check to see if a save is needed.
//   GUI::changeCB()       - Mark the current book as changed.
//   GUI::docTypeCB()      - Handle input on the document type buttons.
//   GUI::inputFilesCB()   - Handle selections in the input files browser.
//   GUI::addFileCB()      - Add a file to the input files list.
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
//

#include "htmldoc.h"

#ifdef HAVE_LIBFLTK

//
// Include necessary headers.
//

#  include <ctype.h>
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
#    include "htmldoc.xbm"
#  endif // WIN32


const char	*GUI::help_dir = DOCUMENTATION;


//
// 'GUI()' - Build the HTMLDOC GUI and load the indicated book as necessary.
//

GUI::GUI(const char *filename)		// Book file to load initially
{
  Fl_Group		*group;		// Group
  Fl_Button		*button;	// Push button
  Fl_Box		*label;		// Label box
  static char		*htmldoc[1] = { "htmldoc" };	// argv[] array
  static Fl_Menu	sizeMenu[] =	// Menu items for page size button */
			{
			  {"A4", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Letter", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Universal", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	tocMenu[] =	// Menu items for TOC chooser
			{
			  {"None", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1 level", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"2 levels", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"3 levels", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"4 Levels", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	formatMenu[] =	// Menu items for header/footer choosers
			{
			  {"Blank", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Title", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Chapter Title", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Heading", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Logo", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"1,2,3,...", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"i,ii,iii,...", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"I,II,III,...", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Ch Page", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	typefaceMenu[] = // Menu items for typeface choosers
			{
			  {"Courier", 0,  0, 0, 0, 0, FL_COURIER, 14, 0},
			  {"Times", 0,  0, 0, 0, 0, FL_TIMES, 14, 0},
			  {"Helvetica", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	fontMenu[] =	// Menu items for font choosers
			{
			  {"Courier", 0,  0, 0, 0, 0, FL_COURIER, 14, 0},
			  {"Courier-Bold", 0,  0, 0, 0, 0, FL_COURIER_BOLD, 14, 0},
			  {"Courier-Oblique", 0,  0, 0, 0, 0, FL_COURIER_ITALIC, 14, 0},
			  {"Courier-BoldOblique", 0,  0, 0, 0, 0, FL_COURIER_BOLD_ITALIC, 14, 0},
			  {"Times-Roman", 0,  0, 0, 0, 0, FL_TIMES, 14, 0},
			  {"Times-Bold", 0,  0, 0, 0, 0, FL_TIMES_BOLD, 14, 0},
			  {"Times-Italic", 0,  0, 0, 0, 0, FL_TIMES_ITALIC, 14, 0},
			  {"Times-BoldItalic", 0,  0, 0, 0, 0, FL_TIMES_BOLD_ITALIC, 14, 0},
			  {"Helvetica", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Helvetica-Bold", 0,  0, 0, 0, 0, FL_HELVETICA_BOLD, 14, 0},
			  {"Helvetica-Oblique", 0,  0, 0, 0, 0, FL_HELVETICA_ITALIC, 14, 0},
			  {"Helvetica-BoldOblique", 0,  0, 0, 0, 0, FL_HELVETICA_BOLD_ITALIC, 14, 0},
			  {0}
			};
  static Fl_Menu	charsetMenu[] =	// Menu items for charset chooser
			{
			  {"8859-1", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-2", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-3", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-4", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-5", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-6", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-7", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-8", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-9", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-14", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"8859-15", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	modeMenu[] =	// Menu items for mode chooser
			{
			  {"Document", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Outline", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Full-Screen", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	layoutMenu[] =	// Menu items for layout chooser
			{
			  {"Single", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"One Column", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Two Column Left", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Two Column Right", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	firstMenu[] =	// Menu items for first chooser
			{
			  {"Page 1", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"TOC", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Chapter 1", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};
  static Fl_Menu	effectMenu[] =	// Menu items for effect chooser
			{
			  {"None", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Box Inward", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Box Outward", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Dissolve", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Down", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Down+Right", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Glitter Right", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Blinds", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Sweep Inward", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Horizontal Sweep Outward", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Blinds", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Sweep Inward", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Vertical Sweep Outward ", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Down", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Left", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Right", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {"Wipe Up", 0,  0, 0, 0, 0, FL_HELVETICA, 14, 0},
			  {0}
			};


  //
  // Create a dialog window...
  //

  window = new Fl_Window(470, 390, "HTMLDOC " SVERSION);
  window->callback((Fl_Callback *)closeBookCB, this);

  controls = new Fl_Group(0, 0, 470, 360);
  tabs     = new Fl_Tabs(10, 10, 450, 260);

  //
  // Input tab...
  //

  inputTab = new Fl_Group(10, 35, 450, 220, "Input");

  group = new Fl_Group(140, 45, 150, 20, "Document Type: ");
  group->align(FL_ALIGN_LEFT);
    typeBook = new CheckButton(140, 45, 60, 20, "Book");
    typeBook->type(FL_RADIO_BUTTON);
    typeBook->setonly();
    typeBook->callback((Fl_Callback *)docTypeCB, this);

    typeWebPage = new CheckButton(200, 45, 90, 20, "Web Page");
    typeWebPage->type(FL_RADIO_BUTTON);
    typeWebPage->callback((Fl_Callback *)docTypeCB, this);
  group->end();

  group = new Fl_Group(140, 70, 215, 20, "Input Files: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  inputFiles = new FileBrowser(140, 70, 215, 125);
  inputFiles->iconsize(20);
  inputFiles->type(FL_MULTI_BROWSER);
  inputFiles->callback((Fl_Callback *)inputFilesCB, this);
  inputFiles->when(FL_WHEN_RELEASE | FL_WHEN_NOT_CHANGED);

  addFile = new Fl_Button(355, 70, 95, 25, "Add Files...");
  addFile->callback((Fl_Callback *)addFileCB, this);

  editFile = new Fl_Button(355, 95, 95, 25, "Edit Files...");
  editFile->deactivate();
  editFile->callback((Fl_Callback *)editFilesCB, this);

  deleteFile = new Fl_Button(355, 120, 95, 25, "Delete Files");
  deleteFile->deactivate();
  deleteFile->callback((Fl_Callback *)deleteFilesCB, this);

  moveUpFile = new Fl_Button(355, 145, 95, 25, "Move Up");
  moveUpFile->deactivate();
  moveUpFile->callback((Fl_Callback *)moveUpFilesCB, this);

  moveDownFile = new Fl_Button(355, 170, 95, 25, "Move Down");
  moveDownFile->deactivate();
  moveDownFile->callback((Fl_Callback *)moveDownFilesCB, this);

  logoImage = new Fl_Input(140, 205, 230, 25, "Logo Image: ");
  logoImage->when(FL_WHEN_CHANGED);
  logoImage->callback((Fl_Callback *)logoImageCB, this);

  logoBrowse = new Fl_Button(370, 205, 80, 25, "Browse...");
  logoBrowse->callback((Fl_Callback *)logoImageCB, this);

  titleImage = new Fl_Input(140, 235, 230, 25, "Title File/Image: ");
  titleImage->when(FL_WHEN_CHANGED);
  titleImage->callback((Fl_Callback *)titleImageCB, this);

  titleBrowse = new Fl_Button(370, 235, 80, 25, "Browse...");
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
    outputFile = new CheckButton(140, 45, 50, 20, "File");
    outputFile->type(FL_RADIO_BUTTON);
    outputFile->setonly();
    outputFile->callback((Fl_Callback *)outputTypeCB, this);

    outputDirectory = new CheckButton(190, 45, 105, 20, "Directory");
    outputDirectory->type(FL_RADIO_BUTTON);
    outputDirectory->callback((Fl_Callback *)outputTypeCB, this);
  group->end();

  outputPath = new Fl_Input(140, 70, 230, 25, "Output Path: ");
  outputPath->when(FL_WHEN_CHANGED);
  outputPath->callback((Fl_Callback *)outputPathCB, this);

  outputBrowse = new Fl_Button(370, 70, 80, 25, "Browse...");
  outputBrowse->callback((Fl_Callback *)outputPathCB, this);

  group = new Fl_Group(140, 100, 255, 20, "Output Format: ");
  group->align(FL_ALIGN_LEFT);
    typeHTML = new CheckButton(140, 100, 65, 20, "HTML");
    typeHTML->type(FL_RADIO_BUTTON);
    typeHTML->setonly();
    typeHTML->callback((Fl_Callback *)outputFormatCB, this);

    typePS = new CheckButton(205, 100, 45, 20, "PS");
    typePS->type(FL_RADIO_BUTTON);
    typePS->callback((Fl_Callback *)outputFormatCB, this);

    typePDF = new CheckButton(250, 100, 55, 20, "PDF");
    typePDF->type(FL_RADIO_BUTTON);
    typePDF->callback((Fl_Callback *)outputFormatCB, this);
  group->end();

  group = new Fl_Group(140, 125, 265, 20, "Output Options: ");
  group->align(FL_ALIGN_LEFT);
  group->end();

  grayscale = new CheckButton(140, 125, 90, 20, "Grayscale");
  grayscale->callback((Fl_Callback *)changeCB, this);

  titlePage = new CheckButton(230, 125, 90, 20, "Title Page");
  titlePage->callback((Fl_Callback *)changeCB, this);

  jpegCompress = new CheckButton(320, 125, 140, 20, "JPEG Big Images");
  jpegCompress->callback((Fl_Callback *)jpegCB, this);

  compGroup = new Fl_Group(140, 150, 310, 40, "Compression: \n ");
  compGroup->align(FL_ALIGN_LEFT);

    compression = new Fl_Slider(140, 150, 310, 20);
    compression->type(FL_HOR_NICE_SLIDER);
    compression->minimum(0.0);
    compression->maximum(9.0);
    compression->value(1.0);
    compression->step(1.0);
    compression->callback((Fl_Callback *)changeCB, this);

    label = new Fl_Box(140, 170, 30, 10, "None");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(170, 170, 30, 10, "Fast");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(420, 170, 30, 10, "Best");
    label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    label->labelsize(10);

  compGroup->end();

  jpegGroup = new Fl_Group(140, 185, 310, 40, "JPEG Quality: \n ");
  jpegGroup->align(FL_ALIGN_LEFT);

    jpegQuality = new Fl_Value_Slider(140, 185, 310, 20);
    jpegQuality->type(FL_HOR_NICE_SLIDER);
    jpegQuality->minimum(50.0);
    jpegQuality->maximum(100.0);
    jpegQuality->value(90.0);
    jpegQuality->step(1.0);
    jpegQuality->callback((Fl_Callback *)changeCB, this);

    label = new Fl_Box(175, 205, 40, 10, "Good");
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->labelsize(10);

    label = new Fl_Box(410, 205, 40, 10, "Best");
    label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    label->labelsize(10);

  jpegGroup->end();

  outputTab->end();

  //
  // Page tab...
  //

  pageTab = new Fl_Group(10, 35, 450, 220, "Page");
  pageTab->hide();

  pageSize = new Fl_Input(140, 45, 100, 25, "Page Size: ");
  pageSize->when(FL_WHEN_CHANGED);
  pageSize->callback((Fl_Callback *)changeCB, this);

  pageSizeMenu = new Fl_Menu_Button(240, 45, 25, 25, "");
  pageSizeMenu->menu(sizeMenu);
  pageSizeMenu->callback((Fl_Callback *)sizeCB, this);

  pageDuplex = new CheckButton(270, 48, 70, 20, "2-Sided");
  pageDuplex->callback((Fl_Callback *)changeCB, this);

  landscape = new CheckButton(345, 48, 90, 20, "Landscape");
  landscape->callback((Fl_Callback *)changeCB, this);

  pageTop = new Fl_Input(225, 75, 60, 25, "Top");
  pageTop->when(FL_WHEN_CHANGED);
  pageTop->callback((Fl_Callback *)changeCB, this);

  pageLeft = new Fl_Input(190, 105, 60, 25, "Left");
  pageLeft->when(FL_WHEN_CHANGED);
  pageLeft->callback((Fl_Callback *)changeCB, this);

  pageRight = new Fl_Input(255, 105, 60, 25, "Right");
  pageRight->when(FL_WHEN_CHANGED);
  pageRight->align(FL_ALIGN_RIGHT);
  pageRight->callback((Fl_Callback *)changeCB, this);

  pageBottom = new Fl_Input(225, 135, 60, 25, "Bottom");
  pageBottom->when(FL_WHEN_CHANGED);
  pageBottom->callback((Fl_Callback *)changeCB, this);

  pageHeaderLeft = new Fl_Choice(140, 165, 100, 25, "Header: ");
  pageHeaderLeft->menu(formatMenu);
  pageHeaderLeft->callback((Fl_Callback *)changeCB, this);

  pageHeaderCenter = new Fl_Choice(245, 165, 100, 25);
  pageHeaderCenter->menu(formatMenu);
  pageHeaderCenter->callback((Fl_Callback *)changeCB, this);

  pageHeaderRight = new Fl_Choice(350, 165, 100, 25);
  pageHeaderRight->menu(formatMenu);
  pageHeaderRight->callback((Fl_Callback *)changeCB, this);

  pageFooterLeft = new Fl_Choice(140, 195, 100, 25, "Footer: ");
  pageFooterLeft->menu(formatMenu);
  pageFooterLeft->callback((Fl_Callback *)changeCB, this);

  pageFooterCenter = new Fl_Choice(245, 195, 100, 25);
  pageFooterCenter->menu(formatMenu);
  pageFooterCenter->callback((Fl_Callback *)changeCB, this);

  pageFooterRight = new Fl_Choice(350, 195, 100, 25);
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
  tocLevels->callback((Fl_Callback *)tocCB, this);

  numberedToc = new CheckButton(245, 47, 160, 20, "Numbered Headings");
  numberedToc->callback((Fl_Callback *)changeCB, this);

  tocHeader = new Fl_Group(140, 75, 310, 25, "Header: ");
  tocHeader->align(FL_ALIGN_LEFT);

    tocHeaderLeft = new Fl_Choice(140, 75, 100, 25);
    tocHeaderLeft->menu(formatMenu);
    tocHeaderLeft->callback((Fl_Callback *)changeCB, this);

    tocHeaderCenter = new Fl_Choice(245, 75, 100, 25);
    tocHeaderCenter->menu(formatMenu);
    tocHeaderCenter->callback((Fl_Callback *)changeCB, this);

    tocHeaderRight = new Fl_Choice(350, 75, 100, 25);
    tocHeaderRight->menu(formatMenu);
    tocHeaderRight->callback((Fl_Callback *)changeCB, this);

  tocHeader->end();

  tocFooter = new Fl_Group(140, 105, 310, 25, "Footer: ");
  tocFooter->align(FL_ALIGN_LEFT);

    tocFooterLeft = new Fl_Choice(140, 105, 100, 25, "Footer: ");
    tocFooterLeft->menu(formatMenu);
    tocFooterLeft->callback((Fl_Callback *)changeCB, this);

    tocFooterCenter = new Fl_Choice(245, 105, 100, 25);
    tocFooterCenter->menu(formatMenu);
    tocFooterCenter->callback((Fl_Callback *)changeCB, this);

    tocFooterRight = new Fl_Choice(350, 105, 100, 25);
    tocFooterRight->menu(formatMenu);
    tocFooterRight->callback((Fl_Callback *)changeCB, this);

  tocFooter->end();

  tocTitle = new Fl_Input(140, 135, 310, 25, "Title: ");
  tocTitle->when(FL_WHEN_CHANGED);
  tocTitle->callback((Fl_Callback *)changeCB, this);

  tocTab->end();

  //
  // Colors tab...
  //

  colorsTab = new Fl_Group(10, 35, 450, 220, "Colors");
  colorsTab->hide();

  bodyColor = new Fl_Input(140, 45, 100, 25, "Body Color: ");
  bodyColor->when(FL_WHEN_CHANGED);
  bodyColor->callback((Fl_Callback *)bodyColorCB, this);

  bodyLookup = new Fl_Button(240, 45, 80, 25, "Lookup...");
  bodyLookup->callback((Fl_Callback *)bodyColorCB, this);

  bodyImage = new Fl_Input(140, 75, 230, 25, "Body Image: ");
  bodyImage->when(FL_WHEN_CHANGED);
  bodyImage->callback((Fl_Callback *)bodyImageCB, this);

  bodyBrowse = new Fl_Button(370, 75, 80, 25, "Browse...");
  bodyBrowse->callback((Fl_Callback *)bodyImageCB, this);

  textColor = new Fl_Input(140, 105, 100, 25, "Text Color: ");
  textColor->when(FL_WHEN_CHANGED);
  textColor->callback((Fl_Callback *)textColorCB, this);

  textLookup = new Fl_Button(240, 105, 80, 25, "Lookup...");
  textLookup->callback((Fl_Callback *)textColorCB, this);

  linkColor = new Fl_Input(140, 135, 100, 25, "Link Color: ");
  linkColor->when(FL_WHEN_CHANGED);
  linkColor->callback((Fl_Callback *)linkColorCB, this);

  linkLookup = new Fl_Button(240, 135, 80, 25, "Lookup...");
  linkLookup->callback((Fl_Callback *)linkColorCB, this);

  linkStyle = new Fl_Choice(140, 165, 100, 25, "Link Style: ");
  linkStyle->add("Plain");
  linkStyle->add("Underline");
  linkStyle->callback((Fl_Callback *)changeCB, this);

  colorsTab->end();

  //
  // Fonts tab...
  //

  fontsTab = new Fl_Group(10, 35, 450, 220, "Fonts");
  fontsTab->hide();

  fontBaseSize = new Fl_Counter(200, 45, 150, 25, "Base Font Size: ");
  fontBaseSize->callback((Fl_Callback *)changeCB, this);
  fontBaseSize->minimum(4.0);
  fontBaseSize->maximum(24.0);
  fontBaseSize->step(0.1);
  fontBaseSize->value(11.0);
  fontBaseSize->align(FL_ALIGN_LEFT);

  fontSpacing = new Fl_Counter(200, 75, 150, 25, "Line Spacing: ");
  fontSpacing->callback((Fl_Callback *)changeCB, this);
  fontSpacing->minimum(1.0);
  fontSpacing->maximum(3.0);
  fontSpacing->step(0.1);
  fontSpacing->value(1.2);
  fontSpacing->align(FL_ALIGN_LEFT);

  bodyFont = new Fl_Choice(200, 105, 100, 25, "Body Typeface: ");
  bodyFont->menu(typefaceMenu);
  bodyFont->callback((Fl_Callback *)changeCB, this);

  headingFont = new Fl_Choice(200, 135, 100, 25, "Heading Typeface: ");
  headingFont->menu(typefaceMenu);
  headingFont->callback((Fl_Callback *)changeCB, this);

  headFootSize = new Fl_Counter(200, 165, 150, 25, "Header/Footer Size: ");
  headFootSize->callback((Fl_Callback *)changeCB, this);
  headFootSize->minimum(4.0);
  headFootSize->maximum(24.0);
  headFootSize->step(0.1);
  headFootSize->value(11.0);
  headFootSize->align(FL_ALIGN_LEFT);

  headFootFont = new Fl_Choice(200, 195, 220, 25, "Header/Footer Font: ");
  headFootFont->menu(fontMenu);
  headFootFont->callback((Fl_Callback *)changeCB, this);

  charset = new Fl_Choice(200, 225, 90, 25, "Character Set: ");
  charset->menu(charsetMenu);
  charset->callback((Fl_Callback *)changeCB, this);

  fontsTab->end();

  //
  // PostScript tab...
  //

  psTab = new Fl_Group(10, 35, 450, 220, "PS");
  psTab->hide();

  psLevel = new Fl_Group(140, 45, 310, 20, "PostScript: ");
  psLevel->align(FL_ALIGN_LEFT);

    ps1 = new CheckButton(140, 45, 70, 20, "Level 1");
    ps1->type(FL_RADIO_BUTTON);
    ps1->callback((Fl_Callback *)psCB, this);

    ps2 = new CheckButton(210, 45, 70, 20, "Level 2");
    ps2->type(FL_RADIO_BUTTON);
    ps2->callback((Fl_Callback *)psCB, this);

    ps3 = new CheckButton(280, 45, 70, 20, "Level 3");
    ps3->type(FL_RADIO_BUTTON);
    ps3->callback((Fl_Callback *)psCB, this);

  psLevel->end();

  psCommands = new CheckButton(140, 70, 310, 20, "Send Printer Commands");
  psCommands->callback((Fl_Callback *)changeCB, this);

  psTab->end();

  //
  // PDF tab...
  //

  pdfTab = new Fl_Group(10, 35, 450, 220, "PDF");
  pdfTab->hide();

  pdfVersion = new Fl_Group(140, 45, 310, 40, "PDF Version: \n ");
  pdfVersion->align(FL_ALIGN_LEFT);

    pdf11 = new CheckButton(140, 45, 125, 20, "1.1 (Acrobat 2.x)");
    pdf11->type(FL_RADIO_BUTTON);
    pdf11->callback((Fl_Callback *)pdfCB, this);

    pdf12 = new CheckButton(270, 45, 125, 20, "1.2 (Acrobat 3.0)");
    pdf12->type(FL_RADIO_BUTTON);
    pdf12->callback((Fl_Callback *)pdfCB, this);

    pdf13 = new CheckButton(140, 65, 125, 20, "1.3 (Acrobat 4.0)");
    pdf13->type(FL_RADIO_BUTTON);
    pdf13->callback((Fl_Callback *)pdfCB, this);

  pdfVersion->end();

  pageMode = new Fl_Choice(140, 90, 120, 25, "Page Mode: ");
  pageMode->menu(modeMenu);
  pageMode->callback((Fl_Callback *)changeCB, this);

  pageLayout = new Fl_Choice(140, 120, 150, 25, "Page Layout: ");
  pageLayout->menu(layoutMenu);
  pageLayout->callback((Fl_Callback *)changeCB, this);

  firstPage = new Fl_Choice(140, 150, 100, 25, "First Page: ");
  firstPage->menu(firstMenu);
  firstPage->callback((Fl_Callback *)changeCB, this);

  pageEffect = new Fl_Choice(140, 180, 210, 25, "Page Effect: ");
  pageEffect->menu(effectMenu);
  pageEffect->callback((Fl_Callback *)effectCB, this);

  pageDuration = new Fl_Value_Slider(140, 210, 310, 20, "Page Duration: ");
  pageDuration->align(FL_ALIGN_LEFT);
  pageDuration->type(FL_HOR_NICE_SLIDER);
  pageDuration->minimum(1.0);
  pageDuration->maximum(60.0);
  pageDuration->value(10.0);
  pageDuration->step(1.0);
  pageDuration->callback((Fl_Callback *)changeCB, this);

  effectDuration = new Fl_Value_Slider(140, 235, 310, 20, "Effect Duration: ");
  effectDuration->align(FL_ALIGN_LEFT);
  effectDuration->type(FL_HOR_NICE_SLIDER);
  effectDuration->minimum(0.5);
  effectDuration->maximum(5.0);
  effectDuration->value(1.0);
  effectDuration->step(0.1);
  effectDuration->callback((Fl_Callback *)changeCB, this);

  pdfTab->end();

  //
  // Security tab...
  //

  securityTab = new Fl_Group(10, 35, 450, 220, "Security");
  securityTab->hide();

  encryption = new Fl_Group(140, 45, 310, 20, "Encryption: ");
  encryption->align(FL_ALIGN_LEFT);

    encryptionNo = new CheckButton(140, 45, 40, 20, "No");
    encryptionNo->type(FL_RADIO_BUTTON);
    encryptionNo->set();
    encryptionNo->callback((Fl_Callback *)encryptionCB, this);

    encryptionYes = new CheckButton(180, 45, 45, 20, "Yes");
    encryptionYes->type(FL_RADIO_BUTTON);
    encryptionYes->callback((Fl_Callback *)encryptionCB, this);

  encryption->end();

  permissions = new Fl_Group(140, 70, 310, 40, "Permissions: ");
  permissions->align(FL_ALIGN_LEFT);

    permPrint    = new CheckButton(140, 70, 80, 20, "Print");
    permModify   = new CheckButton(220, 70, 80, 20, "Modify");
    permCopy     = new CheckButton(140, 90, 80, 20, "Copy");
    permAnnotate = new CheckButton(220, 90, 80, 20, "Annotate");

  permissions->end();

  ownerPassword = new Fl_Secret_Input(140, 115, 150, 25, "Owner Password: ");
  ownerPassword->maximum_size(32);

  userPassword = new Fl_Secret_Input(140, 145, 150, 25, "User Password: ");
  userPassword->maximum_size(32);

  securityTab->end();

  //
  // Options tab...
  //

  optionsTab = new Fl_Group(10, 35, 450, 220, "Options");
  optionsTab->hide();

  htmlEditor = new Fl_Input(140, 45, 215, 25, "HTML Editor: ");
  htmlEditor->value(HTMLEditor);
  htmlEditor->when(FL_WHEN_CHANGED);
  htmlEditor->callback((Fl_Callback *)htmlEditorCB, this);

  htmlBrowse = new Fl_Button(355, 45, 95, 25, "Browse...");
  htmlBrowse->callback((Fl_Callback *)htmlEditorCB, this);

  browserWidth = new Fl_Value_Slider(140, 75, 310, 20, "Browser Width: ");
  browserWidth->align(FL_ALIGN_LEFT);
  browserWidth->type(FL_HOR_NICE_SLIDER);
  browserWidth->minimum(400.0);
  browserWidth->maximum(1200.0);
  browserWidth->value(_htmlBrowserWidth);
  browserWidth->step(5.0);
  browserWidth->callback((Fl_Callback *)changeCB, this);

  saveOptions = new Fl_Button(260, 235, 190, 25, "Save Options and Defaults");
  saveOptions->callback((Fl_Callback *)saveOptionsCB, this);

  optionsTab->end();

  tabs->end();

  //
  // Button bar...
  //

  button = new Fl_Button(10, 330, 50, 25, "Help");
  button->shortcut(FL_F + 1);
  button->callback((Fl_Callback *)helpCB, this);

  button = new Fl_Button(65, 330, 45, 25, "New");
  button->shortcut(FL_CTRL | 'n');
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
          "HTMLDOC " SVERSION " Copyright 1997-2000 by Easy Software Products "
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

  progressBar = new Progress(10, 360, 450, 20, "HTMLDOC " SVERSION " Ready.");

  window->end();

  // Set the class name to "htmldoc".
  window->xclass("htmldoc");

#  ifdef WIN32
  // Load the HTMLDOC icon image...
  window->icon((char *)LoadImage(fl_display, MAKEINTRESOURCE(IDI_ICON),
                                 IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
#  else // X11
  // Open the X display and load the HTMLDOC icon image...
  fl_open_display();
  window->icon((char *)XCreateBitmapFromData(fl_display,
               DefaultRootWindow(fl_display), htmldoc_bits,
	       htmldoc_width, htmldoc_height));
#  endif // WIN32

  window->resizable(tabs);
  window->size_range(470, 390);
  window->show(1, htmldoc);

  fc = new FileChooser(".", "*", FileChooser::SINGLE, "Title");
  fc->iconsize(20);

  help = new HelpDialog();

  if (!FileIcon::first())
    FileIcon::load_system_icons();

  icon = FileIcon::find("file.html", FileIcon::PLAIN);

  // Use cheesy hardcoded "style" stuff until FLTK 2.0...
#  if FL_MAJOR_VERSION < 2
#    ifdef __sgi
  fc->color((Fl_Color)196);
  inputFiles->color((Fl_Color)196);
#    elif defined(WIN32)
  pageSizeMenu->down_box(FL_FLAT_BOX);
  pageSizeMenu->selection_color((Fl_Color)137);
  pageHeaderLeft->down_box(FL_FLAT_BOX);
  pageHeaderLeft->selection_color((Fl_Color)137);
  pageHeaderCenter->down_box(FL_FLAT_BOX);
  pageHeaderCenter->selection_color((Fl_Color)137);
  pageHeaderRight->down_box(FL_FLAT_BOX);
  pageHeaderRight->selection_color((Fl_Color)137);
  pageFooterLeft->down_box(FL_FLAT_BOX);
  pageFooterLeft->selection_color((Fl_Color)137);
  pageFooterCenter->down_box(FL_FLAT_BOX);
  pageFooterCenter->selection_color((Fl_Color)137);
  pageFooterRight->down_box(FL_FLAT_BOX);
  pageFooterRight->selection_color((Fl_Color)137);

  tocLevels->down_box(FL_FLAT_BOX);
  tocLevels->selection_color((Fl_Color)137);
  tocHeaderLeft->down_box(FL_FLAT_BOX);
  tocHeaderLeft->selection_color((Fl_Color)137);
  tocHeaderCenter->down_box(FL_FLAT_BOX);
  tocHeaderCenter->selection_color((Fl_Color)137);
  tocHeaderRight->down_box(FL_FLAT_BOX);
  tocHeaderRight->selection_color((Fl_Color)137);
  tocFooterLeft->down_box(FL_FLAT_BOX);
  tocFooterLeft->selection_color((Fl_Color)137);
  tocFooterCenter->down_box(FL_FLAT_BOX);
  tocFooterCenter->selection_color((Fl_Color)137);
  tocFooterRight->down_box(FL_FLAT_BOX);
  tocFooterRight->selection_color((Fl_Color)137);

  headingFont->down_box(FL_FLAT_BOX);
  headingFont->selection_color((Fl_Color)137);
  bodyFont->down_box(FL_FLAT_BOX);
  bodyFont->selection_color((Fl_Color)137);
  headFootFont->down_box(FL_FLAT_BOX);
  headFootFont->selection_color((Fl_Color)137);
  charset->down_box(FL_FLAT_BOX);
  charset->selection_color((Fl_Color)137);

  pageMode->down_box(FL_FLAT_BOX);
  pageMode->selection_color((Fl_Color)137);
  pageLayout->down_box(FL_FLAT_BOX);
  pageLayout->selection_color((Fl_Color)137);
  firstPage->down_box(FL_FLAT_BOX);
  firstPage->selection_color((Fl_Color)137);
  pageEffect->down_box(FL_FLAT_BOX);
  pageEffect->selection_color((Fl_Color)137);
#    endif // __sgi
#  endif // FL_MAJOR_VERSION < 2

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
}


//
// 'GUI::show()' - Display the window.
//

void
GUI::show(void)
{
  static char	*htmldoc[1] = { "htmldoc" };	// argv[] array


  window->show(1, htmldoc);
}


//
// 'GUI::progress()' - Update the progress bar on the GUI.
//

void
GUI::progress(int  percent,	// I - Percent complete
              char *text)	// I - Text prompt
{
  if (text != NULL)
    progressBar->label(text);
  else if (percent == 0)
    progressBar->label("HTMLDOC " SVERSION " Ready.");

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
  char	size[255];	// Page size string
  char	formats[256];	// Format characters


  prefs_load();

  if (OutputBook)
  {
    typeBook->setonly();
    docTypeCB(typeBook, this);
  }
  else
  {
    typeWebPage->setonly();
    docTypeCB(typeWebPage, this);
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
  formats['t'] = 1;
  formats['c'] = 2;
  formats['h'] = 3;
  formats['l'] = 4;
  formats['1'] = 5;
  formats['i'] = 6;
  formats['I'] = 7;
  formats['C'] = 8;

  pageHeaderLeft->value(formats[Header[0]]);
  pageHeaderCenter->value(formats[Header[1]]);
  pageHeaderRight->value(formats[Header[2]]);

  pageFooterLeft->value(formats[Footer[0]]);
  pageFooterCenter->value(formats[Footer[1]]);
  pageFooterRight->value(formats[Footer[2]]);

  tocLevels->value(TocLevels);
  numberedToc->value(TocNumbers);

  tocHeaderLeft->value(formats[TocHeader[0]]);
  tocHeaderCenter->value(formats[TocHeader[1]]);
  tocHeaderRight->value(formats[TocHeader[2]]);

  tocFooterLeft->value(formats[TocFooter[0]]);
  tocFooterCenter->value(formats[TocFooter[1]]);
  tocFooterRight->value(formats[TocFooter[2]]);

  tocTitle->value(TocTitle);

  headingFont->value(_htmlHeadingFont);
  bodyFont->value(_htmlBodyFont);
  headFootFont->value(HeadFootType * 4 + HeadFootStyle);

  fontBaseSize->value(_htmlSizes[SIZE_P]);
  fontSpacing->value(_htmlSpacings[SIZE_P] / _htmlSizes[SIZE_P]);
  headFootSize->value(HeadFootSize);

  compression->value(Compression);
  compGroup->deactivate();

  jpegCompress->value(OutputJPEG > 0);
  jpegQuality->value(OutputJPEG > 0 ? OutputJPEG : 90);
  jpegGroup->deactivate();

  pdfTab->deactivate();

  if (PDFVersion < 1.2)
  {
    pdf11->setonly();
    pdfCB(pdf11, this);
  }
  else if (PDFVersion < 1.3)
  {
    pdf12->setonly();
    pdfCB(pdf12, this);
  }
  else
  {
    pdf13->setonly();
    pdfCB(pdf13, this);
  }

  pageMode->value(PDFPageMode);

  pageLayout->value(PDFPageLayout);

  firstPage->value(PDFFirstPage);

  pageEffect->value(PDFEffect);
  effectCB(pageEffect, this);

  pageDuration->value(PDFPageDuration);

  effectDuration->value(PDFEffectDuration);

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

  browserWidth->value(_htmlBrowserWidth);

  title(NULL, 0);

  return (1);
}


//
// 'GUI::loadBook()' - Load a book file from disk.
//

int					// O - 1 = success, 0 = fail
GUI::loadBook(const char *filename)	// I - Name of book file
{
  int		i,
		count;
  FILE		*fp;
  char		line[10240],
		*lineptr,
		temp[1024],
		temp2[1024],
		*tempptr;
  char		formats[256];
  static char	*types[] =		// Typeface names...
		{ "Courier", "Times", "Helvetica" };
  static char	*fonts[] =		// Font names...
		{
		  "Courier", "Courier-Bold", "Courier-Oblique",
		  "Courier-BoldOblique", "Times-Roman", "Times-Bold",
		  "Times-Italic", "Times-BoldItalic",
		  "Helvetica", "Helvetica-Bold",
		  "Helvetica-Oblique", "Helvetica-BoldOblique"
		};


  //
  // Initialize the format character lookup table...
  //

  memset(formats, 0, sizeof(formats));
  formats['t'] = 1;
  formats['c'] = 2;
  formats['h'] = 3;
  formats['l'] = 4;
  formats['1'] = 5;
  formats['i'] = 6;
  formats['I'] = 7;
  formats['C'] = 8;

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
    fc->directory(".");

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

    inputFiles->add(line, icon);
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
    else if (strcmp(temp, "--title") == 0 &&
             (*lineptr == '-' || !*lineptr))
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
      else if (strcmp(temp2, "pdf") == 0 ||
               strcmp(temp2, "pdf12") == 0)
      {
        typePDF->setonly();
	pdf12->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf12, this);
      }
      else if (strcmp(temp2, "pdf13") == 0)
      {
        typePDF->setonly();
	pdf13->setonly();
	outputFormatCB(typePDF, this);
	pdfCB(pdf13, this);
      }
    }
    else if (strcmp(temp, "--logo") == 0 ||
             strcmp(temp, "--logoimage") == 0)
      logoImage->value(temp2);
    else if (strcmp(temp, "--titleimage") == 0)
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
      for (i = 0; i < (sizeof(PDFModes) / sizeof(PDFModes[0])); i ++)
        if (strcasecmp(temp2, PDFModes[i]) == 0)
	{
	  pageMode->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--pagelayout") == 0)
    {
      for (i = 0; i < (sizeof(PDFLayouts) / sizeof(PDFLayouts[0])); i ++)
        if (strcasecmp(temp2, PDFLayouts[i]) == 0)
	{
	  pageLayout->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--firstpage") == 0)
    {
      for (i = 0; i < (sizeof(PDFPages) / sizeof(PDFPages[0])); i ++)
        if (strcasecmp(temp2, PDFPages[i]) == 0)
	{
	  firstPage->value(i);
	  break;
	}
    }
    else if (strcmp(temp, "--pageeffect") == 0)
    {
      for (i = 0; i < (sizeof(PDFEffects) / sizeof(PDFEffects[0])); i ++)
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
      userPassword->value(temp);
    else if (strcmp(temp, "--owner-password") == 0)
      ownerPassword->value(temp);
  }

  fclose(fp);

  title(filename, 0);

  return (1);
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
  static char	*formats = ".tchl1iIC";	// Format characters
  static char	*types[] =		// Typeface names...
		{ "Courier", "Times", "Helvetica" };
  static char	*fonts[] =		// Font names...
		{
		  "Courier", "Courier-Bold", "Courier-Oblique",
		  "Courier-BoldOblique", "Times-Roman", "Times-Bold",
		  "Times-Italic", "Times-BoldItalic",
		  "Helvetica", "Helvetica-Bold",
		  "Helvetica-Oblique", "Helvetica-BoldOblique"
		};


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
  else
    fputs("-t pdf13", fp);

  if (outputFile->value())
    fprintf(fp, " -f %s", outputPath->value());
  else
    fprintf(fp, " -d %s", outputPath->value());

  if (typeWebPage->value())
    fputs(" --webpage", fp);
  else
  {
    fputs(" --book", fp);

    if (tocLevels->value() == 0)
      fputs(" --no-toc", fp);
    else
      fprintf(fp, " --toclevels %d", tocLevels->value());

    if (numberedToc->value())
      fputs(" --numbered", fp);

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
GUI::docTypeCB(Fl_Widget *w,		// I - Toggle button widget
               GUI       *gui)	// I - GUI
{
  REF(w);

  gui->title(gui->book_filename, 1);

  if (w == gui->typeBook)
  {
    gui->typeHTML->activate();

    gui->titlePage->value(1);

    gui->tocTab->activate();
    gui->tocLevels->value(3);

    gui->firstPage->activate();
    gui->pageMode->value(1);
  }
  else
  {
    gui->typeHTML->deactivate();

    if (gui->typeHTML->value())
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

  gui->fc->filter("*.{htm,html,shtml,book}");
  gui->fc->type(FileChooser::MULTI);
  gui->fc->label("Add HTML Files?");
  gui->fc->show();
  while (gui->fc->visible())
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

  if (!gui->inputFiles->visible(i))
    gui->inputFiles->topline(i);

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

  if (!gui->inputFiles->visible(i))
    gui->inputFiles->bottomline(i);

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
    gui->fc->filter("*.{gif|jpg|png}");
    gui->fc->label("Logo Image?");
    gui->fc->type(FileChooser::SINGLE);
    gui->fc->show();
    while (gui->fc->visible())
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
    gui->fc->filter("*.{gif|jpg|png|htm|html|shtml}");
    gui->fc->label("Title Image?");
    gui->fc->type(FileChooser::SINGLE);
    gui->fc->show();
    while (gui->fc->visible())
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
  char	filename[1024];			// Name of the output file
  char	*extension;			// Extension of the output file


  if (w == gui->outputBrowse)
  {
    gui->fc->label("Output Path?");
    gui->fc->type(FileChooser::CREATE);

    if (gui->typeHTML->value())
      gui->fc->filter("*.htm*");
    else if (gui->typePDF->value())
      gui->fc->filter("*.pdf");
    else
      gui->fc->filter("*.ps");

    gui->fc->show();
    while (gui->fc->visible())
      Fl::wait();

    if (gui->fc->count())
    {
      // Get the selected file...
      strcpy(filename, file_localize(gui->fc->value(), NULL));
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
      else
      {
        // No extension - add one!
	if (gui->typeHTML->value())
	  strcat(filename, ".html");
	else if (gui->typePS->value())
	  strcat(filename, ".ps");
	else
	  strcat(filename, ".pdf");
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
  gui->title(gui->book_filename, 1);

  if (w == gui->typePDF)
  {
    gui->pdfTab->activate();
    gui->securityTab->activate();
    gui->outputDirectory->deactivate();
  }
  else
  {
    gui->pdfTab->deactivate();
    gui->securityTab->deactivate();
    gui->outputDirectory->activate();
  }

  if (w == gui->typeHTML)
  {
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
  }
  else
    gui->psTab->deactivate();

  if ((w == gui->typePDF && !gui->pdf11->value()) ||
      (w == gui->typePS && gui->ps3->value()))
    gui->compGroup->activate();
  else
    gui->compGroup->deactivate();
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


  if (gui->pdf11->value())
    gui->compGroup->deactivate();
  else
    gui->compGroup->activate();

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

  if (w == gui->ps3)
    gui->compGroup->activate();
  else
    gui->compGroup->deactivate();

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
    gui->fc->filter("*.exe");
#  else
    gui->fc->filter("*");
#  endif // WIN32 || __EMX__
    gui->fc->label("HTML Editor?");
    gui->fc->type(FileChooser::SINGLE);
    gui->fc->show();
    while (gui->fc->visible())
      Fl::wait();

    if (gui->fc->count())
    {
      filename = gui->fc->value();

      if (strstr(filename, "netscape") != NULL ||
          strstr(filename, "NETSCAPE") != NULL)
#if defined(WIN32) || defined(__EMX__)
        sprintf(command, "%s -edit %%s", filename);
#else
        sprintf(command, "%s -remote \'editFile(%%s)\'", filename);
#endif // WIN32 || __EMX__
      else
        sprintf(command, "%s %%s", filename);

      gui->htmlEditor->value(command);
    }
  }

  strcpy(HTMLEditor, gui->htmlEditor->value());
}


//
// 'GUI::saveOptionsCB()' - Save preferences...
//

void
GUI::saveOptionsCB(Fl_Widget *w,
                   GUI       *gui)
{
  static char	*formats = ".tchl1iIC";	// Format characters


  set_page_size((char *)gui->pageSize->value());

  PageLeft     = get_measurement((char *)gui->pageLeft->value());
  PageRight    = get_measurement((char *)gui->pageRight->value());
  PageTop      = get_measurement((char *)gui->pageTop->value());
  PageBottom   = get_measurement((char *)gui->pageBottom->value());

  PageDuplex   = gui->pageDuplex->value();
  Landscape    = gui->landscape->value();
  Compression  = (int)gui->compression->value();
  OutputColor  = !gui->grayscale->value();
  TocNumbers   = gui->numberedToc->value();
  TocLevels    = gui->tocLevels->value();
  TitlePage    = gui->titlePage->value();

  if (gui->jpegCompress->value())
    OutputJPEG = (int)gui->jpegQuality->value();
  else
    OutputJPEG = 0;

  strcpy(TocTitle, gui->tocTitle->value());

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

  HeadFootType  = (typeface_t)(gui->headFootFont->value() / 4);
  HeadFootStyle = (style_t)(gui->headFootFont->value() & 3);
  HeadFootSize  = gui->headFootSize->value();

  if (gui->pdf11->value())
    PDFVersion = 1.1;
  else if (gui->pdf12->value())
    PDFVersion = 1.2;
  else
    PDFVersion = 1.3;

  PDFPageMode       = gui->pageMode->value();
  PDFPageLayout     = gui->pageLayout->value();
  PDFFirstPage      = gui->firstPage->value();
  PDFEffect         = gui->pageEffect->value();
  PDFPageDuration   = gui->pageDuration->value();
  PDFEffectDuration = gui->effectDuration->value();

  Encryption = gui->encryptionYes->value();

  Permissions = -64;
  if (gui->permPrint->value())
    Permissions |= PDF_PERM_PRINT;
  if (gui->permModify->value())
    Permissions |= PDF_PERM_MODIFY;
  if (gui->permCopy->value())
    Permissions |= PDF_PERM_COPY;
  if (gui->permAnnotate->value())
    Permissions |= PDF_PERM_ANNOTATE;

  strcpy(UserPassword, gui->userPassword->value());
  strcpy(OwnerPassword, gui->ownerPassword->value());

  if (gui->ps1->value())
    PSLevel = 1;
  else if (gui->ps2->value())
    PSLevel = 2;
  else
    PSLevel = 3;

  PSCommands = gui->psCommands->value();

  strcpy(BodyColor, gui->bodyColor->value());
  strcpy(BodyImage, gui->bodyImage->value());

  htmlSetTextColor((uchar *)gui->textColor->value());
  htmlSetCharSet(gui->charset->text(gui->charset->value()));

  strcpy(LinkColor, gui->linkColor->value());
  LinkStyle = gui->linkStyle->value();

  _htmlBrowserWidth = gui->browserWidth->value();

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
    gui->fc->filter("*.{gif|jpg|png}");
    gui->fc->label("Body Image?");
    gui->fc->type(FileChooser::SINGLE);
    gui->fc->show();
    while (gui->fc->visible())
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

  sprintf(link, "%s/help.html#%s", help_dir, gui->tabs->value()->label());
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

  gui->fc->filter("*.book");
  gui->fc->label("Book File?");
  gui->fc->type(FileChooser::SINGLE);
  gui->fc->show();
  while (gui->fc->visible())
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
  char		*dir;		// Book directory

  REF(w);

  gui->fc->filter("*.book");
  gui->fc->label("Book File?");
  gui->fc->type(FileChooser::CREATE);
  gui->fc->show();
  while (gui->fc->visible())
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
      sprintf(realname, "%s.book", filename);
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
  static char	*formats = ".tchl1iIC";	// Format characters


  REF(w);

 /*
  * Do we have an output filename?
  */

  if (gui->outputPath->size() == 0)
  {
    gui->tabs->value(gui->outputTab);
    gui->outputPath->take_focus();

    fl_alert("You must specify an output directory or filename before "
             "you click on GENERATE.");

    return;
  }

 /*
  * Disable the GUI while we generate...
  */

  gui->controls->deactivate();
  gui->window->cursor(FL_CURSOR_WAIT);

 /*
  * Set global vars used for converting the HTML files to XYZ format...
  */

  strcpy(bookbase, file_directory(gui->book_filename));

  Verbosity = 1;

  strcpy(LogoImage, gui->logoImage->value());
  strcpy(TitleImage, gui->titleImage->value());
  strcpy(OutputPath, gui->outputPath->value());

  OutputFiles = gui->outputDirectory->value();
  OutputBook  = gui->typeBook->value();

  set_page_size((char *)gui->pageSize->value());

  PageLeft       = get_measurement((char *)gui->pageLeft->value());
  PageRight      = get_measurement((char *)gui->pageRight->value());
  PageTop        = get_measurement((char *)gui->pageTop->value());
  PageBottom     = get_measurement((char *)gui->pageBottom->value());
  PageDuplex     = gui->pageDuplex->value();
  Landscape      = gui->landscape->value();
  Compression    = (int)gui->compression->value();
  OutputColor    = !gui->grayscale->value();
  _htmlGrayscale = gui->grayscale->value();
  TocNumbers     = gui->numberedToc->value();
  TocLevels      = gui->tocLevels->value();
  TitlePage      = gui->titlePage->value();

  if (gui->jpegCompress->value())
    OutputJPEG = (int)gui->jpegQuality->value();
  else
    OutputJPEG = 0;

  strcpy(TocTitle, gui->tocTitle->value());

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

  HeadFootType  = (typeface_t)(gui->headFootFont->value() / 4);
  HeadFootStyle = (style_t)(gui->headFootFont->value() & 3);
  HeadFootSize  = gui->headFootSize->value();

  if (gui->pdf11->value())
    PDFVersion = 1.1;
  else if (gui->pdf12->value())
    PDFVersion = 1.2;
  else
    PDFVersion = 1.3;

  PDFPageMode       = gui->pageMode->value();
  PDFPageLayout     = gui->pageLayout->value();
  PDFFirstPage      = gui->firstPage->value();
  PDFEffect         = gui->pageEffect->value();
  PDFPageDuration   = gui->pageDuration->value();
  PDFEffectDuration = gui->effectDuration->value();

  Encryption = gui->encryptionYes->value();

  Permissions = -64;
  if (gui->permPrint->value())
    Permissions |= PDF_PERM_PRINT;
  if (gui->permModify->value())
    Permissions |= PDF_PERM_MODIFY;
  if (gui->permCopy->value())
    Permissions |= PDF_PERM_COPY;
  if (gui->permAnnotate->value())
    Permissions |= PDF_PERM_ANNOTATE;

  strcpy(UserPassword, gui->userPassword->value());
  strcpy(OwnerPassword, gui->ownerPassword->value());

  if (gui->typePDF->value())
    PSLevel = 0;
  else if (gui->ps1->value())
    PSLevel = 1;
  else if (gui->ps2->value())
    PSLevel = 2;
  else
    PSLevel = 3;

  PSCommands = gui->psCommands->value();

  strcpy(BodyColor, gui->bodyColor->value());
  strcpy(BodyImage, gui->bodyImage->value());

  htmlSetTextColor((uchar *)gui->textColor->value());
  htmlSetCharSet(gui->charset->text(gui->charset->value()));

  strcpy(LinkColor, gui->linkColor->value());
  LinkStyle = gui->linkStyle->value();

  _htmlBrowserWidth = gui->browserWidth->value();

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
      htmlSetVariable(file, (uchar *)"FILENAME",
                      (uchar *)file_basename(filename));

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
    gui->window->cursor(FL_CURSOR_DEFAULT);
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

  if (OutputBook && TocLevels > 0)
    toc = toc_build(document);
  else
    toc = NULL;

 /*
  * Generate the output file(s).
  */

  Errors = 0;

  if (gui->typeHTML->value())
    html_export(document, toc);
  else
    pspdf_export(document, toc);

  htmlDeleteTree(document);
  htmlDeleteTree(toc);

  gui->controls->activate();
  gui->window->cursor(FL_CURSOR_DEFAULT);
  gui->progress(0);

  if (Errors == 0)
    fl_message("Document Generated!");
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


#endif // HAVE_LIBFLTK

//
// End of "$Id: gui.cxx,v 1.32 2000/06/05 03:18:23 mike Exp $".
//
