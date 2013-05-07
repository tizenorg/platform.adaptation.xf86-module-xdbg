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
#include <X11/extensions/XI2proto.h>

#include "xdbg_types.h"
#include "xdbg_evlog_core.h"
#include "xdbg_evlog.h"

char * xDbgEvlogRequestCore (void *dpy, EvlogInfo *evinfo, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->reqType)
    {
    case X_CreateWindow:
        {
            xCreateWindowReq *stuff = (xCreateWindowReq *)req;

            REPLY (": Window(0x%lx) Parent(0x%lx) size(%dx%d) boaderWid(%d) coordinate(%d,%d)",
                stuff->wid,
                stuff->parent,
                stuff->width,
                stuff->height,
                stuff->borderWidth,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ChangeWindowAttributes:
        {
            xChangeWindowAttributesReq *stuff = (xChangeWindowAttributesReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_ChangeSaveSet:
        {
            xChangeSaveSetReq *stuff = (xChangeSaveSetReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_ReparentWindow:
        {
            xReparentWindowReq *stuff = (xReparentWindowReq *)req;
            REPLY (": Window(0x%lx) Parent(0x%lx) coord(%d,%d)",
                stuff->window,
                stuff->parent,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ConfigureWindow:
        {
            xConfigureWindowReq *stuff = (xConfigureWindowReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_CirculateWindow:
        {
            xCirculateWindowReq *stuff = (xCirculateWindowReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            return reply;
        }

    case X_ChangeProperty:
        {
            xChangePropertyReq *stuff = (xChangePropertyReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            REPLY (" Property");
            reply = xDbgGetAtom(dpy, stuff->property, reply, len);
            REPLY (" Type");
            reply = xDbgGetAtom(dpy, stuff->type, reply, len);

            return reply;
        }

    case X_DeleteProperty:
        {
            xDeletePropertyReq *stuff = (xDeletePropertyReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            REPLY (" Property");
            reply = xDbgGetAtom(dpy, stuff->property, reply, len);

            return reply;
        }

    case X_SetSelectionOwner:
        {
            xSetSelectionOwnerReq *stuff = (xSetSelectionOwnerReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->window);

            REPLY (" Selection");
            reply = xDbgGetAtom(dpy, stuff->selection, reply, len);

            return reply;
        }

    case X_ConvertSelection:
        {
            xConvertSelectionReq *stuff = (xConvertSelectionReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->requestor);

            REPLY (" Selection");
            reply = xDbgGetAtom(dpy, stuff->selection, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(dpy, stuff->target, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(dpy, stuff->property, reply, len);

            return reply;
        }

    case X_SendEvent:
        {
            xSendEventReq *stuff = (xSendEventReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->destination);

            return reply;
        }

    case X_GrabPointer:
        {
            xGrabPointerReq *stuff = (xGrabPointerReq *)req;
            REPLY (": XID(0x%lx) ConfineTo(0x%lx) Cursor(0x%lx)",
                stuff->grabWindow,
                stuff->confineTo,
                stuff->cursor);

            return reply;
        }

    case X_GrabButton:
        {
            xGrabButtonReq *stuff = (xGrabButtonReq *)req;
            REPLY (": XID(0x%lx) ConfineTo(0x%lx) Cursor(0x%lx)",
                stuff->grabWindow,
                stuff->confineTo,
                stuff->cursor);

            return reply;
        }

    case X_UngrabButton:
        {
            xUngrabButtonReq *stuff = (xUngrabButtonReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->grabWindow);

            return reply;
        }

    case X_ChangeActivePointerGrab:
        {
            xChangeActivePointerGrabReq *stuff = (xChangeActivePointerGrabReq *)req;
            REPLY (": Cursor(0x%lx)",
                stuff->cursor);

            return reply;
        }

    case X_GrabKeyboard:
        {
            xGrabKeyboardReq *stuff = (xGrabKeyboardReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->grabWindow);

            return reply;
        }

    case X_GrabKey:
        {
            xGrabKeyReq *stuff = (xGrabKeyReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->grabWindow);

            return reply;
        }

    case X_UngrabKey:
        {
            xUngrabKeyReq *stuff = (xUngrabKeyReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->grabWindow);

            return reply;
        }

    case X_SetInputFocus:
        {
            xSetInputFocusReq *stuff = (xSetInputFocusReq *)req;
            REPLY (": XID(0x%lx)",
                stuff->focus);

            return reply;
        }

    case X_CreatePixmap:
        {
            xCreatePixmapReq *stuff = (xCreatePixmapReq *)req;
            REPLY (": Pixmap(0x%lx) Drawable(0x%lx) size(%dx%d)",
                stuff->pid,
                stuff->drawable,
                stuff->width,
                stuff->height);

            return reply;
        }

    case X_ClearArea:
        {
            xClearAreaReq *stuff = (xClearAreaReq *)req;
            REPLY (": XID(0x%lx) area(%d,%d %dx%d)",
                stuff->window,
                stuff->x,
                stuff->y,
                stuff->width,
                stuff->height);

           return reply;
        }

    case X_CopyArea:
        {
            xCopyAreaReq *stuff = (xCopyAreaReq *)req;
            REPLY (": srcXID(0x%lx) dstXID(0x%lx) gc(0x%lx) size(%dx%d) src(%d,%d) dst(%d,%d)",
                stuff->srcDrawable,
                stuff->dstDrawable,
                stuff->gc,
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
            REPLY (": srcXID(0x%lx) dstXID(0x%lx) gc(0x%lx) size(%dx%d) src(%d,%d) dst(%d,%d)",
                stuff->srcDrawable,
                stuff->dstDrawable,
                stuff->gc,
                stuff->width,
                stuff->height,
                stuff->srcX,
                stuff->srcY,
                stuff->dstX,
                stuff->dstY);

            return reply;
        }

    case X_PolyPoint:
        {
            xPolyPointReq *stuff = (xPolyPointReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolyLine:
        {
            xPolyLineReq *stuff = (xPolyLineReq *)req;
            REPLY (": XID(0x%lx gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolySegment:
        {
            xPolySegmentReq *stuff = (xPolySegmentReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolyRectangle:
        {
            xPolyRectangleReq *stuff = (xPolyRectangleReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolyArc:
        {
            xPolyArcReq *stuff = (xPolyArcReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_FillPoly:
        {
            xFillPolyReq *stuff = (xFillPolyReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolyFillRectangle:
        {
            xPolyFillRectangleReq *stuff = (xPolyFillRectangleReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PolyFillArc:
        {
            xPolyFillArcReq *stuff = (xPolyFillArcReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx)",
                stuff->drawable,
                stuff->gc);

            return reply;
        }

    case X_PutImage:
        {
            xPutImageReq *stuff = (xPutImageReq *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) size(%dx%d) dst(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->width,
                stuff->height,
                stuff->dstX,
                stuff->dstY);
            return reply;
        }

    case X_GetImage:
        {
            xGetImageReq *stuff = (xGetImageReq *)req;
            REPLY (": XID(0x%lx) size(%dx%d) dst(%d,%d)",
                stuff->drawable,
                stuff->width,
                stuff->height,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_PolyText8:
        {
            xPolyText8Req *stuff = (xPolyText8Req *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) coord(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_PolyText16:
        {
            xPolyText16Req *stuff = (xPolyText16Req *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) coord(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ImageText8:
        {
            xImageText8Req *stuff = (xImageText8Req *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) coord(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ImageText16:
        {
            xImageText16Req *stuff = (xImageText16Req *)req;
            REPLY (": XID(0x%lx) gc(0x%lx) coord(%d,%d)",
                stuff->drawable,
                stuff->gc,
                stuff->x,
                stuff->y);

            return reply;
        }

    case X_ChangeKeyboardMapping:
        {
            xChangeKeyboardMappingReq *stuff = (xChangeKeyboardMappingReq *)req;
            REPLY (": Key(%d) FstKey(%d) KeySyms(%d)",
                stuff->firstKeyCode,
                stuff->keyCodes,
                stuff->keySymsPerKeyCode);

            return reply;
        }

    case X_GetKeyboardMapping:
        {
            xGetKeyboardMappingReq *stuff = (xGetKeyboardMappingReq *)req;
            REPLY (": FstKey(%d) Count(%d)",
                stuff->firstKeyCode,
                stuff->count);

            return reply;
        }

    case X_ChangePointerControl:
        {
            xChangePointerControlReq *stuff = (xChangePointerControlReq *)req;
            REPLY (": accelNum(%d) accelDenum(%d) threshold(%d)",
                stuff->accelNum,
                stuff->accelDenum,
                stuff->threshold);

            return reply;
        }

    case X_SetPointerMapping:
        {
            xSetPointerMappingReq *stuff = (xSetPointerMappingReq *)req;
            REPLY (": Elts(%d)",
                stuff->nElts);

            return reply;
        }

    case X_SetModifierMapping:
        {
            xSetModifierMappingReq *stuff =(xSetModifierMappingReq *)req;
            REPLY (": NumkeyPerModifier(%d)",
                stuff->numKeyPerModifier);

            return reply;
        }

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
            REPLY (": XID(0x%lx)",
                stuff->id);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


char * xDbgEvlogEventCore (void *dpy, EvlogInfo *evinfo, char *reply, int *len)
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
            REPLY (": Root(0x%lx %d,%d) Event(0x%lx %d,%d) Child(0x%lx)",
                evt->u.keyButtonPointer.root,
                evt->u.keyButtonPointer.rootX,
                evt->u.keyButtonPointer.rootY,
                evt->u.keyButtonPointer.event,
                evt->u.keyButtonPointer.eventX,
                evt->u.keyButtonPointer.eventY,
                evt->u.keyButtonPointer.child);

            return reply;
		}

    case EnterNotify:
    case LeaveNotify:
        {
            REPLY (": Root(0x%lx %d,%d) Event(0x%lx %d,%d) Child(0x%lx)",
                evt->u.enterLeave.root,
                evt->u.enterLeave.rootX,
                evt->u.enterLeave.rootY,
                evt->u.enterLeave.event,
                evt->u.enterLeave.eventX,
                evt->u.enterLeave.eventY,
                evt->u.enterLeave.child);

            return reply;
		}

    case FocusIn:
    case FocusOut:
    case KeymapNotify:
        {
            REPLY (": XID(0x%lx)",
                evt->u.focus.window);

            return reply;
		}

    case Expose:
        {
            REPLY (": XID(0x%lx) size(%dx%d) coord(%d,%d)",
                evt->u.expose.window,
                evt->u.expose.width,
                evt->u.expose.height,
                evt->u.expose.x,
                evt->u.expose.y);

            return reply;
		}

    case GraphicsExpose:
        {
            REPLY (": XID(0x%lx) size(%dx%d) coord(%d,%d)",
                evt->u.graphicsExposure.drawable,
                evt->u.graphicsExposure.width,
                evt->u.graphicsExposure.height,
                evt->u.graphicsExposure.x,
                evt->u.graphicsExposure.y);

            return reply;
		}

    case NoExpose:
        {
            REPLY (": XID(0x%lx)",
                evt->u.noExposure.drawable);

            return reply;
		}


    case VisibilityNotify:
        {
            REPLY (": XID(0x%lx)",
                evt->u.visibility.window);

            return reply;
		}

    case CreateNotify:
        {
            REPLY (": Window(0x%lx) Parent(0x%lx) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                evt->u.createNotify.window,
                evt->u.createNotify.parent,
                evt->u.createNotify.width,
                evt->u.createNotify.height,
                evt->u.createNotify.x,
                evt->u.createNotify.y,
                evt->u.createNotify.borderWidth);

            return reply;
		}

    case DestroyNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx)",
                evt->u.destroyNotify.window,
                evt->u.destroyNotify.event);

            return reply;
		}

    case UnmapNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx)",
                evt->u.unmapNotify.window,
                evt->u.unmapNotify.event);

            return reply;
		}

    case MapNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx)",
                evt->u.mapNotify.window,
                evt->u.mapNotify.event);

            return reply;
		}

    case MapRequest:
        {
            REPLY (": Window(0x%lx) Parent(0x%lx)",
                evt->u.mapRequest.window,
                evt->u.mapRequest.parent);

            return reply;
		}

    case ReparentNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx) parent(0x%lx) coord(%d,%d)",
                evt->u.reparent.window,
                evt->u.reparent.event,
                evt->u.reparent.parent,
                evt->u.reparent.x,
                evt->u.reparent.y);

            return reply;
		}

    case ConfigureNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx) aboveSibling(0x%lx) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                evt->u.configureNotify.window,
                evt->u.configureNotify.event,
                evt->u.configureNotify.aboveSibling,
                evt->u.configureNotify.width,
                evt->u.configureNotify.height,
                evt->u.configureNotify.x,
                evt->u.configureNotify.y,
                evt->u.configureNotify.borderWidth);

            return reply;
		}

    case ConfigureRequest:
        {
            REPLY (": Window(0x%lx) Parent(0x%lx) Sibling(0x%lx) size(%dx%d) coord(%d,%d) borderWidth(%d)",
                evt->u.configureRequest.window,
                evt->u.configureRequest.parent,
                evt->u.configureRequest.sibling,
                evt->u.configureRequest.width,
                evt->u.configureRequest.height,
                evt->u.configureRequest.x,
                evt->u.configureRequest.y,
                evt->u.configureRequest.borderWidth);

            return reply;
		}

    case GravityNotify:
        {
            REPLY (": Window(0x%lx) Event(0x%lx) coord(%d,%d)",
                evt->u.gravity.window,
                evt->u.gravity.event,
                evt->u.gravity.x,
                evt->u.gravity.y);

            return reply;
		}

    case ResizeRequest:
        {
            REPLY (": Window(0x%lx) size(%dx%d)",
                evt->u.resizeRequest.window,
                evt->u.resizeRequest.width,
                evt->u.resizeRequest.height);

            return reply;
		}

    case CirculateNotify:
    case CirculateRequest:
        {
            REPLY (": Window(0x%lx) Event(0x%lx) parent(0x%lx)",
                evt->u.circulate.window,
                evt->u.circulate.event,
                evt->u.circulate.parent);

            return reply;
		}

    case PropertyNotify:
        {
            REPLY (": Window(0x%lx)",
                evt->u.property.window);

            REPLY (" Atom");
            reply = xDbgGetAtom(dpy, evt->u.property.atom, reply, len);
            return reply;
		}

    case SelectionClear:
        {
            REPLY (": Window(0x%lx)",
                evt->u.selectionClear.window);

            REPLY (" Atom");
            reply = xDbgGetAtom(dpy, evt->u.selectionClear.atom, reply, len);

            return reply;
		}

    case SelectionRequest:
        {
            REPLY (": Owner(0x%lx) Requestor(0x%lx)",
                evt->u.selectionRequest.owner,
                evt->u.selectionRequest.requestor);

            REPLY (" selection");
            reply = xDbgGetAtom(dpy, evt->u.selectionRequest.selection, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(dpy, evt->u.selectionRequest.target, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(dpy, evt->u.selectionRequest.property, reply, len);

            return reply;
		}

    case SelectionNotify:
        {
            REPLY (": Requestor(0x%lx)",
                evt->u.selectionNotify.requestor);

            REPLY (" selection");
            reply = xDbgGetAtom(dpy, evt->u.selectionNotify.selection, reply, len);
            REPLY (" Target");
            reply = xDbgGetAtom(dpy, evt->u.selectionNotify.target, reply, len);
            REPLY (" Property");
            reply = xDbgGetAtom(dpy, evt->u.selectionNotify.property, reply, len);

            return reply;
		}

    case ColormapNotify:
        {
            REPLY (": XID(0x%lx) Colormap(0x%lx)",
                evt->u.colormap.window,
                evt->u.colormap.colormap);

            return reply;
		}

    case ClientMessage:
        {
            REPLY (": XID(0x%lx)",
                evt->u.clientMessage.window);

            REPLY (" Type");
            reply = xDbgGetAtom(dpy, evt->u.clientMessage.u.b.type, reply, len);

            return reply;
		}

    case MappingNotify:
    case GenericEvent:
    default:
            break;
    }

    return reply;
}
