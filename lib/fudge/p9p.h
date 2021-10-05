#define P9P_TVERSION                    100
#define P9P_RVERSION                    101
#define P9P_TAUTH                       102
#define P9P_RAUTH                       103
#define P9P_TATTACH                     104
#define P9P_RATTACH                     105
#define P9P_TERROR                      106
#define P9P_RERROR                      107
#define P9P_TFLUSH                      108
#define P9P_RFLUSH                      109
#define P9P_TWALK                       110
#define P9P_RWALK                       111
#define P9P_TOPEN                       112
#define P9P_ROPEN                       113
#define P9P_TCREATE                     114
#define P9P_RCREATE                     115
#define P9P_TREAD                       116
#define P9P_RREAD                       117
#define P9P_TWRITE                      118
#define P9P_RWRITE                      119
#define P9P_TCLUNK                      120
#define P9P_RCLUNK                      121
#define P9P_TREMOVE                     122
#define P9P_RREMOVE                     123
#define P9P_TSTAT                       124
#define P9P_RSTAT                       125
#define P9P_TWSTAT                      126
#define P9P_RWSTAT                      127
#define P9P_OFFSET_SIZE                 0
#define P9P_OFFSET_TYPE                 4
#define P9P_OFFSET_TAG                  5
#define P9P_OFFSET_DATA                 7

struct p9p_event
{

    unsigned char size[4];
    unsigned char type[1];
    unsigned char tag[2];

};

unsigned char p9p_read1(void *buffer, unsigned int o);
unsigned short p9p_read2(void *buffer, unsigned int o);
unsigned int p9p_read4(void *buffer, unsigned int o);
unsigned int p9p_write1(void *buffer, unsigned int o, unsigned char v);
unsigned int p9p_write2(void *buffer, unsigned int o, unsigned short v);
unsigned int p9p_write4(void *buffer, unsigned int o, unsigned int v);
unsigned int p9p_write8(void *buffer, unsigned int o, unsigned int vl, unsigned int vh);
unsigned int p9p_writebuffer(void *buffer, unsigned int o, unsigned int count, void *v);
unsigned int p9p_writestring(void *buffer, unsigned int o, char *v);
unsigned int p9p_mk(void *buffer, unsigned int size, unsigned char type, unsigned short tag);
unsigned int p9p_mkrerror(void *buffer, unsigned short tag, char *ename);
unsigned int p9p_mktattach(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname);
unsigned int p9p_mkrattach(void *buffer, unsigned short tag, char *qid[13]);
unsigned int p9p_mktauth(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname);
unsigned int p9p_mkrauth(void *buffer, unsigned short tag, char *aqid[13]);
unsigned int p9p_mktclunk(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrclunk(void *buffer, unsigned short tag);
unsigned int p9p_mktflush(void *buffer, unsigned short tag, unsigned short oldtag);
unsigned int p9p_mkrflush(void *buffer, unsigned short tag);
unsigned int p9p_mktversion(void *buffer, unsigned short tag, unsigned int msize, char *version);
unsigned int p9p_mkrversion(void *buffer, unsigned short tag, unsigned int msize, char *version);
unsigned int p9p_mktwalk(void *buffer, unsigned short tag, unsigned int fid, unsigned int newfid, unsigned short nwname, char **wname);
unsigned int p9p_mkrwalk(void *buffer, unsigned short tag, unsigned short nwqid, char *wqid[13]);
unsigned int p9p_mktread(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count);
unsigned int p9p_mkrread(void *buffer, unsigned short tag, unsigned int count, void *data);
unsigned int p9p_mktwrite(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count, void *data);
unsigned int p9p_mkrwrite(void *buffer, unsigned short tag, unsigned int count);
unsigned int p9p_mktopen(void *buffer, unsigned short tag, unsigned int fid, unsigned char mode);
unsigned int p9p_mkropen(void *buffer, unsigned short tag, char *qid[13], unsigned int iounit);
unsigned int p9p_mktcreate(void *buffer, unsigned short tag, unsigned int fid, char *name, unsigned int perm, unsigned char mode);
unsigned int p9p_mkrcreate(void *buffer, unsigned short tag, char *qid[13], unsigned int iounit);
unsigned int p9p_mktremove(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrremove(void *buffer, unsigned short tag);
unsigned int p9p_mktstat(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrstat(void *buffer, unsigned short tag, void *stat);
unsigned int p9p_mktwstat(void *buffer, unsigned short tag, unsigned int fid, void *stat);
unsigned int p9p_mkrwstat(void *buffer, unsigned short tag);
