/*
 * "$Id: gui.h,v 1.4 1999/11/09 21:36:23 mike Exp $"
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
  private:

  Fl_Window	*window;
  Fl_Group	*controls;

  Fl_Group	*inputTab;
  CheckButton	*typeBook,
		*typeWebPage;
  FileBrowser	*inputFiles;
  Fl_Button	*addFile,
		*editFile,
		*deleteFile,
		*moveUpFile,
		*moveDownFile;
  Fl_Input	*logoImage;
  Fl_Button	*logoBrowse;
  Fl_Input	*titleImage;
  Fl_Button	*titleBrowse;

  Fl_Group	*outputTab;
  CheckButton	*outputFile,
		*outputDirectory;
  Fl_Input	*outputPath;
  Fl_Button	*outputBrowse;
  CheckButton	*typeHTML,
		*typePS,
		*typePDF;
  CheckButton	*grayscale,
		*titlePage,
		*jpegCompress;
  Fl_Slider	*compression;
  Fl_Value_Slider *jpegQuality;

  Fl_Group	*pageTab;
  Fl_Input	*pageSize;
  CheckButton	*pageDuplex;
  Fl_Input	*pageTop,
		*pageLeft,
		*pageRight,
		*pageBottom;
  Fl_Choice	*pageHeaderLeft,
      		*pageHeaderCenter,
      		*pageHeaderRight,
      		*pageFooterLeft,
      		*pageFooterCenter,
      		*pageFooterRight;

  Fl_Group	*tocTab;
  Fl_Choice	*tocLevels;
  CheckButton	*numberedToc;
  Fl_Group	*tocHeader;
  Fl_Choice	*tocHeaderLeft,
      		*tocHeaderCenter,
      		*tocHeaderRight;
  Fl_Group	*tocFooter;
  Fl_Choice	*tocFooterLeft,
      		*tocFooterCenter,
      		*tocFooterRight;
  Fl_Input	*tocTitle;

  Fl_Group	*colorsTab;
  Fl_Input	*bodyColor;
  Fl_Button	*bodyLookup;
  Fl_Input	*bodyImage;
  Fl_Button	*bodyBrowse;
  Fl_Input	*textColor;
  Fl_Button	*textLookup;


  Fl_Group	*fontsTab;
  Fl_Choice	*headingFont,
		*bodyFont,
		*headFootFont;
  Fl_Counter	*fontBaseSize,
		*fontSpacing,
		*headFootSize;

  Fl_Group	*htmlTab;


  Fl_Group	*optionsTab;
  Fl_Input	*htmlEditor;
  Fl_Button	*htmlBrowse;
  Fl_Group	*psLevel;
  Fl_Button	*ps1,
		*ps2,
		*ps3;
  CheckButton	*psCommands;
  Fl_Group	*pdfVersion;
  Fl_Button	*pdf11,
		*pdf12,
		*pdf13;

  Fl_Button	*bookSave,
		*bookSaveAs,
		*bookGenerate;

  Fl_Group	*progressText;
  Fl_Slider	*progressBar;

  FileChooser	*fc;
  FileIcon	*icon;
  HelpDialog	*help;

  char		book_filename[1024];
  int		book_changed;

  void		title(const char *filename = NULL, int changed = 0);

  static char	*file_localize(char *filename, char *newcwd);

  static void	changeCB(Fl_Widget *w, GUI *gui);

  static void	docTypeCB(Fl_Widget *w, GUI *gui);
  static void	inputFilesCB(Fl_Widget *w, GUI *gui);
  static void	addFileCB(Fl_Widget *w, GUI *gui);
  static void	editFilesCB(Fl_Widget *w, GUI *gui);
  static void	deleteFilesCB(Fl_Widget *w, GUI *gui);
  static void	moveUpFilesCB(Fl_Widget *w, GUI *gui);
  static void	moveDownFilesCB(Fl_Widget *w, GUI *gui);
  static void	logoImageCB(Fl_Widget *w, GUI *gui);
  static void	titleImageCB(Fl_Widget *w, GUI *gui);

  static void	outputTypeCB(Fl_Widget *w, GUI *gui);
  static void	outputPathCB(Fl_Widget *w, GUI *gui);
  static void	outputFormatCB(Fl_Widget *w, GUI *gui);
  static void	jpegCB(Fl_Widget *w, GUI *gui);

  static void	tocCB(Fl_Widget *w, GUI *gui);

  static void	bodyColorCB(Fl_Widget *w, GUI *gui);
  static void	bodyImageCB(Fl_Widget *w, GUI *gui);
  static void	textColorCB(Fl_Widget *w, GUI *gui);

  static void	psCB(Fl_Widget *w, GUI *gui);
  static void	pdfCB(Fl_Widget *w, GUI *gui);
  static void	htmlEditorCB(Fl_Widget *w, GUI *gui);

  static void	helpCB(Fl_Widget *w, GUI *gui);
  static void	newBookCB(Fl_Widget *w, GUI *gui);
  static void	openBookCB(Fl_Widget *w, GUI *gui);
  static void	saveBookCB(Fl_Widget *w, GUI *gui);
  static void	saveAsBookCB(Fl_Widget *w, GUI *gui);
  static void	generateBookCB(Fl_Widget *w, GUI *gui);
  static void	closeBookCB(Fl_Widget *w, GUI *gui);

  public:

  static const char	*help_dir;

  GUI(const char *filename = NULL);
  ~GUI(void);

  int	checkSave();
  void	hide() { window->hide(); help->hide(); fc->hide(); };
  int	loadBook(const char *bookfile);
  int	newBook();
  void	progress(int percent, char *text = NULL);
  int	saveBook(const char *bookfile);
  void	show();
  int	visible() { return (window->visible()); }
};


/*
 * End of "$Id: gui.h,v 1.4 1999/11/09 21:36:23 mike Exp $".
 */
