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

void *message_validate(unsigned int type, void *data, unsigned int count)
{

    switch (type)
    {

    case MESSAGE_CONSOLEDATA:
        return (count >= sizeof (struct message_consoledata)) ? data : 0;

    case MESSAGE_MAIN:
        return (count) ? checksize(data, count, sizeof (struct message_pipe)) : data;

    case MESSAGE_KEYPRESS:
        return checksize(data, count, sizeof (struct message_keypress));

    case MESSAGE_KEYRELEASE:
        return checksize(data, count, sizeof (struct message_keyrelease));

    case MESSAGE_MOUSEMOVE:
        return checksize(data, count, sizeof (struct message_mousemove));

    case MESSAGE_MOUSESCROLL:
        return checksize(data, count, sizeof (struct message_mousescroll));

    case MESSAGE_MOUSEPRESS:
        return checksize(data, count, sizeof (struct message_mousepress));

    case MESSAGE_MOUSERELEASE:
        return checksize(data, count, sizeof (struct message_mouserelease));

    case MESSAGE_TIMERTICK:
        return checksize(data, count, sizeof (struct message_timertick));

    case MESSAGE_VIDEOCONF:
        return checksize(data, count, sizeof (struct message_videoconf));

    case MESSAGE_VIDEOINFO:
        return checksize(data, count, sizeof (struct message_videoinfo));

    case MESSAGE_BLOCKINFO:
        return checksize(data, count, sizeof (struct message_blockinfo));

    case MESSAGE_BLOCKREADREQUEST:
        return checksize(data, count, sizeof (struct message_blockrequest));

    case MESSAGE_BLOCKREADRESPONSE:
        return checksize(data, count, sizeof (struct message_blockresponse));

    case MESSAGE_BLOCKWRITEREQUEST:
        return checksize(data, count, sizeof (struct message_blockrequest));

    case MESSAGE_BLOCKWRITERESPONSE:
        return checksize(data, count, sizeof (struct message_blockresponse));

    case MESSAGE_CLOCKINFO:
        return checksize(data, count, sizeof (struct message_clockinfo));

    case MESSAGE_ETHERNETINFO:
        return checksize(data, count, sizeof (struct message_ethernetinfo));

    case MESSAGE_AUDIOCONF:
        return checksize(data, count, sizeof (struct message_audioconf));

    case MESSAGE_AUDIOINFO:
        return checksize(data, count, sizeof (struct message_audioinfo));

    case MESSAGE_VIDEOFLUSH:
        return checksize(data, count, sizeof (struct message_videoflush));

    case MESSAGE_TIMERREQUEST:
        return checksize(data, count, sizeof (struct message_timerrequest));

    case MESSAGE_TIMEREXPIRE:
        return checksize(data, count, sizeof (struct message_timerexpire));

    case MESSAGE_TIMERINFO:
        return checksize(data, count, sizeof (struct message_timerinfo));

    case MESSAGE_WALKRESPONSE:
        return checksize(data, count, sizeof (struct message_walkresponse));

    case MESSAGE_READREQUEST:
        return checksize(data, count, sizeof (struct message_readrequest));

    case MESSAGE_WRITERESPONSE:
        return checksize(data, count, sizeof (struct message_writeresponse));

    case MESSAGE_STATREQUEST:
        return checksize(data, count, sizeof (struct message_statrequest));

    case MESSAGE_MAPREQUEST:
        return checksize(data, count, sizeof (struct message_maprequest));

    case MESSAGE_MAPRESPONSE:
        return checksize(data, count, sizeof (struct message_mapresponse));

    case MESSAGE_CREATERESPONSE:
        return checksize(data, count, sizeof (struct message_createresponse));

    case MESSAGE_REMOVEREQUEST:
        return checksize(data, count, sizeof (struct message_removerequest));

    case MESSAGE_REMOVERESPONSE:
        return checksize(data, count, sizeof (struct message_removeresponse));

    case MESSAGE_MMAPREQUEST:
        return checksize(data, count, sizeof (struct message_mmaprequest));

    case MESSAGE_MMAPRESPONSE:
        return checksize(data, count, sizeof (struct message_mmapresponse));

    case MESSAGE_MUNMAPREQUEST:
        return checksize(data, count, sizeof (struct message_munmaprequest));

    case MESSAGE_MUNMAPRESPONSE:
        return checksize(data, count, sizeof (struct message_munmapresponse));

    case MESSAGE_WMKEYPRESS:
        return checksize(data, count, sizeof (struct message_wmkeypress));

    case MESSAGE_WMKEYRELEASE:
        return checksize(data, count, sizeof (struct message_wmkeyrelease));

    case MESSAGE_WMMOUSEMOVE:
        return checksize(data, count, sizeof (struct message_wmmousemove));

    case MESSAGE_WMMOUSESCROLL:
        return checksize(data, count, sizeof (struct message_wmmousescroll));

    case MESSAGE_WMMOUSEPRESS:
        return checksize(data, count, sizeof (struct message_wmmousepress));

    case MESSAGE_WMMOUSERELEASE:
        return checksize(data, count, sizeof (struct message_wmmouserelease));

    case MESSAGE_CREATEREQUEST:
        if (count >= sizeof (struct message_createrequest))
        {

            struct message_createrequest *createrequest = data;

            return checkpayload(data, count, sizeof (struct message_createrequest), createrequest->count);

        }

        return 0;

    case MESSAGE_STATRESPONSE:
        if (count >= sizeof (struct message_statresponse))
        {

            struct message_statresponse *statresponse = data;

            return checkpayload(data, count, sizeof (struct message_statresponse), statresponse->count);

        }

        return 0;

    case MESSAGE_WALKREQUEST:
        if (count >= sizeof (struct message_walkrequest))
        {

            struct message_walkrequest *walkrequest = data;

            return checkpayload(data, count, sizeof (struct message_walkrequest), walkrequest->length);

        }

        return 0;

    case MESSAGE_WMEVENT:
        if (count >= sizeof (struct message_wmevent))
        {

            struct message_wmevent *wmevent = data;

            return checkpayload(data, count, sizeof (struct message_wmevent), wmevent->length);

        }

        return 0;

    case MESSAGE_WRITEREQUEST:
        if (count >= sizeof (struct message_writerequest))
        {

            struct message_writerequest *writerequest = data;

            return checkpayload(data, count, sizeof (struct message_writerequest), writerequest->count);

        }

        return 0;

    case MESSAGE_LOGINFO:
        if (count >= sizeof (struct message_loginfo))
        {

            struct message_loginfo *loginfo = data;

            return checksize(data, count, loginfo->count);

        }

        return 0;

    case MESSAGE_READRESPONSE:
        if (count >= sizeof (struct message_readresponse))
        {

            struct message_readresponse *readresponse = data;

            return checkpayload(data, count, sizeof (struct message_readresponse), (readresponse->buffer) ? 0 : readresponse->count);

        }

        return 0;

    }

    return data;

}

