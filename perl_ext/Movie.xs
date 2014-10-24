/* ====================================================================
 * Copyright (c) 2000-2001 by Soheil Seyfaie. All rights reserved.
 * This program is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 * ====================================================================
 *
 * $Author: krechert $
 * $Id: Movie.xs,v 1.13 2007/03/15 20:14:03 krechert Exp $
 */


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "SWF.h"
#include "perl_swf.h"


MODULE = SWF::Movie	PACKAGE = SWF::Movie		PREFIX = SWFMovie_
PROTOTYPES: ENABLE

void
destroySWFMovie(movie)
	SWF::Movie	movie
        ALIAS:
	SWF::Movie::DESTROY = 1
	CODE:
        S_DEBUG(2, fprintf(stderr, "Movie DESTROY CALLED\n"));
	swf_stash_refcnt_dec((SV*)SvRV(ST(0)));
        destroySWFMovie(movie);

SWF::Movie
SWFMovie_new(package="SWF::Movie")
	char *package
	CODE:
        RETVAL = newSWFMovie();
        ST(0) = sv_newmortal();
        sv_setref_pv(ST(0), package, (void*)RETVAL);


SWF::Movie
newSWFMovieWithVersion(version)
	int	version

void
SWFMovie_setRate(movie, rate)
	SWF::Movie	movie
	float	rate

void
SWFMovie_setDimension(movie, x, y)
	SWF::Movie	movie
	int	x
	int	y

void
SWFMovie_setNumberOfFrames(movie, frames)
	SWF::Movie	movie
	int	frames
	ALIAS:
	SWF::Movie::setFrames = 1

void
SWFMovie_setBackground(movie, r, g, b)
	SWF::Movie	movie
	int	r
	int	g
	int	b

void
SWFMovie_protect(movie, password=0)
	SWF::Movie	movie
	char	*password

void
SWFMovie_nextFrame(movie)
	SWF::Movie	movie

void
SWFMovie_labelFrame(movie, label)
	SWF::Movie	movie
	char *	label

void
SWFMovie_namedAnchor(movie, label)
	SWF::Movie	movie
	char *	label

int 
SWFMovie_xs_output(movie, compresslevel=-1)
	SWF::Movie movie
	int compresslevel
	CODE:
	if(compresslevel >= -1)
		Ming_setSWFCompression(compresslevel);
	RETVAL = SWFMovie_output(movie, fileOutputMethod, stdout);

int 
SWFMovie_save(movie, filename, compresslevel=-1)
	SWF::Movie movie
        char *     filename
	int compresslevel
        PREINIT: 
        FILE    *f;
	CODE:
	if (!(f = fopen(filename, "wb"))) {
		fprintf(stderr, "Unable to open %s\n", filename);
		ST(0) = &sv_undef;
	}else{
		if(compresslevel >= -1)
			Ming_setSWFCompression(compresslevel);
                RETVAL = SWFMovie_output(movie, fileOutputMethod, f);
                fclose(f);
	}
        OUTPUT:
        RETVAL

void 
SWFMovie_addExport(movie, block, name)
	SWF::Movie movie 
	SWF::Block block = (SWF__Block) SvIV((SV*)SvRV(ST(1)));
	char *name
	CODE:
	swf_stash_refcnt_inc((SV*)SvRV(ST(0)), (SV*)SvRV(ST(1)));
	SWFMovie_addExport(movie, block, name);

void
SWFMovie_writeExports(movie)
	SWF::Movie movie

SWF::DisplayItem
SWFMovie_add(movie, block)
	SWF::Movie movie
	SWF::Block block = (SWF__Block) SvIV((SV*)SvRV(ST(1)));
	CODE:
	swf_stash_refcnt_inc((SV*)SvRV(ST(0)), (SV*)SvRV(ST(1)));
	RETVAL = SWFMovie_add(movie, block);
	ST(0) = sv_newmortal();
	sv_setref_pv(ST(0), "SWF::DisplayItem", (void*)RETVAL);

void
SWFMovie_remove(movie, item)
        SWF::Movie movie
        SWF::DisplayItem item

void
SWFMovie_setSoundStream(movie, sound)
	SWF::Movie movie
	SWF::SoundStream sound
	CODE:
	swf_stash_refcnt_inc((SV*)SvRV(ST(0)), (SV*)SvRV(ST(1)));
	SWFMovie_setSoundStream(movie, sound);

SWF::SoundInstance
SWFMovie_startSound(movie, sound) 
	SWF::Movie movie
	SWF::Sound sound
	CODE:
	swf_stash_refcnt_inc((SV*)SvRV(ST(0)), (SV*)SvRV(ST(1)));
	RETVAL = SWFMovie_startSound(movie, sound);
	ST(0) = sv_newmortal();
	sv_setref_pv(ST(0), "SWF::SoundInstance", (void*)RETVAL);

void
SWFMovie_stopSound(movie, sound)
	SWF::Movie movie
	SWF::Sound sound
	CODE:
	SWFMovie_stopSound(movie, sound);

void 
SWFMovie_setNetworkAccess(movie, flag)
	SWF::Movie movie
	int flag

void 
SWFMovie_addMetadata(movie, xml)
	SWF::Movie movie
	char *xml

void 
SWFMovie_setScriptLimits(movie, maxRecursion, timeout)
	SWF::Movie movie
	int maxRecursion
	int timeout

void 
SWFMovie_setTabIndex(movie, depth, index)
	SWF::Movie movie
	int depth
	int index
