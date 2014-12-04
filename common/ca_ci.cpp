#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/dvb/ca.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/types.h>
#include <poll.h>
#include <list>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

#include <queue>

#include "ca_ci.h"
#include "lt_debug.h"
#include <cs_api.h>
#include <hardware_caps.h>

#include <dvbci_session.h>
#include <dvbci_appmgr.h>
#include <dvbci_camgr.h>
#include <dvbci_mmi.h>

/* for some debug > set to 1 */
#define x_debug 1
#define y_debug 0

#define TAG_MENU_ANSWER         0x9f880b
#define TAG_ENTER_MENU          0x9f8022

#define lt_debug(args...) _lt_debug(TRIPLE_DEBUG_CA, this, args)

static const char * FILENAME = "[ca.cpp]";

/* für callback */
/* nur diese Message wird vom CI aus neutrinoMessages.h benutzt */
/* für den CamMsgHandler, darum hier einfach mal definiert */
/* die Feinheiten werden ja in CA_MESSAGE verpackt */
uint32_t EVT_CA_MESSAGE = 0x80000000 + 60;

static cs_messenger cam_messenger;

void cs_register_messenger(cs_messenger messenger)
{
	cam_messenger = messenger;
	return;
}

bool cCA::checkQueueSize(tSlot* slot)
{
	return (slot->sendqueue.size() > 0);
}

/* for temporary testing */
void cCA::Test(int slot, CaIdVector caids)
{
	char buf[255];
	char mname[200];
	char fname[20];
	int count, cx, cy, i;
	snprintf(fname, sizeof(fname), "/tmp/ci-slot%d" , slot);
	ModuleName(CA_SLOT_TYPE_CI, slot, mname);
	FILE* fd = fopen(fname, "w");
	if (fd == NULL) return;
	snprintf(buf, sizeof(buf), "%s\n" , mname);
	fputs(buf, fd);
	if (caids.size() > 40)
		count = 40;
	else
		count = caids.size();
	cx = snprintf(buf, sizeof(buf), "Anzahl Caids: %d Slot: %d > ", count, slot);
	for (i = 0; i < count; i++)
	{
		cy = snprintf(buf + cx, sizeof(buf) - cx, "%04x ", caids[i]);
		cx += cy;
	}
	snprintf(buf + cx, sizeof(buf) - cx, "\n");
	fputs(buf, fd);
	fclose(fd);
}

void cCA::DelTest(int slot)
{
	char fname[20];
	snprintf(fname, sizeof(fname), "/tmp/ci-slot%d" , slot);
	if (access(fname, F_OK) == 0) remove(fname);
}

/* helper function to call the cpp thread loop */
void* execute_thread(void *c)
{
	tSlot* slot = (tSlot*) c;
	cCA *obj = (cCA*)slot->pClass;
	obj->slot_pollthread(c);
	return NULL;
}

/* from dvb-apps */
int asn_1_decode(uint16_t * length, unsigned char * asn_1_array, uint32_t asn_1_array_len)
{
	uint8_t length_field;

	if (asn_1_array_len < 1)
		return -1;
	length_field = asn_1_array[0];

	if (length_field < 0x80)
	{
		// there is only one word
		*length = length_field & 0x7f;
		return 1;
	}
	else if (length_field == 0x81)
	{
		if (asn_1_array_len < 2)
			return -1;

		*length = asn_1_array[1];
		return 2;
	}
	else if (length_field == 0x82)
	{
		if (asn_1_array_len < 3)
			return -1;

		*length = (asn_1_array[1] << 8) | asn_1_array[2];
		return 3;
	}

	return -1;
}

//wait for a while for some data und read it if some
eData waitData(int fd, unsigned char* buffer, int* len)
{
	int retval;
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLOUT | POLLPRI | POLLIN;
	retval = poll(&fds, 1, 100);
	if (retval < 0)
	{
		printf("%s data error\n", FILENAME);
		return eDataError;
	}
	else if (retval == 0)
	{
		return eDataTimeout;
	}
	else if (retval > 0)
	{
		if (fds.revents & POLLIN)
		{
			int n = read (fd, buffer, *len);
			if (n > 0)
			{
				*len = n;
				return eDataReady;
			}
			*len = 0;
			return eDataError;
		}
		else if (fds.revents & POLLOUT)
		{
			return eDataWrite;
		}
		else if (fds.revents & POLLPRI)
		{
			return eDataStatusChanged;
		}
	}
	return eDataError;
}

