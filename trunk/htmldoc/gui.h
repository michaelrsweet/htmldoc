/*
 * "$Id$"
 *
 *   GUI definitions for HTMLDOC, an HTML document processing program.
 *
 *   Copyright 1997-2004 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "COPYING.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: ESP Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: info@easysw.com
 *         WWW: http://www.easysw.com
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
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Tooltip.H>


/*
 * Class definition for HTMLDOC dialog...
 */

class GUI
{
  private:

  Fl_Double_Window	*window;

  Fl_Group		*controls;
  Fl_Tabs		*tabs;

  Fl_Group		*inputTab;
  Fl_Round_Button	*typeBook,
			*typeContinuous,
			*typeWebPage;
  Fl_File_Browser	*inputFiles;
  Fl_Button		*addFile,
			*addURL,
			*editFile,
			*deleteFile,
			*moveUpFile,
			*moveDownFile;
  Fl_Input		*logoImage;
  Fl_Button		*logoBrowse;
  Fl_Input		*titleImage;
  Fl_Button		*titleBrowse;

  Fl_Group		*outputTab;
  Fl_Round_Button	*outputFile,
			*outputDirectory;
  Fl_Input		*outputPath;
  Fl_Button		*outputBrowse;
  Fl_Round_Button	*typeHTML,
			*typeHTMLSep,
			*typePS,
			*typePDF;
  Fl_Check_Button	*grayscale,
			*titlePage,
			*jpegCompress;
  Fl_Group		*compGroup;
  Fl_Slider		*compression;
  Fl_Group		*jpegGroup;
  Fl_Value_Slider	*jpegQuality;

  Fl_Group		*pageTab;
  Fl_Input		*pageSize;
  Fl_Menu_Button	*pageSizeMenu;
  Fl_Check_Button	*pageDuplex,
			*landscape;
  Fl_Input		*pageTop,
			*pageLeft,
			*pageRight,
			*pageBottom;
  Fl_Choice		*pageHeaderLeft,
      			*pageHeaderCenter,
      			*pageHeaderRight,
      			*pageHeader1Left,
      			*pageHeader1Center,
      			*pageHeader1Right,
      			*pageFooterLeft,
      			*pageFooterCenter,
      			*pageFooterRight;
  Fl_Choice		*numberUp;

  Fl_Group		*tocTab;
  Fl_Choice		*tocLevels;
  Fl_Check_Button	*numberedToc;
  Fl_Group		*tocHeader;
  Fl_Choice		*tocHeaderLeft,
      			*tocHeaderCenter,
      			*tocHeaderRight;
  Fl_Group		*tocFooter;
  Fl_Choice		*tocFooterLeft,
      			*tocFooterCenter,
      			*tocFooterRight;
  Fl_Input		*tocTitle;

  Fl_Group		*colorsTab;
  Fl_Input		*bodyColor;
  Fl_Button		*bodyLookup;
  Fl_Input		*bodyImage;
  Fl_Button		*bodyBrowse;
  Fl_Input		*textColor;
  Fl_Button		*textLookup;
  Fl_Input		*linkColor;
  Fl_Button		*linkLookup;
  Fl_Choice		*linkStyle;

  Fl_Group		*fontsTab;
  Fl_Choice		*headingFont,
			*bodyFont,
			*headFootFont;
  Fl_Counter		*fontBaseSize,
			*fontSpacing,
			*headFootSize;
  Fl_Choice		*charset;
  Fl_Check_Button	*embedFonts;

  Fl_Group		*psTab;
  Fl_Group		*psLevel;
  Fl_Round_Button	*ps1,
			*ps2,
			*ps3;
  Fl_Check_Button	*psCommands,
			*xrxComments;

  Fl_Group		*pdfTab;
  Fl_Group		*pdfVersion;
  Fl_Round_Button	*pdf11,
			*pdf12,
			*pdf13,
			*pdf14;
  Fl_Choice		*pageMode,
			*pageLayout,
			*firstPage,
			*pageEffect;
  Fl_Value_Slider	*pageDuration,
			*effectDuration;
  Fl_Check_Button	*links;

