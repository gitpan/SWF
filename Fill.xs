/* ====================================================================
 * Copyright (c) 2000-2001 by Soheil Seyfaie. All rights reserved.
 * This program is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 * ====================================================================
 *
 * $Author: soheil $
 * $Id: Fill.xs,v 1.1 2001/09/24 00:37:41 soheil Exp $
 */


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "SWF.h"
#include "perl_swf.h"

MODULE = SWF::Fill		PACKAGE = SWF::Fill		PREFIX = SWFFill_
PROTOTYPES: ENABLE


void 
SWFFill_moveTo(fill, x, y)
        SWF::Fill       fill
        int             x
        int             y

void
SWFFill_scaleTo(fill, x, ...)
	SWF::Fill       fill
        float           x
        float           y = NO_INIT
        CODE:
        y = (items > 2) ? (float)SvNV(ST(2)) : x; 
        SWFFill_scaleXYTo(fill, x, y);

void
SWFFill_rotateTo(fill, degrees)
        SWF::Fill       fill
        float           degrees

void
SWFFill_skewXTo(fill, x)
        SWF::Fill       fill
        float           x

void
SWFFill_skewYTo(fill, y)
        SWF::Fill       fill
        float           y


void
SWFFill_DESTROY(fill)
	SWF::Fill	fill
	CODE:
        S_DEBUG(2, fprintf(stderr, "Fill DESTROY CALLED\n"));
