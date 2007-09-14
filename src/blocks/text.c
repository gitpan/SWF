/*
		Ming, an SWF output library
		Copyright (C) 2002	Opaque Industries - http://www.opaque.net/

		This library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2.1 of the License, or (at your option) any later version.

		This library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
		Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public
		License along with this library; if not, write to the Free Software
		Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
*/

/* $Id: text.c,v 1.30 2006/10/29 11:58:28 fmk_ca Exp $ */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "text.h"
#include "rect.h"
#include "utf8.h"
#include "character.h"
#include "matrix.h"
#include "font.h"

#include "libming.h"

struct SWFText_s
{
	struct SWFCharacter_s character;

	SWFOutput out;
	SWFMatrix matrix;
	byte nAdvanceBits;
	byte nGlyphBits;
	SWFTextRecord initialRecord;
	SWFTextRecord currentRecord;
#if TRACK_ALLOCS
	/* memory node for garbage collection */
	mem_node *gcnode;
#endif
};

struct SWFTextRecord_s
{
	struct SWFTextRecord_s *next;

	byte flags;
	BOOL isBrowserFont;

	// if it's not a browser font, is it a font or a fontchar?
	BOOL isResolved;

	union
	{
		SWFFont font;
		SWFBrowserFont browserFont;
		SWFFontCharacter fontchar;
	} font;

	// color
	byte r;
	byte g;
	byte b;
	byte a;

	// position
	int x;
	int y;

	// metrics
	int height;
	int spacing;

	int strlen;
	unsigned short* string;

	int* advance;
	int nAdvanceBits;
};


static void
writeSWFTextToMethod(SWFBlock block,
										 SWFByteOutputMethod method, void *data)
{
	SWFText text = (SWFText)block;
	int length = 0;
	SWFOutput out;

	if ( text->matrix == NULL )
		text->matrix = newSWFMatrix(0, 0, 0, 0, 0, 0);

	length += (SWFMatrix_numBits(text->matrix)+7)/8;
	length += (SWFRect_numBits(CHARACTER(text)->bounds)+7)/8;
	length += 4;

	out = newSizedSWFOutput(length);

	SWFOutput_writeUInt16(out, CHARACTERID(text));
	SWFOutput_writeRect(out, CHARACTER(text)->bounds);
	SWFOutput_writeMatrix(out, text->matrix);
	SWFOutput_writeUInt8(out, text->nGlyphBits);
	SWFOutput_writeUInt8(out, text->nAdvanceBits);

	SWFOutput_writeToMethod(out, method, data);
	SWFOutput_writeToMethod(text->out, method, data);

	destroySWFOutput(out);
}


static int
completeSWFText(SWFBlock block)
{
	int length = 4;

	SWFText text = (SWFText)block;
	SWFText_resolveCodes(text);

	length += SWFOutput_getLength(text->out);

	if ( text->matrix )
		length += (SWFMatrix_numBits(text->matrix)+7)/8;
	else
		++length;

	length += (SWFRect_numBits(CHARACTER(text)->bounds)+7)/8;

	return length;
}


void
destroySWFText(SWFText text)
{
	SWFTextRecord record = text->initialRecord, next;

	destroySWFOutput(text->out);

	if ( text->matrix != NULL )
		destroySWFMatrix(text->matrix);

	while ( record != NULL )
	{
		next = record->next;
		destroySWFTextRecord(record);
		record = next;
	}

#if TRACK_ALLOCS
	ming_gc_remove_node(text->gcnode);
#endif

	destroySWFCharacter((SWFCharacter) text);
}


SWFText
newSWFText()
{
	SWFText text = (SWFText)malloc(sizeof(struct SWFText_s));

	SWFCharacterInit((SWFCharacter)text);

	CHARACTERID(text) = ++SWF_gNumCharacters;

	BLOCK(text)->type = SWF_DEFINETEXT;
	BLOCK(text)->writeBlock = writeSWFTextToMethod;
	BLOCK(text)->complete = completeSWFText;
	BLOCK(text)->dtor = (destroySWFBlockMethod) destroySWFText;

	CHARACTER(text)->bounds = newSWFRect(0,0,0,0);

	text->out = newSWFOutput();
	text->currentRecord = NULL;
	text->initialRecord = NULL;
	text->matrix = NULL;
	text->nAdvanceBits = 0;

#if TRACK_ALLOCS
	text->gcnode = ming_gc_add_node(text, (dtorfunctype) destroySWFBitmap);
#endif

	return text;
}


