//
// "$Id$"
//
// PostScript device support for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2015 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

//#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <FL/Fl_Native_File_Chooser.H>
#include <stdarg.h>


#include "PostScript.H"



extern "C" {
  static int dont_close(FILE *f)
  {
    return 0;
  }
}






int Fl_PostScript_File_Device::start_job_eps(FILE *ps_output, int lang_level, int x, int y, int w, int h)
{

  Fl_PostScript_Graphics_Driver *ps = driver();
  
  ps->output = ps_output;
  ps->ps_filename_ = NULL;
  ps->start_postscript(lang_level, x, y, w, h);
  ps->close_command(dont_close); // so that end_job() doesn't close the file
  this->set_current();
  return 0;
}






//  Prolog string 

static const char * prolog =
"%%BeginProlog\n"
"/L { /y2 exch def\n"
"/x2 exch def\n"
"/y1 exch def\n"
"/x1 exch def\n"
"newpath   x1 y1 moveto x2 y2 lineto\n"
"stroke}\n"
"bind def\n"


"/R { /dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath stroke\n"
"} bind def\n"

"/CL {\n"
"/dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath\n"
"clip\n"
"} bind def\n"

"/FR { /dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"currentlinewidth 0 setlinewidth newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath fill setlinewidth\n"
"} bind def\n"

"/GS { gsave } bind  def\n"
"/GR { grestore } bind def\n"

"/SP { showpage } bind def\n"
"/LW { setlinewidth } bind def\n"
"/CF /Courier def\n"
"/SF { /CF exch def } bind def\n"
"/fsize 12 def\n"
"/FS { /fsize exch def fsize CF findfont exch scalefont setfont }def \n"


"/GL { setgray } bind def\n"
"/SRGB { setrgbcolor } bind def\n"

"/A85RLE { /ASCII85Decode filter /RunLengthDecode filter } bind def\n" // ASCII85Decode followed by RunLengthDecode filters

//  color images 

"/CI { GS /py exch def /px exch def /sy exch def /sx exch def\n"
"translate \n"
"sx sy scale px py 8 \n"
"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n false 3"
" colorimage GR\n"
"} bind def\n"

//  gray images 

"/GI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale px py 8 \n"


"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n"
"image GR\n"
"} bind def\n"

// single-color bitmask 

"/MI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale px py true \n"
"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n"
"imagemask GR\n"
"} bind def\n"


//  path 

"/BFP { newpath moveto }  def\n"
"/BP { newpath } bind def \n"
"/PL { lineto } bind def \n"
"/PM { moveto } bind def \n"
"/MT { moveto } bind def \n"
"/LT { lineto } bind def \n"
"/EFP { closepath fill } bind def\n"  //was:stroke
"/ELP { stroke } bind def\n"  
"/ECP { closepath stroke } bind def\n"  // Closed (loop)
"/LW { setlinewidth } bind def\n"

// ////////////////////////// misc ////////////////
"/TR { translate } bind def\n"
"/CT { concat } bind def\n"
"/RCT { matrix invertmatrix concat} bind def\n"
"/SC { scale } bind def\n"
//"/GPD { currentpagedevice /PageSize get} def\n"

// show at position with desired width
// usage:
// width (string) x y show_pos_width
"/show_pos_width {GS moveto dup dup stringwidth pop exch length 2 div dup 2 le {pop 9999} if "
"1 sub exch 3 index exch sub exch "
"div 0 2 index 1 -1 scale ashow pop pop GR} bind def\n" // spacing altered to match desired width
//"/show_pos_width {GS moveto dup stringwidth pop 3 2 roll exch div -1 matrix scale concat "
//"show GR } bind def\n" // horizontally scaled text to match desired width

;


static const char * prolog_2 =  // prolog relevant only if lang_level >1

// color image dictionaries
"/CII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceRGB setcolorspace\n"
"/IDD 8 dict def\n"
"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
"/Decode [ 0 1 0 1 0 1 ] def\n"
"end\n"
"IDD image GR} bind def\n"

