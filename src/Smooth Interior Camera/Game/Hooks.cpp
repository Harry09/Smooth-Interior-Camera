/** @file Hooks.cpp
 ** @author Piotr Krupa (piotrkrupa06@gmail.com)
 ** @license MIT License
 **/

#include "Hooks.h"

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Memory/Hooking.Patterns.h"
#include "Memory/MemMgr.h"
#include "Mod/Mod.h"
#include "Mod/Camera.h"
#include "Mod/Config.h"
#include "Game/prism.h"
#include "Common.h"
#include "Version.h"

namespace Hooks
{
	Mod* g_pMod;

	short offset1 = 0;
	short offset2 = 0;


	void CameraEvent(uintptr_t gameCamAddr)
	{
		auto pGameCam = reinterpret_cast<prism::GameCamera*>(gameCamAddr + offset1 - 4);
		auto pGameCamPos = reinterpret_cast<prism::GameCameraPos*>(gameCamAddr + offset2);

		auto pCam = g_pMod->GetCamera();
		pCam->UpdateGameCamera(pGameCamPos);

		if (!g_pMod->IsPlaying() || !g_pMod->IsActive()) // default
		{
			pGameCamPos->m_rx = pGameCam->m_rxEnd;
			pGameCamPos->m_ry = pGameCam->m_ryEnd;
			pGameCam->m_keyboardEv = false;
		}
		else if (g_pMod->IsConfiguring())
		{
			if (pGameCam->m_keyboardEv)
			{
				for (short i = 0; i < 6; ++i)
				{
					if (floatEquals(pGameCam->m_rxEnd, Config::Get()->GetDefaultValue((Config::GameCameraPos)i)))
					{
						g_pMod->Log(SCS_LOG_TYPE_message, "New value for [%d] %f is %f", i, Config::Get()->m_interiorCamPos[i], pGameCamPos->m_rx);
						Config::Get()->m_interiorCamPos[i] = pGameCamPos->m_rx;
						g_pMod->DisableConfigurating();
						Config::Get()->Save();
					}
				}

				pGameCam->m_keyboardEv = false;
			}
		}
		else if (g_pMod->IsActive())
		{
			if (pGameCam->m_keyboardEv)
			{
				float rx = pGameCam->m_rxEnd;

				for (short i = 0; i < 6; ++i)
				{
					if (floatEquals(pGameCam->m_rxEnd, Config::Get()->GetDefaultValue((Config::GameCameraPos)i)))
					{
						rx = Config::Get()->GetValue((Config::GameCameraPos)i);

					#ifdef TESTING
						std::cout << "New value for '" << pGameCam->m_rxEnd << "' is '" << rx << "'\n";
					#endif

						break;
					}
				}

				pCam->UpdateRX(pGameCamPos->m_rx);
				pCam->MoveTo(rx);

				pGameCam->m_keyboardEv = false;
			}
		}
	}

	uintptr_t CameraEvent_addr;

	auto CameraEvent_pattern = "8B 81 ?? ?? 00 00 89 81 ?? ?? 00 00 8B 81 ?? ?? 00 00 89 81 ?? ?? 00 00 C7 81 ?? ?? 00 00 00 00 00 00";

	extern "C"
	{
		ptrdiff_t CameraEvent_Address = 0;
		void Asm_CameraEvent();
	}

	bool Hook_CameraEvent()
	{
		auto pattern = hook::pattern(CameraEvent_pattern);

		if (pattern.size() > 0)
		{
			CameraEvent_addr = reinterpret_cast<uintptr_t>(pattern.count(1).get(0).get<void>(0));

		#ifdef TESTING
			std::cout << "CameraEvent addr: " << std::hex << CameraEvent_addr << "\n";
		#endif

			int left = *reinterpret_cast<std::uint8_t*>(CameraEvent_addr + 2);
			int right = *reinterpret_cast<std::uint8_t*>(CameraEvent_addr + 3);

			offset1 = (static_cast<short>(right) << 8) | left;

			left = *reinterpret_cast<std::uint8_t*>(CameraEvent_addr + 8);
			right = *reinterpret_cast<std::uint8_t*>(CameraEvent_addr + 9);

			offset2 = (static_cast<short>(right) << 8) | left;

			printf("%i $i\n", offset1, offset2);

			CameraEvent_Address = (uintptr_t)CameraEvent;
			MemMgr::LongJmpHook(CameraEvent_addr, (uintptr_t)Asm_CameraEvent);

			return true;
		}
		else
		{
			Mod::Get()->Log(SCS_LOG_TYPE_error, "Data structure is incorrect!");
		#ifdef TESTING
			std::cout << "Hook for CameraEvent not found!\n";
		#endif
			return false;
		}
	}

	bool Init()
	{
	#ifdef TESTING
		std::cout << "Initializing hooks...\n";
	#endif 

		if (!Hook_CameraEvent())
			return false;

	#ifdef TESTING
		std::cout << "Hooks initialized!\n";
	#endif

		g_pMod = Mod::Get();

		return true;
	}

	void Unhook()
	{
		if (CameraEvent_addr != 0)
		{
#ifdef TESTING
			std::cout << "Unhooking...\n";
#endif

			memcpy((uint8_t*)CameraEvent_addr, CameraEvent_pattern, sizeof(CameraEvent_pattern));
		}
	}
}