  Fl_Group		*securityTab;
  Fl_Group		*encryption;
  Fl_Round_Button	*encryptionYes,
			*encryptionNo;
  Fl_Group		*permissions;
  Fl_Check_Button	*permPrint,
			*permModify,
			*permCopy,
			*permAnnotate;
  Fl_Secret_Input	*ownerPassword,
			*userPassword;

  Fl_Group		*optionsTab;
  Fl_Input		*htmlEditor;
  Fl_Button		*htmlBrowse;
  Fl_Value_Slider	*browserWidth;
  Fl_Input		*path;
  Fl_Input		*proxy;
  Fl_Check_Button	*tooltips;
  Fl_Check_Button	*modern_skin;
  Fl_Check_Button	*strict_html;
  Fl_Check_Button	*overflow_errors;

  Fl_Button		*showAbout,
			*showLicense,
  			*saveOptions;

  Fl_Button		*bookHelp,
			*bookNew,
			*bookOpen,
			*bookSave,
			*bookSaveAs,
			*bookGenerate,
			*bookClose;

  Fl_Progress		*progressBar;

  hdBook		*book;
  char			book_filename[1024];
  int			book_changed;

  char			title_string[1024];

  Fl_File_Chooser	*fc;
  Fl_File_Icon		*icon;
  Fl_Help_Dialog	*help;
  Fl_Window		*error_window;
  Fl_Browser		*error_list;
  Fl_Button		*error_ok;

  void		loadSettings();
  void		title(const char *filename = NULL, int changed = 0);

  static void	changeCB(Fl_Widget *w, GUI *gui);

  static void	docTypeCB(Fl_Widget *w, GUI *gui);
  static void	inputFilesCB(Fl_Widget *w, GUI *gui);
  static void	addFileCB(Fl_Widget *w, GUI *gui);
  static void	addURLCB(Fl_Widget *w, GUI *gui);
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

  static void	sizeCB(Fl_Widget *w, GUI *gui);

  static void	tocCB(Fl_Widget *w, GUI *gui);

  static void	bodyColorCB(Fl_Widget *w, GUI *gui);
  static void	bodyImageCB(Fl_Widget *w, GUI *gui);
  static void	textColorCB(Fl_Widget *w, GUI *gui);
  static void	linkColorCB(Fl_Widget *w, GUI *gui);

  static void	psCB(Fl_Widget *w, GUI *gui);

  static void	pdfCB(Fl_Widget *w, GUI *gui);
  static void	effectCB(Fl_Widget *w, GUI *gui);

  static void	encryptionCB(Fl_Widget *w, GUI *gui);

  static void	htmlEditorCB(Fl_Widget *w, GUI *gui);
  static void	tooltipCB(Fl_Widget *w, GUI *gui);
  static void	skinCB(Fl_Widget *w, GUI *gui);
  static void	saveOptionsCB(Fl_Widget *w, GUI *gui);

  static void	helpCB(Fl_Widget *w, GUI *gui);
  static void	newBookCB(Fl_Widget *w, GUI *gui);
  static void	openBookCB(Fl_Widget *w, GUI *gui);
  static void	saveBookCB(Fl_Widget *w, GUI *gui);
  static void	saveAsBookCB(Fl_Widget *w, GUI *gui);
  static void	generateBookCB(Fl_Widget *w, GUI *gui);
  static void	closeBookCB(Fl_Widget *w, GUI *gui);

  static void	errorCB(Fl_Widget *w, GUI *gui);

#ifdef __APPLE__
  static void	appleOpenCB(const char *f);
#endif // __APPLE__

  public:

  static const char	*help_dir;
  static void		showAboutCB(void);
  static void		showLicenseCB(void);


  GUI(const char *filename = NULL);
  ~GUI(void);

  void	add_error(const char *s) { error_list->add(s); }
  int	checkSave();
  void	hide() { window->hide(); help->hide(); fc->hide(); };
  int	loadBook(const char *bookfile);
  int	newBook();
  void	parseOptions(const char *line);
  void	progress(int percent, const char *text = NULL);
  int	saveBook(const char *bookfile);
  void	show();
  int	shown() { return (window->shown()); }
  int	visible() { return (window->visible()); }
};


/*
 * End of "$Id$".
 */