// gray image dict 
"/GII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceGray setcolorspace\n"
"/IDD 8 dict def\n"
"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"

"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
"/Decode [ 0 1 ] def\n"
"end\n"
"IDD image GR} bind def\n"

// Create a custom PostScript font derived from PostScript standard text fonts
// The encoding of this custom font is as follows:
// 0000-00FF  coincides with Unicode, that is to ASCII + Latin-1
// 0100-017F  coincides with Unicode, that is to Latin Extended-A
// 0180-01A6  encodes miscellaneous characters present in PostScript standard text fonts

// use ISOLatin1Encoding for all text fonts
"/ToISO { dup findfont dup length dict copy begin /Encoding ISOLatin1Encoding def currentdict end definefont pop } def\n"
"/Helvetica ToISO /Helvetica-Bold ToISO /Helvetica-Oblique ToISO /Helvetica-BoldOblique ToISO \n"
"/Courier ToISO /Courier-Bold ToISO /Courier-Oblique ToISO /Courier-BoldOblique ToISO \n"
"/Times-Roman ToISO /Times-Bold ToISO /Times-Italic ToISO /Times-BoldItalic ToISO \n"

// define LatinExtA, the encoding of Latin-extended-A + some additional characters
// see http://partners.adobe.com/public/developer/en/opentype/glyphlist.txt for their names
"/LatinExtA \n"
"[ "
" /Amacron /amacron /Abreve /abreve /Aogonek /aogonek\n" // begin of Latin Extended-A code page
" /Cacute  /cacute  /Ccircumflex  /ccircumflex  /Cdotaccent  /cdotaccent  /Ccaron  /ccaron \n"
" /Dcaron  /dcaron   /Dcroat  /dcroat\n"
" /Emacron  /emacron  /Ebreve  /ebreve  /Edotaccent  /edotaccent  /Eogonek  /eogonek  /Ecaron  /ecaron\n"
" /Gcircumflex  /gcircumflex  /Gbreve  /gbreve  /Gdotaccent  /gdotaccent  /Gcommaaccent  /gcommaaccent \n"
" /Hcircumflex /hcircumflex  /Hbar  /hbar  \n"
" /Itilde  /itilde  /Imacron  /imacron  /Ibreve  /ibreve  /Iogonek  /iogonek /Idotaccent  /dotlessi  \n"
" /IJ  /ij  /Jcircumflex  /jcircumflex\n"
" /Kcommaaccent  /kcommaaccent  /kgreenlandic  \n"
" /Lacute  /lacute  /Lcommaaccent  /lcommaaccent   /Lcaron  /lcaron  /Ldotaccent /ldotaccent   /Lslash  /lslash \n"
" /Nacute  /nacute  /Ncommaaccent  /ncommaaccent  /Ncaron  /ncaron  /napostrophe  /Eng  /eng  \n"
" /Omacron  /omacron /Obreve  /obreve  /Ohungarumlaut  /ohungarumlaut  /OE  /oe \n"
" /Racute  /racute  /Rcommaaccent  /rcommaaccent  /Rcaron  /rcaron \n"
" /Sacute /sacute  /Scircumflex  /scircumflex  /Scedilla /scedilla /Scaron  /scaron \n"
" /Tcommaaccent  /tcommaaccent  /Tcaron  /tcaron  /Tbar  /tbar \n"
" /Utilde  /utilde /Umacron /umacron  /Ubreve  /ubreve  /Uring  /uring  /Uhungarumlaut  /uhungarumlaut  /Uogonek /uogonek \n"
" /Wcircumflex  /wcircumflex  /Ycircumflex  /ycircumflex  /Ydieresis \n"
" /Zacute /zacute /Zdotaccent /zdotaccent /Zcaron /zcaron \n"
" /longs \n" // end of Latin Extended-A code page
" /florin  /circumflex  /caron  /breve  /dotaccent  /ring \n" // remaining characters from PostScript standard text fonts
" /ogonek  /tilde  /hungarumlaut  /endash /emdash \n"
" /quoteleft  /quoteright  /quotesinglbase  /quotedblleft  /quotedblright \n"
" /quotedblbase  /dagger  /daggerdbl  /bullet  /ellipsis \n"
" /perthousand  /guilsinglleft  /guilsinglright  /fraction  /Euro \n"
" /trademark /partialdiff  /Delta /summation  /radical \n"
" /infinity /notequal /lessequal /greaterequal /lozenge \n"
" /fi /fl /apple \n"
" ] def \n"
// deal with alternative PostScript names of some characters
" /mycharstrings /Helvetica findfont /CharStrings get def\n"
" /PSname2 { dup mycharstrings exch known {LatinExtA 3 -1 roll 3 -1 roll put}{pop pop} ifelse } def \n"
" 16#20 /Gdot PSname2 16#21 /gdot PSname2 16#30 /Idot PSname2 16#3F /Ldot PSname2 16#40 /ldot PSname2 16#7F /slong PSname2 \n"