//send some data on an fd, for a special slot and connection_id
eData sendData(tSlot* slot, unsigned char* data, int len)
{
	unsigned char *d = (unsigned char*) malloc(len + 5);

	// only poll connection if we are not awaiting an answer
	slot->pollConnection = false;

	/* should we send a data last ? */
	if (data != NULL)
	{
		if ((data[2] >= T_SB) && (data[2] <= T_NEW_T_C))
		{
			memcpy(d, data, len);
		}
		else
		{
			//send data_last and data
			memcpy(d + 5, data, len);
			d[0] = slot->slot;
			d[1] = slot->connection_id;
			d[2] = T_DATA_LAST;
			if (len > 127)
				d[3] = 4;
			else
				d[3] = len + 1;
			d[4] = slot->connection_id;
			len += 5;
		}
	}
	else
	{
		//send a data last only
		d[0] = slot->slot;
		d[1] = slot->connection_id;
		d[2] = T_DATA_LAST;
		d[3] = len + 1;
		d[4] = slot->connection_id;
		len = 5;
	}
#ifdef direct_write
	int res = write(slot->fd, d, len);

	free(d);
	if (res < 0 || res != len)
	{
		printf("error writing data to fd %d, slot %d: %m\n", slot->fd, slot->slot);
		return eDataError;
	}
#else
	slot->sendqueue.push(queueData(d, len));
#endif
	return eDataReady;
}

//send a transport connection create request
bool sendCreateTC(tSlot* slot)
{
	unsigned char* data = (unsigned char*) malloc(sizeof(char) * 5);
	data[0] = slot->slot;
	data[1] = slot->slot + 1; 	/* conid */
	data[2] = T_CREATE_T_C;
	data[3] = 1;
	data[4] = slot->slot + 1 	/* conid */;
	sendData(slot, data, 5);
	return true;
}

