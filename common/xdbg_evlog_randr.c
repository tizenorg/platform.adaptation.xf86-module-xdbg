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

#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randrproto.h>


#include "xdbg_types.h"
#include "xdbg_evlog_randr.h"
#include "xdbg_evlog.h"

static char *
_EvlogRequestRandr (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xReq *req = evinfo->req.ptr;

    switch (req->data)
    {
    case X_RRGetScreenSizeRange:
        {
            xRRGetScreenSizeRangeReq *stuff = (xRRGetScreenSizeRangeReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    case X_RRSetScreenSize:
        {
            xRRSetScreenSizeReq *stuff = (xRRSetScreenSizeReq *)req;
            REPLY (": XID(0x%x) size(%dx%d)",
                (unsigned int)stuff->window,
                stuff->width,
                stuff->height);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" milliSize(%ldx%ld)",
                    (long int)stuff->widthInMillimeters,
                    (long int)stuff->heightInMillimeters);
            }

            return reply;
        }

    case X_RRGetScreenResources:
        {
            xRRGetScreenResourcesReq *stuff = (xRRGetScreenResourcesReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    case X_RRGetOutputInfo:
        {
            xRRGetOutputInfoReq *stuff = (xRRGetOutputInfoReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" config_timestamp(%lums)",
                    (unsigned long)stuff->configTimestamp);
            }

            return reply;
        }

    case X_RRListOutputProperties:
        {
            xRRListOutputPropertiesReq *stuff = (xRRListOutputPropertiesReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            return reply;
        }

    case X_RRQueryOutputProperty:
        {
            xRRQueryOutputPropertyReq *stuff = (xRRQueryOutputPropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            return reply;
        }

    case X_RRConfigureOutputProperty:
        {
            xRRConfigureOutputPropertyReq *stuff = (xRRConfigureOutputPropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" pending(%s) range(%s)",
                    stuff->pending ? "YES" : "NO",
                    stuff->range ? "YES" : "NO");
            }

            return reply;
        }

    case X_RRChangeOutputProperty:
        {
            xRRChangeOutputPropertyReq *stuff = (xRRChangeOutputPropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

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
                    default:  mode = dmode; snprintf (dmode, 10, "%d", stuff->mode); break;
                }

                REPLY ("\n");
                REPLY ("%67s mode(%s) format(%d) nUnits(%ld)",
                    " ",
                    mode,
                    stuff->format,
                    (long int)stuff->nUnits);
            }

            return reply;
        }

    case X_RRDeleteOutputProperty:
        {
            xRRDeleteOutputPropertyReq *stuff = (xRRDeleteOutputPropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);

            return reply;
        }

    case X_RRGetOutputProperty:
        {
            xRRGetOutputPropertyReq *stuff = (xRRGetOutputPropertyReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->output);

            REPLY (" Property");
            reply = xDbgGetAtom(stuff->property, evinfo, reply, len);
            REPLY (" Type");
            reply = xDbgGetAtom(stuff->type, evinfo, reply, len);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" longOffset(%ld) longLength(%ld)",
                    (long int)stuff->longOffset,
                    (long int)stuff->longLength);
            }

            return reply;
        }

    case X_RRGetCrtcInfo:
        {
            xRRGetCrtcInfoReq *stuff = (xRRGetCrtcInfoReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->crtc);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                REPLY (" config_timestamp(%lums)",
                    (unsigned long)stuff->configTimestamp);
            }

            return reply;
        }

    case X_RRSetCrtcConfig:
        {
            xRRSetCrtcConfigReq *stuff = (xRRSetCrtcConfigReq *)req;
            REPLY (": XID(0x%x) coord(%d,%d) ",
                (unsigned int)stuff->crtc,
                stuff->x,
                stuff->y);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *rotation;

                switch (stuff->rotation & 0xf)
                {
                    case RR_Rotate_0:  rotation = "RR_Rotate_0"; break;
                    case RR_Rotate_90:  rotation = "RR_Rotate_90"; break;
                    case RR_Rotate_180:  rotation = "RR_Rotate_180"; break;
                    case RR_Rotate_270:  rotation = "RR_Rotate_270"; break;
                    default:  rotation = "Invaild Rotation"; break;
                }

                REPLY ("\n");
                REPLY ("%67s timestamp(%lums) config_timestamp(%lums) RRmode(0x%x) rotation(%s)",
                    " ",
                    (unsigned long)stuff->timestamp,
                    (unsigned long)stuff->configTimestamp,
                    (unsigned int)stuff->mode,
                    rotation);
            }

            return reply;
        }

    case X_RRGetScreenResourcesCurrent:
        {
            xRRGetScreenResourcesCurrentReq *stuff = (xRRGetScreenResourcesCurrentReq *)req;
            REPLY (": XID(0x%x)",
                (unsigned int)stuff->window);

            return reply;
        }

    default:
            break;
    }

    return reply;
}


