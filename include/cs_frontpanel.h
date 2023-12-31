#ifndef __CS_FRONTPANEL_H__
#define __CS_FRONTPANEL_H__

#define VFDDISPLAYCHARS      0xc0425a00
#define VFDWRITECGRAM        0x40425a01
#define VFDBRIGHTNESS        0xc0425a03
#define VFDPWRLED            0xc0425a04
#define VFDDRIVERINIT        0xc0425a08
#define VFDICONDISPLAYONOFF  0xc0425a0a
#define VFDDISPLAYWRITEONOFF 0xc0425a05

#define VFDCLEARICONS        0xc0425af6
#define VFDSETRF             0xc0425af7
#define VFDSETFAN            0xc0425af8
#define VFDGETWAKEUPMODE     0xc0425af9
#define VFDGETTIME           0xc0425afa
#define VFDSETTIME           0xc0425afb
#define VFDSTANDBY           0xc0425afc
#define VFDREBOOT            0xc0425afd
#define VFDSETLED            0xc0425afe
#define VFDSETMODE           0xc0425aff

#define VFDGETWAKEUPTIME     0xc0425b00
#define VFDGETVERSION        0xc0425b01
#define VFDSETDISPLAYTIME    0xc0425b02
#define VFDSETTIMEMODE       0xc0425b03
#define VFDDISPLAYCLR        0xc0425b00