// proc that gives LatinExtA encoding to a font
"/ToLatinExtA { findfont dup length dict copy begin /Encoding LatinExtA def currentdict end definefont pop } def\n"
// create Ext-versions of standard fonts that use LatinExtA encoding \n"
"/HelveticaExt /Helvetica ToLatinExtA \n"
"/Helvetica-BoldExt /Helvetica-Bold ToLatinExtA /Helvetica-ObliqueExt /Helvetica-Oblique ToLatinExtA  \n"
"/Helvetica-BoldObliqueExt /Helvetica-BoldOblique ToLatinExtA  \n"
"/CourierExt /Courier ToLatinExtA /Courier-BoldExt /Courier-Bold ToLatinExtA  \n"
"/Courier-ObliqueExt /Courier-Oblique ToLatinExtA /Courier-BoldObliqueExt /Courier-BoldOblique ToLatinExtA \n"
"/Times-RomanExt /Times-Roman ToLatinExtA /Times-BoldExt /Times-Bold ToLatinExtA  \n"
"/Times-ItalicExt /Times-Italic ToLatinExtA /Times-BoldItalicExt /Times-BoldItalic ToLatinExtA \n"

// proc to create a Type 0 font with 2-byte encoding 
// that merges a text font with ISO encoding + same font with LatinExtA encoding
"/To2byte { 6 dict begin /FontType 0 def \n"
"/FDepVector 3 1 roll findfont exch findfont 2 array astore def \n"
"/FontMatrix [1  0  0  1  0  0] def /FMapType 6 def /Encoding [ 0 1 0 ] def\n"
// 100: Hexa count of ISO array; A7: hexa count of LatinExtA array
"/SubsVector < 01 0100 00A7 > def\n" 
"currentdict end definefont pop } def\n"
// create Type 0 versions of standard fonts
"/Helvetica2B /HelveticaExt /Helvetica To2byte \n"
"/Helvetica-Bold2B /Helvetica-BoldExt /Helvetica-Bold To2byte \n"
"/Helvetica-Oblique2B /Helvetica-ObliqueExt /Helvetica-Oblique To2byte \n"
"/Helvetica-BoldOblique2B /Helvetica-BoldObliqueExt /Helvetica-BoldOblique To2byte \n"
"/Courier2B /CourierExt /Courier To2byte \n"
"/Courier-Bold2B /Courier-BoldExt /Courier-Bold To2byte \n"
"/Courier-Oblique2B /Courier-ObliqueExt /Courier-Oblique To2byte \n"
"/Courier-BoldOblique2B /Courier-BoldObliqueExt /Courier-BoldOblique To2byte \n"
"/Times-Roman2B /Times-RomanExt /Times-Roman To2byte \n"
"/Times-Bold2B /Times-BoldExt /Times-Bold To2byte \n"
"/Times-Italic2B /Times-ItalicExt /Times-Italic To2byte \n"
"/Times-BoldItalic2B /Times-BoldItalicExt /Times-BoldItalic To2byte \n"
;

