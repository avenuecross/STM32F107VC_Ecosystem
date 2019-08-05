
#ifndef __GUI_TASK_H
#define __GUI_TASK_H

#include "GUI.h"
#include "WM.h"

#define UI_MESSAGE_EVENT  (1<<0)

typedef enum _UI_MSG_INDEX {
	UI_MSG_RESERVED     =  WM_USER,
	UI_MSG_OUTPUT_TEXT,
} UI_MSG_INDEX;

typedef enum _UI_ID_INDEX {
	UI_ID_RESERVED     =  GUI_ID_USER,
	UI_ID_MAINDIALOG,
	UI_ID_MULTIPLYEDIT,
} UI_ID_INDEX;


#endif
