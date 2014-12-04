#ifndef __CA_H_
#define __CA_H_

#include <stdint.h>
#include <asm/types.h>
#include <pthread.h>
#include <list>
#include <queue>
#include <vector>
#include <set>

#include "mmi.h"
#include "cs_types.h"
#include "cs_api.h"

/* constants taken from dvb-apps */
#define T_SB                0x80	// sb                           primitive   h<--m
#define T_RCV               0x81	// receive                      primitive   h-->m
#define T_CREATE_T_C        0x82	// create transport connection  primitive   h-->m
#define T_C_T_C_REPLY       0x83	// ctc reply                    primitive   h<--m
#define T_DELETE_T_C        0x84	// delete tc                    primitive   h<->m
#define T_D_T_C_REPLY       0x85	// dtc reply                    primitive   h<->m
#define T_REQUEST_T_C       0x86	// request transport connection primitive   h<--m
#define T_NEW_T_C           0x87	// new tc / reply to t_request  primitive   h-->m
#define T_T_C_ERROR         0x77	// error creating tc            primitive   h-->m
#define T_DATA_LAST         0xA0	// convey data from higher      constructed h<->m
#define T_DATA_MORE         0xA1	// convey data from higher      constructed h<->m

enum CA_INIT_MASK {
	CA_INIT_SC = 1,
	CA_INIT_CI,
	CA_INIT_BOTH
};

enum CA_SLOT_TYPE {
	CA_SLOT_TYPE_SMARTCARD,
	CA_SLOT_TYPE_CI,
	CA_SLOT_TYPE_ALL
};

enum CA_MESSAGE_FLAGS {
	CA_MESSAGE_EMPTY		= (1 << 0),
	CA_MESSAGE_HAS_PARAM1_DATA	= (1 << 1), /// Free after use!
	CA_MESSAGE_HAS_PARAM1_INT	= (1 << 2),
	CA_MESSAGE_HAS_PARAM1_PTR	= (1 << 3),
	CA_MESSAGE_HAS_PARAM2_INT	= (1 << 4),
	CA_MESSAGE_HAS_PARAM2_PTR	= (1 << 5),
	CA_MESSAGE_HAS_PARAM2_DATA	= (1 << 6),
	CA_MESSAGE_HAS_PARAM3_DATA	= (1 << 7), /// Free after use!
	CA_MESSAGE_HAS_PARAM3_INT	= (1 << 8),
	CA_MESSAGE_HAS_PARAM3_PTR	= (1 << 9),
	CA_MESSAGE_HAS_PARAM4_INT	= (1 << 10),
	CA_MESSAGE_HAS_PARAM4_PTR	= (1 << 11),
	CA_MESSAGE_HAS_PARAM4_DATA	= (1 << 12),
	CA_MESSAGE_HAS_PARAM5_INT	= (1 << 13),
	CA_MESSAGE_HAS_PARAM5_PTR	= (1 << 14),
	CA_MESSAGE_HAS_PARAM5_DATA	= (1 << 15),
	CA_MESSAGE_HAS_PARAM6_INT	= (1 << 16),
	CA_MESSAGE_HAS_PARAM6_PTR	= (1 << 17),
	CA_MESSAGE_HAS_PARAM6_DATA	= (1 << 18),
	CA_MESSAGE_HAS_PARAM1_LONG	= (1 << 19),
	CA_MESSAGE_HAS_PARAM2_LONG	= (1 << 20),
	CA_MESSAGE_HAS_PARAM3_LONG	= (1 << 21),
	CA_MESSAGE_HAS_PARAM4_LONG	= (1 << 22)
};

