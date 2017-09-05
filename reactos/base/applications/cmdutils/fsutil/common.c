/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS FS utility tool
 * FILE:            base/applications/cmdutils/common.c
 * PURPOSE:         FSutil common functions
 * PROGRAMMERS:     Pierre Schweitzer <pierre@reactos.org>
 */

#include "fsutil.h"

int FindHandler(int argc,
                const TCHAR *argv[],
                HandlerItem * HandlersList,
                int HandlerListCount,
                void (*UsageHelper)(const TCHAR *))
{
    int i;
    int ret;
    const TCHAR * Command;

    ret = 1;
    Command = NULL;
    i = HandlerListCount;

    /* If we have a command, does it match a known one? */
    if (argc > 1)
    {
        /* Browse all the known commands finding the right one */
        Command = argv[1];
        for (i = 0; i < HandlerListCount; ++i)
        {
            if (_tcsicmp(Command, HandlersList[i].Command) == 0)
            {
                ret = HandlersList[i].Handler(argc - 1, &argv[1]);
                break;
            }
        }
    }

    /* We failed finding someone to handle the caller's needs, print out */
    if (i == HandlerListCount)
    {
        UsageHelper(Command);
    }

    return ret;
}
