/*
 *  Gerätetypen (MäCAN):
 *  Dienen nur zur Unterscheidung beim Ping, hat keine auswirkungen auf den Betrieb.
 */
#define MCAN_MAGNET	0x0050
#define MCAN_SERVO 	0x0051
#define MCAN_RELAIS	0x0052
#define MCAN_STELLPULT 0x0053
#define MCAN_S88_GBS 0x0054


/*
 *  Adressbereich der Local-IDs:
 */
#define MM_ACC 		  0x3000	//Magnetartikel Motorola
#define DCC_ACC 	  0x3800	//Magbetartikel NRMA_DCC
#define MM_TRACK 	  0x0000	//Gleissignal Motorola
#define DCC_TRACK 	0xC000	//Gleissignal NRMA_DCC

/*
 *  CAN-Befehle (Märklin)
 */
#define SYS_CMD		0x00 	//Systembefehle
 	#define SYS_STOP 	0x00 	//System - Stopp
 	#define SYS_GO		0x01	//System - Go
 	#define SYS_HALT	0x02	//System - Halt
 	#define SYS_STAT	0x0b	//System - Status (sendet geänderte Konfiguration oder übermittelt Messwerte)
#define SWITCH_ACC 	0x0b	//Magnetartikel Schalten
#define S88_EVENT	0x11	//Rückmelde-Event
#define PING 		  0x18	//CAN-Teilnehmer anpingen
#define CONFIG		0x1d	//Konfiguration
#define BOOTLOADER	0x1B

#include <avr/io.h>

uint16_t generateHash(uint32_t uid);

void sendDeviceInfo(uint32_t uid, uint16_t hash, uint32_t serial_nbr, uint8_t meassure_ch, uint8_t config_ch, char *art_nbr, char *name);
void sendConfigInfoDropdown(uint32_t uid, uint16_t hash, uint8_t channel, uint8_t options, uint8_t default_option, char *string);
void sendConfigInfoSlider(uint32_t uid, uint16_t hash, uint8_t channel, uint16_t min_value, uint16_t max_value, uint16_t default_value, char *string);
void sendPingFrame(uint32_t uid, uint16_t hash, uint16_t version, uint16_t type);
void sendConfigConfirm(uint32_t uid, uint16_t hash, uint8_t channel);