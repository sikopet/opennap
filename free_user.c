/* Copyright (C) 2000 drscholl@users.sourceforge.net
   This is free software distributed under the terms of the
   GNU Public License.  See the file COPYING for details.

   $Id$ */

#include <stdlib.h>
#include "opennap.h"
#include "debug.h"

void
free_user (USER * user)
{
    int i;
    HOTLIST *hotlist;

    ASSERT (validate_user (user));

    if (user->con && Num_Servers)
    {
	/* local user, notify peers of this user's departure */
	log ("free_user(): sending QUIT message for user %s", user->nick);
	pass_message_args (user->con, MSG_CLIENT_QUIT, "%s", user->nick);
    }

    /* this marks all the files this user was sharing as invalid.  they
       get reaped in the process of searching to avoid having to remove
       them here */
    if (user->files)
	free_hash (user->files);

    /* remove this user from any channels they were on */
    if (user->channels)
    {
	for (i = 0; i < user->numchannels; i++)
	{
	    /* notify locally connected clients in the same channel that
	       this user has parted */
	    part_channel (user->channels[i], user);
	}
	FREE (user->channels);
    }

    ASSERT (Num_Files >= user->shared);
    Num_Files -= user->shared;
    Num_Gigs -= user->libsize; /* this is in kB */

    /* check the global hotlist for this user to see if anyone wants notice
       of this user's departure */
    hotlist = hash_lookup (Hotlist, user->nick);
    if (hotlist)
    {
	ASSERT (validate_hotlist (hotlist));
	ASSERT (hotlist->numusers > 0);
	for (i = 0; i < hotlist->numusers; i++)
	    send_cmd (hotlist->users[i], MSG_SERVER_USER_SIGNOFF, "%s",
		user->nick);
    }

    FREE (user->nick);
    FREE (user->pass);
    FREE (user->clientinfo);
    FREE (user->server);
    if (user->email)
	FREE (user->email);
    FREE (user);
}
