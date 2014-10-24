/* ====================================================================
 * Copyright (c) 2000-2001 by Soheil Seyfaie. All rights reserved.
 * This program is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 * ====================================================================
 *
 * $Author: whamann $
 */


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "SWF.h"
#include "perl_swf.h"


MODULE = SWF::SoundInstance    PACKAGE = SWF::SoundInstance     PREFIX = SWFSoundInstance_
PROTOTYPES: ENABLE


void
SWFSoundInstance_DESTROY(inst)
	SWF::SoundInstance	inst
	CODE:
        S_DEBUG(2, fprintf(stderr, "SoundInstance DESTROY CALLED\n"));
        destroySWFSoundInstance(inst);


void
SWFSoundInstance_noMultiple(inst)
	SWF::SoundInstance inst
	CODE:
	SWFSoundInstance_setNoMultiple(inst);

void
SWFSoundInstance_loopInPoint(inst, point)
	SWF::SoundInstance inst
	int point
	CODE:
	SWFSoundInstance_setLoopInPoint(inst, point);

void
SWFSoundInstance_loopOutPoint(inst, point)
	SWF::SoundInstance inst
	int point
	CODE:
	SWFSoundInstance_setLoopOutPoint(inst, point);

void
SWFSoundInstance_loopCount(inst, count)
	SWF::SoundInstance inst
	int count
	CODE:
	SWFSoundInstance_setLoopCount(inst, count);
	