/* only dif is type 2 allows transparency.. */
SWFText
newSWFText2()
{
	SWFText text = newSWFText();
	BLOCK(text)->type = SWF_DEFINETEXT2;
	return text;
}


SWFTextRecord
SWFText_addTextRecord(SWFText text)
{
	SWFTextRecord textRecord = (SWFTextRecord)malloc(sizeof(struct SWFTextRecord_s));
	SWFTextRecord current = text->currentRecord;

	textRecord->flags = 0;
	textRecord->string = NULL;
	textRecord->strlen = 0;
	textRecord->next = NULL;
	textRecord->x = 0;
	textRecord->y = 0;
	textRecord->advance = NULL;
	textRecord->nAdvanceBits = 0;

	if ( current == NULL )
	{
		textRecord->isBrowserFont = FALSE;
		textRecord->isResolved = FALSE;
		textRecord->font.font = NULL;
		textRecord->spacing = 0;
		textRecord->height = 240;
		textRecord->r = 0;
		textRecord->g = 0;
		textRecord->b = 0;
		textRecord->a = 0;

		text->initialRecord = textRecord;
	}
	else
	{
		textRecord->isBrowserFont = current->isBrowserFont;
		textRecord->isResolved = current->isResolved;
		textRecord->font = current->font;
		textRecord->spacing = current->spacing;
		textRecord->height = current->height;
		textRecord->r = current->r;
		textRecord->g = current->g;
		textRecord->b = current->b;
		textRecord->a = current->a;

		current->next = textRecord;
	}

	text->currentRecord = textRecord;

	return textRecord;
}


void
destroySWFTextRecord(SWFTextRecord record)
{
	if ( record->string != NULL )
		free(record->string);

	if ( record->advance != NULL )
		free(record->advance);

	free(record);
}


int
SWFTextRecord_getString(SWFTextRecord record, unsigned short** outStr)
{
	*outStr = record->string;
	return record->strlen;
}


SWFTextRecord
SWFText_getInitialRecord(SWFText text)
{
	return text->initialRecord;
}


SWFTextRecord
SWFTextRecord_getNextRecord(SWFTextRecord record)
{
	return record->next;
}


SWFFont
SWFTextRecord_getUnresolvedFont(SWFTextRecord record)
{
	if ( !record->isBrowserFont && !record->isResolved )
		return record->font.font;
	
	return NULL;
}


void
SWFTextRecord_setFontCharacter(SWFTextRecord record, SWFFontCharacter font)
{
	record->font.fontchar = font;
	record->isResolved = TRUE;
	SWFFontCharacter_addTextToList(font, record);
}


int
SWFText_getScaledStringWidth(SWFText text, const char *string)
{
	SWFFont font;
	int height = text->currentRecord->height;
	unsigned short* widestr;
	int len = strlen(string);
	int n, ret;

	widestr = (unsigned short *)malloc(2 * len);
	for(n = 0 ; n < len ; n++)
		widestr[n] = (unsigned char)string[n];

	if ( text->currentRecord->isResolved )
		font = SWFFontCharacter_getFont(text->currentRecord->font.fontchar);
	else
		font = text->currentRecord->font.font;

	if ( text->currentRecord->isBrowserFont )
		ret = 0;
	else
		ret = SWFFont_getScaledWideStringWidth(font, widestr, len) * height / 1024;
	free(widestr);
	return ret;
}

int
SWFText_getScaledUTF8StringWidth(SWFText text, const char *string)
{
	SWFFont font;
	int height = text->currentRecord->height;
	unsigned short* widestr;
	int len, ret;

	len = UTF8ExpandString(string, &widestr);
	if ( text->currentRecord->isResolved )
		font = SWFFontCharacter_getFont(text->currentRecord->font.fontchar);
	else
		font = text->currentRecord->font.font;

	if ( text->currentRecord->isBrowserFont )
		ret = 0;
	else
		ret = SWFFont_getScaledWideStringWidth(font, widestr, len) * height / 1024;
	free(widestr);
	return ret;
}

