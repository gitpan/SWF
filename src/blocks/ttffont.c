/*
    Ming, an SWF output library
    Copyright (C) 2002  Opaque Industries - http://www.opaque.net/

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* $Id: ttffont.c,v 1.8 2004/03/30 15:07:28 strk Exp $ */

#if 0

#include <math.h>
#include "font.h"
#include "method.h"
#include "libming.h"

#define glyphLength(font,glyph) \
  ((font)->glyphOffset[(glyph)+1] - (font)->glyphOffset[(glyph)])

SWFFont loadSWFFontfromTTF(char *filename)
{
  int i;

  TT_Engine *engine;
  TT_Face face;
  TT_Instance instance;
  TT_Glyph glyph;
  TT_Glyph_Metrics metrics;
  TT_Outline outline;
  TT_CharMap charmap;
  TT_Face_Properties properties;
  TT_UShort pid, eid;

  TT_Init_FreeType(engine);

  error = TT_Open_Face(engine, filename, &face);

  if(error)
    fprintf(stderr, "Could not open face.\n");

  TT_Get_Face_Properties(face, &properties);

  for(i=0; i<properties->num_CharMaps; ++i)
    TT_Get_CharMap_ID(face, i, &pid, &eid);

  TT_Get_CharMap(face, charmapIndex, &charmap);

  TT_New_Instance(face, &instance);

  TT_New_Glyph(face, &glyph);

  for(i=0; i<whatever; ++i)
  {
    TT_Load_Glyph(instance, glyph, TT_Char_Index(charmap, i), loadFlags);

    TT_Get_Glyph_Outline(glyph, &outline);

    TT_Get_Glyph_Metrics(glyph, &metrics);
  }

  TT_Done_FreeType(engine);
}


int completeSWFFont(SWFBlock block)
{
  SWFFont font = (SWFFont)block;
  int size, i;

  SWFFont_resolveTextList(font);

  size = 2 + 2*font->nGlyphs;

  /* get length of each glyph from its output buffer */
  for(i=0; i<font->nGlyphs; ++i)
    size += glyphLength(font, font->codeToGlyph[i]);

  return size;
}


void writeSWFFontToMethod(SWFBlock block,
			  SWFByteOutputMethod method, void *data)
{
  SWFFont font = (SWFFont)block;
  int offset, i;
  byte *p, *s;

  methodWriteUInt16(CHARACTERID(font), method, data);

  offset = font->nGlyphs*2;

  /* write offset table for glyphs */
  for(i=0; i<font->nGlyphs; ++i)
  {
    methodWriteUInt16(offset, method, data);
    offset += glyphLength(font, font->codeToGlyph[i]);
  }

  /* write shape records for glyphs */
  for(i=0; i<font->nGlyphs; ++i)
  {
    p = font->glyphOffset[font->codeToGlyph[i]];
    s = font->glyphOffset[font->codeToGlyph[i]+1];

    SWF_assert(p < s);

    while(p < s)
      method(*(p++), data);
  }
}


void destroySWFFont(SWFFont font)
{
  free(font->shapes);
  free(font->name);
  free(font->kernTable);
}


SWFFont newSWFFont()
{
  SWFFont font = (SWFFont)malloc(SWFFONT_SIZE);
  memset(font, 0, SWFFONT_SIZE);

  CHARACTER(font)->number = ++SWF_gNumCharacters;
  BLOCK(font)->type = SWF_DEFINEFONT;
  BLOCK(font)->writeBlock = writeSWFFontToMethod;
  BLOCK(font)->complete = completeSWFFont;
  BLOCK(font)->dtor = destroySWFFont;

  return font;
}


SWFFont loadSWFFontFromTTF(char *file)
{
  SWFFont font = newSWFFont();
  font->file = file;
}

void SWFFont_addTextToList(SWFFont font, struct _textRecord *text)
{
  SWFTextList textList = (SWFTextList)malloc(TEXTLIST_SIZE);
  textList->next = NULL;
  textList->text = text;

  if(font->currentList != NULL)
    font->currentList->next = textList;
  else
    font->textList = textList;

  font->currentList = textList;
}

void SWFFont_addCharToTable(SWFFont font, byte c)
{
  if(font->glyphToCode[c]==0xff) /* assuming one can't actually use all 255 */
  {
    font->codeToGlyph[font->nGlyphs] = font->codeTable[c];
    font->glyphToCode[c] = font->nGlyphs;
    ++font->nGlyphs;
  }
}

/* XXX - big confusion between terms here.  CodeTable isn't font->codeTable */
void SWFFont_buildCodeTable(SWFFont font, SWFTextRecord text)
{
  SWFTextRecord textRecord;
  byte *string;
  int l, i;

  textRecord = text;
  while(textRecord != NULL)
  {
    string = textRecord->string;

    if(string != NULL)
    {
      l = strlen(string);
      for(i=0; i<l; ++i)
      	SWFFont_addCharToTable(font, string[i]);
    }

    textRecord = textRecord->next;
  }
}


/* build code table from text in all proceding Text blocks */
void SWFFont_resolveTextList(SWFFont font)
{
  SWFTextList textList, oldList;

  textList = font->textList;
  while(textList != NULL)
  {
    oldList = textList;
    SWFFont_buildCodeTable(font, textList->text);
    textList = textList->next;
    free(oldList);
  }

  font->textList = NULL;
}

#endif
