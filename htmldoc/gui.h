/*
 * "$Id: gui.h,v 1.3 1999/11/09 01:08:45 mike Exp $"
 *
 *   GUI definitions for HTMLDOC, an HTML document processing program.
 *
 *   Copyright 1997-1999 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the GNU General Public License
 *   which is described in the file "COPYING-2.0".
 */

/*
 * Include necessary headers.
 */

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Window.H>

#include <gui/CheckButton.h>
#include <gui/FileChooser.h>
#include <gui/HelpDialog.h>


/*
 * Class definition for HTMLDOC dialog...
 */

class GUI
{
  public:
      Fl_Window		*window;
      Fl_Group		*controls;

      Fl_Group		*inputTab;
      CheckButton	*typeBook,
			*typeWebPage;
      Fl_Multi_Browser	*inputFiles;
      Fl_Button		*addFile,
			*editFile,
			*deleteFile,
			*moveUpFile,
			*moveDownFile;
      Fl_Input		*logoImage;
      Fl_Button		*logoBrowse;
      Fl_Input		*titleImage;
      Fl_Button		*titleBrowse;

      Fl_Group		*outputTab;
      CheckButton	*outputFile,
			*outputDirectory;
      Fl_Input		*outputPath;
      Fl_Button		*outputBrowse;
      CheckButton	*typeHTML,
			*typePS,
			*typePDF;
      CheckButton	*grayscale,
			*titlePage,
			*jpegCompress;

      Fl_Input		*bodyColor;
      Fl_Button		*bodyLookup;
      Fl_Input		*bodyImage;
      Fl_Button		*bodyBrowse;

      Fl_Group		*pageTab;
      Fl_Input		*pageSize;
      CheckButton	*pageDuplex;
      Fl_Input		*pageTop,
			*pageLeft,
			*pageRight,
			*pageBottom;
      Fl_Choice		*pageHeaderLeft,
      			*pageHeaderCenter,
      			*pageHeaderRight,
      			*pageFooterLeft,
      			*pageFooterCenter,
      			*pageFooterRight;

      Fl_Group		*tocTab;
      Fl_Choice		*tocLevels;
      CheckButton	*numberedToc;
      Fl_Choice		*tocHeaderLeft,
      			*tocHeaderCenter,
      			*tocHeaderRight,
      			*tocFooterLeft,
      			*tocFooterCenter,
      			*tocFooterRight;

      Fl_Group		*fontsTab;
      Fl_Choice		*headingFont,
			*bodyFont,
			*headFootFont;
      Fl_Counter	*fontBaseSize,
			*fontSpacing,
			*headFootSize;

      Fl_Group		*htmlTab;

      Fl_Group		*psTab;

      Fl_Group		*psLevel;
      Fl_Button		*ps1,
			*ps2,
			*ps3;

      Fl_Group		*pdfTab;

      Fl_Group		*pdfVersion;
      Fl_Button		*pdf11,
			*pdf12,
			*pdf13;
      Fl_Slider		*compression;

      Fl_Group		*optionsTab;
      Fl_Input		*htmlEditor;
      Fl_Button		*htmlBrowse;
      Fl_Value_Slider	*jpegQuality;

      Fl_Button		*bookSave,
			*bookSaveAs,
			*bookGenerate;

      Fl_Group		*progressText;
      Fl_Slider		*progressBar;

      FileChooser	*fc;

      char		book_filename[1024];
      int		book_changed;

      GUI(const char *filename = NULL);
      ~GUI(void);

      int  doGUI(void);

      void progress(int percent, char *text = NULL);
      void title(const char *filename = NULL, int changed = 0);

      int  newBook(void);
      int  loadBook(const char *bookfile);
      int  saveBook(const char *bookfile);
      int  checkSave(void);
};


extern void docTypeCB(Fl_Widget *w, GUI *gui);
extern void inputFilesCB(Fl_Widget *w, GUI *gui);
extern void addFileCB(Fl_Widget *w, GUI *gui);
extern void editFilesCB(Fl_Widget *w, GUI *gui);
extern void deleteFilesCB(Fl_Widget *w, GUI *gui);
extern void moveUpFilesCB(Fl_Widget *w, GUI *gui);
extern void moveDownFilesCB(Fl_Widget *w, GUI *gui);
extern void logoImageCB(Fl_Widget *w, GUI *gui);
extern void titleImageCB(Fl_Widget *w, GUI *gui);

extern void outputTypeCB(Fl_Widget *w, GUI *gui);
extern void outputPathCB(Fl_Widget *w, GUI *gui);
extern void outputFormatCB(Fl_Widget *w, GUI *gui);

extern void changeCB(Fl_Widget *w, GUI *gui);
extern void jpegCB(Fl_Widget *w, GUI *gui);
extern void pdfCB(Fl_Widget *w, GUI *gui);
extern void bodyColorCB(Fl_Widget *w, GUI *gui);
extern void bodyImageCB(Fl_Widget *w, GUI *gui);
extern void htmlEditorCB(Fl_Widget *w, GUI *gui);

extern void helpCB(Fl_Widget *w, GUI *gui);
extern void newBookCB(Fl_Widget *w, GUI *gui);
extern void openBookCB(Fl_Widget *w, GUI *gui);
extern void saveBookCB(Fl_Widget *w, GUI *gui);
extern void saveAsBookCB(Fl_Widget *w, GUI *gui);
extern void generateBookCB(Fl_Widget *w, GUI *gui);
extern void closeBookCB(Fl_Widget *w, GUI *gui);

/*
 * End of "$Id: gui.h,v 1.3 1999/11/09 01:08:45 mike Exp $".
 */
