/**
 ****************************************************************************************
 *
 * @file ancs_common.h
 *
 * @brief Header File - ANCS Profile common types.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _ANCS_COMMON_H_
#define _ANCS_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup ANCS Profile
 * @ingroup PROFILE
 * @brief ANCS Profile
 *
 * The ANCS module is the responsible block for implementing the ANCS Profile
 * functionalities in the BLE Host.
 *
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"


#include "prf_types.h"
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */

struct attributeIDReq
{
	uint8_t AttributeID;
	uint16_t Maxlen;
};

struct Notification_Attribute_Command
{
	  uint8_t  CommandID;
		uint32_t NotificationUID;
		struct attributeIDReq AttrID;
};

struct Get_Notification_Attribute_Command
{
		uint16_t length;
	
		uint8_t	 cmd[__ARRAY_EMPTY];

};


struct App_Attribute_command
{
	  uint8_t  CommandID;
		uint8_t  AppIdentifier[__ARRAY_EMPTY];
};
   
struct Get_App_Attribute_command
{
		uint16_t length;
		struct App_Attribute_command cmd;
	
};

///Parameters of the @ref ANCSC_WR_CNTL_POINT_RSP message
struct attributeIDInfo
{
			uint8_t AttributeID;
			uint16_t Attrlen;
			uint8_t data[__ARRAY_EMPTY];
};
struct Get_Notification_Attribute_Rsp
{
    uint8_t  CommandID;
		uint32_t NotificationUID;
		struct attributeIDInfo AttrIDInfo;	
};


struct notificationCharFomat
{
	uint8_t EventID;
	uint8_t EventFlags;
	uint8_t CategoryID;
	uint8_t CategoryCount;
	
	uint32_t NotificationUID;	
};

struct notificationCharInd
{
	uint16_t length;
	struct notificationCharFomat ntf;
};

struct dataCharInd
{
			uint16_t length;
			uint8_t  value[__ARRAY_EMPTY];
};

struct AttributeList
{
	uint8_t AttributeID;
	uint16_t Attrlen;	
	uint8_t  *data;

};


struct ntfAttrRspFomat
{
	uint8_t CommandID;
	uint32_t NotificationUID;
	
	struct AttributeList list;

};

/// Error Codes
enum Error_Codes
{
    //  The commandID was not recognized by the NP
    UNKNOWN_COMMAND    = 0xA0,
	
    /// The command was improperly formatted
    INVALID_COMMAND		 = 0xA1,
	
    //  One of the parameters (for example, the NotificationUID) does not refer to an existing object on the NP 
    INVALID_PARAMETER  = 0xA2,
	
	  //  The action was not performed.
		ACTION_FAILED      = 0xA3,
};

///  CategoryID Values
enum CategoryID_Values
{
		CategoryIDOther = 0,
	
		CategoryIDIncomingCall = 1,
	
		CategoryIDMissedCall = 2,
	
		CategoryIDVoicemail = 3,
	
		CategoryIDSocial = 4,
	
		CategoryIDSchedule = 5,
	
		CategoryIDEmail = 6,
	
		CategoryIDNews = 7,
	
		CategoryIDHealthAndFitness = 8,
	
		CategoryIDBusinessAndFinance = 9,
	
		CategoryIDLocation = 10,
		
		CategoryIDEntertainment = 11,
		
		Reserved_CategoryID_values = 255,

};

///  EventID Values
enum EventID_Values
{
		EventIDNotificationAdded = 0,
	
		EventIDNotificationModified = 1,
	
		EventIDNotificationRemoved = 2,
	
		Reserved_EventID_values = 255,

};

/// EventFlags

enum EventFlags
{
		EventFlagSilent = (1 << 0),
	
		EventFlagImportant = (1 << 1),
	
		EventFlagPreExisting = (1 << 2),
	
		EventFlagPositiveAction = (1 << 3),
	
		EventFlagNegativeAction = (1 << 4),
	
		Reserved_EventFlags = 0 ,
	
};

///  CommandID Values
enum CommandID_Values
{
		CommandIDGetNotificationAttributes = 0,
	
		CommandIDGetAppAttributes = 1,
	
		CommandIDPerformNotificationAction = 2,
	
		Reserved_CommandID_values = 255,
};

/// NotificationAttributeID Values
enum NotificationAttributeID_Values
{
		NotificationAttributeIDAppIdentifier = 0,
	
		NotificationAttributeIDTitle				 = 1,//(Needs to be followed by a 2-bytes max length parameter)

		NotificationAttributeIDSubtitle			 = 2,//(Needs to be followed by a 2-bytes max length parameter)
	
		NotificationAttributeIDMessage       = 3,//(Needs to be followed by a 2-bytes max length parameter)
	
		NotificationAttributeIDMessageSize   = 4,	
	
		NotificationAttributeIDDate					 = 5,
	
		NotificationAttributeIDPositiveActionLabel = 6,
	
		NotificationAttributeIDNegativeActionLabel = 7,
	
		Reserved_NotificationAttributeID_values = 255,
};

/// ActionID Values
enum ActionID_Values
{
		ActionIDPositive = 0,
	
		ActionIDNegative = 1,
	
		Reserved_ActionID_values = 255,
};



/// @} ancs_common

#endif /* _ANCS_COMMON_H_ */
