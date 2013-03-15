#include "NixPlatform.h"

namespace Nix {

    namespace Platform {

        // this is the issue:
        // by default the system assumes we will end up with a libPlatform.a and we don't get one for nix
        // if we turn off the gamepad.
        // unfortunately the gamepad brings in alot of linux joystick crap that we dont care about so i'm
        //making this a stupid stub library.
#ifdef KLAATU
        void INeedLibPlatformToLink()
        {
            volatile short x;
            x=42;
        }
#else
        // The ownership is from the application.
        static GamepadClient* s_gamepadClient = 0;

        GamepadDevice* getGamepad(int index)
        {
            if (!s_gamepadClient)
                return 0;
            return s_gamepadClient->getGamepad(index);
        }

        void setGamepadClient(GamepadClient* client)
        {
            // We need to be able to set a client = 0 for now.
            s_gamepadClient = client;
        }
#endif
    } //namespace Platform

} //namespace Nix