typedef enum
{
#if defined(BOXMODEL_OCTAGON1008)
	ICON_DOLBY = 0x10,
	ICON_DTS,
	ICON_VIDEO,
	ICON_AUDIO,
	ICON_LINK,
	ICON_HDD,
	ICON_DISC,
	ICON_DVB,
	ICON_DVD,
	ICON_TIME,
	ICON_TIMER,
	ICON_CARD,
	ICON_1,
	ICON_2,
	ICON_KEY,
	ICON_16_9,
	ICON_USB,
	ICON_CRYPTED,
	ICON_PLAY,
	ICON_REWIND,
	ICON_PAUSE,
	ICON_FF,
	ICON_REC,
	ICON_ARROW,
	ICON_COLON1,
	ICON_COLON2,
	ICON_COLON3,
	ICON_MAX,
	FP_ICON_USB = ICON_USB,
	FP_ICON_HD = ICON_16_9,
	FP_ICON_HDD = ICON_HDD,
	FP_ICON_LOCK = ICON_CRYPTED,
	FP_ICON_BT,
	FP_ICON_MP3,
	FP_ICON_MUSIC,
	FP_ICON_DD = ICON_DOLBY,
	FP_ICON_MAIL,
	FP_ICON_MUTE = ICON_TIME,
	FP_ICON_PLAY = ICON_PLAY,
	FP_ICON_PAUSE = ICON_PAUSE,
	FP_ICON_FF = ICON_FF,
	FP_ICON_FR = ICON_REWIND,
	FP_ICON_REC = ICON_REC,
	FP_ICON_CLOCK = ICON_TIMER,
	FP_ICON_RADIO = ICON_AUDIO,
	FP_ICON_TV = ICON_VIDEO,
	FP_ICON_DOWNLOAD = ICON_LINK,
	FP_ICON_CAM1 = ICON_REC,
#elif defined(BOXMODEL_FORTIS_HDBOX)
	ICON_USB = 0x10,
	ICON_STANDBY,
	ICON_SAT,
	ICON_REC,
	ICON_TIMESHIFT,
	ICON_TIMER,
	ICON_HD,
	ICON_SCRAMBLED,
	ICON_DOLBY,
	ICON_MUTE,
	ICON_TUNER1,
	ICON_TUNER2,
	ICON_MP3,
	ICON_REPEAT,
	ICON_Play,
	ICON_TER,
	ICON_FILE,
	ICON_480i,
	ICON_480p,
	ICON_576i,
	ICON_576p,
	ICON_720p,
	ICON_1080i,
	ICON_1080p,
	ICON_Circ0,
	ICON_Circ1,
	ICON_Circ2,
	ICON_Circ3,
	ICON_Circ4,
	ICON_Circ5,
	ICON_Circ6,
	ICON_Circ7,
	ICON_Circ8,
	ICON_COLON1,
	ICON_COLON2,
	ICON_COLON3,
	ICON_COLON4,
	ICON_TV,
	ICON_RADIO,
	ICON_MAX,
	FP_ICON_USB = ICON_USB,
	FP_ICON_REC = ICON_REC,
	FP_ICON_CAM1 = ICON_REC,
	FP_ICON_TIMESHIFT = ICON_TIMESHIFT,
	FP_ICON_CLOCK = ICON_TIMER,
	FP_ICON_HD = ICON_HD,
	FP_ICON_LOCK = ICON_SCRAMBLED,
	FP_ICON_DD = ICON_DOLBY,
	FP_ICON_MUTE = ICON_MUTE,
	FP_ICON_BT = 0,
	FP_ICON_MP3 = ICON_MP3,
	FP_ICON_PLAY = ICON_Play,
	FP_ICON_PAUSE = 0,
	FP_ICON_HDD = 0,
	FP_ICON_MAIL = 0,
	FP_ICON_FF = 0,
	FP_ICON_FR = 0,
	FP_ICON_RADIO = ICON_RADIO,
	FP_ICON_TV = ICON_TV,
	FP_ICON_MUSIC = ICON_RADIO,
	FP_ICON_MAX = ICON_MAX
#elif defined(BOXMODEL_UFS910) || defined(BOXMODEL_UFS922) || defined(BOXMODEL_UFS912) || defined(BOXMODEL_UFS913)
	FP_ICON_USB = 0x10,
	FP_ICON_HD,
	FP_ICON_HDD,
	FP_ICON_LOCK,
	FP_ICON_BT,
	FP_ICON_MP3,
	FP_ICON_MUSIC,
	FP_ICON_DD,
	FP_ICON_MAIL,
	FP_ICON_MUTE,
	FP_ICON_PLAY,
	FP_ICON_PAUSE,
	FP_ICON_FF,
	FP_ICON_FR,
	FP_ICON_REC,
	FP_ICON_CLOCK,
	FP_ICON_MAX,
	FP_ICON_CAM1 = FP_ICON_REC,
	FP_ICON_TV = 0,
	FP_ICON_RADIO = FP_ICON_MUSIC,
	FP_ICON_RECORD = FP_ICON_REC,
	FP_ICON_DOWNLOAD = 0,
	FP_ICON_TIMESHIFT = 0
#elif defined(BOXMODEL_IPBOX9900) || defined(BOXMODEL_IPBOX99)
	ICON_STANDBY,
	ICON_SAT,
	ICON_REC,
	ICON_TIMESHIFT,
	ICON_TIMER,
	ICON_HD,
	ICON_SCRAMBLED,
	ICON_MUTE,
	ICON_TUNER1,
	ICON_TUNER2,
	ICON_MP3,
	ICON_REPEAT,
	ICON_PLAY,
	ICON_PAUSE,
	ICON_TER,
	ICON_FILE,
	ICON_480I,
	ICON_480P,
	ICON_576I,
	ICON_576P,
	ICON_720P,
	ICON_1080I,
	ICON_1080P,
	ICON_TV,
	ICON_RADIO,
	ICON_MAX,
	FP_ICON_480P       = ICON_480P,
	FP_ICON_480I       = ICON_480I,
	FP_ICON_576P       = ICON_576P,
	FP_ICON_576I       = ICON_576I,
	FP_ICON_PLAY       = ICON_PLAY,
	FP_ICON_PAUSE      = ICON_PAUSE,
	FP_ICON_1080P      = 0x0A,
	FP_ICON_1080I      = 0x09,
	FP_ICON_720P       = 0x08,
	FP_ICON_POWER      = 0x00,
	FP_ICON_HD         = 0x05,
	FP_ICON_RADIO      = 0x0D,
	FP_ICON_MP3        = FP_ICON_RADIO,
	FP_ICON_MUSIC      = FP_ICON_RADIO,
	FP_ICON_DD         = 0x04,
	FP_ICON_REC        = 0x03,
	FP_ICON_FF = 0,
	FP_ICON_FR = 0,
	FP_ICON_USB = 0,
	FP_ICON_HDD = 0,
	FP_ICON_MUTE = 0,
	FP_ICON_CLOCK = 0,
	FP_ICON_CAM1 = 0,
	FP_ICON_LOCK = 0,
	FP_ICON_DOWNLOAD = 0,
	FP_ICON_TIMESHIFT  = FP_ICON_REC,
	FP_ICON_TV = 0,
	FP_ICON_MAIL = 0,
	FP_ICON_BT = 0,

#else
	FP_ICON_USB = 0x10,
	FP_ICON_HD,
	FP_ICON_HDD,
	FP_ICON_LOCK,
	FP_ICON_BT,
	FP_ICON_MP3,
	FP_ICON_MUSIC,
	FP_ICON_DD,
	FP_ICON_MAIL,
	FP_ICON_MUTE,
	FP_ICON_PLAY,
	FP_ICON_PAUSE,
	FP_ICON_FF,
	FP_ICON_FR,
	FP_ICON_REC,
	FP_ICON_CLOCK,
	FP_ICON_CAM1 = FP_ICON_REC,
	FP_ICON_TV = FP_ICON_MUSIC,
	FP_ICON_RADIO = FP_ICON_MUSIC,
	FP_ICON_RECORD = FP_ICON_REC,
	FP_ICON_DOWNLOAD,
	FP_ICON_TIMESHIFT,
#endif
#if !defined(BOXMODEL_UFS910) && !defined(BOXMODEL_UFS922) && !defined(BOXMODEL_UFS912) && !defined(BOXMODEL_UFS913) && !defined(BOXMODEL_FORTIS_HDBOX)
	FP_ICON_MAX
#endif
} fp_icon;

