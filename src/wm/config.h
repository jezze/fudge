#define CONFIG_UNIT                         4
#define CONFIG_CONTROL_SIZE                 ((render_unit(1) + 8) * 8 / 3)
#define CONFIG_FRAME_WIDTH                  3
#define CONFIG_FRAME_HEIGHT                 3
#define CONFIG_PANEL_BORDER_WIDTH           2
#define CONFIG_PANEL_BORDER_HEIGHT          2
#define CONFIG_SCROLL_WIDTH                 12
#define CONFIG_SCROLL_MINIMUM               24
#define CONFIG_BUTTON_PADDING_WIDTH         (CONFIG_FRAME_WIDTH + render_unit(5))
#define CONFIG_BUTTON_PADDING_HEIGHT        (CONFIG_FRAME_HEIGHT + render_unit(3))
#define CONFIG_CHECKBOX_WIDTH               CONFIG_CONTROL_SIZE
#define CONFIG_CHECKBOX_HEIGHT              CONFIG_CONTROL_SIZE
#define CONFIG_SELECT_PADDING_WIDTH         (CONFIG_FRAME_WIDTH + render_unit(5))
#define CONFIG_SELECT_ARROW_WIDTH           12
#define CONFIG_SELECT_PADDING_HEIGHT        (CONFIG_FRAME_HEIGHT + render_unit(3))
#define CONFIG_SWATCH_PADDING               (CONFIG_FRAME_HEIGHT + render_unit(3))
#define CONFIG_WINDOW_BUTTON_WIDTH          CONFIG_CONTROL_SIZE
#define CONFIG_WINDOW_BUTTON_HEIGHT         CONFIG_CONTROL_SIZE
