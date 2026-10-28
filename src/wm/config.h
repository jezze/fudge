#define CONFIG_UNIT                         4
#define CONFIG_CONTROL_SIZE                 ((render_unit(1) + 8) * 8 / 3)
#define CONFIG_FRAME_WIDTH                  3
#define CONFIG_FRAME_HEIGHT                 3
#define CONFIG_MEDIA_MISSING                0xFF202020
#define CONFIG_PANEL_BORDER_WIDTH           2
#define CONFIG_PANEL_BORDER_HEIGHT          2
#define CONFIG_SCROLL_WIDTH                 12
#define CONFIG_SCROLL_MINIMUM               24
#define CONFIG_CHECKBOX_WIDTH               CONFIG_CONTROL_SIZE
#define CONFIG_CHECKBOX_HEIGHT              CONFIG_CONTROL_SIZE
#define CONFIG_SELECT_ARROW_WIDTH           12
#define CONFIG_SLIDER_WIDTH                 (CONFIG_CONTROL_SIZE * 6)
#define CONFIG_SLIDER_TRACK_HEIGHT          (CONFIG_FRAME_HEIGHT * 2 + 2)
#define CONFIG_SLIDER_KNOB_WIDTH            CONFIG_SCROLL_WIDTH
#define CONFIG_WINDOW_BUTTON_WIDTH          CONFIG_CONTROL_SIZE
#define CONFIG_WINDOW_BUTTON_HEIGHT         CONFIG_CONTROL_SIZE
