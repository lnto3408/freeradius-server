/*
 * rlm_always.c
 *
 *  This program is is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2 if the
 *  License as published by the Free Software Foundation.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright 2000  The FreeRADIUS server project
 */

#include "autoconf.h"
#include "libradius.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "radiusd.h"
#include "modules.h"
#include "conffile.h"

static const char rcsid[] = "$Id$";

/*
 *	The instance data for rlm_always is the list of fake values we are
 *	going to return.
 */
typedef struct rlm_always_t {
	char	*rcode_str;
	int	rcode;
	int	simulcount;
	int	mpp;
} rlm_always_t;

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static CONF_PARSER module_config[] = {
  { "rcode",      PW_TYPE_STRING_PTR, offsetof(rlm_always_t,rcode_str),
    NULL, "fail" },
  { "simulcount", PW_TYPE_INTEGER,    offsetof(rlm_always_t,simulcount),
    NULL, "0" },
  { "mpp",        PW_TYPE_BOOLEAN,    offsetof(rlm_always_t,mpp),
    NULL, "no" },

  { NULL, -1, 0, NULL, NULL }		/* end the list */
};

static int str2rcode(const char *s)
{
	if(!strcasecmp(s, "reject"))
		return RLM_MODULE_REJECT;
	else if(!strcasecmp(s, "fail"))
		return RLM_MODULE_FAIL;
	else if(!strcasecmp(s, "ok"))
		return RLM_MODULE_OK;
	else if(!strcasecmp(s, "handled"))
		return RLM_MODULE_HANDLED;
	else if(!strcasecmp(s, "invalid"))
		return RLM_MODULE_INVALID;
	else if(!strcasecmp(s, "userlock"))
		return RLM_MODULE_USERLOCK;
	else if(!strcasecmp(s, "notfound"))
		return RLM_MODULE_NOTFOUND;
	else if(!strcasecmp(s, "noop"))
		return RLM_MODULE_NOOP;
	else if(!strcasecmp(s, "updated"))
		return RLM_MODULE_UPDATED;
	else {
		radlog(L_ERR|L_CONS,
			"rlm_always: Unknown module rcode '%s'.\n", s);
		return -1;
	}
}

static int always_instantiate(CONF_SECTION *conf, void **instance)
{
	rlm_always_t *data;
	
	/*
	 *	Set up a storage area for instance data
	 */
	data = rad_malloc(sizeof(*data));

	/*
	 *	If the configuration parameters can't be parsed, then
	 *	fail.
	 */
	if (cf_section_parse(conf, data, module_config) < 0) {
		free(data);
		return -1;
	}
	
	/*
	 *	Convert the rcode string to an int, and get rid of it
	 */
	data->rcode = str2rcode(data->rcode_str);
	free(data->rcode_str);
	if (data->rcode == -1) {
		free(data);
		return -1;
	}

	*instance = data;

	return 0;
}

/*
 *	Just return the rcode ... this function is autz, auth, acct, and
 *	preacct!
 */
static int always_return(void *instance, REQUEST *request)
{
	/* quiet the compiler */
	request = request;

	return ((struct rlm_always_t *)instance)->rcode;
}

/*
 *	checksimul fakes some other variables besides the rcode...
 */
static int always_checksimul(void *instance, REQUEST *request)
{
	struct rlm_always_t *inst = instance;

	request->simul_count = inst->simulcount;

	if (inst->mpp)
		request->simul_mpp = 2;

	return inst->rcode;
}

static int always_detach(void *instance)
{
	free(instance);
	return 0;
}

module_t rlm_always = {
	"always",	
	RLM_TYPE_THREAD_SAFE,		/* type */
	NULL,				/* initialization */
	always_instantiate,		/* instantiation */
	always_return,			/* authorization */
	always_return,			/* authentication */
	always_return,			/* preaccounting */
	always_return,			/* accounting */
	always_checksimul,		/* checksimul */
	always_detach,			/* detach */
	NULL,				/* destroy */
};