static char *
_EvlogEventRandr (EvlogInfo *evinfo, int first_base, int detail_level, char *reply, int *len)
{
    xEvent *evt = evinfo->evt.ptr;

    switch ((evt->u.u.type & 0x7F) - first_base)
    {
    case RRScreenChangeNotify:
        {
            xRRScreenChangeNotifyEvent *stuff = (xRRScreenChangeNotifyEvent *) evt;
            REPLY (": Root(0x%x) Window(0x%x)",
                (unsigned int)stuff->root,
                (unsigned int)stuff->window);

            if (detail_level >= EVLOG_PRINT_DETAIL)
            {
                const char *rotation;

                switch (stuff->rotation & 0xf)
                {
                    case RR_Rotate_0:  rotation = "RR_Rotate_0"; break;
                    case RR_Rotate_90:  rotation = "RR_Rotate_90"; break;
                    case RR_Rotate_180:  rotation = "RR_Rotate_180"; break;
                    case RR_Rotate_270:  rotation = "RR_Rotate_270"; break;
                    default:  rotation = "Invaild Rotation"; break;
                }

                REPLY (" sizeID(%d) subPixel(%d) Pixel(%d,%d) Milli(%d,%d)",
                    stuff->sizeID,
                    stuff->subpixelOrder,
                    stuff->widthInPixels,
                    stuff->heightInPixels,
                    stuff->widthInMillimeters,
                    stuff->heightInMillimeters);

                REPLY ("\n");
                REPLY ("%67s rotation(%s) sequence_num(%d) timestamp(%lums) config_timestamp(%lums)",
                    " ",
                    rotation,
                    stuff->sequenceNumber,
                    (unsigned long)stuff->timestamp,
                    (unsigned long)stuff->configTimestamp);
            }

            return reply;
        }

    case RRNotify:
        {
            switch(evt->u.u.detail)
            {
            case RRNotify_CrtcChange:
                {
                    xRRCrtcChangeNotifyEvent *stuff = (xRRCrtcChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%x) Crtc(0x%x) RRmode(0x%x) new_size(%udx%ud) new_coord(%d,%d)",
                        (unsigned int)stuff->window,
                        (unsigned int)stuff->crtc,
                        (unsigned int)stuff->mode,
                        stuff->width,
                        stuff->height,
                        stuff->x,
                        stuff->y);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        const char *rotation;

                        switch (stuff->rotation & 0xf)
                        {
                            case RR_Rotate_0:  rotation = "RR_Rotate_0"; break;
                            case RR_Rotate_90:  rotation = "RR_Rotate_90"; break;
                            case RR_Rotate_180:  rotation = "RR_Rotate_180"; break;
                            case RR_Rotate_270:  rotation = "RR_Rotate_270"; break;
                            default:  rotation = "Invaild Rotation"; break;
                        }

                        REPLY ("\n");
                        REPLY ("%67s rotation(%s) sequence_num(%d) timestamp(%lums)",
                            " ",
                            rotation,
                            stuff->sequenceNumber,
                            (unsigned long)stuff->timestamp);
                    }

                    return reply;
                }

            case RRNotify_OutputChange:
                {
                    xRROutputChangeNotifyEvent *stuff = (xRROutputChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%x) Output(0x%x) Crtc(0x%x) RRmode(0x%x)",
                        (unsigned int)stuff->window,
                        (unsigned int)stuff->output,
                        (unsigned int)stuff->crtc,
                        (unsigned int)stuff->mode);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        const char *rotation, *connection, *subpixelOrder;
                        char dconnection[10], dsubpixelOrder[10];

                        switch (stuff->rotation & 0xf)
                        {
                            case RR_Rotate_0:    rotation = "RR_Rotate_0"; break;
                            case RR_Rotate_90:   rotation = "RR_Rotate_90"; break;
                            case RR_Rotate_180:  rotation = "RR_Rotate_180"; break;
                            case RR_Rotate_270:  rotation = "RR_Rotate_270"; break;
                            default:  rotation = "Invaild Rotation"; break;
                        }

                        switch (stuff->connection)
                        {
                            case RR_Connected:          connection = "RR_Connected"; break;
                            case RR_Disconnected:       connection = "RR_Disconnected"; break;
                            case RR_UnknownConnection:  connection = "RR_UnknownConnection"; break;
                            default:  connection = dconnection; snprintf (dconnection, 10, "%d", stuff->connection); break;
                        }

                        switch (stuff->subpixelOrder)
                        {
                            case SubPixelUnknown:        subpixelOrder = "SubPixelUnknown"; break;
                            case SubPixelHorizontalRGB:  subpixelOrder = "SubPixelHorizontalRGB"; break;
                            case SubPixelHorizontalBGR:  subpixelOrder = "SubPixelHorizontalBGR"; break;
                            case SubPixelVerticalRGB:    subpixelOrder = "SubPixelVerticalRGB"; break;
                            case SubPixelVerticalBGR:    subpixelOrder = "SubPixelVerticalBGR"; break;
                            case SubPixelNone:           subpixelOrder = "SubPixelNone"; break;
                            default:  subpixelOrder = dsubpixelOrder; snprintf (dsubpixelOrder, 10, "%d", stuff->connection); break;
                        }

                        REPLY (" sequence_num(%d)",
                            stuff->sequenceNumber);

                        REPLY ("\n");
                        REPLY ("%67s timestamp(%lums) config_timestamp(%lums) rotation(%s) connection(%s) subpixel_order(%s)",
                            " ",
                            (unsigned long)stuff->timestamp,
                            (unsigned long)stuff->configTimestamp,
                            rotation,
                            connection,
                            subpixelOrder);
                    }

                    return reply;
                }

            case RRNotify_OutputProperty:
                {
                    xRROutputPropertyNotifyEvent *stuff = (xRROutputPropertyNotifyEvent *) evt;
                    REPLY (": XID(0x%x) Output(0x%x)",
                        (unsigned int)stuff->window,
                        (unsigned int)stuff->output);

                    REPLY (" Property");
                    reply = xDbgGetAtom(stuff->atom, evinfo, reply, len);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        const char *state;
                        char dstate[10];

                        switch (stuff->state)
                        {
                            case PropertyNewValue:    state = "PropertyNewValue"; break;
                            case PropertyDelete:   state = "PropertyDelete"; break;
                            default:  state = dstate; snprintf (dstate, 10, "%d", stuff->state); break;
                        }

                        REPLY ("\n");
                        REPLY ("%67s sequence_num(%d) timestamp(%lums) state(%s)",
                            " ",
                            stuff->sequenceNumber,
                            (unsigned long)stuff->timestamp,
                            state);
                    }

                    return reply;
                }

            case RRNotify_ProviderChange:
                {
                    xRRProviderChangeNotifyEvent *stuff = (xRRProviderChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%x) Provider(0x%x)",
                        (unsigned int)stuff->window,
                        (unsigned int)stuff->provider);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        REPLY (" sequence_num(%d) timestamp(%lums)",
                            stuff->sequenceNumber,
                            (unsigned long)stuff->timestamp);
                    }

                    return reply;
                }

            case RRNotify_ProviderProperty:
                {
                    xRRProviderPropertyNotifyEvent *stuff = (xRRProviderPropertyNotifyEvent *) evt;
                    REPLY (": XID(0x%x) Provider(0x%x)",
                        (unsigned int)stuff->window,
                        (unsigned int)stuff->provider);

                    REPLY (" Atom");
                    reply = xDbgGetAtom(stuff->atom, evinfo, reply, len);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        const char *state;
                        char dstate[10];

                        switch (stuff->state)
                        {
                            case PropertyNewValue:    state = "PropertyNewValue"; break;
                            case PropertyDelete:   state = "PropertyDelete"; break;
                            default:  state = dstate; snprintf (dstate, 10, "%d", stuff->state); break;
                        }

                        REPLY (" sequence_num(%d) timestamp(%lums) state(%s)",
                            stuff->sequenceNumber,
                            (unsigned long)stuff->timestamp,
                            state);
                    }

                    return reply;
                }

            case RRNotify_ResourceChange:
                {
                    xRRResourceChangeNotifyEvent *stuff = (xRRResourceChangeNotifyEvent *) evt;
                    REPLY (": XID(0x%x)",
                        (unsigned int)stuff->window);

                    if (detail_level >= EVLOG_PRINT_DETAIL)
                    {
                        REPLY (" sequence_num(%d) timestamp(%lums)",
                            stuff->sequenceNumber,
                            (unsigned long)stuff->timestamp);
                    }

                    return reply;
                }

            default:
                    break;
            }
        }

    default:
            break;
    }

    return reply;
}

