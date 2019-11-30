#ifndef _AMA_IAP2_H_
#define _AMA_IAP2_H_

#include <message.h>

void AmaIap2Init(Task task);
bool AmaIap2SendData(uint8 *data, uint16 length);

#endif /* _AMA_IAP2_H_ */
