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

struct p9p_tversion
{

    unsigned char msize[4];

};

struct p9p_rversion
{

    unsigned char msize[4];

};

struct p9p_twalk
{

    unsigned char fid[4];
    unsigned char newfid[4];
    unsigned char nwname[2];

};

struct p9p_rwalk
{

    unsigned char nwqid[2];

};

struct p9p_tread
{

    unsigned char fid[4];
    unsigned char offset[8];
    unsigned char count[4];

};

struct p9p_rread
{

    unsigned char count[4];

};

struct p9p_twrite
{

    unsigned char fid[4];
    unsigned char offset[8];
    unsigned char count[4];

};

struct p9p_rwrite
{

    unsigned char count[4];

};

struct p9p_topen
{

    unsigned char fid[4];
    unsigned char mode[1];

};

struct p9p_ropen
{

    unsigned char qid[13];
    unsigned char iounit[4];

};

struct p9p_tcreate
{

    unsigned char fid[4];

};

struct p9p_rcreate
{

    unsigned char qid[13];
    unsigned char iounit[4];

};

unsigned char p9p_read1(unsigned char p[1]);
unsigned short p9p_read2(unsigned char p[2]);
unsigned int p9p_read4(unsigned char p[4]);
void p9p_write1(unsigned char p[1], unsigned char v);
void p9p_write2(unsigned char p[2], unsigned short v);
void p9p_write4(unsigned char p[4], unsigned int v);
void p9p_write8(unsigned char p[8], unsigned int vl, unsigned int vh);
void p9p_mktversion(struct message *message, unsigned short tag, unsigned int msize, char *version);
void p9p_mkrversion(struct message *message, unsigned short tag, unsigned int msize, char *version);
void p9p_mktwalk(struct message *message, unsigned short tag, unsigned int fid, unsigned int newfid, char *wname);
void p9p_mkrwalk(struct message *message, unsigned short tag, unsigned short nwqid);
void p9p_mktread(struct message *message, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count);
void p9p_mkrread(struct message *message, unsigned short tag, unsigned int count, void *buffer);
void p9p_mktopen(struct message *message, unsigned short tag, unsigned int fid, unsigned char mode);
void p9p_mkropen(struct message *message, unsigned short tag, unsigned int qid, unsigned int iounit);
