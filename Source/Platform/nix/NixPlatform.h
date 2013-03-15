#ifndef NixPlatform_h
#define NixPlatform_h

#include "NixPlatformExportMacros.h"
#include <vector>
#include <string>

namespace Nix {

    namespace Platform {
        // see the .cpp file for the doofy explanation
#ifdef KLAATU
        NIX_PLATFORM_EXPORT void INeedLibPlatformToLink();
#else
            struct NIX_PLATFORM_EXPORT GamepadDevice {
            bool connected;
            std::string id;
            unsigned long long lastTimestamp;

            std::vector<float> axes;
            std::vector<float> buttons;
        };

        class NIX_PLATFORM_EXPORT GamepadClient {
        public:
            virtual GamepadDevice* getGamepad(int index) = 0;
        };

        NIX_PLATFORM_EXPORT void setGamepadClient(GamepadClient*);
        NIX_PLATFORM_EXPORT GamepadDevice* getGamepad(int index);
#endif // KLAATU
    } //namespace Platform

} //namespace Nix

#endif // NixPlatform_h
