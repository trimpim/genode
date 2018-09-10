/*
 * \brief  Emulation of the Linux kernel API used by DRM
 * \author Pirmin Duss
 * \date   2018-11-13
 *
 * The content of this file, in particular data structures, is partially
 * derived from Linux-internal headers.
 */

#ifndef _LX_EMUL_AMDGPU_H_
#define _LX_EMUL_AMDGPU_H_

#include "lx_emul_common.h"



/***********************
 ** linux/dmaengine.h **
 ***********************/

typedef s32 dma_cookie_t;

enum dma_slave_buswidth {
	DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
	DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
	DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
	DMA_SLAVE_BUSWIDTH_3_BYTES = 3,
	DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
	DMA_SLAVE_BUSWIDTH_8_BYTES = 8,
	DMA_SLAVE_BUSWIDTH_16_BYTES = 16,
	DMA_SLAVE_BUSWIDTH_32_BYTES = 32,
	DMA_SLAVE_BUSWIDTH_64_BYTES = 64,
};

struct dma_chan {
	struct dma_device *device;
	dma_cookie_t cookie;
	dma_cookie_t completed_cookie;

	/* sysfs */
	int chan_id;
	struct dma_chan_dev *dev;

	struct list_head device_node;
	struct dma_chan_percpu __percpu *local;
	int client_count;
	int table_count;

	/* DMA router */
	struct dma_router *router;
	void *route_data;

	void *private;
};


/***********************
 ** linux/pm_domain.h **
 ***********************/

enum gpd_status {
	GPD_STATE_ACTIVE = 0,	/* PM domain is active */
	GPD_STATE_POWER_OFF,	/* PM domain is off */
};

struct gpd_dev_ops {
	int (*start)(struct device *dev);
	int (*stop)(struct device *dev);
};

struct generic_pm_domain {
	struct dev_pm_domain domain;	/* PM domain operations */
	struct list_head gpd_list_node;	/* Node in the global PM domains list */
	struct list_head master_links;	/* Links with PM domain as a master */
	struct list_head slave_links;	/* Links with PM domain as a slave */
	struct list_head dev_list;	/* List of devices */
	struct dev_power_governor *gov;
	struct work_struct power_off_work;
	struct fwnode_handle *provider;	/* Identity of the domain provider */
	bool has_provider;
	const char *name;
	atomic_t sd_count;	/* Number of subdomains with power "on" */
	enum gpd_status status;	/* Current state of the domain */
	unsigned int device_count;	/* Number of devices */
	unsigned int suspended_count;	/* System suspend device counter */
	unsigned int prepared_count;	/* Suspend counter of prepared devices */
	unsigned int performance_state;	/* Aggregated max performance state */
	int (*power_off)(struct generic_pm_domain *domain);
	int (*power_on)(struct generic_pm_domain *domain);
	int (*set_performance_state)(struct generic_pm_domain *genpd,
				     unsigned int state);
	struct gpd_dev_ops dev_ops;
	s64 max_off_time_ns;	/* Maximum allowed "suspended" time. */
	bool max_off_time_changed;
	bool cached_power_down_ok;
	int (*attach_dev)(struct generic_pm_domain *domain,
			  struct device *dev);
	void (*detach_dev)(struct generic_pm_domain *domain,
			   struct device *dev);
	unsigned int flags;		/* Bit field of configs for genpd */
	struct genpd_power_state *states;
	unsigned int state_count; /* number of states */
	unsigned int state_idx; /* state that genpd will go to when off */
	void *free; /* Free the state that was allocated for default */
	ktime_t on_time;
	ktime_t accounting_time;
	const struct genpd_lock_ops *lock_ops;
	union {
		struct mutex mlock;
		struct {
			spinlock_t slock;
			unsigned long lock_flags;
		};
	};

};

static inline struct generic_pm_domain *pd_to_genpd(struct dev_pm_domain *pd)
{
	return container_of(pd, struct generic_pm_domain, domain);
}


/**********************
 ** linux/notifier.h **
 **********************/

#define NOTIFY_DONE 0x0000


/*************************
 ** linux/reservation.h **
 *************************/

int reservation_object_lock_interruptible(struct reservation_object *j,
                                          struct ww_acquire_ctx *);


/***********************
 ** linux/semaphore.h **
 ***********************/

struct semaphore {
	raw_spinlock_t	lock;
	unsigned int	count;
	struct list_head	wait_list;
};


/************************************************************
 ** drivers/gpu/drm/amd/display/modules/inc/mod_freesync.h **
 ************************************************************/

struct mod_freesync_caps {
	bool supported;
	unsigned int min_refresh_in_micro_hz;
	unsigned int max_refresh_in_micro_hz;

	bool btr_supported;
};


/***************************************
 ** include/asm-generic/atomic-long.h **
 ***************************************/

#define ATOMIC_LONG_PFX(x)	atomic64 ## x

