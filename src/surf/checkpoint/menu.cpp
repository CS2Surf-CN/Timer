#include <core/menu.h>
#include <surf/checkpoint/surf_checkpoint.h>

void CSurfCheckpointService::OpenCheckpointsMenu() {
	auto pController = GetPlayer()->GetController();
	if (!pController) {
		return;
	}

	auto hMenu = MENU::Create(pController);

	if (!hMenu) {
		SDK_ASSERT(false);
		return;
	}

	auto pMenu = hMenu.Data();
	pMenu->SetTitle("存点菜单");

	pMenu->AddItem("存点", MENU_HANDLER_L(this) {
		this->SaveCheckpoint();
		this->m_iCurrentCP = this->GetLatestCheckpoint();
	});

	pMenu->AddItem("读点", MENU_HANDLER_L(this) { this->LoadCheckpoint(this->m_iCurrentCP); });
	pMenu->AddItem("上一个", MENU_HANDLER_L(this) { this->LoadPrev(); });
	pMenu->AddItem("下一个", MENU_HANDLER_L(this) { this->LoadNext(); });

	pMenu->AddItem("删除当前存点", MENU_HANDLER_L(this) {
		this->DeleteCheckpoint(this->m_iCurrentCP);
		this->m_iCurrentCP--;
		this->ClampIndex(this->m_iCurrentCP);
	});

	pMenu->AddItem("重置", MENU_HANDLER_L(this) { this->ResetCheckpoint(); });

	pMenu->Display();
}
