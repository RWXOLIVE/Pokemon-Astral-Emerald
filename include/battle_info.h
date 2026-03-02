#ifndef GUARD_BATTLE_INFO_H
#define GUARD_BATTLE_INFO_H

#include "global.h"

void CB2_BattleInfoMenu(void);
void OpenBattleInfoMenu(u32 battler);
bool32 BattleInfo_IsAvailable(void);
void BattleInfo_ShowActionHint(void);
void BattleInfo_HideActionHint(void);

#endif // GUARD_BATTLE_INFO_H