void cCA::process_tpdu(tSlot* slot, unsigned char tpdu_tag, __u8* data, int asn_data_length, int /*con_id*/)
{
	switch (tpdu_tag)
	{
		case T_C_T_C_REPLY:
			printf("%s %s Got CTC Replay (slot %d, con %d)\n", FILENAME, __FUNCTION__, slot->slot, slot->connection_id);
			//answer with data last (and if we have with data)
			sendData(slot, NULL, 0);
			break;
		case T_DELETE_T_C:
			//FIXME: close sessions etc; slot->reset ?
			//we must answer here with t_c_replay
			printf("%s %s Got \"Delete Transport Connection\" from module ->currently not handled!\n", FILENAME, __FUNCTION__);
			break;
		case T_D_T_C_REPLY:
			printf("%s %s Got \"Delete Transport Connection Replay\" from module!\n", FILENAME, __FUNCTION__);
			break;
		case T_REQUEST_T_C:
			printf("%s %s Got \"Request Transport Connection\" from Module ->currently not handled!\n", FILENAME, __FUNCTION__);
			break;
		case T_DATA_MORE:
		{
			int new_data_length = slot->receivedLen + asn_data_length;
			printf("%s %s Got \"Data More\" from Module\n", FILENAME, __FUNCTION__);
			__u8 *new_data_buffer = (__u8*) realloc(slot->receivedData, new_data_length);
			slot->receivedData = new_data_buffer;
			memcpy(slot->receivedData + slot->receivedLen, data, asn_data_length);
			slot->receivedLen = new_data_length;
			break;
		}
		case T_DATA_LAST:
			/* single package */
			if (slot->receivedData == NULL)
			{
				printf("%s -> single package\n", FILENAME);
#if x_debug
				printf("%s -> calling receiveData with data (len %d)\n", FILENAME, asn_data_length);
				for (int i = 0; i < asn_data_length; i++)
					printf("%02x ", data[i]);
				printf("\n");
#endif
				eDVBCISession::receiveData(slot, data, asn_data_length);
				eDVBCISession::pollAll();
			}
			else
			{
				/* chained package */
				int new_data_length = slot->receivedLen + asn_data_length;
				printf("%s -> chained data\n", FILENAME);
				__u8 *new_data_buffer = (__u8*) realloc(slot->receivedData, new_data_length);
				slot->receivedData = new_data_buffer;
				memcpy(slot->receivedData + slot->receivedLen, data, asn_data_length);
				slot->receivedLen = new_data_length;
#if x_debug
				printf("%s -> calling receiveData with data (len %d)\n", FILENAME, asn_data_length);
				for (int i = 0; i < slot->receivedLen; i++)
					printf("%02x ", slot->receivedData[i]);
				printf("\n");
#endif
				eDVBCISession::receiveData(slot, slot->receivedData, slot->receivedLen);
				eDVBCISession::pollAll();

				free(slot->receivedData);
				slot->receivedData = NULL;
				slot->receivedLen = 0;
			}
			break;
		case T_SB:
		{
			if (data[0] & 0x80)
			{
				/* we now wait for an answer so dont poll */
				slot->pollConnection = false;

				/* send the RCV and ask for the data */
				unsigned char send_data[5];
				send_data[0] = slot->slot;
				send_data[1] = slot->connection_id;
				send_data[2] = T_RCV;
				send_data[3] = 1;
				send_data[4] = slot->connection_id;
				write(slot->fd, send_data, 5);
			}
			else
			{
				//if the queue is not empty we dont need to send
				//	a polling to the module.
				//if (checkQueueSize(slot) == false)
				//	slot->pollConnection = true;
			}
			break;
		}
		default:
			printf("%s unhandled tpdu_tag 0x%0x\n", FILENAME, tpdu_tag);
	}
}

bool cCA::SendMessage(const CA_MESSAGE *msg)
{
	lt_debug("%s\n", __func__);
	cam_messenger(EVT_CA_MESSAGE, (uint32_t) msg);
#if y_debug
	printf("*******Message\n");
	printf("msg: %p\n", msg);
	printf("MSGID: %x\n", msg->MsgId);
	printf("SlotType: %x\n", msg->SlotType);
	printf("Slot: %x\n", msg->Slot);
#endif
	return true;
}

void cCA::MenuEnter(enum CA_SLOT_TYPE, uint32_t bSlotIndex)
{
	printf("%s %s bSlotIndex: %d\n", FILENAME, __FUNCTION__, bSlotIndex);

	std::list<tSlot*>::iterator it;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
#if 0
		if ((strstr((*it)->name, "unknown module") != NULL) && ((*it)->slot == bSlotIndex))
		{
			//the module has no real name, this is the matter if something while initializing went wrong
			//so let this take as a reset action for the module so we do not need to add a reset
			//feature to the neutrino menu
			ModuleReset(SlotType, bSlotIndex);

			return;
		}
#endif
		if ((*it)->slot == bSlotIndex)
		{
			if ((*it)->hasAppManager)
				(*it)->appSession->startMMI();
			break;
		}
	}
}

void cCA::MenuAnswer(enum CA_SLOT_TYPE, uint32_t bSlotIndex, uint32_t choice)
{
	printf("%s %s bSlotIndex: %d choice: %c\n", FILENAME, __FUNCTION__, bSlotIndex, choice);

	std::list<tSlot*>::iterator it;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == bSlotIndex)
		{
			if ((*it)->hasMMIManager)
				(*it)->mmiSession->answerText((int) choice);
		}
	}
}

void cCA::InputAnswer(enum CA_SLOT_TYPE, uint32_t bSlotIndex, uint8_t * pBuffer, int nLength)
{
	printf("%s %s bSlotIndex: %d\n", FILENAME, __FUNCTION__, bSlotIndex);

	std::list<tSlot*>::iterator it;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == bSlotIndex)
		{
			if ((*it)->hasMMIManager)
				(*it)->mmiSession->answerEnq((char*) pBuffer, nLength);
			break;
		}
	}
}