#define ATOMIC_LONG_SET_OP(mo) \
static inline void atomic_long_set##mo(atomic_long_t *l, long i) \
{ \
	ATOMIC_LONG_PFX(_t) *v = (ATOMIC_LONG_PFX(_t) *)l; \
	 \
	ATOMIC_LONG_PFX(_set##mo)(v, i); \
}

ATOMIC_LONG_SET_OP()

#define atomic_long_cmpxchg(l, old, new) \
	(ATOMIC_LONG_PFX(_cmpxchg)((ATOMIC_LONG_PFX(_t) *)(l), (old), (new)))

#define atomic_long_xchg(v, new) \
	(ATOMIC_LONG_PFX(_xchg)((ATOMIC_LONG_PFX(_t) *)(v), (new)))


/*******************
 ** linux/chash.h **
 *******************/

#if BITS_PER_LONG == 32
# define _CHASH_LONG_SHIFT 5
#elif BITS_PER_LONG == 64
# define _CHASH_LONG_SHIFT 6
#else
# error "Unexpected BITS_PER_LONG"
#endif

struct __chash_table {
	u8 bits;
	u8 key_size;
	unsigned int value_size;
	u32 size_mask;
	unsigned long *occup_bitmap, *valid_bitmap;
	union {
		u32 *keys32;
		u64 *keys64;
	};
	u8 *values;
};

#define __CHASH_BITMAP_SIZE(bits)				\
	(((1 << (bits)) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define __CHASH_ARRAY_SIZE(bits, size)				\
	((((size) << (bits)) + sizeof(long) - 1) / sizeof(long))

#define __CHASH_DATA_SIZE(bits, key_size, value_size)	\
	(__CHASH_BITMAP_SIZE(bits) * 2 +		\
	 __CHASH_ARRAY_SIZE(bits, key_size) +		\
	 __CHASH_ARRAY_SIZE(bits, value_size))

#define STRUCT_CHASH_TABLE(bits, key_size, value_size)			\
	struct {							\
		struct __chash_table table;				\
		unsigned long data					\
			[__CHASH_DATA_SIZE(bits, key_size, value_size)];\
	}

#define DECLARE_CHASH_TABLE(table, bts, key_sz, val_sz)		\
	STRUCT_CHASH_TABLE(bts, key_sz, val_sz) table


/*******************
 ** linux/kfifo.h **
 *******************/

struct __kfifo {
	unsigned int	in;
	unsigned int	out;
	unsigned int	mask;
	unsigned int	esize;
	void			*data;
};

#define __STRUCT_KFIFO_COMMON(datatype, recsize, ptrtype) \
	union { \
		struct __kfifo	kfifo; \
		datatype		*type; \
		const datatype	*const_type; \
		char			(*rectype)[recsize]; \
		ptrtype			*ptr; \
		ptrtype const	*ptr_const; \
	}

#define __STRUCT_KFIFO(type, size, recsize, ptrtype) \
{ \
	__STRUCT_KFIFO_COMMON(type, recsize, ptrtype); \
	type		buf[((size < 2) || (size & (size - 1))) ? -1 : size]; \
}

#define STRUCT_KFIFO(type, size) \
	struct __STRUCT_KFIFO(type, size, 0, type)

#define DECLARE_KFIFO(fifo, type, size)	STRUCT_KFIFO(type, size) fifo


/***********************************
 ** x86/include/asm/cpufeatures.h **
 ***********************************/

#define X86_FEATURE_HYPERVISOR		( 4*32+31) /* Running on a hypervisor */


/****************************************
 ** include/drm/ttm/ttm_execbuf_util.h **
 ****************************************/

struct ttm_validate_buffer {
	struct list_head head;
	struct ttm_buffer_object *bo;
	bool shared;
};


/******************
 ** linux/hdmi.h **
 ******************/

enum hdmi_picture_aspect {
	HDMI_PICTURE_ASPECT_NONE,
	HDMI_PICTURE_ASPECT_4_3,
	HDMI_PICTURE_ASPECT_16_9,
	HDMI_PICTURE_ASPECT_64_27,
	HDMI_PICTURE_ASPECT_256_135,
	HDMI_PICTURE_ASPECT_RESERVED,
};


/****************************
 ** sound/designware_i2s.h **
 ****************************/

struct i2s_clk_config_data {
	int chan_nr;
	u32 data_width;
	u32 sample_rate;
};

struct i2s_platform_data {
	#define DWC_I2S_PLAY	(1 << 0)
	#define DWC_I2S_RECORD	(1 << 1)
	#define DW_I2S_SLAVE	(1 << 2)
	#define DW_I2S_MASTER	(1 << 3)
	unsigned int cap;
	int channel;
	u32 snd_fmts;
	u32 snd_rates;

	#define DW_I2S_QUIRK_COMP_REG_OFFSET	(1 << 0)
	#define DW_I2S_QUIRK_COMP_PARAM1	(1 << 1)
	#define DW_I2S_QUIRK_16BIT_IDX_OVERRIDE (1 << 2)
	unsigned int quirks;
	unsigned int i2s_reg_comp1;
	unsigned int i2s_reg_comp2;

	void *play_dma_data;
	void *capture_dma_data;
	bool (*filter)(struct dma_chan *chan, void *slave);
	int (*i2s_clk_cfg)(struct i2s_clk_config_data *config);
};

struct i2s_dma_data {
	void *data;
	dma_addr_t addr;
	u32 max_burst;
	enum dma_slave_buswidth addr_width;
	bool (*filter)(struct dma_chan *chan, void *slave);
};

/* I2S DMA registers */
#define I2S_RXDMA		0x01C0
#define I2S_TXDMA		0x01C8

#define TWO_CHANNEL_SUPPORT		2	/* up to 2.0 */
#define FOUR_CHANNEL_SUPPORT	4	/* up to 3.1 */
#define SIX_CHANNEL_SUPPORT		6	/* up to 5.1 */
#define EIGHT_CHANNEL_SUPPORT	8	/* up to 7.1 */


/*****************
 ** sound/pcm.h **
 *****************/

#define SNDRV_PCM_RATE_8000_96000	(SNDRV_PCM_RATE_8000_48000|SNDRV_PCM_RATE_64000|\
		SNDRV_PCM_RATE_88200|SNDRV_PCM_RATE_96000)



/*********************************************
 ** gpu/drm/amd/include/kgd_kfd_interface.h **
 *********************************************/

#define KGD_MAX_QUEUES 128


/********************************************
 ** gpu/drm/amd/include/kgd_pp_interface.h **
 ********************************************/

struct amd_vce_state {
	/* vce clocks */
	u32 evclk;
	u32 ecclk;
	/* gpu clocks */
	u32 sclk;
	u32 mclk;
	u8 clk_idx;
	u8 pstate;
};

enum amd_dpm_forced_level {
	AMD_DPM_FORCED_LEVEL_AUTO = 0x1,
	AMD_DPM_FORCED_LEVEL_MANUAL = 0x2,
	AMD_DPM_FORCED_LEVEL_LOW = 0x4,
	AMD_DPM_FORCED_LEVEL_HIGH = 0x8,
	AMD_DPM_FORCED_LEVEL_PROFILE_STANDARD = 0x10,
	AMD_DPM_FORCED_LEVEL_PROFILE_MIN_SCLK = 0x20,
	AMD_DPM_FORCED_LEVEL_PROFILE_MIN_MCLK = 0x40,
	AMD_DPM_FORCED_LEVEL_PROFILE_PEAK = 0x80,
	AMD_DPM_FORCED_LEVEL_PROFILE_EXIT = 0x100,
};

enum amd_pm_state_type {
	/* not used for dpm */
	POWER_STATE_TYPE_DEFAULT,
	POWER_STATE_TYPE_POWERSAVE,
	/* user selectable states */
	POWER_STATE_TYPE_BATTERY,
	POWER_STATE_TYPE_BALANCED,
	POWER_STATE_TYPE_PERFORMANCE,
	/* internal states */
	POWER_STATE_TYPE_INTERNAL_UVD,
	POWER_STATE_TYPE_INTERNAL_UVD_SD,
	POWER_STATE_TYPE_INTERNAL_UVD_HD,
	POWER_STATE_TYPE_INTERNAL_UVD_HD2,
	POWER_STATE_TYPE_INTERNAL_UVD_MVC,
	POWER_STATE_TYPE_INTERNAL_BOOT,
	POWER_STATE_TYPE_INTERNAL_THERMAL,
	POWER_STATE_TYPE_INTERNAL_ACPI,
	POWER_STATE_TYPE_INTERNAL_ULV,
	POWER_STATE_TYPE_INTERNAL_3DPERF,
};

#define AMD_MAX_VCE_LEVELS 6

enum amd_vce_level {
	AMD_VCE_LEVEL_AC_ALL = 0,     /* AC, All cases */
	AMD_VCE_LEVEL_DC_EE = 1,      /* DC, entropy encoding */
	AMD_VCE_LEVEL_DC_LL_LOW = 2,  /* DC, low latency queue, res <= 720 */
	AMD_VCE_LEVEL_DC_LL_HIGH = 3, /* DC, low latency queue, 1080 >= res > 720 */
	AMD_VCE_LEVEL_DC_GP_LOW = 4,  /* DC, general purpose queue, res <= 720 */
	AMD_VCE_LEVEL_DC_GP_HIGH = 5, /* DC, general purpose queue, 1080 >= res > 720 */
};


/*******************************************
 ** gpu/drm/amd/include/dm_pp_interface.h **
 *******************************************/

enum amd_pp_display_config_type{
	AMD_PP_DisplayConfigType_None = 0,
	AMD_PP_DisplayConfigType_DP54 ,
	AMD_PP_DisplayConfigType_DP432 ,
	AMD_PP_DisplayConfigType_DP324 ,
	AMD_PP_DisplayConfigType_DP27,
	AMD_PP_DisplayConfigType_DP243,
	AMD_PP_DisplayConfigType_DP216,
	AMD_PP_DisplayConfigType_DP162,
	AMD_PP_DisplayConfigType_HDMI6G ,
	AMD_PP_DisplayConfigType_HDMI297 ,
	AMD_PP_DisplayConfigType_HDMI162,
	AMD_PP_DisplayConfigType_LVDS,
	AMD_PP_DisplayConfigType_DVI,
	AMD_PP_DisplayConfigType_WIRELESS,
	AMD_PP_DisplayConfigType_VGA
};

struct single_display_configuration
{
	uint32_t controller_index;
	uint32_t controller_id;
	uint32_t signal_type;
	uint32_t display_state;
	/* phy id for the primary internal transmitter */
	uint8_t primary_transmitter_phyi_d;
	/* bitmap with the active lanes */
	uint8_t primary_transmitter_active_lanemap;
	/* phy id for the secondary internal transmitter (for dual-link dvi) */
	uint8_t secondary_transmitter_phy_id;
	/* bitmap with the active lanes */
	uint8_t secondary_transmitter_active_lanemap;
	/* misc phy settings for SMU. */
	uint32_t config_flags;
	uint32_t display_type;
	uint32_t view_resolution_cx;
	uint32_t view_resolution_cy;
	enum amd_pp_display_config_type displayconfigtype;
	uint32_t vertical_refresh; /* for active display */
};

#define MAX_NUM_DISPLAY 32

struct amd_pp_display_configuration {
	bool nb_pstate_switch_disable;/* controls NB PState switch */
	bool cpu_cc6_disable; /* controls CPU CState switch ( on or off) */
	bool cpu_pstate_disable;
	uint32_t cpu_pstate_separation_time;

	uint32_t num_display;  /* total number of display*/
	uint32_t num_path_including_non_display;
	uint32_t crossfire_display_index;
	uint32_t min_mem_set_clock;
	uint32_t min_core_set_clock;
	/* unit 10KHz x bit*/
	uint32_t min_bus_bandwidth;
	/* minimum required stutter sclk, in 10khz uint32_t ulMinCoreSetClk;*/
	uint32_t min_core_set_clock_in_sr;

	struct single_display_configuration displays[MAX_NUM_DISPLAY];

	uint32_t vrefresh; /* for active display*/

	uint32_t min_vblank_time; /* for active display*/
	bool multi_monitor_in_sync;
	/* Controller Index of primary display - used in MCLK SMC switching hang
	 * SW Workaround*/
	uint32_t crtc_index;
	/* htotal*1000/pixelclk - used in MCLK SMC switching hang SW Workaround*/
	uint32_t line_time_in_us;
	bool invalid_vblank_time;

	uint32_t display_clk;
	/*
	 * for given display configuration if multimonitormnsync == false then
	 * Memory clock DPMS with this latency or below is allowed, DPMS with
	 * higher latency not allowed.
	 */
	uint32_t dce_tolerable_mclk_in_active_latency;
	uint32_t min_dcef_set_clk;
	uint32_t min_dcef_deep_sleep_set_clk;
};


/**********************************************
 ** drivers/gpu/drm/amd/include/amd_shared.h **
 **********************************************/

enum amd_ip_block_type {
	AMD_IP_BLOCK_TYPE_COMMON,
	AMD_IP_BLOCK_TYPE_GMC,
	AMD_IP_BLOCK_TYPE_IH,
	AMD_IP_BLOCK_TYPE_SMC,
	AMD_IP_BLOCK_TYPE_PSP,
	AMD_IP_BLOCK_TYPE_DCE,
	AMD_IP_BLOCK_TYPE_GFX,
	AMD_IP_BLOCK_TYPE_SDMA,
	AMD_IP_BLOCK_TYPE_UVD,
	AMD_IP_BLOCK_TYPE_VCE,
	AMD_IP_BLOCK_TYPE_ACP,
	AMD_IP_BLOCK_TYPE_VCN
};


/*************************
 ** drm/amd_asic_type.h **
 *************************/

enum amd_asic_type {
	CHIP_TAHITI = 0,
	CHIP_PITCAIRN,
	CHIP_VERDE,
	CHIP_OLAND,
	CHIP_HAINAN,
	CHIP_BONAIRE,
	CHIP_KAVERI,
	CHIP_KABINI,
	CHIP_HAWAII,
	CHIP_MULLINS,
	CHIP_TOPAZ,
	CHIP_TONGA,
	CHIP_FIJI,
	CHIP_CARRIZO,
	CHIP_STONEY,
	CHIP_POLARIS10,
	CHIP_POLARIS11,
	CHIP_POLARIS12,
	CHIP_VEGA10,
	CHIP_RAVEN,
	CHIP_LAST,
};


/****************************************
 ** gpu/drm/amd/display/dc/irq_types.h **
 ****************************************/

/* The order of the IRQ sources is important and MUST match the one's
of base driver */
enum dc_irq_source {
	/* Use as mask to specify invalid irq source */
	DC_IRQ_SOURCE_INVALID = 0,

	DC_IRQ_SOURCE_HPD1,
	DC_IRQ_SOURCE_HPD2,
	DC_IRQ_SOURCE_HPD3,
	DC_IRQ_SOURCE_HPD4,
	DC_IRQ_SOURCE_HPD5,
	DC_IRQ_SOURCE_HPD6,

	DC_IRQ_SOURCE_HPD1RX,
	DC_IRQ_SOURCE_HPD2RX,
	DC_IRQ_SOURCE_HPD3RX,
	DC_IRQ_SOURCE_HPD4RX,
	DC_IRQ_SOURCE_HPD5RX,
	DC_IRQ_SOURCE_HPD6RX,

	DC_IRQ_SOURCE_I2C_DDC1,
	DC_IRQ_SOURCE_I2C_DDC2,
	DC_IRQ_SOURCE_I2C_DDC3,
	DC_IRQ_SOURCE_I2C_DDC4,
	DC_IRQ_SOURCE_I2C_DDC5,
	DC_IRQ_SOURCE_I2C_DDC6,

	DC_IRQ_SOURCE_DPSINK1,
	DC_IRQ_SOURCE_DPSINK2,
	DC_IRQ_SOURCE_DPSINK3,
	DC_IRQ_SOURCE_DPSINK4,
	DC_IRQ_SOURCE_DPSINK5,
	DC_IRQ_SOURCE_DPSINK6,

	DC_IRQ_SOURCE_TIMER,

	DC_IRQ_SOURCE_PFLIP_FIRST,
	DC_IRQ_SOURCE_PFLIP1 = DC_IRQ_SOURCE_PFLIP_FIRST,
	DC_IRQ_SOURCE_PFLIP2,
	DC_IRQ_SOURCE_PFLIP3,
	DC_IRQ_SOURCE_PFLIP4,
	DC_IRQ_SOURCE_PFLIP5,
	DC_IRQ_SOURCE_PFLIP6,
	DC_IRQ_SOURCE_PFLIP_UNDERLAY0,
	DC_IRQ_SOURCE_PFLIP_LAST = DC_IRQ_SOURCE_PFLIP_UNDERLAY0,

	DC_IRQ_SOURCE_GPIOPAD0,
	DC_IRQ_SOURCE_GPIOPAD1,
	DC_IRQ_SOURCE_GPIOPAD2,
	DC_IRQ_SOURCE_GPIOPAD3,
	DC_IRQ_SOURCE_GPIOPAD4,
	DC_IRQ_SOURCE_GPIOPAD5,
	DC_IRQ_SOURCE_GPIOPAD6,
	DC_IRQ_SOURCE_GPIOPAD7,
	DC_IRQ_SOURCE_GPIOPAD8,
	DC_IRQ_SOURCE_GPIOPAD9,
	DC_IRQ_SOURCE_GPIOPAD10,
	DC_IRQ_SOURCE_GPIOPAD11,
	DC_IRQ_SOURCE_GPIOPAD12,
	DC_IRQ_SOURCE_GPIOPAD13,
	DC_IRQ_SOURCE_GPIOPAD14,
	DC_IRQ_SOURCE_GPIOPAD15,
	DC_IRQ_SOURCE_GPIOPAD16,
	DC_IRQ_SOURCE_GPIOPAD17,
	DC_IRQ_SOURCE_GPIOPAD18,
	DC_IRQ_SOURCE_GPIOPAD19,
	DC_IRQ_SOURCE_GPIOPAD20,
	DC_IRQ_SOURCE_GPIOPAD21,
	DC_IRQ_SOURCE_GPIOPAD22,
	DC_IRQ_SOURCE_GPIOPAD23,
	DC_IRQ_SOURCE_GPIOPAD24,
	DC_IRQ_SOURCE_GPIOPAD25,
	DC_IRQ_SOURCE_GPIOPAD26,
	DC_IRQ_SOURCE_GPIOPAD27,
	DC_IRQ_SOURCE_GPIOPAD28,
	DC_IRQ_SOURCE_GPIOPAD29,
	DC_IRQ_SOURCE_GPIOPAD30,

	DC_IRQ_SOURCE_DC1UNDERFLOW,
	DC_IRQ_SOURCE_DC2UNDERFLOW,
	DC_IRQ_SOURCE_DC3UNDERFLOW,
	DC_IRQ_SOURCE_DC4UNDERFLOW,
	DC_IRQ_SOURCE_DC5UNDERFLOW,
	DC_IRQ_SOURCE_DC6UNDERFLOW,

	DC_IRQ_SOURCE_DMCU_SCP,
	DC_IRQ_SOURCE_VBIOS_SW,

	DC_IRQ_SOURCE_VUPDATE1,
	DC_IRQ_SOURCE_VUPDATE2,
	DC_IRQ_SOURCE_VUPDATE3,
	DC_IRQ_SOURCE_VUPDATE4,
	DC_IRQ_SOURCE_VUPDATE5,
	DC_IRQ_SOURCE_VUPDATE6,

	DC_IRQ_SOURCE_VBLANK1,
	DC_IRQ_SOURCE_VBLANK2,
	DC_IRQ_SOURCE_VBLANK3,
	DC_IRQ_SOURCE_VBLANK4,
	DC_IRQ_SOURCE_VBLANK5,
	DC_IRQ_SOURCE_VBLANK6,

	DAL_IRQ_SOURCES_NUMBER
};


/***************************
 ** drm/drm_mode_object.h **
 ***************************/

struct drm_mode_object {
	uint32_t id;
	uint32_t type;
	struct drm_object_properties *properties;
	struct kref refcount;
	void (*free_cb)(struct kref *kref);
};


/************************
 ** drm/drm_encoder.h  **
 ************************/

struct drm_encoder;

/**
 * struct drm_encoder_funcs - encoder controls
 *
 * Encoders sit between CRTCs and connectors.
 */
struct drm_encoder_funcs {
	/**
	 * @reset:
	 *
	 * Reset encoder hardware and software state to off. This function isn't
	 * called by the core directly, only through drm_mode_config_reset().
	 * It's not a helper hook only for historical reasons.
	 */
	void (*reset)(struct drm_encoder *encoder);

	/**
	 * @destroy:
	 *
	 * Clean up encoder resources. This is only called at driver unload time
	 * through drm_mode_config_cleanup() since an encoder cannot be
	 * hotplugged in DRM.
	 */
	void (*destroy)(struct drm_encoder *encoder);

	/**
	 * @late_register:
	 *
	 * This optional hook can be used to register additional userspace
	 * interfaces attached to the encoder like debugfs interfaces.
	 * It is called late in the driver load sequence from drm_dev_register().
	 * Everything added from this callback should be unregistered in
	 * the early_unregister callback.
	 *
	 * Returns:
	 *
	 * 0 on success, or a negative error code on failure.
	 */
	int (*late_register)(struct drm_encoder *encoder);

	/**
	 * @early_unregister:
	 *
	 * This optional hook should be used to unregister the additional
	 * userspace interfaces attached to the encoder from
	 * @late_register. It is called from drm_dev_unregister(),
	 * early in the driver unload sequence to disable userspace access
	 * before data structures are torndown.
	 */
	void (*early_unregister)(struct drm_encoder *encoder);
};

struct drm_encoder {
	struct drm_device *dev;
	struct list_head head;

	struct drm_mode_object base;
	char *name;
	/**
	 * @encoder_type:
	 *
	 * One of the DRM_MODE_ENCODER_<foo> types in drm_mode.h. The following
	 * encoder types are defined thus far:
	 *
	 * - DRM_MODE_ENCODER_DAC for VGA and analog on DVI-I/DVI-A.
	 *
	 * - DRM_MODE_ENCODER_TMDS for DVI, HDMI and (embedded) DisplayPort.
	 *
	 * - DRM_MODE_ENCODER_LVDS for display panels, or in general any panel
	 *   with a proprietary parallel connector.
	 *
	 * - DRM_MODE_ENCODER_TVDAC for TV output (Composite, S-Video,
	 *   Component, SCART).
	 *
	 * - DRM_MODE_ENCODER_VIRTUAL for virtual machine displays
	 *
	 * - DRM_MODE_ENCODER_DSI for panels connected using the DSI serial bus.
	 *
	 * - DRM_MODE_ENCODER_DPI for panels connected using the DPI parallel
	 *   bus.
	 *
	 * - DRM_MODE_ENCODER_DPMST for special fake encoders used to allow
	 *   mutliple DP MST streams to share one physical encoder.
	 */
	int encoder_type;

	/**
	 * @index: Position inside the mode_config.list, can be used as an array
	 * index. It is invariant over the lifetime of the encoder.
	 */
	unsigned index;

	/**
	 * @possible_crtcs: Bitmask of potential CRTC bindings, using
	 * drm_crtc_index() as the index into the bitfield. The driver must set
	 * the bits for all &drm_crtc objects this encoder can be connected to
	 * before calling drm_encoder_init().
	 *
	 * In reality almost every driver gets this wrong.
	 *
	 * Note that since CRTC objects can't be hotplugged the assigned indices
	 * are stable and hence known before registering all objects.
	 */
	uint32_t possible_crtcs;

	/**
	 * @possible_clones: Bitmask of potential sibling encoders for cloning,
	 * using drm_encoder_index() as the index into the bitfield. The driver
	 * must set the bits for all &drm_encoder objects which can clone a
	 * &drm_crtc together with this encoder before calling
	 * drm_encoder_init(). Drivers should set the bit representing the
	 * encoder itself, too. Cloning bits should be set such that when two
	 * encoders can be used in a cloned configuration, they both should have
	 * each another bits set.
	 *
	 * In reality almost every driver gets this wrong.
	 *
	 * Note that since encoder objects can't be hotplugged the assigned indices
	 * are stable and hence known before registering all objects.
	 */
	uint32_t possible_clones;

	/**
	 * @crtc: Currently bound CRTC, only really meaningful for non-atomic
	 * drivers.  Atomic drivers should instead check
	 * &drm_connector_state.crtc.
	 */
	struct drm_crtc *crtc;
	struct drm_bridge *bridge;
	const struct drm_encoder_funcs *funcs;
	const struct drm_encoder_helper_funcs *helper_private;
};

#define obj_to_encoder(x) container_of(x, struct drm_encoder, base)


/**************************
 ** uapi/drm/drm_mode.h  **
 **************************/

#define DRM_DISPLAY_MODE_LEN	32


/*********************
 ** drm/drm_modes.h **
 *********************/

enum drm_mode_status {
	MODE_OK = 0,
	MODE_HSYNC,
	MODE_VSYNC,
	MODE_H_ILLEGAL,
	MODE_V_ILLEGAL,
	MODE_BAD_WIDTH,
	MODE_NOMODE,
	MODE_NO_INTERLACE,
	MODE_NO_DBLESCAN,
	MODE_NO_VSCAN,
	MODE_MEM,
	MODE_VIRTUAL_X,
	MODE_VIRTUAL_Y,
	MODE_MEM_VIRT,
	MODE_NOCLOCK,
	MODE_CLOCK_HIGH,
	MODE_CLOCK_LOW,
	MODE_CLOCK_RANGE,
	MODE_BAD_HVALUE,
	MODE_BAD_VVALUE,
	MODE_BAD_VSCAN,
	MODE_HSYNC_NARROW,
	MODE_HSYNC_WIDE,
	MODE_HBLANK_NARROW,
	MODE_HBLANK_WIDE,
	MODE_VSYNC_NARROW,
	MODE_VSYNC_WIDE,
	MODE_VBLANK_NARROW,
	MODE_VBLANK_WIDE,
	MODE_PANEL,
	MODE_INTERLACE_WIDTH,
	MODE_ONE_WIDTH,
	MODE_ONE_HEIGHT,
	MODE_ONE_SIZE,
	MODE_NO_REDUCED,
	MODE_NO_STEREO,
	MODE_NO_420,
	MODE_STALE = -3,
	MODE_BAD = -2,
	MODE_ERROR = -1
};

struct drm_display_mode {
	/**
	 * @head:
	 *
	 * struct list_head for mode lists.
	 */
	struct list_head head;

	/**
	 * @base:
	 *
	 * A display mode is a normal modeset object, possibly including public
	 * userspace id.
	 *
	 * FIXME:
	 *
	 * This can probably be removed since the entire concept of userspace
	 * managing modes explicitly has never landed in upstream kernel mode
	 * setting support.
	 */
	struct drm_mode_object base;

	/**
	 * @name:
	 *
	 * Human-readable name of the mode, filled out with drm_mode_set_name().
	 */
	char name[DRM_DISPLAY_MODE_LEN];

	/**
	 * @status:
	 *
	 * Status of the mode, used to filter out modes not supported by the
	 * hardware. See enum &drm_mode_status.
	 */
	enum drm_mode_status status;

	/**
	 * @type:
	 *
	 * A bitmask of flags, mostly about the source of a mode. Possible flags
	 * are:
	 *
	 *  - DRM_MODE_TYPE_BUILTIN: Meant for hard-coded modes, effectively
	 *    unused.
	 *  - DRM_MODE_TYPE_PREFERRED: Preferred mode, usually the native
	 *    resolution of an LCD panel. There should only be one preferred
	 *    mode per connector at any given time.
	 *  - DRM_MODE_TYPE_DRIVER: Mode created by the driver, which is all of
	 *    them really. Drivers must set this bit for all modes they create
	 *    and expose to userspace.
	 *
	 * Plus a big list of flags which shouldn't be used at all, but are
	 * still around since these flags are also used in the userspace ABI:
	 *
	 *  - DRM_MODE_TYPE_DEFAULT: Again a leftover, use
	 *    DRM_MODE_TYPE_PREFERRED instead.
	 *  - DRM_MODE_TYPE_CLOCK_C and DRM_MODE_TYPE_CRTC_C: Define leftovers
	 *    which are stuck around for hysterical raisins only. No one has an
	 *    idea what they were meant for. Don't use.
	 *  - DRM_MODE_TYPE_USERDEF: Mode defined by userspace, again a vestige
	 *    from older kms designs where userspace had to first add a custom
	 *    mode to the kernel's mode list before it could use it. Don't use.
	 */
	unsigned int type;

	/**
	 * @clock:
	 *
	 * Pixel clock in kHz.
	 */
	int clock;		/* in kHz */
	int hdisplay;
	int hsync_start;
	int hsync_end;
	int htotal;
	int hskew;
	int vdisplay;
	int vsync_start;
	int vsync_end;
	int vtotal;
	int vscan;
	/**
	 * @flags:
	 *
	 * Sync and timing flags:
	 *
	 *  - DRM_MODE_FLAG_PHSYNC: horizontal sync is active high.
	 *  - DRM_MODE_FLAG_NHSYNC: horizontal sync is active low.
	 *  - DRM_MODE_FLAG_PVSYNC: vertical sync is active high.
	 *  - DRM_MODE_FLAG_NVSYNC: vertical sync is active low.
	 *  - DRM_MODE_FLAG_INTERLACE: mode is interlaced.
	 *  - DRM_MODE_FLAG_DBLSCAN: mode uses doublescan.
	 *  - DRM_MODE_FLAG_CSYNC: mode uses composite sync.
	 *  - DRM_MODE_FLAG_PCSYNC: composite sync is active high.
	 *  - DRM_MODE_FLAG_NCSYNC: composite sync is active low.
	 *  - DRM_MODE_FLAG_HSKEW: hskew provided (not used?).
	 *  - DRM_MODE_FLAG_BCAST: not used?
	 *  - DRM_MODE_FLAG_PIXMUX: not used?
	 *  - DRM_MODE_FLAG_DBLCLK: double-clocked mode.
	 *  - DRM_MODE_FLAG_CLKDIV2: half-clocked mode.
	 *
	 * Additionally there's flags to specify how 3D modes are packed:
	 *
	 *  - DRM_MODE_FLAG_3D_NONE: normal, non-3D mode.
	 *  - DRM_MODE_FLAG_3D_FRAME_PACKING: 2 full frames for left and right.
	 *  - DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE: interleaved like fields.
	 *  - DRM_MODE_FLAG_3D_LINE_ALTERNATIVE: interleaved lines.
	 *  - DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL: side-by-side full frames.
	 *  - DRM_MODE_FLAG_3D_L_DEPTH: ?
	 *  - DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH: ?
	 *  - DRM_MODE_FLAG_3D_TOP_AND_BOTTOM: frame split into top and bottom
	 *    parts.
	 *  - DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF: frame split into left and
	 *    right parts.
	 */
	unsigned int flags;

	/**
	 * @width_mm:
	 *
	 * Addressable size of the output in mm, projectors should set this to
	 * 0.
	 */
	int width_mm;

	/**
	 * @height_mm:
	 *
	 * Addressable size of the output in mm, projectors should set this to
	 * 0.
	 */
	int height_mm;

	/**
	 * @crtc_clock:
	 *
	 * Actual pixel or dot clock in the hardware. This differs from the
	 * logical @clock when e.g. using interlacing, double-clocking, stereo
	 * modes or other fancy stuff that changes the timings and signals
	 * actually sent over the wire.
	 *
	 * This is again in kHz.
	 *
	 * Note that with digital outputs like HDMI or DP there's usually a
	 * massive confusion between the dot clock and the signal clock at the
	 * bit encoding level. Especially when a 8b/10b encoding is used and the
	 * difference is exactly a factor of 10.
	 */
	int crtc_clock;
	int crtc_hdisplay;
	int crtc_hblank_start;
	int crtc_hblank_end;
	int crtc_hsync_start;
	int crtc_hsync_end;
	int crtc_htotal;
	int crtc_hskew;
	int crtc_vdisplay;
	int crtc_vblank_start;
	int crtc_vblank_end;
	int crtc_vsync_start;
	int crtc_vsync_end;
	int crtc_vtotal;

	/**
	 * @private:
	 *
	 * Pointer for driver private data. This can only be used for mode
	 * objects passed to drivers in modeset operations. It shouldn't be used
	 * by atomic drivers since they can store any additional data by
	 * subclassing state structures.
	 */
	int *private;

	/**
	 * @private_flags:
	 *
	 * Similar to @private, but just an integer.
	 */
	int private_flags;

	/**
	 * @vrefresh:
	 *
	 * Vertical refresh rate, for debug output in human readable form. Not
	 * used in a functional way.
	 *
	 * This value is in Hz.
	 */
	int vrefresh;

	/**
	 * @hsync:
	 *
	 * Horizontal refresh rate, for debug output in human readable form. Not
	 * used in a functional way.
	 *
	 * This value is in kHz.
	 */
	int hsync;

	/**
	 * @picture_aspect_ratio:
	 *
	 * Field for setting the HDMI picture aspect ratio of a mode.
	 */
	enum hdmi_picture_aspect picture_aspect_ratio;
};


/***********************************************
 ** gpu/drm/amd/display/amdgpu_dm/amdgpu_dm.h **
 ***********************************************/

struct amdgpu_dm_prev_state {
	struct drm_framebuffer *fb;
	int32_t x;
	int32_t y;
	struct drm_display_mode mode;
};

struct common_irq_params {
	struct amdgpu_device *adev;
	enum dc_irq_source irq_src;
};

struct irq_list_head {
	struct list_head head;
	/* In case this interrupt needs post-processing, 'work' will be queued*/
	struct work_struct work;
};

struct amdgpu_display_manager {
	struct dal *dal;
	struct dc *dc;
	struct cgs_device *cgs_device;
	/* lock to be used when DAL is called from SYNC IRQ context */
	spinlock_t dal_lock;

	struct amdgpu_device *adev;	/*AMD base driver*/
	struct drm_device *ddev;	/*DRM base driver*/
	u16 display_indexes_num;

	struct amdgpu_dm_prev_state prev_state;

	/*
	 * 'irq_source_handler_table' holds a list of handlers
	 * per (DAL) IRQ source.
	 *
	 * Each IRQ source may need to be handled at different contexts.
	 * By 'context' we mean, for example:
	 * - The ISR context, which is the direct interrupt handler.
	 * - The 'deferred' context - this is the post-processing of the
	 *	interrupt, but at a lower priority.
	 *
	 * Note that handlers are called in the same order as they were
	 * registered (FIFO).
	 */
	struct irq_list_head irq_handler_list_low_tab[DAL_IRQ_SOURCES_NUMBER];
	struct list_head irq_handler_list_high_tab[DAL_IRQ_SOURCES_NUMBER];

	struct common_irq_params
	pflip_params[DC_IRQ_SOURCE_PFLIP_LAST - DC_IRQ_SOURCE_PFLIP_FIRST + 1];

	struct common_irq_params
	vblank_params[DC_IRQ_SOURCE_VBLANK6 - DC_IRQ_SOURCE_VBLANK1 + 1];

	/* this spin lock synchronizes access to 'irq_handler_list_table' */
	spinlock_t irq_handler_list_table_lock;

	/* Timer-related data. */
	struct list_head timer_handler_list;
	struct workqueue_struct *timer_workqueue;

	/* Use dal_mutex for any activity which is NOT syncronized by
	 * DRM mode setting locks.
	 * For example: amdgpu_dm_hpd_low_irq() calls into DAL *without*
	 * DRM mode setting locks being acquired. This is where dal_mutex
	 * is acquired before calling into DAL. */
	struct mutex dal_mutex;

	struct backlight_device *backlight_dev;

	const struct dc_link *backlight_link;

	struct work_struct mst_hotplug_work;

	struct mod_freesync *freesync_module;

	/**
	 * Caches device atomic state for suspend/resume
	 */
	struct drm_atomic_state *cached_state;
#if defined(CONFIG_DRM_AMD_DC_FBC)
	struct dm_comressor_info compressor;
#endif
};


/***********************************
 ** include/uapi/drm/amdgpu_drm.h **
 ***********************************/

#define AMDGPU_GEM_DOMAIN_CPU		0x1
#define AMDGPU_GEM_DOMAIN_GTT		0x2
#define AMDGPU_GEM_DOMAIN_VRAM		0x4
#define AMDGPU_GEM_DOMAIN_GDS		0x8
#define AMDGPU_GEM_DOMAIN_GWS		0x10
#define AMDGPU_GEM_DOMAIN_OA		0x20

/* Flag that BO sharing will be explicitly synchronized */
#define AMDGPU_GEM_CREATE_EXPLICIT_SYNC		(1 << 7)
/* Flag that BO sharing will be explicitly synchronized */
#define AMDGPU_GEM_CREATE_EXPLICIT_SYNC		(1 << 7)


/**************************************
 ** gpu/drm/amd/amdgpu/amdgpu_mode.h **
 **************************************/

enum amdgpu_rmx_type {
	RMX_OFF,
	RMX_FULL,
	RMX_CENTER,
	RMX_ASPECT
};

enum amdgpu_underscan_type {
	UNDERSCAN_OFF,
	UNDERSCAN_ON,
	UNDERSCAN_AUTO,
};

#define AMDGPU_MAX_I2C_BUS 16

struct amdgpu_encoder {
	struct drm_encoder base;
	uint32_t encoder_enum;
	uint32_t encoder_id;
	uint32_t devices;
	uint32_t active_device;
	uint32_t flags;
	uint32_t pixel_clock;
	enum amdgpu_rmx_type rmx_type;
	enum amdgpu_underscan_type underscan_type;
	uint32_t underscan_hborder;
	uint32_t underscan_vborder;
	struct drm_display_mode native_mode;
	void *enc_priv;
	int audio_polling_active;
	bool is_ext_encoder;
	u16 caps;
};


/*****************************************
 ** gpu/drm/amd/amdgpu/amdgpu_debugfs.h **
 *****************************************/

struct amdgpu_debugfs {
	const struct drm_info_list	*files;
	unsigned		num_files;
};


/**************************************
 ** gpu/drm/amd/amdgpu/amdgpu_ring.h **
 **************************************/

#define AMDGPU_MAX_RINGS		18


/*************************************
 ** gpu/drm/amd/amdgpu/amdgpu_ids.h **
 *************************************/

#define AMDGPU_NUM_VMID	16

struct amdgpu_vmid_mgr {
	struct mutex		lock;
	unsigned		num_ids;
	struct list_head	ids_lru;
	struct amdgpu_vmid	ids[AMDGPU_NUM_VMID];
	atomic_t		reserved_vmid_num;
};


/**************************************
 ** gpu/drm/amd/amdgpu/amdgpu_gart.h **
 **************************************/

struct amdgpu_gart {
	u64				table_addr;
	struct amdgpu_bo		*robj;
	void				*ptr;
	unsigned			num_gpu_pages;
	unsigned			num_cpu_pages;
	unsigned			table_size;
#ifdef CONFIG_DRM_AMDGPU_GART_DEBUGFS
	struct page			**pages;
#endif
	bool				ready;

	/* Asic default pte flags */
	uint64_t			gart_pte_flags;

	const struct amdgpu_gart_funcs *gart_funcs;
};


/************************************
 ** gpu/drm/amd/amdgpu/amdgpu_vm.h **
 ************************************/

#define AMDGPU_MAX_VMHUBS			2

enum amdgpu_vm_level {
	AMDGPU_VM_PDB2,
	AMDGPU_VM_PDB1,
	AMDGPU_VM_PDB0,
	AMDGPU_VM_PTB
};

struct amdgpu_vm_manager {
	/* Handling of VMIDs */
	struct amdgpu_vmid_mgr			id_mgr[AMDGPU_MAX_VMHUBS];

	/* Handling of VM fences */
	u64					fence_context;
	unsigned				seqno[AMDGPU_MAX_RINGS];

	uint64_t				max_pfn;
	uint32_t				num_level;
	uint32_t				block_size;
	uint32_t				fragment_size;
	enum amdgpu_vm_level			root_level;
	/* vram base address for page table entry  */
	u64					vram_base_offset;
	/* vm pte handling */
	const struct amdgpu_vm_pte_funcs        *vm_pte_funcs;
	struct amdgpu_ring                      *vm_pte_rings[AMDGPU_MAX_RINGS];
	unsigned				vm_pte_num_rings;
	atomic_t				vm_pte_next_ring;

	/* partial resident texture handling */
	spinlock_t				prt_lock;
	atomic_t				num_prt_users;

	/* controls how VM page tables are updated for Graphics and Compute.
	 * BIT0[= 0] Graphics updated by SDMA [= 1] by CPU
	 * BIT1[= 0] Compute updated by SDMA [= 1] by CPU
	 */
	int					vm_update_mode;

	/* PASID to VM mapping, will be used in interrupt context to
	 * look up VM of a page fault
	 */
	struct idr				pasid_idr;
	spinlock_t				pasid_lock;
};


/*********************************
 ** gpu/drm/amd/amdgpu/amdgpu.h **
 *********************************/

#define AMDGPU_DEBUGFS_MAX_COMPONENTS		32
#define AMDGPU_BIOS_NUM_SCRATCH			16

struct amdgpu_atif_notification_cfg {
	bool enabled;
	int command_code;
};

struct amdgpu_atif_notifications {
	bool display_switch;
	bool expansion_mode_change;
	bool thermal_state;
	bool forced_power_state;
	bool system_power_state;
	bool display_conf_change;
	bool px_gfx_switch;
	bool brightness_change;
	bool dgpu_display_event;
};

struct amdgpu_atif_functions {
	bool system_params;
	bool sbios_requests;
	bool select_active_disp;
	bool lid_state;
	bool get_tv_standard;
	bool set_tv_standard;
	bool get_panel_expansion_mode;
	bool set_panel_expansion_mode;
	bool temperature_change;
	bool graphics_device_types;
};

struct amdgpu_atif {
	struct amdgpu_atif_notifications notifications;
	struct amdgpu_atif_functions functions;
	struct amdgpu_atif_notification_cfg notification_cfg;
	struct amdgpu_encoder *encoder_for_bl;
};

struct amdgpu_atcs_functions {
	bool get_ext_state;
	bool pcie_perf_req;
	bool pcie_dev_rdy;
	bool pcie_bus_width;
};

struct amdgpu_atcs {
	struct amdgpu_atcs_functions functions;
};

struct amdgpu_device;

typedef uint32_t (*amdgpu_rreg_t)(struct amdgpu_device*, uint32_t);
typedef void (*amdgpu_wreg_t)(struct amdgpu_device*, uint32_t, uint32_t);
typedef uint32_t (*amdgpu_block_rreg_t)(struct amdgpu_device*, uint32_t, uint32_t);
typedef void (*amdgpu_block_wreg_t)(struct amdgpu_device*, uint32_t, uint32_t, uint32_t);

struct amdgpu_doorbell {
	/* doorbell mmio */
	resource_size_t		base;
	resource_size_t		size;
	u32 __iomem		*ptr;
	u32			num_doorbells;	/* Number of doorbells actually reserved for amdgpu. */
};

#define AMDGPU_MAX_PPLL 3

struct amdgpu_mc {
	resource_size_t		aper_size;
	resource_size_t		aper_base;
	resource_size_t		agp_base;
	/* for some chips with <= 32MB we need to lie
	 * about vram size near mc fb location */
	u64			mc_vram_size;
	u64			visible_vram_size;
	u64			gart_size;
	u64			gart_start;
	u64			gart_end;
	u64			vram_start;
	u64			vram_end;
	unsigned		vram_width;
	u64			real_vram_size;
	int			vram_mtrr;
	u64                     mc_mask;
	const struct firmware   *fw;	/* MC firmware */
	uint32_t                fw_version;
	struct amdgpu_irq_src	vm_fault;
	uint32_t		vram_type;
	uint32_t                srbm_soft_reset;
	bool			prt_warning;
	uint64_t		stolen_size;
	/* apertures */
	u64					shared_aperture_start;
	u64					shared_aperture_end;
	u64					private_aperture_start;
	u64					private_aperture_end;
	/* protects concurrent invalidation */
	spinlock_t		invalidate_lock;
	bool			translate_further;
};

struct amdgpu_dummy_page {
	struct page	*page;
	dma_addr_t	addr;
};

struct amdgpu_clock {
	struct amdgpu_pll ppll[AMDGPU_MAX_PPLL];
	struct amdgpu_pll spll;
	struct amdgpu_pll mpll;
	/* 10 Khz units */
	uint32_t default_mclk;
	uint32_t default_sclk;
	uint32_t default_dispclk;
	uint32_t current_dispclk;
	uint32_t dp_extclk;
	uint32_t max_pixel_clock;
};

struct amdgpu_device {
	struct device			*dev;
	struct drm_device		*ddev;
	struct pci_dev			*pdev;

#ifdef CONFIG_DRM_AMD_ACP
	struct amdgpu_acp		acp;
#endif

	/* ASIC */
	enum amd_asic_type		asic_type;
	uint32_t			family;
	uint32_t			rev_id;
	uint32_t			external_rev_id;
	unsigned long			flags;
	int				usec_timeout;
	const struct amdgpu_asic_funcs	*asic_funcs;
	bool				shutdown;
	bool				need_dma32;
	bool				accel_working;
	struct work_struct		reset_work;
	struct notifier_block		acpi_nb;
	struct amdgpu_i2c_chan		*i2c_bus[AMDGPU_MAX_I2C_BUS];
	struct amdgpu_debugfs		debugfs[AMDGPU_DEBUGFS_MAX_COMPONENTS];
	unsigned			debugfs_count;
#if defined(CONFIG_DEBUG_FS)
	struct dentry			*debugfs_regs[AMDGPU_DEBUGFS_MAX_COMPONENTS];
#endif
	struct amdgpu_atif		atif;
	struct amdgpu_atcs		atcs;
	struct mutex			srbm_mutex;
	/* GRBM index mutex. Protects concurrent access to GRBM index */
	struct mutex                    grbm_idx_mutex;
	struct dev_pm_domain		vga_pm_domain;
	bool				have_disp_power_ref;

	/* BIOS */
	bool				is_atom_fw;
	uint8_t				*bios;
	uint32_t			bios_size;
	struct amdgpu_bo		*stolen_vga_memory;
	uint32_t			bios_scratch_reg_offset;
	uint32_t			bios_scratch[AMDGPU_BIOS_NUM_SCRATCH];

	/* Register/doorbell mmio */
	resource_size_t			rmmio_base;
	resource_size_t			rmmio_size;
	void __iomem			*rmmio;
	/* protects concurrent MM_INDEX/DATA based register access */
	spinlock_t mmio_idx_lock;
	/* protects concurrent SMC based register access */
	spinlock_t smc_idx_lock;
	amdgpu_rreg_t			smc_rreg;
	amdgpu_wreg_t			smc_wreg;
	/* protects concurrent PCIE register access */
	spinlock_t pcie_idx_lock;
	amdgpu_rreg_t			pcie_rreg;
	amdgpu_wreg_t			pcie_wreg;
	amdgpu_rreg_t			pciep_rreg;
	amdgpu_wreg_t			pciep_wreg;
	/* protects concurrent UVD register access */
	spinlock_t uvd_ctx_idx_lock;
	amdgpu_rreg_t			uvd_ctx_rreg;
	amdgpu_wreg_t			uvd_ctx_wreg;
	/* protects concurrent DIDT register access */
	spinlock_t didt_idx_lock;
	amdgpu_rreg_t			didt_rreg;
	amdgpu_wreg_t			didt_wreg;
	/* protects concurrent gc_cac register access */
	spinlock_t gc_cac_idx_lock;
	amdgpu_rreg_t			gc_cac_rreg;
	amdgpu_wreg_t			gc_cac_wreg;
	/* protects concurrent se_cac register access */
	spinlock_t se_cac_idx_lock;
	amdgpu_rreg_t			se_cac_rreg;
	amdgpu_wreg_t			se_cac_wreg;
	/* protects concurrent ENDPOINT (audio) register access */
	spinlock_t audio_endpt_idx_lock;
	amdgpu_block_rreg_t		audio_endpt_rreg;
	amdgpu_block_wreg_t		audio_endpt_wreg;
	void __iomem                    *rio_mem;
	resource_size_t			rio_mem_size;
	struct amdgpu_doorbell		doorbell;

	/* clock/pll info */
	struct amdgpu_clock            clock;

	/* MC */
	struct amdgpu_mc		mc;
	struct amdgpu_gart		gart;
	struct amdgpu_dummy_page	dummy_page;
	struct amdgpu_vm_manager	vm_manager;
	struct amdgpu_vmhub             vmhub[AMDGPU_MAX_VMHUBS];

	/* memory management */
	struct amdgpu_mman		mman;
	struct amdgpu_vram_scratch	vram_scratch;
	struct amdgpu_wb		wb;
	atomic64_t			num_bytes_moved;
	atomic64_t			num_evictions;
	atomic64_t			num_vram_cpu_page_faults;
	atomic_t			gpu_reset_counter;
	atomic_t			vram_lost_counter;

	/* data for buffer migration throttling */
	struct {
		spinlock_t		lock;
		s64			last_update_us;
		s64			accum_us; /* accumulated microseconds */
		s64			accum_us_vis; /* for visible VRAM */
		u32			log2_max_MBps;
	} mm_stats;

	/* display */
	bool				enable_virtual_display;
	struct amdgpu_mode_info		mode_info;
	/* For pre-DCE11. DCE11 and later are in "struct amdgpu_device->dm" */
	struct work_struct		hotplug_work;
	struct amdgpu_irq_src		crtc_irq;
	struct amdgpu_irq_src		pageflip_irq;
	struct amdgpu_irq_src		hpd_irq;

	/* rings */
	u64				fence_context;
	unsigned			num_rings;
	struct amdgpu_ring		*rings[AMDGPU_MAX_RINGS];
	bool				ib_pool_ready;
	struct amdgpu_sa_manager	ring_tmp_bo;

	/* interrupts */
	struct amdgpu_irq		irq;

	/* powerplay */
	struct amd_powerplay		powerplay;
	bool				pp_force_state_enabled;

	/* dpm */
	struct amdgpu_pm		pm;
	u32				cg_flags;
	u32				pg_flags;

	/* amdgpu smumgr */
	struct amdgpu_smumgr smu;

	/* gfx */
	struct amdgpu_gfx		gfx;

	/* sdma */
	struct amdgpu_sdma		sdma;

	/* uvd */
	struct amdgpu_uvd		uvd;

	/* vce */
	struct amdgpu_vce		vce;

	/* vcn */
	struct amdgpu_vcn		vcn;

	/* firmwares */
	struct amdgpu_firmware		firmware;

	/* PSP */
	struct psp_context		psp;

	/* GDS */
	struct amdgpu_gds		gds;

	/* display related functionality */
	struct amdgpu_display_manager dm;

	struct amdgpu_ip_block          ip_blocks[AMDGPU_MAX_IP_NUM];
	int				num_ip_blocks;
	struct mutex	mn_lock;
	DECLARE_HASHTABLE(mn_hash, 7);

	/* tracking pinned memory */
	u64 vram_pin_size;
	u64 invisible_pin_size;
	u64 gart_pin_size;

	/* amdkfd interface */
	struct kfd_dev          *kfd;

	/* soc15 register offset based on ip, instance and  segment */
	uint32_t		*reg_offset[MAX_HWIP][HWIP_MAX_INSTANCE];

	const struct amdgpu_nbio_funcs	*nbio_funcs;

	/* delayed work_func for deferring clockgating during resume */
	struct delayed_work     late_init_work;

	struct amdgpu_virt	virt;
	/* firmware VRAM reservation */
	struct amdgpu_fw_vram_usage fw_vram_usage;

	/* link all shadow bo */
	struct list_head                shadow_list;
	struct mutex                    shadow_list_lock;
	/* keep an lru list of rings by HW IP */
	struct list_head		ring_lru_list;
	spinlock_t			ring_lru_list_lock;

	/* record hw reset is performed */
	bool has_hw_reset;
	u8				reset_magic[AMDGPU_RESET_MAGIC_NUM];

	/* record last mm index being written through WREG32*/
	unsigned long last_mm_index;
	bool                            in_gpu_reset;
	struct mutex  lock_reset;
};


#endif  /* _LX_EMUL_AMDGPU_H_ */