typedef enum
{
	FP_FLAG_NONE		= 0x00,
	FP_FLAG_SCROLL_ON	= 0x01, /* switch scrolling on */
	FP_FLAG_SCROLL_LTR	= 0x02, /* scroll from left to rightinstead of default right to left direction (i.e. for arabic text) */
	FP_FLAG_SCROLL_SIO	= 0x04, /* start/stop scrolling with empty screen (scroll in/out) */
	FP_FLAG_SCROLL_DELAY	= 0x08, /* delayed scroll start */
	FP_FLAG_ALIGN_LEFT	= 0x10, /* align the text in display from the left (default) */
	FP_FLAG_ALIGN_RIGHT	= 0x20, /* align the text in display from the right (arabic) */
	FP_FLAG_UPDATE_SCROLL_POS = 0x40, /* update the current position for scrolling */
} fp_flag;

typedef struct
{
	unsigned char brightness;
	unsigned char flags;
	unsigned char current_hour;
	unsigned char current_minute;
	unsigned char timer_minutes_hi;
	unsigned char timer_minutes_lo;
} fp_standby_data_t;

typedef enum
{
	FP_LED_1_ON	= 0x81,
	FP_LED_2_ON	= 0x82,
	FP_LED_3_ON	= 0x83,
	FP_LED_1_OFF	= 0x01,
	FP_LED_2_OFF	= 0x02,
	FP_LED_3_OFF	= 0x03
} fp_led_ctrl_t;

typedef struct
{
	unsigned char source;
	unsigned char time_minutes_hi;
	unsigned char time_minutes_lo;
} fp_wakeup_data_t;

typedef enum
{
	FP_WAKEUP_SOURCE_TIMER  = 0x01,
	FP_WAKEUP_SOURCE_BUTTON = 0x02,
	FP_WAKEUP_SOURCE_REMOTE = 0x04,
	FP_WAKEUP_SOURCE_PWLOST = 0x7F,
	FP_WAKEUP_SOURCE_POWER  = 0xFF
} fp_wakeup_source;

typedef struct
{
	unsigned short addr;
	unsigned short cmd;
} fp_standby_cmd_data_t;

typedef enum
{
	FP_DISPLAY_TEXT_NONE = 0,
	FP_DISPLAY_TEXT_LIMITED,
	FP_DISPLAY_TEXT_ALL
} fp_display_text_type_t;

typedef enum
{
	FP_DISPLAY_TYPE_NONE = 0,
	FP_DISPLAY_TYPE_VFD,
	FP_DISPLAY_TYPE_OLED,
	FP_DISPLAY_TYPE_LED_SEGMENT
} fp_display_type_t;

typedef struct
{
	fp_display_type_t display_type;
	unsigned short xres, yres;
	unsigned int segment_count;
	fp_display_text_type_t text_support;
	bool number_support;
} fp_display_caps_t;

#endif // __CS_FRONTPANEL_H__