void cCA::MenuClose(enum CA_SLOT_TYPE, uint32_t bSlotIndex)
{
	printf("%s %s bSlotIndex: %d\n", FILENAME, __FUNCTION__, bSlotIndex);
	std::list<tSlot*>::iterator it;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == bSlotIndex)
		{
			if ((*it)->hasMMIManager)
				(*it)->mmiSession->stopMMI();
			break;
		}
	}
}

uint32_t cCA::GetNumberCISlots(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
	return num_slots;
}

uint32_t cCA::GetNumberSmartCardSlots(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
	return 0;
}

void cCA::ModuleName(enum CA_SLOT_TYPE, uint32_t slot, char * Name)
{
	std::list<tSlot*>::iterator it;
	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == slot)
		{
			strcpy(Name, (*it)->name);
			break;
		}
	}
}

bool cCA::ModulePresent(enum CA_SLOT_TYPE, uint32_t slot)
{
	std::list<tSlot*>::iterator it;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == slot)
		{
			return (*it)->camIsReady;
			break;
		}
	}
	return false;
}

void cCA::ModuleReset(enum CA_SLOT_TYPE, uint32_t slot)
{
	std::list<tSlot*>::iterator it;
	bool haveFound = false;

	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->slot == slot)
		{
			haveFound = true;
			break;
		}
	}
	if (haveFound)
	{
		(*it)->status = eStatusReset;
		usleep(200000);
		eDVBCISession::deleteSessions((tSlot*)(*it));
		(*it)->mmiSession = NULL;
		(*it)->hasMMIManager = false;
		(*it)->hasCAManager = false;
		(*it)->hasDateTime = false;
		(*it)->hasAppManager = false;
		(*it)->mmiOpened = false;
		(*it)->camIsReady = false;
		(*it)->counter = 0;
		(*it)->init = false;
		(*it)->pollConnection = false;
		sprintf((*it)->name, "unknown module %d", (*it)->slot);
		(*it)->cam_caids.clear();

		(*it)->newCapmt = false;
		(*it)->inUse = false;
		(*it)->tpid = 0;
		(*it)->pmtlen = 0;
		(*it)->source = TUNER_A;
		(*it)->camask = 0;
		memset((*it)->pmtdata, 0, sizeof((*it)->pmtdata));

		while((*it)->sendqueue.size())
		{
			delete [] (*it)->sendqueue.top().data;
			(*it)->sendqueue.pop();
		}

		if (ioctl((*it)->fd, CA_RESET, (*it)->slot) < 0)
			printf("IOCTL CA_RESET failed for slot %d\n", slot);
		usleep(200000);
		(*it)->status = eStatusNone;
	}
}

int cCA::GetCAIDS(CaIdVector &Caids)
{
	std::list<tSlot*>::iterator it;
	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->camIsReady)
		{
			for (unsigned int i = 0; i < (*it)->cam_caids.size(); i++)
				Caids.push_back((*it)->cam_caids[i]);
		}
	}
	return 0;
}

bool cCA::StopRecordCI( u64 tpid, u8 source, u32 calen)
{
	printf("%s:%s\n", FILENAME, __func__);
	std::list<tSlot*>::iterator it;
	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->inUse && (*it)->tpid == tpid && (*it)->source == source && !calen)
		{
			(*it)->inUse = false;
			return true;
		}
	}
	return false;
}

