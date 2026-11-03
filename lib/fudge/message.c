#include "buffer.h"
#include "message.h"

static void *checksize(void *data, unsigned int count, unsigned int size)
{

    return (count == size) ? data : 0;

}

static void *checkpayload(void *data, unsigned int count, unsigned int size, unsigned int payload)
{

    return (count - size == payload) ? data : 0;

}

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, void *data)
{

    message->event = event;
    message->source = source;
    message->length = length;
    message->data = data;

}

/* the pipe of a task started outside a job: no routes, TERM goes to the owner */
void message_initpipe(struct message_pipe *pipe)
{

    buffer_clear(pipe, sizeof (struct message_pipe));

    pipe->nterms = 1;
    pipe->terms[0].event = MESSAGE_TYPE_TERM;
    pipe->terms[0].to = MESSAGE_TYPE_TERM;

}

void *message_validate(unsigned int type, void *data, unsigned int count)
{

    switch (type)
    {

    case MESSAGE_TYPE_MAIN:
        return checksize(data, count, sizeof (struct message_pipe));

    case MESSAGE_TYPE_KEYPRESS:
        return checksize(data, count, sizeof (struct message_keypress));

    case MESSAGE_TYPE_KEYRELEASE:
        return checksize(data, count, sizeof (struct message_keyrelease));

    case MESSAGE_TYPE_MOUSEMOVE:
        return checksize(data, count, sizeof (struct message_mousemove));

    case MESSAGE_TYPE_MOUSESCROLL:
        return checksize(data, count, sizeof (struct message_mousescroll));

    case MESSAGE_TYPE_MOUSEPRESS:
        return checksize(data, count, sizeof (struct message_mousepress));

    case MESSAGE_TYPE_MOUSERELEASE:
        return checksize(data, count, sizeof (struct message_mouserelease));

    case MESSAGE_TYPE_TIMERTICK:
        return checksize(data, count, sizeof (struct message_timertick));

    case MESSAGE_TYPE_VIDEOCONF:
        return checksize(data, count, sizeof (struct message_videoconf));

    case MESSAGE_TYPE_VIDEOINFO:
        return checksize(data, count, sizeof (struct message_videoinfo));

    case MESSAGE_TYPE_BLOCKINFO:
        return checksize(data, count, sizeof (struct message_blockinfo));

    case MESSAGE_TYPE_BLOCKREADREQUEST:
        return checksize(data, count, sizeof (struct message_blockrequest));

    case MESSAGE_TYPE_BLOCKREADRESPONSE:
        return checksize(data, count, sizeof (struct message_blockresponse));

    case MESSAGE_TYPE_BLOCKWRITEREQUEST:
        return checksize(data, count, sizeof (struct message_blockrequest));

    case MESSAGE_TYPE_BLOCKWRITERESPONSE:
        return checksize(data, count, sizeof (struct message_blockresponse));

    case MESSAGE_TYPE_CLOCKINFO:
        return checksize(data, count, sizeof (struct message_clockinfo));

    case MESSAGE_TYPE_ETHERNETINFO:
        return checksize(data, count, sizeof (struct message_ethernetinfo));

    case MESSAGE_TYPE_AUDIOCONF:
        return checksize(data, count, sizeof (struct message_audioconf));

    case MESSAGE_TYPE_AUDIOINFO:
        return checksize(data, count, sizeof (struct message_audioinfo));

    case MESSAGE_TYPE_VIDEOFLUSH:
        return checksize(data, count, sizeof (struct message_videoflush));

    case MESSAGE_TYPE_TIMERREQUEST:
        return checksize(data, count, sizeof (struct message_timerrequest));

    case MESSAGE_TYPE_TIMEREXPIRE:
        return checksize(data, count, sizeof (struct message_timerexpire));

    case MESSAGE_TYPE_TIMERINFO:
        return checksize(data, count, sizeof (struct message_timerinfo));

    case MESSAGE_TYPE_WALKRESPONSE:
        return checksize(data, count, sizeof (struct message_walkresponse));

    case MESSAGE_TYPE_READREQUEST:
        return checksize(data, count, sizeof (struct message_readrequest));

    case MESSAGE_TYPE_WRITERESPONSE:
        return checksize(data, count, sizeof (struct message_writeresponse));

    case MESSAGE_TYPE_STATREQUEST:
        return checksize(data, count, sizeof (struct message_statrequest));

    case MESSAGE_TYPE_MAPREQUEST:
        return checksize(data, count, sizeof (struct message_maprequest));

    case MESSAGE_TYPE_MAPRESPONSE:
        return checksize(data, count, sizeof (struct message_mapresponse));

    case MESSAGE_TYPE_CREATERESPONSE:
        return checksize(data, count, sizeof (struct message_createresponse));

    case MESSAGE_TYPE_REMOVEREQUEST:
        return checksize(data, count, sizeof (struct message_removerequest));

    case MESSAGE_TYPE_REMOVERESPONSE:
        return checksize(data, count, sizeof (struct message_removeresponse));

    case MESSAGE_TYPE_MMAPREQUEST:
        return checksize(data, count, sizeof (struct message_mmaprequest));

    case MESSAGE_TYPE_MMAPRESPONSE:
        return checksize(data, count, sizeof (struct message_mmapresponse));

    case MESSAGE_TYPE_MUNMAPREQUEST:
        return checksize(data, count, sizeof (struct message_munmaprequest));

    case MESSAGE_TYPE_MUNMAPRESPONSE:
        return checksize(data, count, sizeof (struct message_munmapresponse));

    case MESSAGE_TYPE_WMKEYPRESS:
        return checksize(data, count, sizeof (struct message_wmkeypress));

    case MESSAGE_TYPE_WMKEYRELEASE:
        return checksize(data, count, sizeof (struct message_wmkeyrelease));

    case MESSAGE_TYPE_WMMOUSEMOVE:
        return checksize(data, count, sizeof (struct message_wmmousemove));

    case MESSAGE_TYPE_WMMOUSESCROLL:
        return checksize(data, count, sizeof (struct message_wmmousescroll));

    case MESSAGE_TYPE_WMMOUSEPRESS:
        return checksize(data, count, sizeof (struct message_wmmousepress));

    case MESSAGE_TYPE_WMMOUSERELEASE:
        return checksize(data, count, sizeof (struct message_wmmouserelease));

    case MESSAGE_TYPE_CREATEREQUEST:
        if (count >= sizeof (struct message_createrequest))
        {

            struct message_createrequest *createrequest = data;

            return checkpayload(data, count, sizeof (struct message_createrequest), createrequest->count);

        }

        return 0;

    case MESSAGE_TYPE_STATRESPONSE:
        if (count >= sizeof (struct message_statresponse))
        {

            struct message_statresponse *statresponse = data;

            return checkpayload(data, count, sizeof (struct message_statresponse), statresponse->count);

        }

        return 0;

    case MESSAGE_TYPE_WALKREQUEST:
        if (count >= sizeof (struct message_walkrequest))
        {

            struct message_walkrequest *walkrequest = data;

            return checkpayload(data, count, sizeof (struct message_walkrequest), walkrequest->length);

        }

        return 0;

    case MESSAGE_TYPE_WMEVENT:
        if (count >= sizeof (struct message_wmevent))
        {

            struct message_wmevent *wmevent = data;

            return checkpayload(data, count, sizeof (struct message_wmevent), wmevent->length);

        }

        return 0;

    case MESSAGE_TYPE_WRITEREQUEST:
        if (count >= sizeof (struct message_writerequest))
        {

            struct message_writerequest *writerequest = data;

            return checkpayload(data, count, sizeof (struct message_writerequest), writerequest->count);

        }

        return 0;

    case MESSAGE_TYPE_LOGINFO:
        if (count >= sizeof (struct message_loginfo))
        {

            struct message_loginfo *loginfo = data;

            return checkpayload(data, count, sizeof (struct message_loginfo), loginfo->count);

        }

        return 0;

    case MESSAGE_TYPE_READRESPONSE:
        if (count >= sizeof (struct message_readresponse))
        {

            struct message_readresponse *readresponse = data;

            return checkpayload(data, count, sizeof (struct message_readresponse), (readresponse->buffer) ? 0 : readresponse->count);

        }

        return 0;

    }

    return data;

}

