/**************************************************************************

xdbg

Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved

Contact: Boram Park <boram1288.park@samsung.com>
         Sangjin LEE <lsj119@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

#include <dix.h>
#define XREGISTRY
#include <registry.h>
#include <xace.h>
#include <xacestr.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <windowstr.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_core.h"
#include "xdbg_evlog.h"


static char*
_getWindowAttributeMask (CARD32 mask, char *reply, int *len)
{
    int i;
    int init = 0;

    for (i = 0 ; i < sizeof(mask) * 4 ; i++)
    {
        if(mask & (1 << i))
        {
            if (init)
                REPLY(" ");
            else
                init = 1;

            switch (1 << i)
            {
                case CWBackPixmap: REPLY("CWBackPixmap"); break;
                case CWBackPixel: REPLY("CWBackPixel"); break;
                case CWBorderPixmap: REPLY("CWBorderPixmap"); break;
                case CWBorderPixel: REPLY("CWBorderPixel"); break;
                case CWBitGravity: REPLY("CWBitGravity"); break;
                case CWWinGravity: REPLY("CWWinGravity"); break;
                case CWBackingStore: REPLY("CWBackingStore"); break;
                case CWBackingPlanes: REPLY("CWBackingPlanes"); break;
                case CWBackingPixel: REPLY("CWBackingPixel"); break;
                case CWOverrideRedirect: REPLY("CWOverrideRedirect"); break;
                case CWSaveUnder: REPLY("CWSaveUnder"); break;
                case CWEventMask: REPLY("CWEventMask"); break;
                case CWDontPropagate: REPLY("CWDontPropagate"); break;
                case CWColormap: REPLY("CWColormap"); break;
                case CWCursor: REPLY("CWCursor"); break;
            }
        }
    }

    return reply;
}

static char*
_getConfigureWindowMask (CARD16 mask, char *reply, int *len)
{
    int i;
    int init = 0;

    for (i = 0 ; i < sizeof(mask) * 4 ; i++)
    {
        if(mask & (1 << i))
        {
            if (init)
                REPLY(" ");
            else
                init = 1;

            switch (1 << i)
            {
                case CWX: REPLY("CWX"); break;
                case CWY: REPLY("CWY"); break;
                case CWWidth: REPLY("CWWidth"); break;
                case CWHeight: REPLY("CWHeight"); break;
                case CWBorderWidth: REPLY("CWBorderWidth"); break;
                case CWSibling: REPLY("CWSibling"); break;
                case CWStackMode: REPLY("CWStackMode"); break;
            }
        }
    }

    return reply;
}

static char*
_getKeyMask (CARD16 mask, char *reply, int *len)
{
    int i;
    int init = 0;

    for (i = 0 ; i < sizeof(mask) * 4 ; i++)
    {
        if(mask & (1 << i))
        {
            if (init)
                REPLY(" ");
            else
                init = 1;

            switch (1 << i)
            {
                case ShiftMask: REPLY("ShiftMask"); break;
                case LockMask: REPLY("LockMask"); break;
                case ControlMask: REPLY("ControlMask"); break;
                case Mod1Mask: REPLY("Mod1Mask"); break;
                case Mod2Mask: REPLY("Mod2Mask"); break;
                case Mod3Mask: REPLY("Mod3Mask"); break;
                case Mod4Mask: REPLY("Mod4Mask"); break;
                case Mod5Mask: REPLY("Mod5Mask"); break;
                case Button1Mask: REPLY("Button1Mask"); break;
                case Button2Mask: REPLY("Button2Mask"); break;
                case Button3Mask: REPLY("Button3Mask"); break;
                case Button4Mask: REPLY("Button4Mask"); break;
                case Button5Mask: REPLY("Button5Mask"); break;
                case AnyModifier: REPLY("AnyModifier"); break;
            }
        }
    }

    return reply;
}

static char*
_getEventMask (CARD32 mask, char *reply, int *len)
{
    int i;
    int init = 0;

    for (i = 0 ; i < sizeof(mask) * 4 ; i++)
    {
        if(mask & (1 << i))
        {
            if (init)
                REPLY(" ");
            else
                init = 1;

            switch (1 << i)
            {
                case NoEventMask: REPLY("NoEventMask"); break;
                case KeyPressMask: REPLY("KeyPressMask"); break;
                case KeyReleaseMask: REPLY("KeyReleaseMask"); break;
                case ButtonPressMask: REPLY("ButtonPressMask"); break;
                case ButtonReleaseMask: REPLY("ButtonReleaseMask"); break;
                case EnterWindowMask: REPLY("EnterWindowMask"); break;
                case LeaveWindowMask: REPLY("LeaveWindowMask"); break;
                case PointerMotionMask: REPLY("PointerMotionMask"); break;
                case PointerMotionHintMask: REPLY("PointerMotionHintMask"); break;
                case Button1MotionMask: REPLY("Button1MotionMask"); break;
                case Button2MotionMask: REPLY("Button2MotionMask"); break;
                case Button3MotionMask: REPLY("Button3MotionMask"); break;
                case Button4MotionMask: REPLY("Button4MotionMask"); break;
                case Button5MotionMask: REPLY("Button5MotionMask"); break;
                case ButtonMotionMask: REPLY("ButtonMotionMask"); break;
                case KeymapStateMask: REPLY("KeymapStateMask"); break;
                case ExposureMask: REPLY("ExposureMask"); break;
                case VisibilityChangeMask: REPLY("VisibilityChangeMask"); break;
                case StructureNotifyMask: REPLY("StructureNotifyMask"); break;
                case ResizeRedirectMask: REPLY("ResizeRedirectMask"); break;
                case SubstructureNotifyMask: REPLY("SubstructureNotifyMask"); break;
                case SubstructureRedirectMask: REPLY("SubstructureRedirectMask"); break;
                case FocusChangeMask: REPLY("FocusChangeMask"); break;
                case PropertyChangeMask: REPLY("PropertyChangeMask"); break;
                case ColormapChangeMask: REPLY("ColormapChangeMask"); break;
                case OwnerGrabButtonMask: REPLY("OwnerGrabButtonMask"); break;
            }
        }
    }

    return reply;
}

char * xDbgEvlogRequestCore (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->reqType)
    {
    case X_CreateWindow:
        {
            xCreateWindowReq *stuff = (xCreateWindowReq *)req;

            REPLY (": Window(0x%x) Parent(0x%x) size(%dx%d) boaderWid(%d) coordinate(%d,%d)",
                (unsigned int)stuff->wid,
                (unsigned int)stuff->parent,
                stuff->width,
                stuff->height,
                stuff->borderWidth,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *visual, *class;
                char dvisual[10], dclass[10];

                switch (stuff->visual)
                {
                    case CopyFromParent:  visual = "CopyFromParent"; break;
                    default:  visual = dvisual; sprintf (dvisual, "0x%x", (unsigned int)stuff->visual); break;
                }

                switch (stuff->class)
                {
                    case CopyFromParent:  class = "CopyFromParent"; break;
                    case InputOutput:  class = "InputOutput"; break;
                    case InputOnly:  class = "InputOnly"; break;
                    default:  class = dclass; sprintf (dclass, "0x%x", stuff->class); break;
                }

                REPLY ("\n");
                REPLY ("%67s depth(%d) visual_ID(%s) class(%s)\n",
                    " ",
                    stuff->depth,
                    visual,
                    class);

               REPLY ("%67s mask", " ");
               REPLY ("(");
                reply = _getWindowAttributeMask(stuff->mask, reply, len);
               REPLY (")");
            }

            return reply;
        }

    case X_ChangeWindowAttributes:
        {
            xChangeWindowAttributesReq *stuff = (xChangeWindowAttributesReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
               REPLY (" value_mask");
               REPLY ("(");
                reply = _getWindowAttributeMask(stuff->valueMask, reply, len);
               REPLY (")");
            }

            return reply;
        }

    case X_ChangeSaveSet:
        {
            xChangeSaveSetReq *stuff = (xChangeSaveSetReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *mode;
                char dmode[10];

                switch (stuff->mode)
                {
                    case SetModeInsert:  mode = "SetModeInsert"; break;
                    case SetModeDelete:  mode = "SetModeDelete"; break;
                    default:  mode = dmode; sprintf (dmode, "%d", stuff->mode); break;
                }

                REPLY (" mode(%s)",
                    mode);
            }

            return reply;
        }

    case X_ReparentWindow:
        {
            xReparentWindowReq *stuff = (xReparentWindowReq *)req;
            REPLY (": Window(0x%x) Parent(0x%x) coord(%d,%d)",
                (unsigned int)stuff->window,
                (unsigned int)stuff->parent,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ConfigureWindow:
        {
            xConfigureWindowReq *stuff = (xConfigureWindowReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" mask");
                REPLY ("(");
                reply = _getConfigureWindowMask(stuff->mask, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_CirculateWindow:
        {
            xCirculateWindowReq *stuff = (xCirculateWindowReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *direction;
                char ddirection[10];

                switch (stuff->direction)
                {
                    case RaiseLowest:  direction = "RaiseLowest"; break;
                    case LowerHighest:  direction = "LowerHighest"; break;
                    default:  direction = ddirection; sprintf (ddirection, "%d", stuff->direction); break;
                }

                REPLY (" direction(%s)",
                    direction);
            }

            return reply;
        }

    case X_ChangeProperty:
        {
            xChangePropertyReq *stuff = (xChangePropertyReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            REPLY (" Type");
            reply = xDbgGetAtom(stuff->type, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *mode;
                char dmode[10];

                switch (stuff->mode)
                {
                    case PropModeReplace:  mode = "PropModeReplace"; break;
                    case PropModePrepend:  mode = "PropModePrepend"; break;
                    case PropModeAppend:  mode = "PropModeAppend"; break;
                    default:  mode = dmode; sprintf (dmode, "%d", stuff->mode); break;
                }

                REPLY ("\n");
                REPLY ("%67s mode(%s) format(%d) nUnits(%ld)",
                    " ",
                    mode,
                    stuff->format,
                    stuff->nUnits);
            }

            return reply;
        }

    case X_DeleteProperty:
        {
            xDeletePropertyReq *stuff = (xDeletePropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            return reply;
        }

    case X_GetProperty:
        {
            xGetPropertyReq *stuff = (xGetPropertyReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);
            REPLY (" Type");
            reply = xDbgGetAtom(stuff->type, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s delete(%s) longOffset(%ld) longLength(%ld)",
                    " ",
                    stuff->delete ? "YES" : "NO",
                    stuff->longOffset,
                    stuff->longLength);
            }

            return reply;
        }

    case X_SetSelectionOwner:
        {
            xSetSelectionOwnerReq *stuff = (xSetSelectionOwnerReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            REPLY (" Selection");
            reply = xDbgGetAtom(stuff->selection, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    stuff->time);
            }

            return reply;
        }

    case X_ConvertSelection:
        {
            xConvertSelectionReq *stuff = (xConvertSelectionReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->requestor);

            REPLY (" Selection");
            reply = xDbgGetAtom(stuff->selection, evinfo, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(stuff->target, evinfo, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    stuff->time);
            }

            return reply;
        }

    case X_SendEvent:
        {
            xSendEventReq *stuff = (xSendEventReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->destination);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" propagate(%s)",
                    stuff->propagate ? "YES" : "NO");

                REPLY (" event_mask");
                REPLY ("(");
                reply = _getEventMask(stuff->eventMask, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_GrabPointer:
        {
            xGrabPointerReq *stuff = (xGrabPointerReq *)req;

            REPLY (": XID(0x%x) ConfineTo(0x%x) Cursor(0x%x)",
                (unsigned int)stuff->grabWindow,
                (unsigned int)stuff->confineTo,
                (unsigned int)stuff->cursor);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *pointer_mode, *keyboard_mode;
                char dpointer_mode[10], dkeyboard_mode[10];

                switch (stuff->pointerMode)
                {
                    case GrabModeSync:  pointer_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  pointer_mode = "GrabModeAsync"; break;
                    default:  pointer_mode = dpointer_mode; sprintf (dpointer_mode, "%d", stuff->pointerMode); break;
                }

                switch (stuff->keyboardMode)
                {
                    case GrabModeSync:  keyboard_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  keyboard_mode = "GrabModeAsync"; break;
                    default:  keyboard_mode = dkeyboard_mode; sprintf (dkeyboard_mode, "%d", stuff->keyboardMode); break;
                }

                REPLY (" pointer_mode(%s) keyboard_mode(%s) time(%lums)\n",
                    pointer_mode,
                    keyboard_mode,
                    stuff->time);

                REPLY (" event_mask");
                REPLY ("(");
                reply = _getEventMask(stuff->eventMask, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_GrabButton:
        {
            xGrabButtonReq *stuff = (xGrabButtonReq *)req;

            REPLY (": XID(0x%x) ConfineTo(0x%x) Cursor(0x%x)",
                (unsigned int)stuff->grabWindow,
                (unsigned int)stuff->confineTo,
                (unsigned int)stuff->cursor);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *pointer_mode, *keyboard_mode, *button;
                char dpointer_mode[10], dkeyboard_mode[10], dbutton[10];

                switch (stuff->pointerMode)
                {
                    case GrabModeSync:  pointer_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  pointer_mode = "GrabModeAsync"; break;
                    default:  pointer_mode = dpointer_mode; sprintf (dpointer_mode, "%d", stuff->pointerMode); break;
                }

                switch (stuff->keyboardMode)
                {
                    case GrabModeSync:  keyboard_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  keyboard_mode = "GrabModeAsync"; break;
                    default:  keyboard_mode = dkeyboard_mode; sprintf (dkeyboard_mode, "%d", stuff->keyboardMode); break;
                }

                switch (stuff->button)
                {
                    case Button1:  button = "Button1"; break;
                    case Button2:  button = "Button2"; break;
                    case Button3:  button = "Button3"; break;
                    case Button4:  button = "Button4"; break;
                    case Button5:  button = "Button5"; break;
                    default:  button = dbutton; sprintf (dbutton, "%d", stuff->button); break;
                }

                REPLY ("\n");
                REPLY ("%67s event_mask(0x%x) pointer_mode(%s) keyboard_mode(%s) button(%s)",
                    " ",
                    stuff->eventMask,
                    pointer_mode,
                    keyboard_mode,
                    button);

                REPLY (" modifiers");
                REPLY ("(");
                reply = _getKeyMask(stuff->modifiers, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_UngrabButton:
        {
            xUngrabButtonReq *stuff = (xUngrabButtonReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" modifiers");
                REPLY ("(");
                reply = _getKeyMask(stuff->modifiers, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_ChangeActivePointerGrab:
        {
            xChangeActivePointerGrabReq *stuff = (xChangeActivePointerGrabReq *)req;
            REPLY (": Cursor(0x%x)",
                (unsigned int)stuff->cursor);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    stuff->time);

                REPLY (" event_mask");
                REPLY ("(");
                reply = _getEventMask(stuff->eventMask, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_GrabKeyboard:
        {
            xGrabKeyboardReq *stuff = (xGrabKeyboardReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *pointer_mode, *keyboard_mode;
                char dpointer_mode[10], dkeyboard_mode[10];

                switch (stuff->pointerMode)
                {
                    case GrabModeSync:  pointer_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  pointer_mode = "GrabModeAsync"; break;
                    default:  pointer_mode = dpointer_mode; sprintf (dpointer_mode, "%d", stuff->pointerMode); break;
                }

                switch (stuff->keyboardMode)
                {
                    case GrabModeSync:  keyboard_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  keyboard_mode = "GrabModeAsync"; break;
                    default:  keyboard_mode = dkeyboard_mode; sprintf (dkeyboard_mode, "%d", stuff->keyboardMode); break;
                }

                REPLY (" owner_events(%s) pointer_mode(%s) keyboard_mode(%s)",
                    stuff->ownerEvents ? "YES" : "NO",
                    pointer_mode,
                    keyboard_mode);
            }

            return reply;
        }

    case X_GrabKey:
        {
            xGrabKeyReq *stuff = (xGrabKeyReq *)req;

            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *pointer_mode, *keyboard_mode;
                char dpointer_mode[10], dkeyboard_mode[10];

                switch (stuff->pointerMode)
                {
                    case GrabModeSync:  pointer_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  pointer_mode = "GrabModeAsync"; break;
                    default:  pointer_mode = dpointer_mode; sprintf (dpointer_mode, "%d", stuff->pointerMode); break;
                }

                switch (stuff->keyboardMode)
                {
                    case GrabModeSync:  keyboard_mode = "GrabModeSync"; break;
                    case GrabModeAsync:  keyboard_mode = "GrabModeAsync"; break;
                    default:  keyboard_mode = dkeyboard_mode; sprintf (dkeyboard_mode, "%d", stuff->keyboardMode); break;
                }

                REPLY (" key(%d) pointer_mode(%s) keyboard_mode(%s)\n",
                    stuff->key,
                    pointer_mode,
                    keyboard_mode);

                REPLY (" modifiers");
                REPLY ("(");
                reply = _getKeyMask(stuff->modifiers, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_UngrabKey:
        {
            xUngrabKeyReq *stuff = (xUngrabKeyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->grabWindow);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" key(%d)",
                    stuff->key);

                REPLY (" modifiers");
                REPLY ("(");
                reply = _getKeyMask(stuff->modifiers, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case X_SetInputFocus:
        {
            xSetInputFocusReq *stuff = (xSetInputFocusReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->focus);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" reverTo(%d) time(%lums)",
                    stuff->revertTo,
                    stuff->time);
            }

            return reply;
        }

    case X_CreatePixmap:
        {
            xCreatePixmapReq *stuff = (xCreatePixmapReq *)req;
            REPLY (": Pixmap(0x%x) Drawable(0x%x) size(%dx%d)",
                (unsigned int)stuff->pid,
                (unsigned int)stuff->drawable,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" depth(%d)",
                    stuff->depth);
            }

            return reply;
        }

    case X_ClearArea:
        {
            xClearAreaReq *stuff = (xClearAreaReq *)req;
            REPLY (": XID(0x%x) area(%d,%d %dx%d)",
                (unsigned int)stuff->window,
                stuff->x,
                stuff->y,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" exposures(%s)",
                    stuff->exposures ? "YES" : "NO");
            }

           return reply;
        }

    case X_CopyArea:
        {
            xCopyAreaReq *stuff = (xCopyAreaReq *)req;
            REPLY (": srcXID(0x%x) dstXID(0x%x) gc(0x%x) size(%dx%d) src(%d,%d) dst(%d,%d)",
                (unsigned int)stuff->srcDrawable,
                (unsigned int)stuff->dstDrawable,
                (unsigned int)stuff->gc,
                stuff->width,
                stuff->height,
                stuff->srcX,
                stuff->srcY,
                stuff->dstX,
                stuff->dstY);

            return reply;
        }

    case X_CopyPlane:
        {
            xCopyPlaneReq *stuff = (xCopyPlaneReq *)req;
            REPLY (": srcXID(0x%x) dstXID(0x%x) gc(0x%x) size(%dx%d) src(%d,%d) dst(%d,%d)",
                (unsigned int)stuff->srcDrawable,
                (unsigned int)stuff->dstDrawable,
                (unsigned int)stuff->gc,
                stuff->width,
                stuff->height,
                stuff->srcX,
                stuff->srcY,
                stuff->dstX,
                stuff->dstY);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" bit_plane(0x%x)",
                    (unsigned int)stuff->bitPlane);
            }

            return reply;
        }

    case X_PolyPoint:
        {
            xPolyPointReq *stuff = (xPolyPointReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *coord_mode;
                char dcoord_mode[10];

                switch (stuff->coordMode)
                {
                    case CoordModeOrigin:  coord_mode = "CoordModeOrigin"; break;
                    case CoordModePrevious:  coord_mode = "CoordModePrevious"; break;
                    default:  coord_mode = dcoord_mode; sprintf (dcoord_mode, "%d", stuff->coordMode); break;
                }

                REPLY (" coord_mode(%s)",
                    coord_mode);
            }

            return reply;
        }

    case X_PolyLine:
        {
            xPolyLineReq *stuff = (xPolyLineReq *)req;
            REPLY (": XID(0x%x gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *coord_mode;
                char dcoord_mode[10];

                switch (stuff->coordMode)
                {
                    case CoordModeOrigin:  coord_mode = "CoordModeOrigin"; break;
                    case CoordModePrevious:  coord_mode = "CoordModePrevious"; break;
                    default:  coord_mode = dcoord_mode; sprintf (dcoord_mode, "%d", stuff->coordMode); break;
                }

                REPLY (" coord_mode(%s)",
                    coord_mode);
            }

            return reply;
        }

    case X_PolySegment:
        {
            xPolySegmentReq *stuff = (xPolySegmentReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            return reply;
        }

    case X_PolyRectangle:
        {
            xPolyRectangleReq *stuff = (xPolyRectangleReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            return reply;
        }

    case X_PolyArc:
        {
            xPolyArcReq *stuff = (xPolyArcReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            return reply;
        }

    case X_FillPoly:
        {
            xFillPolyReq *stuff = (xFillPolyReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *shape, *coord_mode;
                char dshape[10], dcoord_mode[10];

                switch (stuff->shape)
                {
                    case Complex:  shape = "Complex"; break;
                    case Nonconvex:  shape = "Nonconvex"; break;
                    case Convex:  shape = "Convex"; break;
                    default:  shape = dshape; sprintf (dshape, "%d", stuff->shape); break;
                }

                switch (stuff->coordMode)
                {
                    case CoordModeOrigin:  coord_mode = "CoordModeOrigin"; break;
                    case CoordModePrevious:  coord_mode = "CoordModePrevious"; break;
                    default:  coord_mode = dcoord_mode; sprintf (dcoord_mode, "%d", stuff->coordMode); break;
                }

                REPLY (" shape(%s) coord_mode(%s)",
                    shape,
                    coord_mode);
            }

            return reply;
        }

    case X_PolyFillRectangle:
        {
            xPolyFillRectangleReq *stuff = (xPolyFillRectangleReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            return reply;
        }

    case X_PolyFillArc:
        {
            xPolyFillArcReq *stuff = (xPolyFillArcReq *)req;
            REPLY (": XID(0x%x) gc(0x%x)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc);

            return reply;
        }

    case X_PutImage:
        {
            xPutImageReq *stuff = (xPutImageReq *)req;
            REPLY (": XID(0x%x) gc(0x%x) size(%dx%d) dst(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->width,
                stuff->height,
                stuff->dstX,
                stuff->dstY);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *format;
                char dformat[10];

                switch (stuff->format)
                {
                    case XYBitmap:  format = "XYBitmap"; break;
                    case XYPixmap:  format = "XYPixmap"; break;
                    case ZPixmap:  format = "ZPixmap"; break;
                    default:  format = dformat; sprintf (dformat, "%d", stuff->format); break;
                }

                REPLY (" format(%s) depth(%d)",
                    format,
                    stuff->depth);
            }

            return reply;
        }

    case X_GetImage:
        {
            xGetImageReq *stuff = (xGetImageReq *)req;
            REPLY (": XID(0x%x) size(%dx%d) dst(%d,%d)",
                (unsigned int)stuff->drawable,
                stuff->width,
                stuff->height,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *format;
                char dformat[10];

                switch (stuff->format)
                {
                    case XYBitmap:  format = "XYBitmap"; break;
                    case XYPixmap:  format = "XYPixmap"; break;
                    case ZPixmap:  format = "ZPixmap"; break;
                    default:  format = dformat; sprintf (dformat, "%d", stuff->format); break;
                }

                REPLY (" format(%s) plane_mask(0x%x)",
                    format,
                    (unsigned int)stuff->planeMask);
            }

            return reply;
        }

    case X_PolyText8:
        {
            xPolyText8Req *stuff = (xPolyText8Req *)req;
            REPLY (": XID(0x%x) gc(0x%x) coord(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_PolyText16:
        {
            xPolyText16Req *stuff = (xPolyText16Req *)req;
            REPLY (": XID(0x%x) gc(0x%x) coord(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ImageText8:
        {
            xImageText8Req *stuff = (xImageText8Req *)req;
            REPLY (": XID(0x%x) gc(0x%x) coord(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" nchars(%d)",
                    stuff->nChars);
            }

            return reply;
        }

    case X_ImageText16:
        {
            xImageText16Req *stuff = (xImageText16Req *)req;
            REPLY (": XID(0x%x) gc(0x%x) coord(%d,%d)",
                (unsigned int)stuff->drawable,
                (unsigned int)stuff->gc,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" nchars(%d)",
                    stuff->nChars);
            }

            return reply;
        }

    case X_ChangeKeyboardMapping:
        {
            xChangeKeyboardMappingReq *stuff = (xChangeKeyboardMappingReq *)req;
            REPLY (": first_key_code(%d) key_syms_per_key_code(%d)",
                stuff->firstKeyCode,
                stuff->keySymsPerKeyCode);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" key_codes(%d)",
                    stuff->keyCodes);
            }

            return reply;
        }

    case X_GetKeyboardMapping:
        {
            xGetKeyboardMappingReq *stuff = (xGetKeyboardMappingReq *)req;
            REPLY (": first_key_code(%d)",
                stuff->firstKeyCode);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" count(%d)",
                    stuff->count);
            }

            return reply;
        }

    case X_ChangePointerControl:
        {
            xChangePointerControlReq *stuff = (xChangePointerControlReq *)req;
            REPLY (": accelNum(%d) accelDenum(%d) threshold(%d)",
                stuff->accelNum,
                stuff->accelDenum,
                stuff->threshold);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" do_accel(%s) do_thresh(%s)",
                    stuff->doAccel ? "YES" : "NO",
                    stuff->doThresh ? "YES" : "NO");
            }

            return reply;
        }

    case X_SetPointerMapping:
        {
            xSetPointerMappingReq *stuff = (xSetPointerMappingReq *)req;

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (": Elts(%d)",
                    stuff->nElts);
            }

            return reply;
        }

    case X_SetModifierMapping:
        {
            xSetModifierMappingReq *stuff =(xSetModifierMappingReq *)req;
            REPLY (": num_key_per_modifier(%d)",
                stuff->numKeyPerModifier);

            return reply;
        }

    case X_ListProperties:
    case X_DestroyWindow:
    case X_DestroySubwindows:
    case X_MapWindow:
    case X_MapSubwindows:
    case X_UnmapWindow:
    case X_UnmapSubwindows:
    case X_GetGeometry:
    case X_QueryTree:
    case X_UngrabPointer:
    case X_UngrabKeyboard:
    case X_FreePixmap:
    case X_KillClient:
        {
            xResourceReq *stuff = (xResourceReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->id);

            return reply;
        }

    default:
            break;
    }

    return reply;
}

char * xDbgEvlogEventCore (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch (evt->u.u.type & 0x7F)
    {
    case KeyPress:
    case KeyRelease:
    case ButtonPress:
    case ButtonRelease:
    case MotionNotify:
        {
            REPLY (": Root(0x%x %d,%d) Event(0x%x %d,%d) Child(0x%x)",
                (unsigned int)evt->u.keyButtonPointer.root,
                evt->u.keyButtonPointer.rootX,
                evt->u.keyButtonPointer.rootY,
                (unsigned int)evt->u.keyButtonPointer.event,
                evt->u.keyButtonPointer.eventX,
                evt->u.keyButtonPointer.eventY,
                (unsigned int)evt->u.keyButtonPointer.child);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s state(0x%x) same_screen(%s)",
                    " ",
                    evt->u.keyButtonPointer.state,
                    evt->u.keyButtonPointer.sameScreen ? "YES" : "NO");
            }

            return reply;
        }

    case EnterNotify:
    case LeaveNotify:
        {
            REPLY (": Root(0x%x %d,%d) Event(0x%x %d,%d) Child(0x%x)",
                (unsigned int)evt->u.enterLeave.root,
                evt->u.enterLeave.rootX,
                evt->u.enterLeave.rootY,
                (unsigned int)evt->u.enterLeave.event,
                evt->u.enterLeave.eventX,
                evt->u.enterLeave.eventY,
                (unsigned int)evt->u.enterLeave.child);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s time(%lums) state(0x%x) same_screen(%s) focus(%s)",
                    " ",
                    evt->u.enterLeave.time,
                    evt->u.enterLeave.state,
                    evt->u.enterLeave.flags & ELFlagSameScreen ? "YES" : "NO",
                    evt->u.enterLeave.flags & ELFlagFocus ? "YES" : "NO");
            }

            return reply;
        }

    case FocusIn:
    case FocusOut:
    case KeymapNotify:
        {
            REPLY (": XID(0x%x)",
                (unsigned int)evt->u.focus.window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char* mode;
                char dmode[10];

                switch (evt->u.focus.mode)
                {
                    case NotifyNormal:  mode = "NotifyNormal"; break;
                    case NotifyGrab:  mode = "NotifyGrab"; break;
                    case NotifyUngrab:  mode = "NotifyUngrab"; break;
                    case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
                    default:  mode = dmode, sprintf (dmode, "%u", evt->u.focus.mode); break;
                }

                REPLY (" mode(%s)",
                    mode);
            }

            return reply;
        }

    case Expose:
        {
            REPLY (": XID(0x%x) size(%dx%d) coord(%d,%d)",
                (unsigned int)evt->u.expose.window,
                evt->u.expose.width,
                evt->u.expose.height,
                evt->u.expose.x,
                evt->u.expose.y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" count(%d)",
                    evt->u.expose.count);
            }

            return reply;
        }

    case GraphicsExpose:
        {
            REPLY (": XID(0x%x) size(%dx%d) coord(%d,%d)",
                (unsigned int)evt->u.graphicsExposure.drawable,
                evt->u.graphicsExposure.width,
                evt->u.graphicsExposure.height,
                evt->u.graphicsExposure.x,
                evt->u.graphicsExposure.y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *major;
                char dmajor[10];

                switch (evt->u.graphicsExposure.majorEvent)
                {
                    case X_CopyArea:  major = "CopyArea";  break;
                    case X_CopyPlane:  major = "CopyPlane";  break;
                    default: major = dmajor; sprintf(dmajor, "%d", evt->u.graphicsExposure.majorEvent); break;
                }

                REPLY (" major_event(%s) minor_event(%d) count(%d)",
                    major,
                    evt->u.graphicsExposure.minorEvent,
                    evt->u.graphicsExposure.count);
            }

            return reply;
        }

    case NoExpose:
        {
            REPLY (": XID(0x%x)",
                (unsigned int)evt->u.noExposure.drawable);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *major;
                char dmajor[10];

                switch (evt->u.noExposure.majorEvent)
                {
                    case X_CopyArea:  major = "CopyArea";  break;
                    case X_CopyPlane:  major = "CopyPlane";  break;
                    default:  major = dmajor; sprintf (dmajor, "%d", evt->u.noExposure.majorEvent); break;
                }

                REPLY (" major_event(%s) minor_event(%d)",
                    major,
                    evt->u.noExposure.minorEvent);
            }

            return reply;
        }

    case VisibilityNotify:
        {
            REPLY (": XID(0x%x)",
                (unsigned int)evt->u.visibility.window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *state;
                char dstate[10];

                switch (evt->u.visibility.state)
                {
                    case VisibilityUnobscured:  state = "VisibilityUnobscured"; break;
                    case VisibilityPartiallyObscured:  state = "VisibilityPartiallyObscured"; break;
                    case VisibilityFullyObscured:  state = "VisibilityFullyObscured"; break;
                    default:  state = dstate; sprintf (dstate, "%d", evt->u.visibility.state); break;
                }

                REPLY (" state(%s)",
                    state);
            }

            return reply;
        }

    case CreateNotify:
        {
            REPLY (": Window(0x%x) Parent(0x%x) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                (unsigned int)evt->u.createNotify.window,
                (unsigned int)evt->u.createNotify.parent,
                evt->u.createNotify.width,
                evt->u.createNotify.height,
                evt->u.createNotify.x,
                evt->u.createNotify.y,
                evt->u.createNotify.borderWidth);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" override(%s)",
                    evt->u.createNotify.override ? "YES" : "NO");
            }

            return reply;
        }

    case DestroyNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x)",
                (unsigned int)evt->u.destroyNotify.window,
                (unsigned int)evt->u.destroyNotify.event);

            return reply;
		}

    case UnmapNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x)",
                (unsigned int)evt->u.unmapNotify.window,
                (unsigned int)evt->u.unmapNotify.event);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" from_Configure(%s)",
                    evt->u.unmapNotify.fromConfigure ? "YES" : "NO");
            }

            return reply;
        }

    case MapNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x)",
                (unsigned int)evt->u.mapNotify.window,
                (unsigned int)evt->u.mapNotify.event);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" override(%s)",
                    evt->u.mapNotify.override ? "YES" : "NO");
            }

            return reply;
        }

    case MapRequest:
        {
            REPLY (": Window(0x%x) Parent(0x%x)",
                (unsigned int)evt->u.mapRequest.window,
                (unsigned int)evt->u.mapRequest.parent);

            return reply;
        }

    case ReparentNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x) Parent(0x%x) coord(%d,%d)",
                (unsigned int)evt->u.reparent.window,
                (unsigned int)evt->u.reparent.event,
                (unsigned int)evt->u.reparent.parent,
                evt->u.reparent.x,
                evt->u.reparent.y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" override(%s)",
                    evt->u.reparent.override ? "YES" : "NO");
            }

            return reply;
        }

    case ConfigureNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x) AboveSibling(0x%x) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                (unsigned int)evt->u.configureNotify.window,
                (unsigned int)evt->u.configureNotify.event,
                (unsigned int)evt->u.configureNotify.aboveSibling,
                evt->u.configureNotify.width,
                evt->u.configureNotify.height,
                evt->u.configureNotify.x,
                evt->u.configureNotify.y,
                evt->u.configureNotify.borderWidth);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" override(%s)",
                    evt->u.configureNotify.override ? "YES" : "NO");
            }

            return reply;
        }

    case ConfigureRequest:
        {
            REPLY (": Window(0x%x) Parent(0x%x) Sibling(0x%x) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                (unsigned int)evt->u.configureRequest.window,
                (unsigned int)evt->u.configureRequest.parent,
                (unsigned int)evt->u.configureRequest.sibling,
                evt->u.configureRequest.width,
                evt->u.configureRequest.height,
                evt->u.configureRequest.x,
                evt->u.configureRequest.y,
                evt->u.configureRequest.borderWidth);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY ("\n");
                REPLY ("%67s value_mask",
                    " ");
                REPLY ("(");
                reply = _getConfigureWindowMask(evt->u.configureRequest.valueMask, reply, len);
                REPLY (")");
            }

            return reply;
        }

    case GravityNotify:
        {
            REPLY (": Window(0x%x) Event(0x%x) coord(%d,%d)",
                (unsigned int)evt->u.gravity.window,
                (unsigned int)evt->u.gravity.event,
                evt->u.gravity.x,
                evt->u.gravity.y);

            return reply;
        }

    case ResizeRequest:
        {
            REPLY (": Window(0x%x) size(%dx%d)",
                (unsigned int)evt->u.resizeRequest.window,
                evt->u.resizeRequest.width,
                evt->u.resizeRequest.height);

            return reply;
        }

    case CirculateNotify:
    case CirculateRequest:
        {
            REPLY (": Window(0x%x) Event(0x%x) parent(0x%x)",
                (unsigned int)evt->u.circulate.window,
                (unsigned int)evt->u.circulate.event,
                (unsigned int)evt->u.circulate.parent);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *place;
                char dplace[10];

                switch (evt->u.circulate.place)
                {
                    case PlaceOnTop:  place = "PlaceOnTop"; break;
                    case PlaceOnBottom:  place = "PlaceOnBottom"; break;
                    default:  place = dplace; sprintf (dplace, "%d", evt->u.circulate.place); break;
                }

                REPLY (" place(%s)",
                    place);
            }

            return reply;
        }

    case PropertyNotify:
        {
            REPLY (": Window(0x%x)",
                (unsigned int)evt->u.property.window);

            REPLY (" Property");
            reply = xDbgGetAtom(evt->u.property.atom, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *state;
                char dstate[10];

                switch (evt->u.property.state)
                {
                    case PropertyNewValue:  state = "PropertyNewValue"; break;
                    case PropertyDelete:  state = "PropertyDelete"; break;
                    default:  state = dstate; sprintf (dstate, "%d", evt->u.property.state); break;
                }

                REPLY ("\n");
                REPLY ("%67s time(%lums) state(%s)",
                    " ",
                    evt->u.property.time,
                    state);
            }

            return reply;
        }

    case SelectionClear:
        {
            REPLY (": Window(0x%x)",
                (unsigned int)evt->u.selectionClear.window);

            REPLY (" Atom");
            reply = xDbgGetAtom(evt->u.selectionClear.atom, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    evt->u.selectionClear.time);
            }

            return reply;
		}

    case SelectionRequest:
        {
            REPLY (": Owner(0x%x) Requestor(0x%x)",
                (unsigned int)evt->u.selectionRequest.owner,
                (unsigned int)evt->u.selectionRequest.requestor);

            REPLY (" selection");
            reply = xDbgGetAtom(evt->u.selectionRequest.selection, evinfo, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(evt->u.selectionRequest.target, evinfo, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(evt->u.selectionRequest.property, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    evt->u.selectionRequest.time);
            }

            return reply;
        }

    case SelectionNotify:
        {
            REPLY (": Requestor(0x%x)",
                (unsigned int)evt->u.selectionNotify.requestor);

            REPLY (" selection");
            reply = xDbgGetAtom(evt->u.selectionNotify.selection, evinfo, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(evt->u.selectionNotify.target, evinfo, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(evt->u.selectionNotify.property, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" time(%lums)",
                    evt->u.selectionNotify.time);
            }

            return reply;
        }

    case ColormapNotify:
        {
            REPLY (": XID(0x%x) Colormap(0x%x)",
                (unsigned int)evt->u.colormap.window,
                (unsigned int)evt->u.colormap.colormap);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *state;
                char dstate[10];

                switch (evt->u.colormap.state)
                {
                    case ColormapInstalled:  state = "ColormapInstalled"; break;
                    case ColormapUninstalled:  state = "ColormapUninstalled"; break;
                    default:  state = dstate; sprintf (dstate, "%d", evt->u.colormap.state); break;
                }

                REPLY (" new(%s) state(%s)",
	            evt->u.colormap.new ? "YES" : "NO",
                    state);
            }

            return reply;
        }

    case ClientMessage:
        {
            REPLY (": XID(0x%x)",
                (unsigned int)evt->u.clientMessage.window);

            REPLY (" Type");
            reply = xDbgGetAtom(evt->u.clientMessage.u.b.type, evinfo, reply, len);

            return reply;
		}

    case MappingNotify:
    case GenericEvent:
    default:
            break;
    }

    return reply;
}

char * xDbgEvlogReplyCore (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqType)
    {
    case X_GetGeometry:
        {
            if (evinfo->rep.isStart)
            {
                xGetGeometryReply *stuff = (xGetGeometryReply *)rep;

                REPLY (": XID(0x%x) coord(%d,%d %dx%d) borderWidth(%d)",
                    (unsigned int)stuff->root,
                    stuff->x,
                    stuff->y,
                    stuff->width,
                    stuff->height,
                    stuff->borderWidth);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_QueryTree:
        {
            if (evinfo->rep.isStart)
            {
                xQueryTreeReply *stuff = (xQueryTreeReply *)rep;

                REPLY (": XID(0x%x) Parent(0x%x) ChildrenNum(%d)",
                    (unsigned int)stuff->root,
                    (unsigned int)stuff->parent,
                    stuff->nChildren);
            }
            else
            {
                Window *stuff = (Window *)rep;
                int i;

                REPLY ("childIDs");
                REPLY ("(");
                for (i = 0 ; i < evinfo->rep.size / sizeof(Window) ; i++)
                {
                    REPLY("0x%x", (unsigned int)stuff[i]);
                    if(i != evinfo->rep.size / sizeof(Window) - 1)
                        REPLY (", ");
                }
                REPLY (")");
            }

            return reply;
        }

    case X_GetProperty:
        {
            if (evinfo->rep.isStart)
            {
                xGetPropertyReply *stuff = (xGetPropertyReply *)rep;

                REPLY (": PropertyType");
                reply = xDbgGetAtom(stuff->propertyType, evinfo, reply, len);

                REPLY (" bytesAfter(0x%x) format(%d) ItemNum(%ld)",
                    (unsigned int)stuff->bytesAfter,
                    stuff->format,
                    stuff->nItems);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_ListProperties:
        {
            if (evinfo->rep.isStart)
            {
                xListPropertiesReply *stuff = (xListPropertiesReply *)rep;

                REPLY (" PropertieNum(%d)",
                    stuff->nProperties);
            }
            else
            {
                Atom *stuff = (Atom *)rep;
                int i;

                REPLY ("Properties");
                REPLY ("(");
                for (i = 0 ; i < evinfo->rep.size / sizeof(Atom) ; i ++)
                {
                    reply = xDbgGetAtom(stuff[i], evinfo, reply, len);
                    if(i != evinfo->rep.size / sizeof(Atom) - 1)
                        REPLY (", ");
                }
                REPLY (")");
            }

            return reply;
        }

    case X_GetImage:
        {
            if (evinfo->rep.isStart)
            {
                xGetImageReply *stuff = (xGetImageReply *)rep;

                REPLY (": XID(0x%x)",
                    (unsigned int)stuff->visual);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_GetKeyboardControl:
        {
            if (evinfo->rep.isStart)
            {
                xGetKeyboardControlReply *stuff = (xGetKeyboardControlReply *)rep;

                REPLY (": keyClickPercent(%d) bellPercent(%d), bellPitch(%d) bellDuration(%d)",
                    stuff->keyClickPercent,
                    stuff->bellPercent,
                    stuff->bellPitch,
                    stuff->bellDuration);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    case X_GetPointerControl:
        {
            if (evinfo->rep.isStart)
            {
                xGetPointerControlReply *stuff = (xGetPointerControlReply *)rep;

                REPLY (": accelNumerator(%d) accelDenominator(%d), threshold(%d)",
                    stuff->accelNumerator,
                    stuff->accelDenominator,
                    stuff->threshold);
            }
            else
            {
                return reply;
            }

            return reply;
        }

    default:
        break;
    }

    return reply;
}