static const char * prolog_2_pixmap =  // prolog relevant only if lang_level == 2 for pixmaps/masked color images
"/pixmap_mat {[ pixmap_sx 0 0 pixmap_sy neg 0 pixmap_sy ]}  bind def\n"

"/pixmap_dict {"
"<< /PatternType 1 "
"/PaintType 1 "
"/TilingType 2 "
"/BBox [0  0  pixmap_sx  pixmap_sy] "
"/XStep pixmap_sx "
"/YStep pixmap_sy\n"
"/PaintProc "
"{ begin "
"pixmap_w pixmap_h scale "
"pixmap_sx pixmap_sy 8 "
"pixmap_mat "
"currentfile A85RLE "
"false 3 "
"colorimage "
"end "
"} bind "
">>\n"
"} bind def\n"

"/pixmap_plot {"
"GS "
"/pixmap_sy exch def /pixmap_sx exch def\n"
"/pixmap_h exch def /pixmap_w exch def\n"
"translate\n"
"pixmap_dict matrix makepattern setpattern\n"
"pixmap_w pixmap_h scale\n"
"pixmap_sx pixmap_sy\n"
"true\n"
"pixmap_mat\n"
"currentfile A85RLE\n"
"imagemask\n"
"GR\n"
"} bind def\n"
;

static const char * prolog_3 = // prolog relevant only if lang_level >2

// masked color images 
"/CIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceRGB setcolorspace\n"

"/IDD 8 dict def\n"

"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

"/Decode [ 0 1 0 1 0 1 ] def\n"
"end\n"

"/IMD 8 dict def\n"
"IMD begin\n"
"/ImageType 1 def\n"
"/Width mx def\n"           
"/Height my def\n"
"/BitsPerComponent 1 def\n"
//  "/Interpolate inter def\n"
"/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
"/Decode [ 1 0 ] def\n"
"end\n"

"<<\n"
"/ImageType 3\n"
"/InterleaveType 2\n"
"/MaskDict IMD\n"
"/DataDict IDD\n"
">> image GR\n"
"} bind def\n"


//  masked gray images 
"/GIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceGray setcolorspace\n"

"/IDD 8 dict def\n"

"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

"/Decode [ 0 1 ] def\n"
"end\n"

"/IMD 8 dict def\n"

"IMD begin\n"
"/ImageType 1 def\n"
"/Width mx def\n"           
"/Height my def\n"
"/BitsPerComponent 1 def\n"
"/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
"/Decode [ 1 0 ] def\n"
"end\n"

"<<\n"
"/ImageType 3\n"
"/InterleaveType 2\n"
"/MaskDict IMD\n"
"/DataDict IDD\n"
">> image GR\n"
"} bind def\n"


"\n"
;

