#define I915_VENDOR                     0x8086
#define I915_DEVICES                    11
#define I915_MMIOSIZE                   0x200000
#define I915_MINSIZE                    64
#define I915_TIMEOUT                    100000
#define I915_EVASION                    16
#define I915_PIPESTRIDE                 0x1000
#define I915_SCALERSTRIDE               0x800

#define I915_AUX_SIZE                   20
#define I915_AUX_RETRIES                32
#define I915_EDIDSIZE                   128
#define I915_EDIDBUFFER                 256
#define I915_EDIDDESCRIPTORS            4
#define I915_EDIDDESCRIPTOR             18
#define I915_EDIDCTA                    0x02
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

#define I915_HDMI_MAXCLOCK              340000
#define I915_HDMI_MINWIDTH              640
#define I915_HDMI_MINHEIGHT             480
#define I915_HDMI_MAXWIDTH              4096
#define I915_HDMI_MAXHEIGHT             4096
#define I915_DCOMIN                     7998000
#define I915_DCOMAX                     10000000
#define I915_DCOMID                     9000000
#define I915_DIVIDERS                   37
#define I915_HDMI_SWING                 0x6
#define I915_HDMI_NSCALAR               0x7F
#define I915_HDMI_CURSOR                0x35
#define I915_HDMI_POSTCURSOR            0x0A
#define I915_WMLEVELS                   8
#define I915_DBUFSLICE                  1024
#define I915_LANES                      4
#define I915_SECONDMAP                  0xA8000000
#define I915_SECONDLARGE                0x1000000
#define I915_SECONDSMALL                0x800000
#define I915_SECONDALIGN                0x40000
#define I915_SECONDWIDTH                1920
#define I915_SECONDHEIGHT               1080
#define I915_STOLENRESERVE              0x800000
#define I915_GSM                        0x800000
#define I915_PAGESIZE                   0x1000
#define I915_GMCH_CTRL                  0x50
#define I915_BDSM                       0xC0
#define I915_BDSMLEGACY                 0x5C
#define I915_BDSM_MASK                  0xFFF00000u
#define I915_EDIDTRIES                  3
#define I915_SHPD_FILTER                0x1D9

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
#define I915_FUSE_STATUS                0x42000
#define I915_PWR_WELL_CTL               0x45404
#define I915_PWR_WELL_CTL_DDI           0x45454
#define I915_DBUF_CTL_S2                0x44FE8
#define I915_WM_LINETIME_A              0x45270
#define I915_WM_LINETIME_B              0x45274
#define I915_DPLL_ENABLE                0x46010
#define I915_TRANS_CLK_SEL_B            0x46144
#define I915_DSSM                       0x51004
#define I915_TRANS_HTOTAL_B             0x61000
#define I915_TRANS_HBLANK_B             0x61004
#define I915_TRANS_HSYNC_B              0x61008
#define I915_TRANS_VTOTAL_B             0x6100C
#define I915_TRANS_VBLANK_B             0x61010
#define I915_TRANS_VSYNC_B              0x61014
#define I915_PIPESRC_B                  0x6101C
#define I915_TRANS_VSYNCSHIFT_B         0x61028
#define I915_TRANS_DDI_FUNC_CTL_B       0x61400
#define I915_DDI_BUF_CTL_B              0x64100
#define I915_PHYB_CL_DW5                0x6C014
#define I915_PHYB_CL_DW10               0x6C028
#define I915_PHYB_COMP_DW0              0x6C100
#define I915_PHYB_PCS_DW1_GRP           0x6C604
#define I915_PHYB_PCS_DW1_LN0           0x6C804
#define I915_PHYB_TX_DW2_GRP            0x6C688
#define I915_PHYB_TX_DW2_LN0            0x6C888
#define I915_PHYB_TX_DW4_LN0            0x6C890
#define I915_PHYB_TX_DW5_GRP            0x6C694
#define I915_PHYB_TX_DW5_LN0            0x6C894
#define I915_PHYB_TX_DW7_GRP            0x6C69C
#define I915_PHYB_TX_DW7_LN0            0x6C89C
#define I915_PHY_LANESTRIDE             0x100
#define I915_PIPE_MBUS_DBOX_CTL_A       0x7003C
#define I915_PIPE_MBUS_DBOX_CTL_B       0x7103C
#define I915_PIPEMISC_B                 0x71030
#define I915_TRANS_CONF_B               0x71008
#define I915_PLANE_WM_1_A               0x70240
#define I915_PLANE_WM_1_B               0x71240
#define I915_PLANE_WM_TRANS_1_A         0x70268
#define I915_PLANE_WM_TRANS_1_B         0x71268
#define I915_PLANE_BUF_CFG_1_A          0x7027C
#define I915_PLANE_BUF_CFG_1_B          0x7127C
#define I915_PLANE_CTL_1_B              0x71180
#define I915_PLANE_POS_1_B              0x7118C
#define I915_PLANE_STRIDE_1_B           0x71188
#define I915_PLANE_SIZE_1_B             0x71190
#define I915_PLANE_SURF_1_B             0x7119C
#define I915_PLANE_OFFSET_1_B           0x711A4
#define I915_PLANE_COLOR_CTL_1_B        0x711CC
#define I915_DPCLKA_CFGCR0              0x164280
#define I915_DPLL_CFGCR0                0x164284
#define I915_DPLL_CFGCR1                0x164288
#define I915_DPLL_STRIDE                8
#define I915_SDEIMR                     0xC4004
#define I915_SDEIIR                     0xC4008
#define I915_SDEIER                     0xC400C
#define I915_SHPD_FILTER_CNT            0xC4038
#define I915_DISPLAY_INT_CTL            0x44200
#define I915_GFX_MSTR_IRQ               0x190010
#define I915_GFX_FLSH_CNTL              0x101008
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
#define I915_GMBUS1_INDEXSHIFT          8
#define I915_GMBUS1_ADDRESSSHIFT        1
#define I915_GMBUS1_READ                0x01u
#define I915_GMBUS2_WAITPHASE           (0x01u << 14)
#define I915_GMBUS2_HWREADY             (0x01u << 11)
#define I915_GMBUS2_NAK                 (0x01u << 10)
#define I915_GMBUS2_ACTIVE              (0x01u << 9)
#define I915_FUSE_PG2                   (0x01u << 25)
#define I915_FUSE_PG3                   (0x01u << 24)
#define I915_PWR_WELL_PW2               (0x01u << 2)
#define I915_PWR_WELL_PW3               (0x01u << 4)
#define I915_PWR_WELL_DDIB              (0x01u << 2)
#define I915_PWR_WELL_REQUEST           1
#define I915_DBUF_REQUEST               (0x01u << 31)
#define I915_DBUF_STATE                 (0x01u << 30)
#define I915_DBUF_TRACKER               (0x1Fu << 19)
#define I915_DBUF_TRACKERVALUE          (0x08u << 19)
#define I915_DPLL_ON                    (0x01u << 31)
#define I915_DPLL_LOCK                  (0x01u << 30)
#define I915_DPLL_POWER                 (0x01u << 27)
#define I915_DPLL_POWERSTATE            (0x01u << 26)
#define I915_DPLL_FRACTIONSHIFT         10
#define I915_DPLL_QDIVSHIFT             10
#define I915_DPLL_QDIVMODE              (0x01u << 9)
#define I915_DPLL_KDIVSHIFT             6
#define I915_DPLL_PDIVSHIFT             2
#define I915_DSSM_REFCLK                (0x07u << 29)
#define I915_DSSM_REFCLK24              (0x00u << 29)
#define I915_DPCLKA_DDIA                0x03u
#define I915_DPCLKA_DDIASHIFT           0
#define I915_DPCLKA_DDIASHIFTB          2
#define I915_DPCLKA_DDIAOFF             (0x01u << 10)
#define I915_DPCLKA_DDIBOFF             (0x01u << 11)
#define I915_TRANS_CLK_SEL_PORTB        (0x02u << 28)
#define I915_COMP_INIT                  (0x01u << 31)
#define I915_SUS_CLOCK                  0x03u
#define I915_PWR_DOWN_LN                (0x0Fu << 4)
#define I915_COMMON_KEEPER              (0x01u << 26)
#define I915_TX_DW2_SWINGUPPER          (0x01u << 15)
#define I915_TX_DW2_SWINGLOWER          (0x07u << 11)
#define I915_TX_DW2_RCOMP               0xFFu
#define I915_TX_DW2_RCOMPVALUE          0x98u
#define I915_TX_DW4_LOADGEN             (0x01u << 31)
#define I915_TX_DW4_POST1SHIFT          12
#define I915_TX_DW4_POST2SHIFT          6
#define I915_TX_DW4_CURSORS             0x3FFFFu
#define I915_TX_DW5_TRAINING            (0x01u << 31)
#define I915_TX_DW5_TAP2                (0x01u << 30)
#define I915_TX_DW5_TAP3                (0x01u << 29)
#define I915_TX_DW5_SCALING             (0x07u << 18)
#define I915_TX_DW5_SCALINGVALUE        (0x02u << 18)
#define I915_TX_DW5_RTERM               (0x07u << 3)
#define I915_TX_DW5_RTERMVALUE          (0x06u << 3)
#define I915_TX_DW7_NSCALAR             (0x7Fu << 24)
#define I915_TX_DW7_NSCALARSHIFT        24
#define I915_DDI_FUNC_ENABLE            (0x01u << 31)
#define I915_DDI_FUNC_PORTB             (0x02u << 27)
#define I915_DDI_FUNC_DVI               (0x01u << 24)
#define I915_DDI_FUNC_VSYNCHIGH         (0x01u << 17)
#define I915_DDI_FUNC_HSYNCHIGH         (0x01u << 16)
#define I915_DTD_INTERLACED             0x80u
#define I915_DTD_DIGITALSEPARATE        0x18u
#define I915_DTD_VSYNCHIGH              0x04u
#define I915_DTD_HSYNCHIGH              0x02u
#define I915_DDI_BUF_ENABLE             (0x01u << 31)
#define I915_DDI_BUF_IDLE               (0x01u << 7)
#define I915_PIPEMISC_BPC               (0x07u << 5)
#define I915_TRANS_CONF_STATE           (0x01u << 30)
#define I915_PLANE_COLOR_GAMMADISABLE   (0x01u << 13)
#define I915_DISPLAY_INT_ENABLE         (0x01u << 31)
#define I915_DISPLAY_INT_PCH            (0x01u << 23)
#define I915_MSTR_ENABLE                (0x01u << 31)
#define I915_MSTR_DISPLAY               (0x01u << 16)
#define I915_PTE_PRESENT                0x01u
#define I915_PCI_INTXDISABLE            (0x01u << 10)
#define I915_TRANS_CONF_ENABLE          (0x01u << 31)
#define I915_PS_CTRL_ENABLE             (0x01u << 31)
#define I915_PLANE_CTL_ENABLE           (0x01u << 31)
#define I915_PLANE_CTL_FORMAT           (0x1Fu << 23)
#define I915_PLANE_CTL_FORMAT_XRGB8888  (0x08u << 23)
#define I915_PLANE_CTL_TILED            (0x07u << 10)