enum CA_MESSAGE_MSGID {
	CA_MESSAGE_MSG_INSERTED,
	CA_MESSAGE_MSG_REMOVED,
	CA_MESSAGE_MSG_INIT_OK,
	CA_MESSAGE_MSG_INIT_FAILED,
	CA_MESSAGE_MSG_MMI_MENU,
	CA_MESSAGE_MSG_MMI_MENU_ENTER,
	CA_MESSAGE_MSG_MMI_MENU_ANSWER,
	CA_MESSAGE_MSG_MMI_LIST,
	CA_MESSAGE_MSG_MMI_TEXT,
	CA_MESSAGE_MSG_MMI_REQ_INPUT,
	CA_MESSAGE_MSG_MMI_CLOSE,
	CA_MESSAGE_MSG_INTERNAL,
	CA_MESSAGE_MSG_PMT_ARRIVED,
	CA_MESSAGE_MSG_CAPMT_ARRIVED,
	CA_MESSAGE_MSG_CAT_ARRIVED,
	CA_MESSAGE_MSG_ECM_ARRIVED,
	CA_MESSAGE_MSG_EMM_ARRIVED,
	CA_MESSAGE_MSG_CHANNEL_CHANGE,
	CA_MESSAGE_MSG_GUI_READY,
	CA_MESSAGE_MSG_EXIT
};

typedef struct CA_MESSAGE {
	uint32_t MsgId;
	enum CA_SLOT_TYPE SlotType;
	int Slot;
	uint32_t Flags;
	union {
		uint8_t *Data[4];
		uint32_t Param[4];
		void *Ptr[4];
		uint64_t ParamLong[4];
	} Msg;
} CA_MESSAGE;

typedef std::set<int> ca_map_t;
typedef ca_map_t::iterator ca_map_iterator_t;

typedef std::vector<u16>			CaIdVector;
typedef std::vector<u16>::iterator		CaIdVectorIterator;
typedef std::vector<u16>::const_iterator	CaIdVectorConstIterator;

#define CA_MESSAGE_SIZE		sizeof(CA_MESSAGE)
#define CA_MESSAGE_ENTRIES	256
#define CA_MESSAGE_ENTRIES_CI	128
#define CA_MESSAGE_ENTRIES_SC	64

#ifndef CS_CA_PDATA
#define CS_CA_PDATA		void
#endif

typedef enum {
	TUNER_A,
	TUNER_B,
	TUNER_C,
	TUNER_D
} source_t;

typedef enum {
	eDataTimeout,
	eDataError,
	eDataReady,
	eDataWrite,
	eDataStatusChanged
} eData;

typedef enum {
	eStatusNone,
	eStatusWait,
	eStatusReset
} eStatus;

struct queueData
{
	__u8 prio;
	unsigned char *data;
	unsigned int len;
	queueData(unsigned char *_data, unsigned int _len, __u8 _prio = 0)
		:prio(_prio), data(_data), len(_len)
	{

	}
	bool operator < ( const struct queueData &a ) const
	{
		return prio < a.prio;
	}
};

class eDVBCIMMISession;
class eDVBCIApplicationManagerSession;
class eDVBCICAManagerSession;

typedef struct
{
	pthread_t slot_thread;
	unsigned int slot;
	int fd;
	int connection_id;
	eStatus status;

	int receivedLen;
	unsigned char* receivedData;

	void* pClass;

	bool pollConnection;
	bool camIsReady;

	eDVBCIMMISession* mmiSession;
	eDVBCIApplicationManagerSession* appSession;
	eDVBCICAManagerSession* camgrSession;

	bool hasAppManager;
	bool hasMMIManager;
	bool hasCAManager;
	bool hasDateTime;
	bool mmiOpened;
	bool init;

	char name[512];

	bool newCapmt;
	bool inUse;
	u64 tpid;
	u32 pmtlen;
	u8 source;
	u8 camask;
	unsigned char pmtdata[1024];

	int counter;
	CaIdVector cam_caids;
	std::priority_queue<queueData> sendqueue;
} tSlot;

eData sendData(tSlot *slot, unsigned char* data, int len);

typedef std::list<tSlot*>::iterator SlotIt;

