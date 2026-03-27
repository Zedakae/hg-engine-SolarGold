#include "../include/types.h"
#include "../include/script.h"
#include "../include/repel.h"
#include "../include/constants/file.h"

#define SCRIPT_NEW_CMD_REPEL_USE            0
#define SCRIPT_NEW_CMD_REPEL_TOGGLE_ON      100
#define SCRIPT_NEW_CMD_REPEL_TOGGLE_OFF     101

#define SCRIPT_NEW_CMD_MAX                  256

BOOL Script_RunNewCmd(SCRIPTCONTEXT *ctx) {
    u8 sw = ScriptReadByte(ctx);
    u16 UNUSED arg0 = ScriptReadHalfword(ctx);

    switch (sw) {
        case SCRIPT_NEW_CMD_REPEL_USE:;
#ifdef IMPLEMENT_REUSABLE_REPELS
            u16 most_recent_repel = Repel_GetMostRecent();
            SetScriptVar(arg0, most_recent_repel);
            Repel_Use(most_recent_repel, HEAPID_MAIN_HEAP);
#endif
            break;
            
        case SCRIPT_NEW_CMD_REPEL_TOGGLE_ON:;
            {
                SaveData *saveData = SaveBlock2_get();
                void *roamerSaveData = EncDataSave_GetSaveDataPtr(saveData);
                u8 *repel_addr = SaveData_GetRepelPtr(roamerSaveData);
                
                SetScriptFlag(0x1002);
                *repel_addr = 255;
            }
            break;
            
        case SCRIPT_NEW_CMD_REPEL_TOGGLE_OFF:;
            {
                SaveData *saveData = SaveBlock2_get();
                void *roamerSaveData = EncDataSave_GetSaveDataPtr(saveData);
                u8 *repel_addr = SaveData_GetRepelPtr(roamerSaveData);
                
                ClearScriptFlag(0x1002);
                *repel_addr = 0;
            }
            break;

        default: break;
    }

    return FALSE;
}

// Construct all new script commands here, and define/add them to the scriptmacros.s under example RepelToggleOff.