static char *
_EvlogReplyRandr (EvlogInfo *evinfo, int detail_level, char *reply, int *len)
{
    xGenericReply *rep = evinfo->rep.ptr;

    switch (evinfo->rep.reqData)
    {
        case X_RRGetScreenSizeRange:
        {
            if (evinfo->rep.isStart)
            {
                xRRGetScreenSizeRangeReply *stuff = (xRRGetScreenSizeRangeReply *)rep;
                REPLY (": minSize(%dx%d) maxSize(%dx%d)",
                    stuff->minWidth,
                    stuff->minHeight,
                    stuff->maxWidth,
                    stuff->maxHeight);
            }
            else
            {
                return reply;
            }

            return reply;
        }

        case X_RRGetScreenResources:
        {
            static int nCrtcs, nOutputs, nModes, nbytesNames;

            if (evinfo->rep.isStart)
            {
                xRRGetScreenResourcesReply *stuff = (xRRGetScreenResourcesReply *)rep;
                REPLY (": Timestamp(%ldms) ConfigTimestamp(%ldms) nCrtcs(%d) nOutputs(%d) nModes(%d) nbytesNames(%d)",
                    (long int)stuff->timestamp,
                    (long int)stuff->configTimestamp,
                    stuff->nCrtcs,
                    stuff->nOutputs,
                    stuff->nModes,
                    stuff->nbytesNames);

                nCrtcs = stuff->nCrtcs;
                nOutputs = stuff->nOutputs;
                nModes = stuff->nModes;
                nbytesNames = stuff->nbytesNames;
            }
            else
            {
                RRCrtc *crtcs = (RRCrtc *)rep;
                RROutput *outputs = (RROutput *)(crtcs + nCrtcs);
                xRRModeInfo *modeinfos = (xRRModeInfo *)(outputs + nOutputs);
                CARD8 *names = (CARD8 *)(modeinfos + nModes);
                char temp[64] = {0, };
                int i;

                names[nbytesNames] = '\0';

                REPLY ("Crtcs");
                REPLY ("(");
                for (i = 0 ; i < nCrtcs ; i++)
                {
                    REPLY ("0x%x", (unsigned int)crtcs[i]);
                    if(i != nCrtcs - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Outputs");
                REPLY ("(");
                for (i = 0 ; i < nOutputs ; i++)
                {
                    REPLY ("0x%x", (unsigned int)outputs[i]);
                    if(i != nOutputs - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Modes");
                REPLY ("(");
                for (i = 0 ; i < nModes ; i++)
                {
                    REPLY ("0x%x %dx%d", (unsigned int)modeinfos[i].id, modeinfos[i].width, modeinfos[i].height);
                    if(i != nModes - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Names");

                int min = MIN (sizeof (temp) - 1, nbytesNames);
                strncpy (temp, (char *)names, min);
                temp[min] = '\0';

                REPLY ("(");
                REPLY ("%s", temp);
                REPLY (")");

            }

            return reply;
        }

        case X_RRGetOutputInfo:
        {
            static int nCrtcs, nModes, nClones, namelength;
            if (evinfo->rep.isStart)
            {
                xRRGetOutputInfoReply *stuff = (xRRGetOutputInfoReply *)rep;
                REPLY (": Timestamp(%ldms) Crtc(0x%x) mmSize(%ldx%ld) nCrtcs(%d) nModes(%d) nPreferred(%d) nClones(%d)",
                    (long int)stuff->timestamp,
                    (unsigned int)stuff->crtc,
                    (long int)stuff->mmWidth,
                    (long int)stuff->mmHeight,
                    stuff->nCrtcs,
                    stuff->nModes,
                    stuff->nPreferred,
                    stuff->nClones);

                nCrtcs = stuff->nCrtcs;
                nModes = stuff->nModes;
                nClones = stuff->nClones;
                namelength = stuff->nameLength;
            }
            else
            {
                RRCrtc *crtcs = (RRCrtc *) rep;
                RRMode *modes = (RRMode *) (crtcs + nCrtcs);
                RROutput *clones = (RROutput *) (modes + nModes);
                char *name = (char *) (clones + nClones);
                int i;

                name[namelength] = '\0';

                REPLY ("Crtcs");
                REPLY ("(");
                for (i = 0 ; i < nCrtcs ; i++)
                {
                    REPLY ("0x%x", (unsigned int)crtcs[i]);
                    if(i != nCrtcs - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Modes");
                REPLY ("(");
                for (i = 0 ; i < nModes ; i++)
                {
                    REPLY ("0x%x", (unsigned int)modes[i]);
                    if(i != nModes - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Clone");
                REPLY ("(");
                for (i = 0 ; i < nClones ; i++)
                {
                    REPLY ("0x%x", (unsigned int)clones[i]);
                    if(i != nClones - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Name");
                REPLY ("(");
                REPLY ("%s", name);
                REPLY (")");
            }

            return reply;
        }

        case X_RRListOutputProperties:
        {
            if (evinfo->rep.isStart)
            {
                xRRListOutputPropertiesReply *stuff = (xRRListOutputPropertiesReply *)rep;
                REPLY (": nAtoms(%d)",
                    stuff->nAtoms);
            }
            else
            {
                Atom *stuff = (Atom *)rep;
                int i;

                REPLY ("Properties:");
                for (i = 0 ; i < evinfo->rep.size / sizeof(Atom) ; i++)
                {
                    reply = xDbgGetAtom(stuff[i], evinfo, reply, len);
                    if(i != evinfo->rep.size / sizeof(Atom) - 1)
                        REPLY (", ");
                }
            }

            return reply;
        }

        case X_RRGetOutputProperty:
        {
            if (evinfo->rep.isStart)
            {
                xRRGetOutputPropertyReply *stuff = (xRRGetOutputPropertyReply *)rep;
                REPLY (": Atoms");
                reply = xDbgGetAtom(stuff->propertyType, evinfo, reply, len);

                REPLY (" bytesAfter(%ld) nItems(%ld)",
                    (long int)stuff->bytesAfter,
                    (long int)stuff->nItems);
            }
            else
            {
                return reply;
            }

            return reply;
        }

        case X_RRGetCrtcInfo:
        {
            static int nOutput, nPossibleOutput;

            if (evinfo->rep.isStart)
            {
                xRRGetCrtcInfoReply *stuff = (xRRGetCrtcInfoReply *)rep;
                REPLY (" Timestamp(%ldms) coord(%d,%d %dx%d) RRmode(0x%x) rot(%d) rots(%d) nOutput(%d) nPossibleOutput(%d)",
                    (long int)stuff->timestamp,
                    stuff->x,
                    stuff->y,
                    stuff->width,
                    stuff->height,
                    (unsigned int)stuff->mode,
                    stuff->rotation,
                    stuff->rotations,
                    stuff->nOutput,
                    stuff->nPossibleOutput);

                nOutput = stuff->nOutput;
                nPossibleOutput = stuff->nPossibleOutput;
            }
            else
            {
                RROutput *outputs = (RROutput *) rep;
                RROutput *possible = (RROutput *) (outputs + nOutput);
                int i;

                REPLY ("Outputs");
                REPLY ("(");
                for (i = 0 ; i < nOutput ; i++)
                {
                    REPLY ("0x%x", (unsigned int)outputs[i]);
                    if(i != nOutput - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Possible");
                REPLY ("(");
                for (i = 0 ; i < nPossibleOutput ; i++)
                {
                    REPLY ("0x%x", (unsigned int)possible[i]);
                    if(i != nPossibleOutput - 1)
                        REPLY (", ");
                }
                REPLY (")");
            }

            return reply;
        }

        case X_RRSetCrtcConfig:
        {
            if (evinfo->rep.isStart)
            {
                xRRSetCrtcConfigReply *stuff = (xRRSetCrtcConfigReply *)rep;

                REPLY (" newTimestamp(%ldms)",
                    (long int)stuff->newTimestamp);
            }
            else
            {
                return reply;
            }

            return reply;
        }

        case X_RRGetScreenResourcesCurrent:
        {
            static int nCrtcs, nOutputs, nModes, nbytesNames;

            if (evinfo->rep.isStart)
            {
                xRRGetScreenResourcesReply *stuff = (xRRGetScreenResourcesReply *)rep;

                REPLY (" Timestamp(%ldms) ConfigTimestamp(%ldms) nCrtcs(%d) nOutputs(%d) nModes(%d) nbytesNames(%d)",
                    (long int)stuff->timestamp,
                    (long int)stuff->configTimestamp,
                    stuff->nCrtcs,
                    stuff->nOutputs,
                    stuff->nModes,
                    stuff->nbytesNames);

                nCrtcs = stuff->nCrtcs;
                nOutputs = stuff->nOutputs;
                nModes = stuff->nModes;
                nbytesNames = stuff->nbytesNames;
            }
            else
            {
                RRCrtc *crtcs = (RRCrtc *)rep;
                RROutput *outputs = (RROutput *)(crtcs + nCrtcs);
                xRRModeInfo *modeinfos = (xRRModeInfo *)(outputs + nOutputs);
                CARD8 *names = (CARD8 *)(modeinfos + nModes);
                char temp[64] = {0, };
                int i;

                REPLY ("Crtcs");
                REPLY ("(");
                for (i = 0 ; i < nCrtcs ; i++)
                {
                    REPLY ("0x%x", (unsigned int)crtcs[i]);
                    if(i != nCrtcs - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Outputs");
                REPLY ("(");
                for (i = 0 ; i < nOutputs ; i++)
                {
                    REPLY ("0x%x", (unsigned int)outputs[i]);
                    if(i != nOutputs - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Modes");
                REPLY ("(");
                for (i = 0 ; i < nModes ; i++)
                {
                    REPLY ("0x%x %dx%d", (unsigned int)modeinfos[i].id, modeinfos[i].width, modeinfos[i].height);
                    if(i != nModes - 1)
                        REPLY (", ");
                }
                REPLY (")");

                REPLY (" Names");

                int min = MIN (sizeof (temp) - 1, nbytesNames);
                strncpy (temp, (char *)names, min);
                temp[min] = '\0';

                REPLY ("(");
                REPLY ("%s", temp);
                REPLY (")");
            }

            return reply;
        }

    default:
            break;
    }

    return reply;
}

void
xDbgEvlogRandrGetBase (ExtensionInfo *extinfo)
{
#ifdef XDBG_CLIENT
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->req_func = _EvlogRequestRandr;
    extinfo->evt_func = _EvlogEventRandr;
    extinfo->rep_func = _EvlogReplyRandr;
#else
    ExtensionEntry *xext = CheckExtension (RANDR_NAME);
    RETURN_IF_FAIL (xext != NULL);
    RETURN_IF_FAIL (extinfo != NULL);

    extinfo->opcode = xext->base;
    extinfo->evt_base = xext->eventBase;
    extinfo->err_base = xext->errorBase;
    extinfo->req_func = _EvlogRequestRandr;
    extinfo->evt_func = _EvlogEventRandr;
    extinfo->rep_func = _EvlogReplyRandr;
#endif
}