/// CA module class
class cCA {
private:
	/// Static instance of the CA module
//	static cCA *inst;
	/// Private constructor (singleton method)
	cCA(void);
	/// Private data for the CA module
	CS_CA_PDATA *privateData;
	enum CA_INIT_MASK initMask;
	int num_slots;
	bool init;
	void SendPMT();
	pthread_mutex_t ciMutex;
	std::list<tSlot*> slot_data;
	pthread_t slot_thread;

public:
	bool Init(void);
	cCA(int Slots);
	/// Returns the number of CI slots
	uint32_t GetNumberCISlots(void);
	/// Returns the number of Smartcard slots
	uint32_t GetNumberSmartCardSlots(void);
	/// Singleton
	static cCA *GetInstance(void);
	/// Send PMT to a individual or to all available modules (DEPRECATED)
	bool SendPMT(int Unit, unsigned char *Data, int Len, enum CA_SLOT_TYPE SlotType = CA_SLOT_TYPE_ALL);
	/// Sends a message to the CA thread
	bool SendMessage(const CA_MESSAGE *Msg);
	/// Sets which modules to initialize. It is only
	/// possible to change this once!
	void SetInitMask(enum CA_INIT_MASK InitMask);
	/// Sets the frequency (in Hz) of the TS stream input (only valid for CI)
	void SetTSClock(u32 /*Speed*/) { return; };
	/// Start the CA module
	bool Start(void);
	/// Stops the CA module
	void Stop(void);
	/// Notify that the GUI is ready to receive messages
	/// (CA messages coming from a module)
	void Ready(bool Set);
	/// Resets a module (if possible)
	void ModuleReset(enum CA_SLOT_TYPE, uint32_t Slot);
	/// Checks if a module is present
	bool ModulePresent(enum CA_SLOT_TYPE, uint32_t Slot);
	/// Returns the module name in array Name
	void ModuleName(enum CA_SLOT_TYPE, uint32_t Slot, char *Name);
	/// Notify the module we want to enter menu
	void MenuEnter(enum CA_SLOT_TYPE, uint32_t Slot);
	/// Notify the module with our answer (choice nr)
	void MenuAnswer(enum CA_SLOT_TYPE, uint32_t Slot, uint32_t choice);
	/// Notify the module with our answer (binary)
	void InputAnswer(enum CA_SLOT_TYPE, uint32_t Slot, uint8_t * Data, int Len);
	/// Notify the module we closed the menu
	void MenuClose(enum CA_SLOT_TYPE, uint32_t Slot);
	/// Get the supported CAIDs
//	int GetCAIDS(CaIdVector & /*Caids*/) { return 0; };
	int GetCAIDS(CaIdVector &Caids);
	/// Send a CA-PMT object and Raw unparsed PMT to the CA layer
	bool SendCAPMT(u64 /*Source*/, u8 /*DemuxSource*/, u8 /*DemuxMask*/, const unsigned char * /*CAPMT*/, u32 /*CAPMTLen*/, const unsigned char * /*RawPMT*/, u32 /*RawPMTLen*/,
		unsigned char scrambled = 0, ca_map_t camap = std::set<int>(), int mode = 0, bool enabled = false);

	bool StopRecordCI( u64 tpid, u8 source, u32 calen);
	SlotIt FindFreeSlot(ca_map_t camap, unsigned char scrambled);
	bool SendDateTime(void);
	bool SendCaPMT(tSlot* slot);
	void slot_pollthread(void *c);
	void setSource(tSlot* slot);
	bool checkQueueSize(tSlot* slot);
	void process_tpdu(tSlot* slot, unsigned char tpdu_tag, __u8* data, int asn_data_length, int con_id);

	void Test(int slot, CaIdVector caids);
	void DelTest(int slot);
	/// Virtual destructor
	virtual ~cCA();
};

#endif // __CA_H_