SlotIt cCA::FindFreeSlot(ca_map_t camap, unsigned char scrambled)
{
	printf("%s:%s\n", FILENAME, __func__);
	std::list<tSlot*>::iterator it;
	ca_map_iterator_t caIt;
	unsigned int i;
	for (it = slot_data.begin(); it != slot_data.end(); ++it)
	{
		if ((*it)->camIsReady && (*it)->hasCAManager && (*it)->hasAppManager && !(*it)->inUse)
		{
#if x_debug
			printf("Slot Caids: %d > ", (*it)->cam_caids.size());
			for (i = 0; i < (*it)->cam_caids.size(); i++)
				printf("%04x ", (*it)->cam_caids[i]);
			printf("\n");
#endif
			if (scrambled)
			{
				for (i = 0; i < (*it)->cam_caids.size(); i++)
				{
					caIt = camap.find((*it)->cam_caids[i]);
					if (caIt != camap.end())
					{
						printf("Found: %04x\n", *caIt);
						return it;
					}
				}
			}
			else
			{
				return it;
			}
		}

	}
	return it;
}

/* erstmal den capmt wie er von Neutrino kommt in den Slot puffern */
bool cCA::SendCAPMT(u64 tpid, u8 source_demux, u8 camask, const unsigned char * cabuf, u32 calen, const unsigned char * /*rawpmt*/, u32 /*rawlen*/, unsigned char scrambled, ca_map_t cm, int mode, bool enabled)
{
	printf("%s:%s\n", FILENAME, __func__);
	if (!num_slots) return true;	/* stb's without ci-slots */
#if x_debug
	printf("TPID: %llX\n", tpid);
	printf("SOURCE_DEMUX: %X\n", source_demux);
	printf("CA_MASK: %X\n", camask);
	printf("CALEN: %d\n", calen);
	printf("Scrambled: %d\n", scrambled);
	printf("Mode: %d\n", mode);
	printf("Enabled: %s\n", enabled ? "START" : "STOP");
#endif
	if (mode && scrambled && !enabled)
	{
		if (StopRecordCI(tpid, source_demux, calen))
			printf("CI set free\n");
	}

	if (calen == 0)
		return true;
	SlotIt It = FindFreeSlot(cm, scrambled);

	if ((*It))
	{
		printf("Slot: %d\n", (*It)->slot);
		if (scrambled || (!scrambled && (*It)->source == source_demux))
		{
			if ((*It)->tpid != tpid)
			{
				(*It)->tpid = tpid;
				(*It)->source = source_demux;
				(*It)->pmtlen = calen;
				for (unsigned int i = 0; i < calen; i++)
					(*It)->pmtdata[i] = cabuf[i];
				(*It)->newCapmt = true;
			}
			if (mode && scrambled && enabled)
				(*It)->inUse = true;
		}
	}
	else
	{
		printf("No free ci-slot\n");
	}
#if x_debug
	if (!cm.empty())
	{
		printf("Service Caids: ");
		for (ca_map_iterator_t it = cm.begin(); it != cm.end(); ++it)
		{
			printf("%04x ", (*it));
		}
		printf("\n");
	}
	else
	{
		printf("CaMap Empty\n");
	}
#endif
	return true;
}

cCA::cCA(int Slots)
{
	printf("%s %s %d\n", FILENAME, __FUNCTION__, Slots);

	int fd, i;
	char filename[128];
	num_slots = Slots;

	for (i = 0; i < Slots; i++)
	{
		sprintf(filename, "/dev/dvb/adapter0/ci%d", i);
		fd = open(filename, O_RDWR | O_NONBLOCK);
		if (fd < 0)
		{
			printf("failed to open %s ->%m", filename);
		}
		tSlot* slot = (tSlot*) malloc(sizeof(tSlot));
		slot->slot = i;
		slot->fd = fd;
		slot->connection_id = 0;
		slot->status = eStatusNone;
		slot->receivedLen = 0;
		slot->receivedData = NULL;
		slot->pClass = this;
		slot->pollConnection = false;
		slot->camIsReady = false;
		slot->hasMMIManager = false;
		slot->hasCAManager = false;
		slot->hasDateTime = false;
		slot->hasAppManager = false;
		slot->mmiOpened = false;

		slot->newCapmt = false;
		slot->inUse = false;
		slot->tpid = 0;
		slot->pmtlen = 0;
		slot->source = TUNER_A;
		slot->camask = 0;
		memset(slot->pmtdata, 0, sizeof(slot->pmtdata));

		slot->counter = 0;
		slot->init = false;
		sprintf(slot->name, "unknown module %d", i);
		slot_data.push_back(slot);
		/* now reset the slot so the poll pri can happen in the thread */
		if (ioctl(fd, CA_RESET, i) < 0)
			printf("IOCTL CA_RESET failed for slot %p\n", slot);
		/* create a thread for each slot */
		if (fd > 0)
		{
			if (pthread_create(&slot->slot_thread, 0, execute_thread, (void*)slot))
			{
				printf("pthread_create");
			}
		}
	}
}

