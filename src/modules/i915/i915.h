#define I915_VENDOR                     0x8086
#define I915_DEVICES                    11
#define I915_MMIOSIZE                   0x80000
#define I915_MINSIZE                    64
#define I915_TIMEOUT                    100000
#define I915_EVASION                    16

#define I915_TRANS_HTOTAL_A             0x60000
#define I915_TRANS_VTOTAL_A             0x6000C
#define I915_PIPESRC_A                  0x6001C
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
#define I915_TRANS_CONF_ENABLE          (0x01u << 31)
#define I915_PS_CTRL_ENABLE             (0x01u << 31)
#define I915_PLANE_CTL_ENABLE           (0x01u << 31)
#define I915_PLANE_CTL_FORMAT           (0x1Fu << 23)
#define I915_PLANE_CTL_FORMAT_XRGB8888  (0x08u << 23)
#define I915_PLANE_CTL_TILED            (0x07u << 10)
