/**
 * This file is part of the hry-sic project
 * @ Author: Piotr Krupa <piotrkrupa06@gmail.com>
 * @ License: MIT License
 */

#include "CameraHook.hpp"

#include <memory>

#include <Hry/Memory/Detour.hpp>
#include <Hry/Memory/Hooking.Patterns.hpp>

#include "Game/InteriorCamera.hpp"

#include "SIC.hpp"

static std::uint16_t cameraEventOffset = 0;
static std::uint16_t camRotationOffset = 0;
static std::unique_ptr<hry::Detour> detour;

int64_t __stdcall CameraUpdate(uintptr_t gameCameraPtr)
{
    auto cameraEvent =
        reinterpret_cast<prism::InteriorCameraEvent*>(gameCameraPtr + cameraEventOffset);
    auto cameraRotation = reinterpret_cast<Rotation*>(gameCameraPtr + camRotationOffset);

    auto result = detour->getOriginal<decltype(CameraUpdate)>()(gameCameraPtr);

    CameraHook::OnCameraUpdate({ cameraEvent, cameraRotation });

    return result;
}

bool CameraHook::Install()
{
    hry::pattern functionPattern(
        std::string_view("48 83 EC ?  48 8B D1  45 32 C0  48 8B 89 ? ? ? ?  48 85 C9  74 ? "));
    hry::pattern dataFieldsPattern(
        std::string_view("F3 0F 10 ? ? ? 00 00  F3 0F 10 ? ? ? 00 00  83 F8 01  75 ?  F3 0F 11 ? ? "
                         "? 00 00  F3 0F 11 ? ? ? 00 00  89 9F ? ? 00 00  E9 ? ? 00 00"));

    SIC::Logger->info("Searching for pattern...");

    if (functionPattern.size() > 0 && dataFieldsPattern.size() > 0)
    {
        uintptr_t CameraUpdate_addr =
            reinterpret_cast<uintptr_t>(functionPattern.count(1).get(0).get<uintptr_t>(0));
        uintptr_t CameraUpdate_dataOffset =
            reinterpret_cast<uintptr_t>(dataFieldsPattern.count(1).get(0).get<uintptr_t>(0));

        cameraEventOffset = *reinterpret_cast<std::uint16_t*>(CameraUpdate_dataOffset + 4) - 4;
        camRotationOffset = *reinterpret_cast<std::uint16_t*>(CameraUpdate_dataOffset + 25);

        SIC::Logger->info("Offset 1: {}, Offset 2: {}", cameraEventOffset, camRotationOffset);

        detour = std::unique_ptr<hry::Detour>(
            new hry::Detour((void*)CameraUpdate_addr, (void*)CameraUpdate));

        if (detour->create() != hry::Detour::Status::Ok)
        {
            SIC::Logger->error("Cannot create hook");
            return false;
        }

        if (detour->enable() != hry::Detour::Status::Ok)
        {
            SIC::Logger->error("Cannot enable hook");
            return false;
        }

        SIC::Logger->info("Hook successfully installed");

        return true;
    }

    return false;
}

void CameraHook::Uninstall()
{
    detour.reset();
}