cCA::~cCA()
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
}

static cCA* pcCAInstance = NULL;

cCA * cCA::GetInstance()
{
	printf("%s %s\n", FILENAME, __FUNCTION__);

	if (pcCAInstance == NULL)
	{
		hw_caps_t *caps = get_hwcaps();
		pcCAInstance = new cCA(caps->has_CI);
	}
	return pcCAInstance;
}

cCA::cCA(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
}

void cCA::setSource(tSlot* slot)
{
	char buf[64];
	snprintf(buf, 64, "/proc/stb/tsmux/ci%d_input", slot->slot);
	FILE *ci = fopen(buf, "wb");

	if (ci > (void*)0)
	{
		switch (slot->source)
		{
			case TUNER_A:
				fprintf(ci, "A");
				break;
			case TUNER_B:
				fprintf(ci, "B");
				break;
			case TUNER_C:
				fprintf(ci, "C");
				break;
			case TUNER_D:
				fprintf(ci, "D");
				break;
		}
		fclose(ci);
	}
}

void cCA::slot_pollthread(void *c)
{
	ca_slot_info_t info;
	unsigned char data[1024];
	tSlot* slot = (tSlot*) c;

	while (1)
	{
		int len = 1024;
		unsigned char* d;
		eData status;

		switch (slot->status)
		{
			case eStatusReset:
				while (slot->status == eStatusReset)
				{
					usleep(1000);
				}
				break;
			case eStatusNone:
			{
				if (slot->camIsReady)
				{
					if (sendCreateTC(slot))
					{
						slot->status = eStatusWait;
						slot->camIsReady = true;
					}
					else
					{
						usleep(100000);
					}
				}
				else
				{
					/* wait for pollpri */
					status = waitData(slot->fd, data, &len);
					if (status == eDataStatusChanged)
					{
						info.num = slot->slot;

						if (ioctl(slot->fd, CA_GET_SLOT_INFO, &info) < 0)
							printf("IOCTL CA_GET_SLOT_INFO failed for slot %d\n", slot->slot);

						printf("flags %d %d %d ->slot %d\n", info.flags, CA_CI_MODULE_READY, info.flags & CA_CI_MODULE_READY, slot->slot);

						if (info.flags & CA_CI_MODULE_READY)
						{
							printf("1. cam (%d) status changed ->cam now present\n", slot->slot);

							slot->mmiSession = NULL;
							slot->hasMMIManager = false;
							slot->hasCAManager = false;
							slot->hasDateTime = false;
							slot->hasAppManager = false;
							slot->mmiOpened = false;
							slot->init = false;
							sprintf(slot->name, "unknown module %d", slot->slot);
							slot->status = eStatusNone;

							/* Send a message to Neutrino cam_menu handler */
							CA_MESSAGE* pMsg = (CA_MESSAGE*) malloc(sizeof(CA_MESSAGE));
							memset(pMsg, 0, sizeof(CA_MESSAGE));
							pMsg->MsgId = CA_MESSAGE_MSG_INSERTED;
							pMsg->SlotType = CA_SLOT_TYPE_CI;
							pMsg->Slot = slot->slot;
							SendMessage(pMsg);

							slot->camIsReady = true;
							//setSource(slot);
						}
						else
						{
							//noop
						}
					}
				}
			} /* case statusnone */
			break;
			case eStatusWait:
			{
				status = waitData(slot->fd, data, &len);
				if (status == eDataReady)
				{
					slot->pollConnection = false;
					d = data;
					/* taken from the dvb-apps */
					int data_length = len - 2;
					d += 2; /* remove leading slot and connection id */
					while (data_length > 0)
					{
						unsigned char tpdu_tag = d[0];
						unsigned short asn_data_length;
						int length_field_len;
						if ((length_field_len = asn_1_decode(&asn_data_length, d + 1, data_length - 1)) < 0)
						{
							printf("Received data with invalid asn from module on slot %02x\n", slot->slot);
							break;
						}

						if ((asn_data_length < 1) || (asn_data_length > (data_length - (1 + length_field_len))))
						{
							printf("Received data with invalid length from module on slot %02x\n", slot->slot);
							break;
						}
						slot->connection_id = d[1 + length_field_len];
						//printf("Setting connection_id from received data to %d\n", slot->connection_id);
						d += 1 + length_field_len + 1;
						data_length -= (1 + length_field_len + 1);
						asn_data_length--;
						process_tpdu(slot, tpdu_tag, d, asn_data_length, slot->connection_id);
						// skip over the consumed data
						d += asn_data_length;
						data_length -= asn_data_length;
					} // while (data_length)
				} /*if data ready */
				else if (status == eDataWrite)
				{
					if (!slot->sendqueue.empty())
					{
						const queueData &qe = slot->sendqueue.top();
						int res = write(slot->fd, qe.data, qe.len);
						if (res >= 0 && (unsigned int)res == qe.len)
						{
							delete [] qe.data;
							slot->sendqueue.pop();
						}
						else
						{
							printf("r = %d, %m\n", res);
						}
					}
					else
					{
						//printf("sendqueue emtpy\n");
						if ((checkQueueSize(slot) == false) && ((!slot->hasCAManager) || (slot->mmiOpened) || (slot->counter < 30)))
							slot->pollConnection = true;
						if (slot->hasCAManager && !slot->mmiOpened)
							slot->counter++;
						if (slot->mmiOpened)
							slot->counter = 0;
					}
				}
				else if (status == eDataStatusChanged)
				{
					info.num = slot->slot;

					if (ioctl(slot->fd, CA_GET_SLOT_INFO, &info) < 0)
						printf("IOCTL CA_GET_SLOT_INFO failed for slot %d\n", slot->slot);

					printf("flags %d %d %d ->slot %d\n", info.flags, CA_CI_MODULE_READY, info.flags & CA_CI_MODULE_READY, slot->slot);

					if ((slot->camIsReady == false) && (info.flags & CA_CI_MODULE_READY))
					{
						printf("2. cam (%d) status changed ->cam now present\n", slot->slot);

						slot->mmiSession = NULL;
						slot->hasMMIManager = false;
						slot->hasCAManager = false;
						slot->hasDateTime = false;
						slot->hasAppManager = false;
						slot->mmiOpened = false;
						slot->init = false;
						sprintf(slot->name, "unknown module %d", slot->slot);
						slot->status = eStatusNone;

						/* Send a message to Neutrino cam_menu handler */
						CA_MESSAGE* pMsg = (CA_MESSAGE*) malloc(sizeof(CA_MESSAGE));
						memset(pMsg, 0, sizeof(CA_MESSAGE));
						pMsg->MsgId = CA_MESSAGE_MSG_INSERTED;
						pMsg->SlotType = CA_SLOT_TYPE_CI;
						pMsg->Slot = slot->slot;
						SendMessage(pMsg);

						slot->camIsReady = true;
					}
					else if ((slot->camIsReady == true) && (!(info.flags & CA_CI_MODULE_READY)))
					{
						printf("cam (%d) status changed ->cam now _not_ present\n", slot->slot);
						eDVBCISession::deleteSessions(slot);
						slot->mmiSession = NULL;
						slot->hasMMIManager = false;
						slot->hasCAManager = false;
						slot->hasDateTime = false;
						slot->hasAppManager = false;
						slot->mmiOpened = false;
						slot->init = false;
						slot->counter = 0;
						slot->pollConnection = false;
						sprintf(slot->name, "unknown module %d", slot->slot);
						slot->status = eStatusNone;
						slot->cam_caids.clear();

						slot->newCapmt = false;
						slot->inUse = false;
						slot->tpid = 0;
						slot->pmtlen = 0;
						slot->source = TUNER_A;
						slot->camask = 0;
						memset(slot->pmtdata, 0, sizeof(slot->pmtdata));

						/* temporary : delete testfile */
						DelTest(slot->slot);
						/* Send a message to Neutrino cam_menu handler */
						CA_MESSAGE* pMsg = (CA_MESSAGE*) malloc(sizeof(CA_MESSAGE));
						memset(pMsg, 0, sizeof(CA_MESSAGE));
						pMsg->MsgId = CA_MESSAGE_MSG_REMOVED;
						pMsg->SlotType = CA_SLOT_TYPE_CI;
						pMsg->Slot = slot->slot;
						SendMessage(pMsg);

						while(slot->sendqueue.size())
						{
							delete [] slot->sendqueue.top().data;
							slot->sendqueue.pop();
						}
						slot->camIsReady = false;
						usleep(100000);
					}
				}
				if (!checkQueueSize(slot) && slot->pollConnection)
				{
					//printf("****poll1\n");
					sendData(slot, NULL, 0);
				}
			}
			break;
			default:
				printf("unknown state %d\n", slot->status);
				break;
		}

		if (slot->hasCAManager && slot->hasAppManager && !slot->init)
		{
			slot->init = true;

			slot->cam_caids = slot->camgrSession->getCAIDs();

			printf("Anzahl Caids: %d Slot: %d > ", slot->cam_caids.size(), slot->slot);
			for (unsigned int i = 0; i < slot->cam_caids.size(); i++)
			{
				printf("%04x ", slot->cam_caids[i]);

			}
			printf("\n");

			/* temporary : write testfile */
			Test(slot->slot, slot->cam_caids);

			/* Send a message to Neutrino cam_menu handler */
			CA_MESSAGE* pMsg = (CA_MESSAGE*) malloc(sizeof(CA_MESSAGE));
			memset(pMsg, 0, sizeof(CA_MESSAGE));
			pMsg->MsgId = CA_MESSAGE_MSG_INIT_OK;
			pMsg->SlotType = CA_SLOT_TYPE_CI;
			pMsg->Slot = slot->slot;
			SendMessage(pMsg);
			/* resend a capmt if we have one. this is not very proper but I cant any mechanism in
			neutrino currently. so if a cam is inserted a pmt is not resend */

			SendCaPMT(slot);
		}
		if (slot->hasCAManager && slot->hasAppManager && slot->newCapmt)
		{
			SendCaPMT(slot);
			slot->newCapmt = false;
		}
	}
}

