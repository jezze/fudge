#define I915_GMBUS0                     0x05100
#define I915_GMBUS1                     0x05104
#define I915_GMBUS2                     0x05108
#define I915_GMBUS3                     0x0510C
#define I915_GMBUS4                     0x05110
#define I915_GMBUS5                     0x05120

#define I915_VGA0                       0x06000
#define I915_VGA1                       0x06004
#define I915_VGA_PD                     0x06010

#define I915_DPLLA_CONTROL              0x06014
#define I915_DPLLB_CONTROL              0x06018
#define I915_DPLLA_MD                   0x0601C
#define I915_DPLLB_MD                   0x06020

#define I915_FPA0                       0x06040
#define I915_FPA1                       0x06044
#define I915_FPB0                       0x06048
#define I915_FPB1                       0x0604C

#define I915_DPLL_TEST                  0x0606C

#define I915_DISPLAYA_HTOTAL            0x60000
#define I915_DISPLAYA_HBLANK            0x60004
#define I915_DISPLAYA_HSYNC             0x60008
#define I915_DISPLAYA_VTOTAL            0x6000C
#define I915_DISPLAYA_VBLANK            0x60010
#define I915_DISPLAYA_VSYNC             0x60014
#define I915_DISPLAYA_PIPESRC           0x6001C
#define I915_DISPLAYB_HTOTAL            0x61000
#define I915_DISPLAYB_HBLANK            0x61004
#define I915_DISPLAYB_HSYNC             0x61008
#define I915_DISPLAYB_VTOTAL            0x6100C
#define I915_DISPLAYB_VBLANK            0x61010
#define I915_DISPLAYB_VSYNC             0x61014
#define I915_DISPLAYB_PIPESRC           0x6101C

#define I915_ADPA_CONTROL               0x61100

#define I915_HDMIB_CONTROL              0x61140

#define I915_DP_TEST1                   0x61150
#define I915_DP_TEST2                   0x61154

#define I915_HDMIC_CONTROL              0x61160

#define I915_LVDS_PORT_CONTROL          0x61180
#define I915_LVDS_POWER_STATUS          0x61200
#define I915_LVDS_POWER_CONTROL         0x61204

#define I915_PIPEA_DSL                  0x70000
#define I915_PIPEA_CONFIG               0x70008
#define I915_PIPEA_STATUS               0x70024

#define I915_DISPLAYA_CONTROL           0x70180
#define I915_DISPLAYA_ADDRESS           0x70184
#define I915_DISPLAYA_SIZE              0x70190
#define I915_DISPLAYA_SURF              0x7019C

#define I915_PIPEB_DSL                  0x71000
#define I915_PIPEB_CONFIG               0x71008
#define I915_PIPEB_STATUS               0x71024

#define I915_DISPLAYB_CONTROL           0x71180
#define I915_DISPLAYB_ADDRESS           0x71184
#define I915_DISPLAYB_SIZE              0x71190
#define I915_DISPLAYB_SURF              0x7119C

#define I915_VGA_CONTROL                0x71400

#define I915_DPLL_CONTROL_VGA_DIV1      (0x01u << 16)
#define I915_DPLL_CONTROL_VGA_DIV2      (0x02u << 16)
#define I915_DPLL_CONTROL_VGA_DIV3      (0x04u << 16)
#define I915_DPLL_CONTROL_VGA_DIV4      (0x08u << 16)
#define I915_DPLL_CONTROL_VGA_DIV5      (0x10u << 16)
#define I915_DPLL_CONTROL_VGA_DIV6      (0x20u << 16)
#define I915_DPLL_CONTROL_VGA_DIV7      (0x30u << 16)
#define I915_DPLL_CONTROL_VGA_DIV8      (0x40u << 16)
#define I915_DPLL_CONTROL_MODE0_DIV5    (0x01u << 24)
#define I915_DPLL_CONTROL_MODE0_DIV10   (0x02u << 24)
#define I915_DPLL_CONTROL_MODE1_DIV7    (0x01u << 24)
#define I915_DPLL_CONTROL_MODE1_DIV14   (0x02u << 24)
#define I915_DPLL_CONTROL_MODE0         (0x01u << 26)
#define I915_DPLL_CONTROL_MODE1         (0x01u << 27)
#define I915_DPLL_CONTROL_DISABLE_VGA   (0x01u << 28)
#define I915_DPLL_CONTROL_ENABLE_VCO    (0x01u << 31)

#define I915_DISPLAY_CONTROL_8BPP       (0x02u << 26)
#define I915_DISPLAY_CONTROL_16BPP      (0x05u << 26)
#define I915_DISPLAY_CONTROL_32BPP      (0x07u << 26)
#define I915_DISPLAY_CONTROL_ENGAMMA    (0x01u << 30)
#define I915_DISPLAY_CONTROL_ENPLANE    (0x01u << 31)

#define I915_PIPE_CONFIG_STATE          (0x01u << 30)
#define I915_PIPE_CONFIG_ENABLE         (0x01u << 31)

#define I915_PIPE_STATUS_VBLANK         (0x01u << 1)
#define I915_PIPE_STATUS_VSYNC          (0x01u << 9)

#define I915_LVDS_CONTROL_ENABLE        (0x01u << 31)
#define I915_LVDS_CONTROL_PORT          (0x01u << 30)

#define I915_VGA_CONTROL_DISABLE        (0x01u << 31)
#define I915_VGA_CONTROL_PIPE           (0x01u << 29)
