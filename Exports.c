/* ====================================================================
 * Copyright (c) 2000-2003 by Soheil Seyfaie. All rights reserved.
 * This program is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 * ====================================================================
 *
 * $Id: Exports.c,v 1.7 2003/09/13 09:23:38 peterdd Exp $
 */

#include "perl_swf.h"

static char *SWF_Fill_tag[] = {
     "SWFFILL_SOLID",
     "SWFFILL_GRADIENT",
     "SWFFILL_LINEAR_GRADIENT",
     "SWFFILL_RADIAL_GRADIENT",
     "SWFFILL_BITMAP",
     "SWFFILL_TILED_BITMAP",
     "SWFFILL_CLIPPED_BITMAP",
     NULL,
};

static char *SWF_Button_tag[] = {
     "SWFBUTTON_HIT",
     "SWFBUTTON_DOWN",
     "SWFBUTTON_OVER",
     "SWFBUTTON_UP",
     "SWFBUTTON_MOUSEUPOUTSIDE",
     "SWFBUTTON_DRAGOVER",
     "SWFBUTTON_DRAGOUT",
     "SWFBUTTON_MOUSEUP",
     "SWFBUTTON_MOUSEDOWN",
     "SWFBUTTON_MOUSEOUT",
     "SWFBUTTON_MOUSEOVER",
     "SWFBUTTON_KEYPRESS",
     "SWFBUTTON_ONKEYPRESS",
     NULL,
};

static char *SWF_Sound_tag[] = {
     "SWF_SOUND_NOT_COMPRESSED",
     "SWF_SOUND_ADPCM_COMPRESSED",
     "SWF_SOUND_MP3_COMPRESSED",
     "SWF_SOUND_NOT_COMPRESSED_LE",
     "SWF_SOUND_NELLY_COMPRESSED",
     "SWF_SOUND_5KHZ",
     "SWF_SOUND_11KHZ",
     "SWF_SOUND_22KHZ",
     "SWF_SOUND_44KHZ",
     "SWF_SOUND_8BITS",
     "SWF_SOUND_16BITS",
     "SWF_SOUND_MONO",
     "SWF_SOUND_STEREO",
     NULL,
};

static char *SWF_TextField_tag[] = {
     "SWFTEXTFIELD_NOEDIT",
     "SWFTEXTFIELD_PASSWORD",
     "SWFTEXTFIELD_DRAWBOX",
     "SWFTEXTFIELD_MULTILINE",
     "SWFTEXTFIELD_WORDWRAP",
     "SWFTEXTFIELD_NOSELECT",
     "SWFTEXTFIELD_ALIGN_LEFT",
     "SWFTEXTFIELD_ALIGN_RIGHT",
     "SWFTEXTFIELD_ALIGN_CENTER",
     "SWFTEXTFIELD_ALIGN_JUSTIFY",
     "SWFTEXTFIELD_HTML",
     "SWFTEXTFIELD_HASLENGTH",
     "SWFTEXTFIELD_USEFONT",
     "SWFTEXTFIELD_AUTOSIZE",
     NULL,
};


static char *SWF_DisplayItem_tag[] = {
     "SWFACTION_ONLOAD",
     "SWFACTION_ENTERFRAME",
     "SWFACTION_UNLOAD",
     "SWFACTION_MOUSEMOVE",
     "SWFACTION_MOUSEDOWN",
     "SWFACTION_MOUSEUP",
     "SWFACTION_KEYDOWN",
     "SWFACTION_KEYUP",
     "SWFACTION_DATA",
     NULL,
};


static char **export_tags(char *tag) {
   switch (*tag) {
   case 'B':
     if(strEQ("Button", tag))
       return SWF_Button_tag;
   case 'D':
     if(strEQ("DisplayItem", tag))
       return SWF_DisplayItem_tag;
   case 'F':
     if(strEQ("Fill", tag))
       return SWF_Fill_tag;   
	 case 'S':
     if(strEQ("Sound", tag))
       return SWF_Sound_tag;
   case 'T':
     if(strEQ("Text", tag))
       return SWF_TextField_tag;
   default:
     croak("unknown tag `%s'", tag);
   }
}


void export_cv(SV *class, SV *caller, char *sub) 
{ 
    GV *gv; 
#if 0
    fprintf(stderr, "Here is the result: *%s::%s = \\&%s::%s\n", 
            SvPVX(caller), sub, SvPVX(class), sub); 
#endif 
    gv = gv_fetchpv(form("%_::%s", caller, sub), TRUE, SVt_PVCV); 
    GvCV(gv) = perl_get_cv(form("%_::%s", class, sub), TRUE); 
    GvIMPORTED_CV_on(gv); 
    GvMULTI_on(gv);
} 

static void my_import(SV *pclass, SV *caller, SV *sv)
{
    char *sym = SvPV(sv,na), **tags;
    int i;

    switch (*sym) {
    case ':':
        ++sym;
        tags = export_tags(sym);
        for(i=0; tags[i]; i++) {
            export_cv(pclass, caller, tags[i]);
        }
        break;
    case '$':
    case '%':
    case '*':
    case '@':
        croak("\"%s\" is not exported by the SWF module", sym);
    case '&':
        ++sym;
    default:
        if(isALPHA(sym[0])) {
            export_cv(pclass, caller, sym);
            break;
        }
        else {
            croak("Can't export symbol: %s", sym);
        }
    }
}
