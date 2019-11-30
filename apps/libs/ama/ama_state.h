#ifndef __AMA_STATE_H_
#define __AMA_STATE_H_

#include "ama.h"
#include "ama_private.h"

typedef enum _AMA_SETUP_STATE {
    AMA_SETUP_STATE_READY ,
    AMA_SETUP_STATE_WAITING ,  // just got at least one protbuf message
    AMA_SETUP_STATE_START,
    AMA_SETUP_STATE_COMPLETED,
    AMA_SETUP_STATE_SYNCHRONIZE

}AMA_SETUP_STATE;

/* Internal APIs */
ama_error_code_t amaLibGetState(uint32 feature, uint32* Pstate, ama_state_value_case_t* pValueCase);
ama_error_code_t amaLibSetState(uint32 feature, uint32 state, ama_state_value_case_t valueCase);
void amaStateInit(void);
AMA_SETUP_STATE AmaLibGetSetupState(void);
void AmaLibCompleteSetUp(bool completed) ;
bool AmaLibSendBooleanStateEvent(uint32 feature,bool True, bool get);
bool AmaLibSendIntegerStateEvent(uint32 feature ,uint16 integer, bool get);
bool AmaLibIsCompleteSetUp(void);

#endif /* __AMA_STATE_H_ */