int
SWFText_getScaledWideStringWidth(SWFText text, const unsigned short *string)
{
	SWFFont font;
	int height = text->currentRecord->height;
	int len;

	for(len = 0 ; *string ; len++)
		;
	if ( text->currentRecord->isResolved )
		font = SWFFontCharacter_getFont(text->currentRecord->font.fontchar);
	else
		font = text->currentRecord->font.font;

	if ( text->currentRecord->isBrowserFont )
		return 0;
	else
		return SWFFont_getScaledWideStringWidth(font, string, len) * height / 1024;
}


short
SWFText_getScaledAscent(SWFText text)
{
	SWFFont font = text->currentRecord->font.font;
	int height = text->currentRecord->height;

	if ( text->currentRecord->isBrowserFont )
		return 0;
	else
		return SWFFont_getScaledAscent(font) * height / 1024;
}


short
SWFText_getScaledDescent(SWFText text)
{
	SWFFont font = text->currentRecord->font.font;
	int height = text->currentRecord->height;

	if ( text->currentRecord->isBrowserFont )
		return 0;
	else
		return SWFFont_getScaledDescent(font) * height / 1024;
}


short
SWFText_getScaledLeading(SWFText text)
{
	SWFFont font = text->currentRecord->font.font;
	int height = text->currentRecord->height;

	if ( text->currentRecord->isBrowserFont )
		return 0;
	else
		return SWFFont_getScaledLeading(font) * height / 1024;
}


void 
SWFText_setFont(SWFText text, void* font)
{
	SWFTextRecord textRecord = text->currentRecord;

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	textRecord->flags |= SWF_TEXT_HAS_FONT;

	/* XXX */
	textRecord->isBrowserFont = (BLOCK(font)->type == SWF_DEFINEEDITTEXT);

	if ( textRecord->isBrowserFont )
		SWF_error("cannot use browser font for SWFText");
//		textRecord->font.browserFont = (SWFBrowserFont)font;
	else
		textRecord->font.font = (SWFFont)font;
}


void
SWFText_setScaledHeight(SWFText text, int height)
{
	SWFTextRecord textRecord = text->currentRecord;

	if ( textRecord == NULL ||
			 (textRecord->string != NULL && height != textRecord->height) )
	{
		textRecord = SWFText_addTextRecord(text);
	}

	textRecord->flags |= SWF_TEXT_HAS_FONT;
	textRecord->height = height;
}


void
SWFText_setScaledSpacing(SWFText text, int spacing)
{
	SWFTextRecord textRecord = text->currentRecord;

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	textRecord->spacing = spacing;
}


void
SWFText_setColor(SWFText text, byte r, byte g, byte b, byte a)
{
	SWFTextRecord textRecord = text->currentRecord;

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	textRecord->flags |= SWF_TEXT_HAS_COLOR;
	textRecord->r = r;
	textRecord->g = g;
	textRecord->b = b;
	textRecord->a = a;
}


void
SWFText_scaledMoveTo(SWFText text, int x, int y)
{
	SWFTextRecord textRecord = text->currentRecord;

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	if ( x != 0 )
	{
		textRecord->flags |= SWF_TEXT_HAS_X;
		textRecord->x = x;
	}

	if ( y != 0 )
	{
		textRecord->flags |= SWF_TEXT_HAS_Y;
		textRecord->y = y;
	}
}


/* adds textrecord type 0 to output */
/* font defn is normalized to height 1024 */
/* glyph codes changed to (correct) character codes by Raff */

void
SWFText_addWideString(SWFText text, const unsigned short* widestring,
											int len, int* advance)
{
	SWFTextRecord textRecord = text->currentRecord;

	/* marginally sloppy to tack on a new record,
		 but I don't want to deal with concats */

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	if ( textRecord->font.font == NULL )
		SWF_error("font must be set before calling addString");

	textRecord->advance = advance;
	textRecord->strlen = len;
	textRecord->string = (unsigned short*)malloc(sizeof(unsigned short) * len);
	memcpy(textRecord->string, widestring, sizeof(unsigned short) * len);
}


