#define I915_VENDOR                     0x8086
#define I915_DEVICES                    11
#define I915_MMIOSIZE                   0x100000
#define I915_MINSIZE                    64
#define I915_TIMEOUT                    100000
#define I915_EVASION                    16

#define I915_AUX_SIZE                   20
#define I915_AUX_RETRIES                32
#define I915_EDIDSIZE                   128
#define I915_EDIDCHUNK                  16
#define I915_EDIDADDRESS                0x50
#define I915_EDIDTIMING                 54

#define I915_ASLS                       0xFC
#define I915_OPREGIONMAP                0xB0000000
#define I915_OPREGIONSIZE               0x8000
#define I915_OPREGION_VERSION           0x14
#define I915_OPREGION_MAILBOXES         0x58
#define I915_OPREGION_MAILBOX_VBT       (0x01u << 3)
#define I915_OPREGION_VBT               0x400
#define I915_OPREGION_RVDA              0x3BA
#define I915_OPREGION_RVDS              0x3C2
#define I915_VBT_BDB                    0x1C
#define I915_BDB_VERSION                0x10
#define I915_BDB_HEADERSIZE             0x12
#define I915_BDB_SIZE                   0x14
#define I915_BDB_GENERAL                2
#define I915_GENERAL_CHILDSIZE          4
#define I915_GENERAL_CHILDREN           5
#define I915_CHILD_TYPE                 2
#define I915_CHILD_PORT                 16
#define I915_CHILD_DDC                  19
#define I915_CHILD_AUX                  25
#define I915_CHILD_FLAGS                33
#define I915_CHILD_MINSIZE              34
#define I915_PORTS                      22

#define I915_COMBOPORTS                 3
#define I915_DDCCOMBOPINS               3
#define I915_DDCTCPIN                   5

#define I915_TRANS_HTOTAL_A             0x60000
#define I915_TRANS_VTOTAL_A             0x6000C
#define I915_PIPESRC_A                  0x6001C
#define I915_SDEISR                     0xC4000
#define I915_SHOTPLUG_CTL_DDI           0xC4030
#define I915_SHOTPLUG_CTL_TC            0xC4034
#define I915_GMBUS0                     0xC5100
#define I915_GMBUS1                     0xC5104
#define I915_GMBUS2                     0xC5108
#define I915_GMBUS3                     0xC510C
#define I915_GMBUS5                     0xC5120
#define I915_AUX_CTL_A                  0x64010
#define I915_AUX_DATA_A                 0x64014
#define I915_PS_WIN_POS_1_A             0x68170
#define I915_PS_WIN_SZ_1_A              0x68174
#define I915_PS_CTRL_1_A                0x68180
#define I915_PIPEDSL_A                  0x70000
#define I915_TRANS_CONF_A               0x70008
#define I915_PLANE_CTL_1_A              0x70180
#define I915_PLANE_STRIDE_1_A           0x70188
#define I915_PLANE_SIZE_1_A             0x70190
#define I915_PLANE_SURF_1_A             0x7019C

#define I915_PIPEDSL_LINE               0xFFFFFu
#define I915_AUX_CTL_BUSY               (0x01u << 31)
#define I915_AUX_CTL_DONE               (0x01u << 30)
#define I915_AUX_CTL_TIMEOUT            (0x01u << 28)
#define I915_AUX_CTL_TIMEOUTMAX         (0x03u << 26)
#define I915_AUX_CTL_RECEIVEERROR       (0x01u << 25)
#define I915_AUX_CTL_SIZESHIFT          20
#define I915_AUX_CTL_SIZE               0x1Fu
#define I915_AUX_CTL_SYNC               ((31u << 5) | 31u)
#define I915_AUX_I2CWRITE               0x0
#define I915_AUX_I2CREAD                0x1
#define I915_AUX_I2CMOT                 0x4
#define I915_AUX_NATIVEREAD             0x9
#define I915_AUX_REPLYNACK              0x50
#define I915_AUX_REPLYDEFER             0xA0
#define I915_CHILD_TYPE_INTERNAL        (0x01u << 12)
#define I915_CHILD_TYPE_NOTHDMI         (0x01u << 11)
#define I915_CHILD_TYPE_TMDS            (0x01u << 4)
#define I915_CHILD_TYPE_DP              (0x01u << 2)
#define I915_CHILD_FLAGS_TYPEC          (0x01u << 0)
#define I915_CHILD_FLAGS_TBT            (0x01u << 1)
#define I915_SDEISR_COMBO               16
#define I915_SDEISR_TC                  24
#define I915_SHOTPLUG_ENABLE            0x08u
#define I915_SHOTPLUG_STATUS            0x03u
#define I915_SHOTPLUG_BITS              4
#define I915_GMBUS0_RATE100KHZ          (0x00u << 8)
#define I915_GMBUS1_CLEAR               (0x01u << 31)
#define I915_GMBUS1_READY               (0x01u << 30)
#define I915_GMBUS1_STOP                (0x04u << 25)
#define I915_GMBUS1_INDEX               (0x02u << 25)
#define I915_GMBUS1_WAIT                (0x01u << 25)
#define I915_GMBUS1_COUNTSHIFT          16
#define I915_GMBUS1_ADDRESSSHIFT        1
#define I915_GMBUS1_READ                0x01u
#define I915_GMBUS2_WAITPHASE           (0x01u << 14)
#define I915_GMBUS2_HWREADY             (0x01u << 11)
#define I915_GMBUS2_NAK                 (0x01u << 10)
#define I915_GMBUS2_ACTIVE              (0x01u << 9)
#define I915_TRANS_CONF_ENABLE          (0x01u << 31)
#define I915_PS_CTRL_ENABLE             (0x01u << 31)
#define I915_PLANE_CTL_ENABLE           (0x01u << 31)
#define I915_PLANE_CTL_FORMAT           (0x1Fu << 23)
#define I915_PLANE_CTL_FORMAT_XRGB8888  (0x08u << 23)
#define I915_PLANE_CTL_TILED            (0x07u << 10)