// end prolog 



  
int Fl_PostScript_Graphics_Driver::start_postscript (int lang_level, int xx, int yy, int ww, int hh){
//returns 0 iff OK
  int pagecount = 1;
  lang_level_ = lang_level;
  
 Fl_Paged_Device::Page_Format format = Fl_Paged_Device::A4;
 //Fl_Paged_Device::Page_Layout layout = Fl_Paged_Device::LANDSCAPE;
 Fl_Paged_Device::Page_Layout layout = Fl_Paged_Device::PORTRAIT;
  int w, h, x;

    left_margin = 0;
    top_margin = 0;
  pw_ = ww;
  ph_ = hh;
  
  
  fputs("%!PS-Adobe-3.0\n", output);
  fputs("%%Creator: FLTK\n", output);
  if (lang_level_>1)
    fprintf(output, "%%%%LanguageLevel: %i\n" , lang_level_);
  if ((pages_ = pagecount))
    fprintf(output, "%%%%Pages: %i\n", pagecount);
  else
    fputs("%%Pages: (atend)\n", output);
  fprintf(output, "%%%%BeginFeature: *PageSize custom\n");
  w = ww;
  h = hh;
  
  
  
  if (lang_level_ == 3 && (layout & Fl_Paged_Device::LANDSCAPE) ) { x = w; w = h; h = x; }
  fprintf(output, "<</PageSize[%d %d]>>setpagedevice\n", w, h );
  
 
  fputs("%%EndFeature\n", output);
  fputs("%%EndComments\n", output);
  fputs(prolog, output);
  if (lang_level_ > 1) {
    fputs(prolog_2, output);
    }
  if (lang_level_ == 2) {
    fputs(prolog_2_pixmap, output);
    }
  if (lang_level_ > 2)
    fputs(prolog_3, output);
  if (lang_level_ >= 3) {
    fputs("/CS { clipsave } bind def\n", output);
    fputs("/CR { cliprestore } bind def\n", output);
  } else {
    fputs("/CS { GS } bind def\n", output);
    fputs("/CR { GR } bind def\n", output);
  }
  page_policy_ = 1;
  
  
  fputs("%%EndProlog\n",output);
  if (lang_level_ >= 2)
    fprintf(output,"<< /Policies << /Pagesize 1 >> >> setpagedevice\n");
  reset();

  nPages=0;
  return 0;
}




int Fl_PostScript_File_Device::start_page_eps (void)
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->page1(ps->page_format_);
  x_offset = 0;
  y_offset = 0;
  ps->scale_x = ps->scale_y = 1.;
  ps->angle = 0;
  fprintf(ps->output, "GR GR GS %d %d translate GS\n", ps->left_margin, ps->top_margin);
  return 0;
}





void Fl_PostScript_Graphics_Driver::page1(int format) {
  int media=0;
  if (nPages){
    fprintf(output, "CR\nGR\nGR\nGR\nSP\nrestore\n");
  }
  ++nPages;
  fprintf(output, "%%%%Page: %i %i\n" , nPages , nPages);
  fprintf(output, "%%%%PageBoundingBox: 0 0 %d %d\n", (int)pw_ , (int)ph_);
    fprintf(output, "%%%%PageOrientation: Portrait\n");
 /* if (pw_>ph_){
    fprintf(output, "%%%%PageOrientation: Landscape\n");
  }else{
    fprintf(output, "%%%%PageOrientation: Portrait\n");
  }*/
  
  fprintf(output, "%%%%BeginPageSetup\n");
  if((media & Fl_Paged_Device::MEDIA) &&(lang_level_>1)){
    int r = media & Fl_Paged_Device::REVERSED;
    if(r) r = 2;
    fprintf(output, "<< /PageSize [%i %i] /Orientation %i>> setpagedevice\n", (int)(pw_+.5), (int)(ph_+.5), r);
  }
  fprintf(output, "%%%%EndPageSetup\n");
  
/*  pw_ = pw;
  ph_ = ph;*/
  reset();
  
  fprintf(output, "save\n");
  fprintf(output, "GS\n");
  clocale_printf( "%g %g TR\n", (double)0 /*lm_*/ , ph_ /* - tm_*/);
  fprintf(output, "1 -1 SC\n");
  line_style(0);
  fprintf(output, "GS\n");
  
 /* if (!((media & Fl_Paged_Device::MEDIA) &&(lang_level_>1))){
    if (pw_ > ph_) {
      if(media & Fl_Paged_Device::REVERSED) {
        fprintf(output, "-90 rotate %i 0 translate\n", int(-pw_));
	}
      else {
        fprintf(output, "90 rotate -%i -%i translate\n", (lang_level_ == 2 ? int(pw_ - ph_) : 0), int(ph_));
	}
      }
      else {
	if(media & Fl_Paged_Device::REVERSED)
	  fprintf(output, "180 rotate %i %i translate\n", int(-pw_), int(-ph_));
	}
  }*/
  fprintf(output, "GS\nCS\n");
}




//
// End of "$Id$".
//
