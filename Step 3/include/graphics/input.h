/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_INPUT_H
#define STEP_3_INPUT_H

#define XK_LATIN1
#define XK_MISCELLANY
#include <graphics/keys.h>
#include <engine/util/std.h>

namespace Raytracing {
    
    struct InputState {
        double* mousePos = new double[2];
        double* mousePosLast = new double[2];
        double* mouseDelta = new double[2];
        bool* keysDown = new bool[0xffff];
        bool* state = new bool[0xffff];
        bool* mouseDown = new bool[0xff];
        bool* mouseState = new bool;
        
        // make sure the input starts in a known state
        InputState() {
            for (int i = 0; i < 2; i++)
                mousePos[i] = 0;
            for (int i = 0; i < 2; i++)
                mousePosLast[i] = 0;
            for (int i = 0; i < 2; i++)
                mouseDelta[i] = 0;
            for (int i = 0; i < 0xffff; i++)
                keysDown[i] = false;
            for (int i = 0; i < 0xff; i++)
                mouseDown[i] = false;
            *state = false;
            *mouseState = false;
        }
    };
    
    struct MouseMovementData {
        double x, y;
        MouseMovementData(double x, double y): x(x), y(y) {}
    };
    
    extern InputState* _inputState;
    
    class Input {
        public:
            static inline void keyPressed(int key){_inputState->keysDown[key] = true; _inputState->state[key] = true;}
            static inline void keyReleased(int key){_inputState->keysDown[key] = false;}
            static inline void mousePressed(int button){_inputState->mouseDown[button] = true; *_inputState->mouseState=true;}
            static inline void mouseReleased(int button){_inputState->mouseDown[button] = false;}
            static inline void moveMove(double x, double y) {
                _inputState->mousePos[0] = x; _inputState->mousePos[1] = y;
            }
            static inline bool isKeyDown(int key){return _inputState->keysDown[key];}
            static inline void state(){
                // update the state used in single keypress events and the change in mouse position
                for (int i = 0; i < 0xffff; i++)
                    _inputState->state[i]=false;
                *_inputState->mouseState=false;
                
                _inputState->mouseDelta[0] = _inputState->mousePosLast[0] - _inputState->mousePos[0];
                _inputState->mouseDelta[1] = _inputState->mousePosLast[1] - _inputState->mousePos[1];
                _inputState->mousePosLast[0] = _inputState->mousePos[0];
                _inputState->mousePosLast[1] = _inputState->mousePos[1];
            }
            static inline bool isState(int key){return _inputState->state[key];}
            static inline bool isMouseState(){return *_inputState->mouseState;}
            static inline MouseMovementData getMousePosition(){return {_inputState->mousePos[0], _inputState->mousePos[1]};}
            static inline MouseMovementData getMouseDelta(){return {_inputState->mouseDelta[0], _inputState->mouseDelta[1]};}
    };
    
    static void deleteKeys(){
        delete[](_inputState->keysDown);
        delete[](_inputState->mouseDown);
        delete[](_inputState->mouseDelta);
        delete[](_inputState->state);
    }
    
}

#endif //STEP_3_INPUT_H