void
SWFText_addUTF8String(SWFText text, const char* string, int* advance)
{
	unsigned short* widestring;
	int len = UTF8ExpandString(string, &widestring);

	SWFTextRecord textRecord = text->currentRecord;

	/* marginally sloppy to tack on a new record,
		 but I don't want to deal with concats */

	if ( textRecord == NULL || textRecord->string != NULL )
		textRecord = SWFText_addTextRecord(text);

	if ( textRecord->font.font == NULL )
		SWF_error("font must be set before calling addString");

	textRecord->advance = advance;
	textRecord->strlen = len;
	textRecord->string = widestring;
}


void
SWFText_addString(SWFText text, const char* string, int* advance)
{
	int len = strlen(string);
	int i;
	unsigned short* widestring = (unsigned short*) malloc(len * sizeof(unsigned short) );
	for ( i = 0; i < len; ++i )
		widestring[i] = string[i] & 0xff;
	SWFText_addWideString(text, widestring, len, advance);
	free(widestring);
}


static void
SWFTextRecord_computeAdvances(SWFTextRecord textRecord)
{
	int i;
	int len = textRecord->strlen;
	unsigned short* widestring = textRecord->string;
	unsigned short glyph;

	SWFFontCharacter fontchar = textRecord->font.fontchar;
	SWFFont font = SWFFontCharacter_getFont(fontchar);

	if(!len) return;	// do not try to calculate 1st char of null string

	/* compute advances (spacing) from spacing, advance and kern table */

	if ( textRecord->advance == NULL )
	{
		textRecord->advance = (int*)malloc(sizeof(int) * len);
		memset(textRecord->advance, 0, sizeof(int) * len);
	}

	glyph = SWFFontCharacter_getGlyphCode(fontchar, widestring[0]);

	for ( i=0; i<len; ++i )
	{
		int adv;
		unsigned short nextglyph;

		adv = SWFFont_getCharacterAdvance(font, (unsigned short)glyph);
		adv += textRecord->spacing;

		/* get kerning from font's kern table */

		if ( i < len-1 )
		{	nextglyph = SWFFontCharacter_getGlyphCode(fontchar, widestring[i+1]);
			adv += SWFFont_getCharacterKern(font, glyph, nextglyph);
			glyph = nextglyph;
		}

		if ( textRecord->advance != NULL )
			adv += textRecord->advance[i];

		textRecord->advance[i] = adv * textRecord->height / 1024;

		textRecord->nAdvanceBits = max(textRecord->nAdvanceBits,
																	 SWFOutput_numSBits(textRecord->advance[i]));

	}
}


/* turn textRecord list into output code */

