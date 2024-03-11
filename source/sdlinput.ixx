module;

#include <common.hxx>

export module SDL_Input;

import common;

class SDL_Input
{
private:
    static inline bool SDLinit = false;
    static inline bool controllerConnected = false;
    static inline bool joyStickAPI = 0;
    static inline bool hasRumble = false;

    /*
    
    SDL has two different controller APIs. The modern, GameController API, and the legacy
    Joystick & Haptic API(s) for older hardware. I'd like to support both :)
    
    Sidenote: The Haptic API also supports haptic mice (apparently a thing), possible future thing?

    */

    static SDL_GameController* gameController;

    static SDL_Joystick* joystick;
    static SDL_Haptic* haptic;
public:
    SDL_Input()
    {
        FusionFix::onInitEvent() += [](){
             SDLinit = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER) > 0; // JOYSTICK = Controller, HAPTIC = Vibration, SENSOR = Gyro
        };

        FusionFix::onShutdownEvent() += []() {

            // Cleanup
            if (SDLinit) {
                if (gameController) {
                    SDL_GameControllerClose(gameController);
                    gameController = NULL;
                }

                if (haptic) {
                    SDL_HapticClose(haptic);
                    haptic = NULL;
                }

                if (joystick) {
                    SDL_JoystickClose(joystick);
                    joystick = NULL;
                }

                SDL_Quit();
            }
        };

        FusionFix::onGameProcessEvent() += []() {
            if (SDLinit) {
                // Handle SDL events
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    switch (e.type) {
                    case SDL_CONTROLLERDEVICEADDED:
                        {
                            if (controllerConnected) { break; }
                            gameController = SDL_GameControllerOpen(e.cdevice.which);
                            if (gameController) {
                                hasRumble = SDL_GameControllerHasRumble(gameController);
                                controllerConnected = true; joyStickAPI = false;
                            }
                            break;
                        }

                    case SDL_JOYDEVICEADDED:
                        {
                            if (controllerConnected) { break; }
                            joystick = SDL_JoystickOpen(e.jdevice.which);
                            if (joystick) {
                                controllerConnected = true;
                                joyStickAPI = true;
                                if(SDL_JoystickHasRumble(joystick)){
                                    haptic = SDL_HapticOpenFromJoystick(joystick);
                                    hasRumble = (haptic && (SDL_HapticRumbleInit(haptic) == 0 /* == Great success! */));
                                }
                            }
                            break;
                        }

                    case SDL_CONTROLLERDEVICEREMOVED:
                        {
                            if (gameController && controllerConnected && !joyStickAPI) {
                                SDL_GameControllerClose(gameController);
                                gameController = NULL;
                                controllerConnected = false;
                            }
                            break;
                        }

                    case SDL_JOYDEVICEREMOVED: 
                        {
                            if (joystick && controllerConnected && joyStickAPI) {
                                if (SDL_JoystickHasRumble(joystick) && haptic) {
                                    SDL_HapticClose(haptic);
                                    haptic = NULL;
                                }

                                SDL_JoystickClose(joystick);
                                joystick = NULL;
                                controllerConnected = false;
                            }
                            break;
                        }
                    }
                }
            }
        };
    }
} SDL_Input;