cCA *CA = cCA::GetInstance();

bool cCA::SendCaPMT(tSlot* slot)
{
	printf("%s:%s\n", FILENAME, __func__);
	if ((slot->fd > 0) && (slot->camIsReady))
	{
		if (slot->hasCAManager)
		{
#if x_debug
			printf("buffered capmt(0x%X): > \n", slot->pmtlen);
			for (unsigned int i = 0; i < slot->pmtlen; i++)
				printf("%02X ", slot->pmtdata[i]);
			printf("\n");
#endif
			if (slot->pmtlen == 0)
				return true;
			slot->camgrSession->sendSPDU(0x90, 0, 0, slot->pmtdata, slot->pmtlen);
		}
	}

	if (slot->fd > 0)
	{
		setSource(slot);
		slot->counter = 0;
	}
	return true;
}

bool cCA::Init(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
	return true;
}

bool cCA::SendDateTime(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
	return false;
}

bool cCA::Start(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
	return true;
}

void cCA::Stop(void)
{
	printf("%s %s\n", FILENAME, __FUNCTION__);
}

void cCA::Ready(bool p)
{
	printf("%s %s param:%d\n", FILENAME, __FUNCTION__, (int)p);
}

void cCA::SetInitMask(enum CA_INIT_MASK p)
{
	printf("%s %s param:%d\n", FILENAME, __FUNCTION__, (int)p);
}