void
SWFText_resolveCodes(SWFText text)
{
	SWFTextRecord textRecord, oldRecord;
	SWFOutput out = text->out;
	int nGlyphBits = 0;
	int len, i;
	int curX = 0, curY = 0, curH = 0;

	textRecord = text->initialRecord;

	while ( textRecord != NULL )
	{
		SWFTextRecord_computeAdvances(textRecord);

		text->nAdvanceBits = max(text->nAdvanceBits, textRecord->nAdvanceBits);

		if ( textRecord->flags & SWF_TEXT_HAS_FONT )
		{
			if ( textRecord->isBrowserFont )
			{
				/* XXX - assume browser fonts have 8bit glyph table? */
				nGlyphBits = max(nGlyphBits, 8);
			}
			else
			{
				int fontGlyphs = SWFFontCharacter_getNGlyphs(textRecord->font.fontchar);
				nGlyphBits = max(nGlyphBits, SWFOutput_numBits(fontGlyphs-1));
			}
		}

		textRecord = textRecord->next;
	}

	textRecord = text->initialRecord;

	while ( textRecord != NULL )
	{
		oldRecord = textRecord;

		if ( textRecord->string == NULL || textRecord->strlen == 0 )
		{
			textRecord = textRecord->next;
			destroySWFTextRecord(oldRecord);
			continue;
		}

		SWFOutput_byteAlign(out);

		/* Raff says the spec lies- there's always a change record, even if
			 it's empty, and the string record length is the full 8 bits. */

		SWFOutput_writeUInt8(out, textRecord->flags | SWF_TEXT_STATE_CHANGE);

		if ( textRecord->flags & SWF_TEXT_HAS_FONT )
		{
			if ( textRecord->isBrowserFont )
				SWFOutput_writeUInt16(out, CHARACTERID(textRecord->font.browserFont));
			else
				SWFOutput_writeUInt16(out, CHARACTERID(textRecord->font.fontchar));
		}

		if ( textRecord->flags & SWF_TEXT_HAS_COLOR )
		{
			SWFOutput_writeUInt8(out, textRecord->r);
			SWFOutput_writeUInt8(out, textRecord->g);
			SWFOutput_writeUInt8(out, textRecord->b);

			if ( BLOCK(text)->type == SWF_DEFINETEXT2 )
				SWFOutput_writeUInt8(out, textRecord->a);
		}

		if ( textRecord->flags & SWF_TEXT_HAS_X )
		{
			SWFOutput_writeUInt16(out, textRecord->x);
			curX = textRecord->x;
		}

		if ( textRecord->flags & SWF_TEXT_HAS_Y )
		{
			SWFOutput_writeUInt16(out, textRecord->y);
			curY = textRecord->y;
		}

		if ( textRecord->flags & SWF_TEXT_HAS_FONT )
		{
			SWFOutput_writeUInt16(out, textRecord->height);
			curH = textRecord->height;
		}

		/* record type 0: string data */

		len = textRecord->strlen;

		if ( len >= 256 )
			SWF_error("Found text record >= 256 characters!");

		SWFOutput_writeUInt8(out, len);

		/* XXX - er, browser fonts in text objects crash the player..
			 Maybe because there's no definefontinfo block? */

		if ( textRecord->isBrowserFont )
		{
			for ( i=0; i<len; ++i )
			{
				SWFOutput_writeBits(out, textRecord->string[i], nGlyphBits);
				SWFOutput_writeBits(out, textRecord->advance[i], text->nAdvanceBits);

				/* XXX - fudging the text character bounds since we don't
					 have font metrics */

				if ( CHARACTER(text)->bounds )
				{
					SWFRect_includePoint(CHARACTER(text)->bounds, curX, curY, 0);
					SWFRect_includePoint(CHARACTER(text)->bounds,
															 curX + curH, curY + curH, 0);
				}
				else
				{
					CHARACTER(text)->bounds =
						newSWFRect(curX, curX + curH, curY, curY + curH);
				}

				curX += curH;
			}
		}
		else
		{
			SWFFontCharacter fontchar = textRecord->font.fontchar;
			SWFFont font = SWFFontCharacter_getFont(fontchar);

			if ( font == NULL )
				SWF_error("Couldn't find font");

			for ( i=0; i<len; ++i )
			{
				SWFRect glyphBounds;
				int minX, maxX, minY, maxY;

				unsigned short code =
					SWFFontCharacter_getGlyphCode(fontchar, textRecord->string[i]);

				glyphBounds = SWFFont_getGlyphBounds(font, code);

				SWFRect_getBounds(glyphBounds, &minX, &maxX, &minY, &maxY);

				SWFOutput_writeBits(out, textRecord->string[i],	nGlyphBits);
				SWFOutput_writeBits(out, textRecord->advance[i], text->nAdvanceBits);

				if ( CHARACTER(text)->bounds )
				{
					SWFRect_includePoint(CHARACTER(text)->bounds,
															 curX + minX * curH / 1024,
															 curY + minY * curH / 1024, 0);

					SWFRect_includePoint(CHARACTER(text)->bounds,
															 curX + maxX * curH / 1024,
															 curY + maxY * curH / 1024, 0);
				}
				else
				{
					CHARACTER(text)->bounds =
						newSWFRect(curX + minX * curH /1024, curX + maxX * curH /1024,
											 curY + minY * curH /1024, curY + maxY * curH /1024);
				}

				if ( textRecord->advance != NULL )
					curX += textRecord->advance[i];
			}
		}

		textRecord = textRecord->next;
		destroySWFTextRecord(oldRecord);
	}

	SWFOutput_writeUInt8(out, 0); /* end text records */

	text->nGlyphBits = nGlyphBits;
	text->initialRecord = NULL;
	text->currentRecord = NULL;
}


/*
 * Local variables:
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
