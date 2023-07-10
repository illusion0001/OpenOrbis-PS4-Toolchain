#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // atexit()

#include <orbis/CommonDialog.h>
#include <orbis/MsgDialog.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>

// Dialog types
#define MDIALOG_OK       0
#define MDIALOG_YESNO    1


static inline void _orbisCommonDialogSetMagicNumber(uint32_t* magic, const OrbisCommonDialogBaseParam* param)
{
    *magic = (uint32_t)(ORBIS_COMMON_DIALOG_MAGIC_NUMBER + (uint64_t)param);
}

static inline void _orbisCommonDialogBaseParamInit(OrbisCommonDialogBaseParam *param)
{
    memset(param, 0x0, sizeof(OrbisCommonDialogBaseParam));
    param->size = (uint32_t)sizeof(OrbisCommonDialogBaseParam);
    _orbisCommonDialogSetMagicNumber(&(param->magic), param);
}

static inline void orbisMsgDialogParamInitialize(OrbisMsgDialogParam *param)
{
    memset(param, 0x0, sizeof(OrbisMsgDialogParam));
    _orbisCommonDialogBaseParamInit(&param->baseParam);
    param->size = sizeof(OrbisMsgDialogParam);
}

int show_dialog(int dialog_type, const char * format, ...)
{
    OrbisMsgDialogParam param;
    OrbisMsgDialogUserMessageParam userMsgParam;
    OrbisMsgDialogResult result;

    char str[0x800];
    memset(str, 0, sizeof(str));

    va_list opt;
    va_start(opt, format);
    vsprintf(str, format, opt);
    va_end(opt);

    sceMsgDialogInitialize();
    orbisMsgDialogParamInitialize(&param);
    param.mode = ORBIS_MSG_DIALOG_MODE_USER_MSG;

    memset(&userMsgParam, 0, sizeof(userMsgParam));
    userMsgParam.msg = str;
    userMsgParam.buttonType = (dialog_type ? ORBIS_MSG_DIALOG_BUTTON_TYPE_YESNO_FOCUS_NO : ORBIS_MSG_DIALOG_BUTTON_TYPE_OK);
    param.userMsgParam = &userMsgParam;

    if (sceMsgDialogOpen(&param) < 0)
        return 0;

    do { } while (sceMsgDialogUpdateStatus() != ORBIS_COMMON_DIALOG_STATUS_FINISHED);
    sceMsgDialogClose();

    memset(&result, 0, sizeof(result));
    sceMsgDialogGetResult(&result);
    sceMsgDialogTerminate();

    return (result.buttonId == ORBIS_MSG_DIALOG_BUTTON_ID_YES);
}

int sysctlbyname(const char *name, void *oldp, size_t *oldlenp, const void *newp, size_t newlen);

void on_exit(void)
{
    puts(__PRETTY_FUNCTION__);
    sceSystemServiceLoadExec("exit", NULL);
}

int main()
{
    // Load the Message Dialog module
    if (sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG) < 0 ||
        sceCommonDialogInitialize() < 0)
    {
        printf("Failed to initialize CommonDialog\n");
        return 0;
    }
    printf("atexit: 0x%08x\n", atexit(on_exit));
    int64_t tsc = 0;
    size_t len = 8;
    sysctlbyname("machdep.tsc_freq", &tsc, &len, NULL, 0);
    show_dialog(MDIALOG_OK, "machdep.tsc_freq: (%li) 0x%lx", tsc, tsc);
    printf("machdep.tsc_freq: (%li) 0x%lx\n", tsc, tsc);
    return 0;
}
