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

	void CameraEvent(prism::Camera *pGameCam)
	{
		if (!pGameCam)
			return;

		auto pCam = g_pMod->GetCamera();
		pCam->UpdateGameCamera(pGameCam);

		if (!g_pMod->IsPlaying() || !g_pMod->IsActive()) // default
		{
			pGameCam->m_rx = pGameCam->m_rx_predef;
			pGameCam->m_ry = pGameCam->m_ry_predef;
			pGameCam->m_keyboardEv = false;
		}
		else if (g_pMod->IsConfiguring())
		{
			if (pGameCam->m_keyboardEv)
			{
				for (short i = 0; i < 6; ++i)
				{
					if (floatEquals(pGameCam->m_rx_predef, Config::Get()->GetDefaultValue((Config::CameraPos)i)))
					{
						g_pMod->Log(SCS_LOG_TYPE_message, "New value for [%d] %f is %f", i, Config::Get()->m_interiorCamPos[i], pGameCam->m_rx);
						Config::Get()->m_interiorCamPos[i] = pGameCam->m_rx;
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
				float rx = pGameCam->m_rx_predef;

				for (short i = 0; i < 6; ++i)
				{
					if (floatEquals(pGameCam->m_rx_predef, Config::Get()->GetDefaultValue((Config::CameraPos)i)))
					{
						rx = Config::Get()->GetValue((Config::CameraPos)i);

					#ifdef TESTING
						std::cout << "New value for '" << pGameCam->m_rx_predef << "' is '" << rx << "'\n";
					#endif

						break;
					}
				}

				pCam->UpdateRX(pGameCam->m_rx);
				pCam->MoveTo(rx);

				pGameCam->m_keyboardEv = false;
			}
		}
	}

	uintptr_t CameraEvent_addr;

#if defined(HOOK_V1)
	auto CameraEvent_pattern = "8B 81 B0 02 00 00 89 81 48 03 00 00 8B 81 B4 02 00 00 89 81 4C 03 00 00 C7 81 AC 02 00 00 00 00 00 00";
#elif defined(HOOK_V2)
	auto CameraEvent_pattern = "8B 81 B8 02 00 00 89 81 50 03 00 00 8B 81 BC 02 00 00 89 81 54 03 00 00 C7 81 B4 02 00 00 00 00 00 00";
#endif

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
